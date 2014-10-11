/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file chess_engine_search.cpp
 * @author Hironori Ishibashi
 * @brief チェスエンジンの本体の実装。 (探索関数)
 */

#include "chess_engine.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <thread>
#include <mutex>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <functional>
#include "common.h"
#include "transposition_table.h"
#include "move_maker.h"
#include "pv_line.h"
#include "evaluator.h"
#include "uci_shell.h"
#include "position_record.h"
#include "job.h"
#include "helper_queue.h"
#include "params.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // クイース探索。
  int ChessEngine::Quiesce(int depth, std::uint32_t level, int alpha, int beta,
  int material, TranspositionTable& table) {
    // 探索中止の時。
    if (ShouldBeStopped()) return alpha;
    if (is_job_ended_) return alpha;

    // ノード数を加算。
    ++(shared_st_ptr_->searched_nodes_);

    // 最大探索数。
    Util::UpdateMax(shared_st_ptr_->searched_level_, level);

    // 勝負に十分な駒がなければ0点。
    if (!HasSufficientMaterial()) {
      return SCORE_DRAW;
    }

    // サイド。
    Side side = to_move_;
    Side enemy_side = Util::SwitchOppositeSide(side);

    // stand_pad。
    int stand_pad = evaluator_.Evaluate(material);

    // アルファ値、ベータ値を調べる。
    if (stand_pad >= beta) {
      return stand_pad;
    }
    Util::UpdateMax(alpha, stand_pad);

    // 探索できる限界を超えているか。
    // 超えていればこれ以上探索しない。
    if (level >= MAX_PLYS) {
      return alpha;
    }

    // 候補手を作る。
    // 駒を取る手だけ。
    MoveMaker& maker = maker_table_[level];
    if (IsAttacked(king_[side], enemy_side)) {
      maker.GenMoves<GenMoveType::ALL>(0, 0, 0, 0);
    } else {
      maker.GenMoves<GenMoveType::CAPTURE>(0, 0, 0, 0);
    }

    // 探索する。
    int margin = GetMargin(depth);

    // --- Futility Pruning --- //
    bool enable_futility_pruning =
    shared_st_ptr_->search_params_ptr_->enable_futility_pruning_;

    for (Move move = maker.PickMove(); move; move = maker.PickMove()) {
      // 次の自分のマテリアル。
      int next_my_material = GetNextMyMaterial(material, move);

      MakeMove(move);

      // 合法手かどうか調べる。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // Futility Pruning。
      if (enable_futility_pruning) {
        if ((next_my_material + margin) <= alpha) {
          UnmakeMove(move);
          continue;
        }
      }

      // 次の手を探索。
      int score = -Quiesce(depth - 1, level + 1, -beta, -alpha,
      -next_my_material, table);

      UnmakeMove(move);

      // アルファ値、ベータ値を調べる。
      Util::UpdateMax(alpha, score);
      if (alpha >= beta) {
        break;
      }
    }

    return alpha;
  }

  // Internal Iterative Deepeningをする。 (Search()用テンプレート部品)
  template<NodeType Type>
  struct DoIID {
    static void F(ChessEngine& engine, bool is_checked, Move prev_best,
    Hash pos_hash, int depth, std::uint32_t level, int alpha, int beta,
    int material, TranspositionTable& table) {}
  };
  template<>
  struct DoIID<NodeType::PV> {
    static void F(ChessEngine& engine, bool is_checked, Move prev_best,
    Hash pos_hash, int depth, std::uint32_t level, int alpha, int beta,
    int material, TranspositionTable& table) {
      const SearchParams& params =
      *(engine.shared_st_ptr_->search_params_ptr_);

      if (params.enable_iid_) {
        // 前回の繰り返しの最善手があればIIDしない。
        if (prev_best) {
          engine.shared_st_ptr_->iid_stack_[level] = prev_best;
        } else {
          if (!is_checked
          && (depth >= params.iid_limit_depth_)) {
            // Internal Iterative Deepening。
            engine.Search<NodeType::PV>(pos_hash, params.iid_search_depth_,
            level, alpha, beta, material, table);

            engine.shared_st_ptr_->iid_stack_[level] =
            engine.pv_line_table_[level][0];
          }
        }
      }
    }
  };

  // Null Move Reductionをする。 (Search()用テンプレート部品)
  template<NodeType Type>
  struct DoNMR {
    static void F(ChessEngine& engine, bool is_checked, Hash pos_hash,
    int& depth, std::uint32_t level, int beta, int material,
    TranspositionTable& table, int& null_reduction) {}
  };
  template<>
  struct DoNMR<NodeType::NON_PV> {
    static void F(ChessEngine& engine, bool is_checked, Hash pos_hash,
    int& depth, std::uint32_t level, int beta, int material,
    TranspositionTable& table, int& null_reduction) {
      const SearchParams& params =
      *(engine.shared_st_ptr_->search_params_ptr_);

      if (params.enable_nmr_) {
        if (!(engine.is_null_searching_) && !is_checked
        && (depth >= params.nmr_limit_depth_)) {
          // Null Move Reduction。
          Move null_move = 0;

          engine.is_null_searching_ = true;
          engine.MakeNullMove(null_move);

          // Null Move Search。
          int score = -(engine.Search<NodeType::NON_PV>(pos_hash,
          depth - params.nmr_search_reduction_ - 1, level + 1, -(beta),
          -(beta - 1), -material, table));

          engine.UnmakeNullMove(null_move);
          engine.is_null_searching_ = false;

          if (score >= beta) {
            null_reduction = params.nmr_reduction_;

            depth = Util::GetMax(depth - null_reduction, 1);
          }
        }
      }
    }
  };

  // ProbCutをする。 (Search()用テンプレート部品)
  template<NodeType Type>
  struct DoProbCut {
    static void F(ChessEngine& engine, Side side, Side enemy_side,
    bool is_checked, ScoreType& score_type, Hash pos_hash,
    int depth, std::uint32_t level, int& alpha, int beta, int material,
    TranspositionTable& table) {}
  };
  template<>
  struct DoProbCut<NodeType::NON_PV> {
    static void F(ChessEngine& engine, Side side, Side enemy_side,
    bool is_checked, ScoreType& score_type, Hash pos_hash,
    int depth, std::uint32_t level, int& alpha, int beta, int material,
    TranspositionTable& table) {
      const SearchParams& params =
      *(engine.shared_st_ptr_->search_params_ptr_);

      MoveMaker& maker = engine.maker_table_[level];

      if (params.enable_probcut_) {
        if (!(engine.is_null_searching_) && !is_checked
        && (depth >= params.probcut_limit_depth_)) {
          // 手を作る。
          maker.RegenMoves();

          // 浅読みパラメータ。
          int prob_beta = beta + params.probcut_margin_;
          int prob_depth = depth - params.probcut_search_reduction_;

          // 探索。
          for (Move move = maker.PickMove(); move; move = maker.PickMove()) {
            // 次のノードへの準備。
            Hash next_hash = engine.GetNextHash(pos_hash, move);
            int next_my_material = engine.GetNextMyMaterial(material, move);

            engine.MakeMove(move);

            // 合法手じゃなければ次の手へ。
            if (engine.IsAttacked(engine.king_[side], enemy_side)) {
              engine.UnmakeMove(move);
              continue;
            }

            int score = -(engine.Search<NodeType::NON_PV>(next_hash,
            prob_depth - 1, level + 1, -prob_beta, -(prob_beta - 1),
            -next_my_material, table));

            engine.UnmakeMove(move);

            // ベータカット。
            if (score >= prob_beta) {
              // PVライン。
              engine.pv_line_table_[level].SetMove(move);
              engine.pv_line_table_[level].
              Insert(engine.pv_line_table_[level + 1]);

              // 取らない手。
              if (!(move & CAPTURED_PIECE_MASK)) {
                // 手の情報を得る。
                Square from = GetFrom(move);
                Square to = GetTo(move);

                // キラームーブ。
                if (params.enable_killer_) {
                  engine.shared_st_ptr_->killer_stack_[level][0] = move;
                  if (params.enable_killer_2_) {
                    engine.shared_st_ptr_->killer_stack_[level + 2][1] = move;
                  }
                }

                // ヒストリー。
                if (params.enable_history_) {
                  engine.shared_st_ptr_->history_[side][from][to] +=
                  Util::TransDepthToHistory(depth);

                  Util::UpdateMax
                  (engine.shared_st_ptr_->history_[side][from][to],
                  engine.shared_st_ptr_->history_max_);
                }
              }

              score_type = ScoreType::BETA;
              alpha = beta;
              break;
            }
          }
        }
      }
    }
  };

  // 通常の探索。
  template<NodeType Type>
  int ChessEngine::Search(Hash pos_hash, int depth, std::uint32_t level,
  int alpha, int beta, int material, TranspositionTable& table) {
    // 探索中止の時。
    if (ShouldBeStopped()) return alpha;
    if (is_job_ended_) return alpha;

    // ノード数を加算。
    ++(shared_st_ptr_->searched_nodes_);

    // 最大探索数。
    Util::UpdateMax(shared_st_ptr_->searched_level_, level);

    // サイドとチェックされているか。
    Side side = to_move_;
    Side enemy_side = Util::SwitchOppositeSide(side);
    bool is_checked = IsAttacked(king_[side], enemy_side);

    // PVLineをリセット。
    pv_line_table_[level].ResetLine();

    // 可読性のため、参照を定義。
    const SearchParams& params = *(shared_st_ptr_->search_params_ptr_);

    // 勝負に十分な駒がなければ0点。
    if (!HasSufficientMaterial()) {
      return SCORE_DRAW;
    }

    // --- 繰り返しチェック (繰り返しなら0点。) --- //
    position_memo_[level] = pos_hash;
    if (params.enable_repetition_check_) {
      if (level <= 2) {
        // お互いの初手。 (今までの配置を調べる。)
        for (auto& position : shared_st_ptr_->position_history_) {
          if (position == *this) {
            return SCORE_DRAW;
          }
        }
      } else if (params.enable_repetition_check_after_2nd_) {
        // お互いの2手目以降。 (2手前のハッシュを比較するだけ。)
        if (level <= 4) {
          // お互いの2手目。
          int index = shared_st_ptr_->position_history_.size() - (-level + 5);
          if ((index >= 0)
          && (shared_st_ptr_->position_history_[index].pos_hash()
          == pos_hash)) {
            return SCORE_DRAW;
          }
        } else {
          // お互いの3手目以降。
          if (position_memo_[level - 4] == pos_hash) {
            return SCORE_DRAW;
          }
        }
      }
    }

    // --- トランスポジションテーブル --- //
    Move prev_best = 0;
    if (params.enable_ttable_) {
      table.Lock();  // ロック。

      // 前回の繰り返しの最善手を含めたエントリーを得る。
      const TTEntry& tt_entry = table.GetEntry(pos_hash, depth - 1);
      if (tt_entry) {
        ScoreType score_type = tt_entry.score_type();

        // 前回の最善手を得る。
        if (score_type != ScoreType::ALPHA) {
          prev_best = tt_entry.best_move();
        }

        // 探索済み局面をチェック。
        // 局面の繰り返し対策などのため、
        // 自分の初手と相手の初手の場合(level < 2の場合)は参照しない。
        if ((level >= 2) && (tt_entry.depth() >= depth)) {
          Move best_move = prev_best;
          int score = tt_entry.score();

          if (tt_entry.score_type() == ScoreType::EXACT) {
            // エントリーが正確な値。
            if (!(tt_entry.best_move() & CAPTURED_PIECE_MASK)
            && (level < MAX_PLYS)) {
              // キラームーブをセット。
              if (params.enable_killer_) {
                shared_st_ptr_->killer_stack_[level][0] = best_move;
                if (params.enable_killer_2_) {
                  shared_st_ptr_->killer_stack_[level + 2][1] = best_move;
                }
              }
            }

            pv_line_table_[level].score(score);
            pv_line_table_[level].mate_in(tt_entry.mate_in());
            table.Unlock();  // ロック解除。
            return score;
          } else if (tt_entry.score_type() == ScoreType::ALPHA) {
            // エントリーがアルファ値。
            if (score <= alpha) {
              // アルファ値以下が確定。
              pv_line_table_[level].score(score);
              pv_line_table_[level].mate_in(tt_entry.mate_in());
              table.Unlock();  // ロック解除。
              return score;
            }

            // ベータ値を下げられる。
            if (score < beta) beta = score + 1;
          } else {
            // エントリーがベータ値。
            Move best_move = tt_entry.best_move();
            if (!(tt_entry.best_move() & CAPTURED_PIECE_MASK)
            && (level < MAX_PLYS)) {
              // キラームーブをセット。
              if (params.enable_killer_) {
                shared_st_ptr_->killer_stack_[level][0] = best_move;
                if (params.enable_killer_2_) {
                  shared_st_ptr_->killer_stack_[level + 2][1] = best_move;
                }
              }
            }

            if (score >= beta) {
              // ベータ値以上が確定。
              pv_line_table_[level].score(score);
              pv_line_table_[level].mate_in(tt_entry.mate_in());
              table.Unlock();  // ロック解除。
              return score;
            }

            // アルファ値を上げられる。
            if (score > alpha) alpha = score - 1;
          }
        }
      }
      table.Unlock();  // ロック解除。
    }

    // 深さが0ならクイース。 (無効なら評価値を返す。)
    // 限界探索数を超えていてもクイース。
    if ((depth <= 0) || (level >= MAX_PLYS)) {
      if (params.enable_quiesce_search_) {
        // クイース探索ノードに移行するため、ノード数を減らしておく。
        --(shared_st_ptr_->searched_nodes_);
        return Quiesce(depth, level, alpha, beta, material, table);
      } else {
        return evaluator_.Evaluate(material);
      }
    }

    // --- Internal Iterative Deepening --- //
    // テンプレート部品。
    DoIID<Type>::F(*this, is_checked, prev_best, pos_hash, depth, level,
    alpha, beta, material, table);

    // --- Null Move Reduction --- //
    int null_reduction = 0;
    // テンプレート部品。
    DoNMR<Type>::F(*this, is_checked, pos_hash, depth, level, beta, material,
    table, null_reduction);

    // 手を作る。
    maker_table_[level].GenMoves<GenMoveType::ALL>(prev_best,
    shared_st_ptr_->iid_stack_[level],
    shared_st_ptr_->killer_stack_[level][0],
    shared_st_ptr_->killer_stack_[level][1]);

    ScoreType score_type = ScoreType::ALPHA;

    // --- ProbCut --- //
    // テンプレート部品。
    DoProbCut<Type>::F(*this, side, enemy_side, is_checked, score_type,
    pos_hash, depth, level, alpha, beta, material, table);

    // --- Check Extension --- //
    if (params.enable_check_extension_ && is_checked) {
      depth += 1;
    }

    // 準備。
    int num_all_moves = maker_table_[level].RegenMoves();
    int num_moves = 0;
    int margin = GetMargin(depth);

    // 仕事の生成。
    Job& job = job_table_[level];
    job.Init(maker_table_[level]);
    job.client_ptr_ = this;
    job.record_ptr_ = nullptr;
    job.node_type_ = Type;
    job.pos_hash_ = pos_hash;
    job.depth_ = depth;
    job.level_ = level;
    job.alpha_ = alpha;
    job.beta_ = beta;
    job.table_ptr_ = &table;
    job.pv_line_ptr_ = &pv_line_table_[level];
    job.is_null_searching_ = is_null_searching_;
    job.null_reduction_ = null_reduction;
    job.score_type_ = score_type;
    job.material_ = material;
    job.is_checked_ = is_checked;
    job.num_all_moves_ = num_all_moves;
    job.has_legal_move_ = false;

    // パラメータ保存。
    // YBWC。
    int ybwc_after_moves = params.ybwc_after_moves_;
    int ybwc_limit_depth = params.ybwc_limit_depth_;

    // History Pruning。
    bool enable_history_pruning = params.enable_history_pruning_;

    int history_pruning_limit_depth = params.history_pruning_limit_depth_;

    int history_pruning_move_threshold = Util::GetMax(static_cast<int>
    (num_all_moves * params.history_pruning_move_threshold_),
    params.history_pruning_after_moves_);

    std::uint64_t history_pruning_threshold = shared_st_ptr_->history_max_
    * params.history_pruning_threshold_;

    int history_pruning_reduction = params.history_pruning_reduction_;

    // Late Move Reduction。
    bool enable_lmr = params.enable_lmr_;

    int lmr_limit_depth = params.lmr_limit_depth_;

    int lmr_threshold = Util::GetMax(static_cast<int>
    (num_all_moves * params.lmr_threshold_), params.lmr_after_moves_);

    int lmr_search_reduction = params.lmr_search_reduction_;

    // Futility Pruning。
    bool enable_futility_pruning = params.enable_futility_pruning_;

    int futility_pruning_depth = params.futility_pruning_depth_;

    for (Move move = job.PickMove(); move; move = job.PickMove()) {
      // 探索終了ならループを抜ける。
      if (ShouldBeStopped() || is_job_ended_ || (job.alpha_ >= job.beta_)) {
        break;
      }

      // 別スレッドに助けを求める。(YBWC)
      if ((job.depth_ >= ybwc_limit_depth) && (num_moves > ybwc_after_moves)) {
        shared_st_ptr_->helper_queue_ptr_->Help(job);
      }

      // 次のハッシュ。
      Hash next_hash = GetNextHash(job.pos_hash_, move);

      // 次の自分のマテリアル。
      int next_my_material = GetNextMyMaterial(job.material_, move);

      MakeMove(move);

      // 合法手じゃなければ次の手へ。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // 合法手があったのでフラグを立てる。
      job.has_legal_move_ = true;

      num_moves = job.Count();

      // -- Futility Pruning --- //
      if (enable_futility_pruning) {
        if (job.depth_ <= futility_pruning_depth) {
          if ((next_my_material + margin) <= job.alpha_) {
            UnmakeMove(move);
            continue;
          }
        }
      }

      // 手の情報を得る。
      Square from = GetFrom(move);
      Square to = GetTo(move);

      // 探索。
      int score = 0;
      int temp_alpha = job.alpha_;
      int temp_beta = job.beta_;
      int new_depth = job.depth_;

      // History PruningとLate Move Reductionの共通条件。
      bool is_hp_or_lmr_ok = false;
      if (!(job.is_checked_) && !(job.null_reduction_)
      && !(move & (CAPTURED_PIECE_MASK | PROMOTION_MASK))
      && !EqualMove(move, shared_st_ptr_->killer_stack_[level][0])
      && !EqualMove(move, shared_st_ptr_->killer_stack_[level][1])) {
        is_hp_or_lmr_ok = true;
      }

      // --- History Pruning --- //
      if (Type == NodeType::NON_PV) {
        if (enable_history_pruning) {
          if (is_hp_or_lmr_ok && (new_depth >= history_pruning_limit_depth)
          && (num_moves > history_pruning_move_threshold)
          && (shared_st_ptr_->history_[side][from][to]
          < history_pruning_threshold)) {
            new_depth -= history_pruning_reduction;
          }
        }
      }

      // --- Late Move Reduction --- //
      if (enable_lmr) {
        if (is_hp_or_lmr_ok && (new_depth >= lmr_limit_depth)
        && (num_moves > lmr_threshold)) {
          score = -Search<NodeType::NON_PV>(next_hash,
          new_depth - lmr_search_reduction - 1, job.level_ + 1,
          -(temp_alpha + 1), -temp_alpha, -next_my_material,
          *(job.table_ptr_));
        } else {
          // PVSearchをするためにtemp_alphaより大きくしておく。
          score = temp_alpha + 1;
        }
      } else {
        // PVSearchをするためにtemp_alphaより大きくしておく。
        score = temp_alpha + 1;
      }

      if (score > temp_alpha) {
        // --- PVSearch --- //
        if ((num_moves <= 1) || (Type == NodeType::NON_PV)) {
          // フルウィンドウで探索。
          score = -Search<Type>(next_hash, new_depth - 1, job.level_ + 1,
          -temp_beta, -temp_alpha, -next_my_material, *(job.table_ptr_));
        } else {
          // PV発見後のPVノード。
          // ゼロウィンドウ探索。
          score = -Search<NodeType::NON_PV>(next_hash, new_depth - 1,
          job.level_ + 1, -(temp_alpha + 1), -temp_alpha, -next_my_material,
          *(job.table_ptr_));

          if ((score > temp_alpha) && (score < temp_beta)) {
            // Fail Lowならず。
            // Fail-Softなので、Beta値以上も探索しない。
            // フルウィンドウで再探索。
            score = -Search<NodeType::PV>(next_hash, new_depth - 1,
            job.level_ + 1, -temp_beta, -temp_alpha, -next_my_material,
            *(job.table_ptr_));
          }
        }
      }

      UnmakeMove(move);

      job.Lock();  // ロック。

      // 探索終了ならループを抜ける。
      if (ShouldBeStopped() || is_job_ended_ || (job.alpha_ >= job.beta_)) {
        job.Unlock();  // ロック解除。
        break;
      }

      // アルファ値を更新。
      if (score > job.alpha_) {
        // 評価値のタイプをセット。
        if (job.score_type_ == ScoreType::ALPHA) {
          job.score_type_ = ScoreType::EXACT;
        }

        // PVライン。
        job.pv_line_ptr_->SetMove(move);
        job.pv_line_ptr_->Insert(pv_line_table_[level + 1]);
        job.pv_line_ptr_->score(score);

        job.alpha_ = score;
      }

      // ベータ値を調べる。
      if (job.alpha_ >= job.beta_) {
        // 評価値の種類をセット。
        job.score_type_ = ScoreType::BETA;

        // 取らない手。
        if (!(move & CAPTURED_PIECE_MASK)) {
          // キラームーブ。
          if (params.enable_killer_) {
            shared_st_ptr_->killer_stack_[level][0] = move;
            if (params.enable_killer_2_) {
              shared_st_ptr_->killer_stack_[level + 2][1] = move;
            }
          }

          // ヒストリー。
          if (params.enable_history_) {
            shared_st_ptr_->history_[side][from][to] += 
            Util::TransDepthToHistory(job.depth_);

            Util::UpdateMax(shared_st_ptr_->history_max_,
            shared_st_ptr_->history_[side][from][to]);
          }
        }

        // ベータカット。
        job.NotifyBetaCut();
        job.Unlock();  // ロック解除。
        break;
      }

      job.Unlock();  // ロック解除。
    }

    // スレッドを合流。
    job.WaitForHelpers();


    // このノードでゲーム終了だった場合。
    if (!(job.has_legal_move_)) {
      if (is_checked) {
        // チェックメイト。
        int score = SCORE_LOSE;
        pv_line_table_[level].score(score);
        pv_line_table_[level].mate_in(level);
        return score;
      } else {
        // ステールメイト。
        int score = SCORE_DRAW;
        pv_line_table_[level].score(score);
        return score;
      }
    }

    // トランスポジションテーブルに登録。
    // Null Move探索中の局面は登録しない。
    // Null Move Reductionされていた場合、容量節約のため登録しない。
    if (params.enable_ttable_) {
      if (!is_null_searching_ && !null_reduction && !ShouldBeStopped()) {
        table.Add(pos_hash, depth, job.alpha_, job.score_type_,
        pv_line_table_[level][0], pv_line_table_[level].mate_in());
      }
    }

    // 探索結果を返す。
    pv_line_table_[level].score(job.alpha_);
    return job.alpha_;
  }
  // 実体化。
  template int ChessEngine::Search<NodeType::PV>(Hash pos_hash, int depth,
  std::uint32_t level, int alpha, int beta, int material,
  TranspositionTable& table);
  template int ChessEngine::Search<NodeType::NON_PV>(Hash pos_hash, int depth,
  std::uint32_t level, int alpha, int beta, int material,
  TranspositionTable& table);

  // 探索のルート。
  PVLine ChessEngine::SearchRoot(TranspositionTable& table,
  const std::vector<Move>& moves_to_search, UCIShell& shell) {
    constexpr int level = 0;

    // --- 初期化 --- //
    shared_st_ptr_->searched_nodes_ = 0;
    shared_st_ptr_->searched_level_ = 0;
    shared_st_ptr_->start_time_ = SysClock::now();
    for (Side side = 0; side < NUM_SIDES; ++side) {
      for (Square from = 0; from < NUM_SQUARES; ++from) {
        for (Square to = 0; to < NUM_SQUARES; ++to) {
          shared_st_ptr_->history_[side][from][to] = 0;
        }
      }
    }
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
      position_memo_[i] = 0;
      shared_st_ptr_->iid_stack_[i] = 0;
      shared_st_ptr_->killer_stack_[i][0] = 0;
      shared_st_ptr_->killer_stack_[i][1] = 0;
      shared_st_ptr_->killer_stack_[i + 2][0] = 0;
      shared_st_ptr_->killer_stack_[i + 2][1] = 0;
      maker_table_[i].ResetStack();
      pv_line_table_[i].ResetLine();
      job_table_[i] = Job();
      record_table_[i] = PositionRecord();
    }
    shared_st_ptr_->history_max_ = 1;
    shared_st_ptr_->stop_now_ = false;
    shared_st_ptr_->i_depth_ = 1;
    is_null_searching_ = false;

    // PVLineに最初の候補手を入れておく。
    MoveMaker temp_maker(*this);
    temp_maker.GenMoves<GenMoveType::ALL>(0, 0, 0, 0);
    pv_line_table_[level].SetMove(temp_maker.PickMove());

    // スレッドの準備。
    shared_st_ptr_->helper_queue_ptr_.reset(new HelperQueue());
    std::vector<std::unique_ptr<ChessEngine>> child_vec(0);
    for (unsigned int i = 0; i < thread_vec_.size(); ++i) {
      child_vec.push_back
      (std::unique_ptr<ChessEngine>(new ChessEngine(*this)));

      child_vec.back()->shared_st_ptr_ = shared_st_ptr_;

      ChessEngine* child_ptr = child_vec.back().get();

      thread_vec_[i] = std::thread([child_ptr, &shell]() {
          child_ptr->ThreadYBWC(shell);
      });
    }

    // --- Iterative Deepening --- //
    Move prev_best = 0;
    Hash pos_hash = position_memo_[level] = GetCurrentHash();
    int material = GetMaterial(to_move_);
    int alpha = -MAX_VALUE;
    int beta = MAX_VALUE;
    Side side = to_move_;
    Side enemy_side = Util::SwitchOppositeSide(side);
    bool is_checked = IsAttacked(king_[side], enemy_side);
    bool found_mate = false;
    // 可読性のため、参照を定義。
    const SearchParams& params = *(shared_st_ptr_->search_params_ptr_);
    for (shared_st_ptr_->i_depth_ = 1; shared_st_ptr_->i_depth_ <= MAX_PLYS;
    ++(shared_st_ptr_->i_depth_)) {
      // 探索終了。
      if (ShouldBeStopped()) break;

      int depth = shared_st_ptr_->i_depth_;

      // ノードを加算。
      ++(shared_st_ptr_->searched_nodes_);

      // 探索したレベルをリセット。
      shared_st_ptr_->searched_level_ = 0;

      // 勝負に十分な駒がなければ探索しない。
      if (!HasSufficientMaterial()) {
        pv_line_table_[level].score(SCORE_DRAW);

        Chrono::milliseconds time =
        Chrono::duration_cast<Chrono::milliseconds>
        (SysClock::now() - shared_st_ptr_->start_time_);

        shell.PrintPVInfo(depth, 0, pv_line_table_[level].score(), time,
        shared_st_ptr_->searched_nodes_, pv_line_table_[level]);

        continue;
      }

      // メイトをすでに見つけていたら探索しない。
      if (found_mate) {
        Chrono::milliseconds time =
        Chrono::duration_cast<Chrono::milliseconds>
        (SysClock::now() - shared_st_ptr_->start_time_);

        shell.PrintPVInfo(depth, 0, pv_line_table_[level].score(), time,
        shared_st_ptr_->searched_nodes_, pv_line_table_[level]);

        continue;
      }

      // --- Aspiration Windows --- //
      int delta = params.aspiration_windows_delta_;
      // 探索窓の設定。
      if (params.enable_aspiration_windows_
      && (depth >= params.aspiration_windows_limit_depth_)) {
        beta = alpha + delta;
        alpha -= delta;
      } else {
        alpha = -MAX_VALUE;
        beta = MAX_VALUE;
      }

      // 標準出力に深さ情報を送る。
      shell.PrintDepthInfo(depth);

      // --- Check Extension --- //
      if (params.enable_check_extension_ && is_checked) {
        depth += 1;
      }

      // 仕事を作る。
      int num_all_moves =
      maker_table_[level].GenMoves<GenMoveType::ALL>(prev_best,
      shared_st_ptr_->iid_stack_[level],
      shared_st_ptr_->killer_stack_[level][0],
      shared_st_ptr_->killer_stack_[level][1]);
      Job& job = job_table_[level];
      job.Init(maker_table_[level]);
      job.client_ptr_ = this;
      job.record_ptr_ = nullptr;
      job.node_type_ = NodeType::PV;
      job.pos_hash_ = pos_hash;
      job.depth_ = depth;
      job.level_ = level;
      job.alpha_ = alpha;
      job.beta_ = beta;
      job.delta_ = delta;
      job.table_ptr_ = &table;
      job.pv_line_ptr_ = &pv_line_table_[level];
      job.is_null_searching_ = is_null_searching_;
      job.null_reduction_ = 0;
      job.score_type_ = ScoreType::EXACT;
      job.material_ = material;
      job.is_checked_ = is_checked;
      job.num_all_moves_ = num_all_moves;
      job.has_legal_move_ = false;
      job.moves_to_search_ptr_ = &moves_to_search;
      job.next_print_info_time_ =
      SysClock::now() + Chrono::milliseconds(1000);

      // ヘルプして待つ。
      shared_st_ptr_->helper_queue_ptr_->HelpRoot(job);
      job.WaitForHelpers();

      // 最善手を記録する。
      prev_best = pv_line_table_[level][0];

      // アルファ、ベータ、デルタを更新。
      alpha = job.alpha_;
      beta = job.beta_;
      delta = job.delta_;

      // メイトを見つけたらフラグを立てる。
      // 直接ループを抜けない理由は、depth等の終了条件対策。
      if (pv_line_table_[level].mate_in() >= 0) {
        found_mate = true;
      }
    }

    // スレッドをジョイン。
    shared_st_ptr_->helper_queue_ptr_->ReleaseHelpers();
    for (auto& thread : thread_vec_) {
      if (thread.joinable()) {
        thread.join();
      }
    }

    // 最後に情報を送る。
    shell.PrintOtherInfo
    (Chrono::duration_cast<Chrono::milliseconds>
    (SysClock::now() - (shared_st_ptr_->start_time_)),
    shared_st_ptr_->searched_nodes_, table.GetUsedPermill());


    // 探索終了したけど、まだ思考を止めてはいけない場合、関数を終了しない。
    while (!ShouldBeStopped()) continue;

    return pv_line_table_[level];
  }

  // YBWC探索用スレッド。
  void ChessEngine::ThreadYBWC(UCIShell& shell) {
    // 仕事ループ。
    while (true) {
      if (ShouldBeStopped()) break;

      // 準備。
      my_job_ptr_ = nullptr;
      is_job_ended_ = false;

      // 仕事を拾う。
      my_job_ptr_ = shared_st_ptr_->helper_queue_ptr_->GetJob();

      if (!my_job_ptr_ || !(my_job_ptr_->client_ptr_)) {
        break;
      } else {
        // お知らせ関数を登録。
        my_job_ptr_->AddBetaCutListener([this](Job& job) {
          if (&job == this->my_job_ptr_) {
            this->is_job_ended_ = true;
            for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
              this->job_table_[i].NotifyBetaCut();
            }
          }
        });

        my_job_ptr_->Lock();  // ロック。

        // お知らせを受け取る間もなく、
        // すでに仕事が終了しているかチェックする。
        if (my_job_ptr_->alpha_ >= my_job_ptr_->beta_) {
          is_job_ended_ = true;
          my_job_ptr_->Unlock();
          my_job_ptr_->FinishMyJob();
          continue;
        }

        // 駒の配置を読み込む。
        LoadRecord(*(my_job_ptr_->record_ptr_));

        // Null Move探索中かどうかをセット。
        is_null_searching_ = my_job_ptr_->is_null_searching_;

        my_job_ptr_->Unlock();  // ロック解除。

        if (my_job_ptr_->level_ <= 0) {
          // ルートノード。
          SearchRootParallel(*my_job_ptr_, shell);
        } else {
          // ルートではないノード。
          if (my_job_ptr_->node_type_ == NodeType::PV) {
            SearchParallel<NodeType::PV>(*my_job_ptr_);
          } else {
            SearchParallel<NodeType::NON_PV>(*my_job_ptr_);
          }
        }
      }
    }
  }

  // 並列探索。
  template<NodeType Type>
  void ChessEngine::SearchParallel(Job& job) {
    // 仕事ループ。
    Side side = to_move_;
    Side enemy_side = Util::SwitchOppositeSide(side);
    int num_moves = 0;
    int margin = GetMargin(job.depth_);

    // 可読性のため、参照を定義。
    const SearchParams& params = *(shared_st_ptr_->search_params_ptr_);

    // パラメータ保存。
    // YBWC。
    int ybwc_after_moves = params.ybwc_after_moves_;
    int ybwc_limit_depth = params.ybwc_limit_depth_;

    // History Pruning。
    bool enable_history_pruning = params.enable_history_pruning_;

    int history_pruning_limit_depth = params.history_pruning_limit_depth_;

    int history_pruning_move_threshold = Util::GetMax(static_cast<int>
    (job.num_all_moves_ * params.history_pruning_move_threshold_),
    params.history_pruning_after_moves_);

    std::uint64_t history_pruning_threshold =
    shared_st_ptr_->history_max_ * params.history_pruning_threshold_;

    int history_pruning_reduction = params.history_pruning_reduction_;

    // Late Move Reduction。
    bool enable_lmr = params.enable_lmr_;

    int lmr_limit_depth = params.lmr_limit_depth_;

    int lmr_threshold = Util::GetMax
    (static_cast<int>(job.num_all_moves_ * params.lmr_threshold_),
    params.lmr_after_moves_);

    int lmr_search_reduction = params.lmr_search_reduction_;

    // Futility Pruning。
    bool enable_futility_pruning = params.enable_futility_pruning_;

    int futility_pruning_depth = params.futility_pruning_depth_;

    for (Move move = job.PickMove(); move; move = job.PickMove()) {
      // 探索終了ならループを抜ける。
      if (ShouldBeStopped() || is_job_ended_ || (job.alpha_ >= job.beta_)) {
        break;
      }

      // 別スレッドに助けを求める。(YBWC)
      if ((job.depth_ >= ybwc_limit_depth) && (num_moves > ybwc_after_moves)) {
        shared_st_ptr_->helper_queue_ptr_->Help(job);
      }

      // 次のハッシュ。
      Hash next_hash = GetNextHash(job.pos_hash_, move);

      // 次の局面のマテリアルを得る。
      int next_my_material = GetNextMyMaterial(job.material_, move);

      MakeMove(move);

      // 合法手じゃなければ次の手へ。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      num_moves = job.Count();

      // 合法手が見つかったのでフラグを立てる。
      job.has_legal_move_ = true;

      // --- Futility Pruning --- //
      if (enable_futility_pruning) {
        if (job.depth_ <= futility_pruning_depth) {
          if ((next_my_material + margin) <= job.alpha_) {
            UnmakeMove(move);
            continue;
          }
        }
      }

      // 手の情報を得る。
      Square from = GetFrom(move);
      Square to = GetTo(move);

      // 探索。
      int score = 0;
      int temp_alpha = job.alpha_;
      int temp_beta = job.beta_;
      int new_depth = job.depth_;

      // History PruningとLate Move Reductionの共通条件。
      bool is_hp_or_lmr_ok = false;
      if (!(job.is_checked_) && !(job.null_reduction_)
      && !(move & (CAPTURED_PIECE_MASK | PROMOTION_MASK))
      && !EqualMove(move, shared_st_ptr_->killer_stack_[job.level_][0])
      && !EqualMove(move, shared_st_ptr_->killer_stack_[job.level_][1])) {
        is_hp_or_lmr_ok = true;
      }

      // --- History Pruning --- //
      if (Type == NodeType::NON_PV) {
        if (enable_history_pruning) {
          if (is_hp_or_lmr_ok && (new_depth >= history_pruning_limit_depth)
          && (num_moves > history_pruning_move_threshold)
          && (shared_st_ptr_->history_[side][from][to]
          < history_pruning_threshold)) {
            new_depth -= history_pruning_reduction;
          }
        }
      }

      // --- Late Move Reduction --- //
      if (enable_lmr) {
        if (is_hp_or_lmr_ok && (new_depth >= lmr_limit_depth)
        && (num_moves > lmr_threshold)) {
          score = -Search<NodeType::NON_PV>(next_hash,
          new_depth - lmr_search_reduction - 1, job.level_ + 1,
          -(temp_alpha + 1), -temp_alpha, -next_my_material,
          *(job.table_ptr_));
        } else {
          // PVSearchをするためにtemp_alphaより大きくしておく。
          score = temp_alpha + 1;
        }
      } else {
        // PVSearchをするためにtemp_alphaより大きくしておく。
        score = temp_alpha + 1;
      }

      if (score > temp_alpha) {
        // --- PVSearch --- //
        if ((num_moves <= 1) || (Type == NodeType::NON_PV)) {
          // フルウィンドウ探索。
          score = -Search<Type>(next_hash, new_depth - 1,
          job.level_ + 1, -temp_beta, -temp_alpha, -next_my_material,
          *(job.table_ptr_));
        } else {
          // PV発見後。
          // ゼロウィンドウ探索。
          score = -Search<NodeType::NON_PV>(next_hash, new_depth - 1,
          job.level_ + 1, -(temp_alpha + 1), -temp_alpha,
          -next_my_material, *(job.table_ptr_));

          if ((score > temp_alpha) && (score < temp_beta)) {
            // Fail Lowならず。
            // Fail-Softなので、ベータ値以上も探索しない。
            score = -Search<NodeType::PV>(next_hash, new_depth - 1,
            job.level_ + 1, -temp_beta, -temp_alpha, -next_my_material,
            *(job.table_ptr_));
          }
        }
      }

      UnmakeMove(move);

      job.Lock();  // ロック。

      // 探索終了ならループを抜ける。
      if (ShouldBeStopped() || is_job_ended_ || (job.alpha_ >= job.beta_)) {
        job.Unlock();  // ロック解除。
        break;
      }

      // アルファ値を更新。
      if (score > job.alpha_) {
        // 評価値のタイプをセット。
        if (job.score_type_ == ScoreType::ALPHA) {
          job.score_type_ = ScoreType::EXACT;
        }

        // PVラインをセット。
        job.pv_line_ptr_->SetMove(move);
        job.pv_line_ptr_->Insert(pv_line_table_[job.level_ + 1]);
        job.pv_line_ptr_->score(score);

        job.alpha_ = score;
      }

      // ベータ値を調べる。
      if (job.alpha_ >= job.beta_) {
        // 評価値の種類をセット。
        job.score_type_ = ScoreType::BETA;

        // 取らない手。
        if (!(move & CAPTURED_PIECE_MASK)) {
          // キラームーブ。
          if (params.enable_killer_) {
            shared_st_ptr_->killer_stack_[job.level_][0] = move;
            if (params.enable_killer_2_) {
              shared_st_ptr_->killer_stack_[job.level_ + 2][1] = move;
            }
          }

          // ヒストリー。
          if (params.enable_history_) {
            shared_st_ptr_->history_[side][from][to] +=
            Util::TransDepthToHistory(job.depth_);

            Util::UpdateMax(shared_st_ptr_->history_max_,
            shared_st_ptr_->history_[side][from][to]);
          }
        }

        // ベータカット。
        job.NotifyBetaCut();
        job.Unlock();  // ロック解除。
        break;
      }

      job.Unlock();  // ロック解除。
    }

    // 仕事終了。
    job.FinishMyJob();
  }
  // 実体化。
  template void ChessEngine::SearchParallel<NodeType::PV>(Job& job);
  template void ChessEngine::SearchParallel<NodeType::NON_PV>(Job& job);

  // ルートノードで呼び出される、別スレッド用探索関数。
  void ChessEngine::SearchRootParallel(Job& job, UCIShell& shell) {
    // 仕事ループ。
    Side side = to_move_;
    Side enemy_side = Util::SwitchOppositeSide(side);
    int num_moves = 0;
    // 可読性のため、参照を定義。
    const SearchParams& params = *(shared_st_ptr_->search_params_ptr_);

    // パラメータを保存。
    // YBWC。
    int ybwc_after_moves = params.ybwc_after_moves_;
    int ybwc_limit_depth = params.ybwc_limit_depth_;

    // Late Move Reduction。
    bool enable_lmr = params.enable_lmr_;

    int lmr_limit_depth = params.lmr_limit_depth_;

    int lmr_threshold = Util::GetMax(static_cast<int>
    (job.num_all_moves_ * params.lmr_threshold_), params.lmr_after_moves_);

    int lmr_search_reduction = params.lmr_search_reduction_;

    for (Move move = job.PickMove(); move; move = job.PickMove()) {
      if (ShouldBeStopped()) break;

      // 定時(1秒)報告の情報を送る。
      job.Lock();  // ロック。
      TimePoint now = SysClock::now();
      if (now > job.next_print_info_time_) {
        shell.PrintOtherInfo(Chrono::duration_cast<Chrono::milliseconds>
        (now - shared_st_ptr_->start_time_),
        shared_st_ptr_->searched_nodes_, job.table_ptr_->GetUsedPermill());

        job.next_print_info_time_ = now + Chrono::milliseconds(1000);
      }
      job.Unlock();  // ロック解除。

      // 探索すべき手が指定されていれば、今の手がその手かどうか調べる。
      if (!(job.moves_to_search_ptr_->empty())) {
        bool hit = false;
        for (auto move_2 : *(job.moves_to_search_ptr_)) {
          if (EqualMove(move_2, move)) {
            // 探索すべき手だった。
            hit = true;
            break;
          }
        }
        if (!hit) {
          // 探索すべき手ではなかった。
          // 次の手へ。
          continue;
        }
      }

      // 別スレッドに助けを求める。(YBWC)
      if ((job.depth_ >= ybwc_limit_depth) && (num_moves > ybwc_after_moves)) {
        shared_st_ptr_->helper_queue_ptr_->Help(job);
      }

      // 次のハッシュ。
      Hash next_hash = GetNextHash(job.pos_hash_, move);

      // 次の局面のマテリアル。
      int next_my_material = GetNextMyMaterial(job.material_, move);

      MakeMove(move);

      // 合法手じゃなければ次の手へ。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      num_moves = job.Count();

      // 現在探索している手の情報を表示。
      job.Lock();  // ロック。
      shell.PrintCurrentMoveInfo(move, num_moves);
      job.Unlock();  // ロック解除。

      // --- PVSearch --- //
      int score = 0;
      int temp_alpha = job.alpha_;
      int temp_beta = job.beta_;
      if (num_moves <= 1) {
        while (true) {
          // 探索終了。
          if (ShouldBeStopped()) break;

          // フルでPVを探索。
          score = -Search<NodeType::PV> (next_hash, job.depth_ - 1,
          job.level_ + 1, -temp_beta, -temp_alpha, -next_my_material,
          *(job.table_ptr_));
          
          // アルファ値、ベータ値を調べる。
          job.Lock();  // ロック。
          if (score >= temp_beta) {
            // 探索失敗。
            // ベータ値を広げる。
            job.delta_ *= 2;
            temp_beta = score + job.delta_;
            if (job.beta_ >= temp_beta) {
              temp_beta = job.beta_;
            } else {
              job.beta_ = temp_beta;
            }

            if ((pv_line_table_[job.level_ + 1].mate_in() >= 0)
            && ((pv_line_table_[job.level_ + 1].mate_in() % 2) == 1)) {
              // メイトっぽかった場合。
              job.beta_ = MAX_VALUE;
              temp_beta = MAX_VALUE;
            }

            job.Unlock();  // ロック解除。
            continue;

          } else if (score <= temp_alpha) {
            // 探索失敗。
            // アルファ値を広げる。
            job.delta_ *= 2;
            temp_alpha = score - job.delta_;
            if (job.alpha_ <= temp_alpha) {
              temp_alpha = job.alpha_;
            } else {
              job.alpha_ = temp_alpha;
            }

            if ((pv_line_table_[job.level_ + 1].mate_in() >= 0)
            && ((pv_line_table_[job.level_ + 1].mate_in() % 2) == 0)) {
              // メイトされていたかもしれない場合。
              job.alpha_ = -MAX_VALUE;
              temp_alpha = -MAX_VALUE;
            }

            job.Unlock();  // ロック解除。
            continue;

          } else {
            job.Unlock();  // ロック解除。
            break;
          }
          job.Unlock();  // ロック解除。
        }
      } else {
        // --- Late Move Reduction --- //
        if (enable_lmr) {
          if (!(job.is_checked_)
          && (job.depth_ >= lmr_limit_depth)
          && (num_moves > lmr_threshold)
          && !(move & (CAPTURED_PIECE_MASK | PROMOTION_MASK))) {
            // ゼロウィンドウ探索。
            score = -Search<NodeType::NON_PV>(next_hash,
            job.depth_ - lmr_search_reduction - 1, job.level_ + 1,
            -(temp_alpha + 1), -temp_alpha, -next_my_material,
            *(job.table_ptr_));
          } else {
            // 普通に探索するためにscoreをalphaより大きくしておく。
            score = temp_alpha + 1;
          }
        } else {
          // 普通に探索するためにscoreをalphaより大きくしておく。
          score = temp_alpha + 1;
        }

        // 普通の探索。
        if (score > temp_alpha) {
          // ゼロウィンドウ探索。
          score = -Search<NodeType::NON_PV>(next_hash, job.depth_ - 1,
          job.level_ + 1, -(temp_alpha + 1), -temp_alpha,
          -next_my_material, *(job.table_ptr_));

          if (score > temp_alpha) {
            while (true) {
              // 探索終了。
              if (ShouldBeStopped()) break;

              // フルウィンドウで再探索。
              score = -Search<NodeType::PV>(next_hash, job.depth_ - 1,
              job.level_ + 1, -temp_beta, -temp_alpha, -next_my_material,
              *(job.table_ptr_));

              // ベータ値を調べる。
              job.Lock();  // ロック。
              if (score >= temp_beta) {
                // 探索失敗。
                // ベータ値を広げる。
                job.delta_ *= 2;
                temp_beta = score + job.delta_;
                if (job.beta_ >= temp_beta) {
                  temp_beta = job.beta_;
                } else {
                  job.beta_ = temp_beta;
                }

                if ((pv_line_table_[job.level_ + 1].mate_in() >= 0)
                && ((pv_line_table_[job.level_ + 1].mate_in() % 2) == 1)) {
                  // メイトっぽかった場合。
                  job.beta_ = MAX_VALUE;
                  temp_beta = MAX_VALUE;
                }

                job.Unlock();  // ロック解除。
                continue;

              } else {
                job.Unlock();  // ロック解除。
                break;
              }
              job.Unlock();  // ロック解除。
            }
          }
        }
      }

      UnmakeMove(move);

      // ストップがかかっていたらループを抜ける。
      if (ShouldBeStopped()) break;

      // 最善手を見つけた。
      job.Lock();  // ロック。
      if (score > job.alpha_) {
        // PVラインにセット。
        job.pv_line_ptr_->SetMove(move);
        job.pv_line_ptr_->Insert(pv_line_table_[job.level_ + 1]);
        job.pv_line_ptr_->score(score);

        // 標準出力にPV情報を表示。
        Chrono::milliseconds time =
        Chrono::duration_cast<Chrono::milliseconds>
        (SysClock::now() - shared_st_ptr_->start_time_);

        shell.PrintPVInfo(job.depth_, shared_st_ptr_->searched_level_, score,
        time, shared_st_ptr_->searched_nodes_, *(job.pv_line_ptr_));

        job.alpha_ = score;
      }
      job.Unlock();  // ロック解除。
    }

    // 仕事終了。
    job.FinishMyJob();
  }

  // SEEで候補手を評価する。
  int ChessEngine::SEE(Move move) const {
    int score = 0;
    if (!(shared_st_ptr_->search_params_ptr_->enable_see_)) return score;

    if (move) {
      // 手の情報を得る。
      Square to = GetTo(move);
      MoveType move_type = GetMoveType(move);

      // キングを取る手なら無視。
      if (piece_board_[to] == KING) {
        return score;
      }

      // 取る駒の価値を得る。
      int capture_value = 0;
      if (move_type == EN_PASSANT) {
        // アンパッサン。
        capture_value = shared_st_ptr_->search_params_ptr_->material_[PAWN];
      } else {
        capture_value =
        shared_st_ptr_->search_params_ptr_->material_[piece_board_[to]];
      }

      // ポーンの昇格。
      if ((move & PROMOTION_MASK)) {
        capture_value +=
        shared_st_ptr_->search_params_ptr_->material_[GetPromotion(move)]
        - shared_st_ptr_->search_params_ptr_->material_[PAWN];
      }

      Side side = to_move_;

      ChessEngine* self = const_cast<ChessEngine*>(this);
      self->MakeMove(move);

      // 違法な手なら計算しない。
      if (!(IsAttacked(king_[side], Util::SwitchOppositeSide(side)))) {
        // 再帰して次の局面の評価値を得る。
        score = capture_value - self->SEE(GetNextSEEMove(to));
      }

      self->UnmakeMove(move);
    }

    return score;
  }

  // SEE()で使う、次の手を得る。
  Move ChessEngine::GetNextSEEMove(Square target) const {
    // キングがターゲットの時はなし。
    if (target == king_[Util::SwitchOppositeSide(to_move_)]) {
      return 0;
    }

    // 価値の低いものから調べる。
    for (Piece piece_type = PAWN; piece_type <= KING; ++piece_type) {
      Bitboard attackers = 0;
      Piece promotion = EMPTY;
      switch (piece_type) {
        case PAWN:
          attackers =
          Util::GetPawnAttack(Util::SwitchOppositeSide(to_move_), target)
          & position_[to_move_][PAWN];
          if (((to_move_ == WHITE)
          && (Util::SQUARE_TO_RANK[target] == RANK_8))
          || ((to_move_ == BLACK)
          && (Util::SQUARE_TO_RANK[target] == RANK_1))) {
            promotion = QUEEN;
          }
          break;
        case KNIGHT:
          attackers =
          Util::GetKnightMove(target) & position_[to_move_][KNIGHT];
          break;
        case BISHOP:
          attackers = GetBishopAttack(target) & position_[to_move_][BISHOP];
          break;
        case ROOK:
          attackers = GetRookAttack(target) & position_[to_move_][ROOK];
          break;
        case QUEEN:
          attackers = GetQueenAttack(target) & position_[to_move_][QUEEN];
          break;
        case KING:
          attackers = Util::GetKingMove(target) & position_[to_move_][KING];
          break;
        default:
          throw SayuriError
          ("駒の種類が不正です。in ChessEngine::GetNextSEEMove()");
          break;
      }
      if (attackers) {
        Move move = 0;
        SetFrom(move, Util::GetSquare(attackers));
        SetTo(move, target);
        SetPromotion(move, promotion);
        SetMoveType(move, NORMAL);
        return move;
      }
    }

    return 0;
  }

  // 探索のストップ条件を設定する。
  void ChessEngine::SetStopper(std::uint32_t max_depth,
  std::uint64_t max_nodes, const Chrono::milliseconds& thinking_time,
  bool infinite_thinking) {
    shared_st_ptr_->max_depth_ = Util::GetMin(max_depth, MAX_PLYS);
    shared_st_ptr_->max_nodes_ = Util::GetMin(max_nodes, MAX_NODES);
    shared_st_ptr_->thinking_time_ = thinking_time;
    shared_st_ptr_->infinite_thinking_ = infinite_thinking;
  }

  // 無限に思考するかどうかのフラグをセットする。
  void ChessEngine::EnableInfiniteThinking(bool enable) {
    shared_st_ptr_->infinite_thinking_ = enable;
  }

  // 探索を中断しなければいけないかどうかを判断する。
  bool ChessEngine::ShouldBeStopped() {
    // 最低1手は考える。
    if (shared_st_ptr_->i_depth_ <= 1) return false;

    if (shared_st_ptr_->stop_now_) return true;
    if (shared_st_ptr_->infinite_thinking_) return false;
    if (shared_st_ptr_->i_depth_ > shared_st_ptr_->max_depth_) {
      shared_st_ptr_->stop_now_ = true;
      return true;
    }
    if (shared_st_ptr_->searched_nodes_ >= shared_st_ptr_->max_nodes_) {
      shared_st_ptr_->stop_now_ = true;
      return true;
    }
    TimePoint now = SysClock::now();
    if ((now - (shared_st_ptr_->start_time_))
    >= shared_st_ptr_->thinking_time_) {
      shared_st_ptr_->stop_now_ = true;
      return true;
    }
    return false;
  }
}  // namespace Sayuri
