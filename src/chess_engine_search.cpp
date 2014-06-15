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

    // ノード数を加算。
    shared_st_ptr_->num_searched_nodes_++;

    // 最大探索数。
    if (level > searched_level_) {
      searched_level_ = level;
    }

    // サイド。
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;

    // stand_pad。
    int stand_pad = evaluator_.Evaluate(material);

    // アルファ値、ベータ値を調べる。
    if (stand_pad >= beta) {
      return beta;
    }
    if (stand_pad > alpha) {
      alpha = stand_pad;
    }

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
    shared_st_ptr_->search_params_ptr_->enable_futility_pruning();

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
      if (score > alpha) {
        alpha = score;
      }
      if (score >= beta) {
        alpha = beta;
        break;
      }
    }

    return alpha;
  }

  // 通常の探索。
  template<NodeType Type>
  int ChessEngine::Search(Hash pos_hash, int depth, std::uint32_t level,
  int alpha, int beta, int material, TranspositionTable& table) {
    // 探索中止の時。
    if (ShouldBeStopped()) return alpha;

    // ノード数を加算。
    shared_st_ptr_->num_searched_nodes_++;

    // 最大探索数。
    if (level > searched_level_) {
      searched_level_ = level;
    }

    // サイドとチェックされているか。
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    bool is_checked = IsAttacked(king_[side], enemy_side);

    // PVLineをリセット。
    pv_line_table_[level].ResetLine();

    // Repetition Checking。 (繰り返しなら0点。)
    position_memo_[level] = pos_hash;
    if (level <= 2) {
      // お互いの初手。 (今までの配置を調べる。)
      for (auto& position : shared_st_ptr_->position_history_) {
        if (position == *this) {
          int score = SCORE_DRAW;
          if (score < alpha) return alpha;
          if (score > beta) return beta;
          return score;
        }
      }
    } else if (!is_checked) {
      // お互いの2手目以降。 (2手前のハッシュを比較するだけ。)
      if (level <= 4) {
        // お互いの2手目。
        int index = shared_st_ptr_->position_history_.size() - (-level + 5);
        if ((index >= 0)
        && (shared_st_ptr_->position_history_[index].pos_hash() == pos_hash)) {
          int score = SCORE_DRAW;
          if (score < alpha) return alpha;
          if (score > beta) return beta;
          return score;
        }
      } else {
        // お互いの3手目以降。
        if (position_memo_[level - 4] == pos_hash) {
          int score = SCORE_DRAW;
          if (score < alpha) return alpha;
          if (score > beta) return beta;
          return score;
        }
      }
    }

    // --- トランスポジションテーブル --- //
    Move prev_best = 0;
    if (shared_st_ptr_->search_params_ptr_->enable_ttable()) {
      table.Lock();
      // 前回の繰り返しの最善手を得る。
      const TTEntry& prev_entry = table.GetEntry(pos_hash, depth - 1);
      if (prev_entry && (prev_entry.score_type() != ScoreType::ALPHA)) {
        prev_best = prev_entry.best_move();
      }
      // 局面の繰り返し対策などのため、
      // 自分の初手と相手の初手の場合(level < 2の場合)は参照しない。
      // 前回の繰り返しの最善手を得る。
      if (level >= 2) {
        const TTEntry& tt_entry = table.GetEntry(pos_hash, depth);
        if (tt_entry) {
          int score = tt_entry.score();
          if (tt_entry.score_type() == ScoreType::EXACT) {
            // エントリーが正確な値。
            Move best_move = tt_entry.best_move();
            if (!(tt_entry.best_move() & CAPTURED_PIECE_MASK)
            && (level < MAX_PLYS)) {
              // キラームーブをセット。
              if (shared_st_ptr_->search_params_ptr_->enable_killer()) {
                shared_st_ptr_->killer_stack_[level][0] = best_move;
                if (shared_st_ptr_->search_params_ptr_->enable_killer_2()) {
                  shared_st_ptr_->killer_stack_[level + 2][1] = best_move;
                }
              }
            }

            pv_line_table_[level].mate_in(tt_entry.mate_in());

            if (score >= beta) {
              pv_line_table_[level].score(beta);
              table.Unlock();
              return beta;
            }

            if (score <= alpha) {
              pv_line_table_[level].score(alpha);
              table.Unlock();
              return alpha;
            }

            pv_line_table_[level].score(score);
            table.Unlock();
            return score;
          } else if (tt_entry.score_type() == ScoreType::ALPHA) {
            // エントリーがアルファ値。
            if (score <= alpha) {
              // アルファ値以下が確定。
              pv_line_table_[level].score(alpha);
              pv_line_table_[level].mate_in(tt_entry.mate_in());
              table.Unlock();
              return alpha;
            }

            // ベータ値を下げられる。
            if (score < beta) beta = score + 1;
          } else {
            // エントリーがベータ値。
            Move best_move = tt_entry.best_move();
            if (!(tt_entry.best_move() & CAPTURED_PIECE_MASK)
            && (level < MAX_PLYS)) {
              // キラームーブをセット。
              if (shared_st_ptr_->search_params_ptr_->enable_killer()) {
                shared_st_ptr_->killer_stack_[level][0] = best_move;
                if (shared_st_ptr_->search_params_ptr_->enable_killer_2()) {
                  shared_st_ptr_->killer_stack_[level + 2][1] = best_move;
                }
              }
            }

            if (score >= beta) {
              // ベータ値以上が確定。
              pv_line_table_[level].score(beta);
              pv_line_table_[level].mate_in(tt_entry.mate_in());
              table.Unlock();
              return beta;
            }

            // アルファ値を上げられる。
            if (score > alpha) alpha = score - 1;
          }
        }
      }
      table.Unlock();
    }

    // 深さが0ならクイース。
    // 限界探索数を超えていてもクイース。
    if ((depth <= 0) || (level >= MAX_PLYS)) {
      // クイース探索ノードに移行するため、ノード数を減らしておく。
      shared_st_ptr_->num_searched_nodes_--;
      return Quiesce(depth, level, alpha, beta, material, table);
    }

    // --- Internal Iterative Deepening --- //
    if (Type == NodeType::PV) {
      if (shared_st_ptr_->search_params_ptr_->enable_iid()) {
        // 前回の繰り返しの最善手があればIIDしない。
        if (prev_best) {
          shared_st_ptr_->iid_stack_[level] = prev_best;
        } else {
          if (!is_checked && (depth
          >= shared_st_ptr_->search_params_ptr_->iid_limit_depth())) {
            // Internal Iterative Deepening。
            Search<NodeType::PV>(pos_hash,
            shared_st_ptr_->search_params_ptr_->iid_search_depth(), level,
            alpha, beta, material, table);

            shared_st_ptr_->iid_stack_[level] = pv_line_table_[level][0];
          }
        }
      }
    }

    // --- Null Move Reduction --- //
    int null_reduction = 0;
    if (Type == NodeType::NON_PV) {
      if (shared_st_ptr_->search_params_ptr_->enable_nmr()) {
        if (!is_null_searching_ && !is_checked && (depth
        >= shared_st_ptr_->search_params_ptr_->nmr_limit_depth())) {
          // Null Move Reduction。
          Move null_move = 0;

          is_null_searching_ = true;
          MakeNullMove(null_move);

          // Null Move Search。
          int score = -Search<NodeType::NON_PV>(pos_hash, depth
          - shared_st_ptr_->search_params_ptr_->nmr_search_reduction() - 1,
          level + 1, -(beta), -(beta - 1), -material, table);

          UnmakeNullMove(null_move);
          is_null_searching_ = false;

          if (score >= beta) {
            null_reduction =
            shared_st_ptr_->search_params_ptr_->nmr_reduction();

            depth -= null_reduction;
            if ((depth <= 0)) {
              // クイース探索ノードに移行するため、ノード数を減らしておく。
              shared_st_ptr_->num_searched_nodes_--;
              return Quiesce(depth, level, alpha, beta, material, table);
            }
          }
        }
      }
    }

    // 手を作る。
    MoveMaker& maker = maker_table_[level];
    maker.GenMoves<GenMoveType::ALL>(prev_best,
    shared_st_ptr_->iid_stack_[level],
    shared_st_ptr_->killer_stack_[level][0],
    shared_st_ptr_->killer_stack_[level][1]);

    // --- ProbCut --- //
    if ((Type == NodeType::NON_PV)) {
      if (shared_st_ptr_->search_params_ptr_->enable_probcut()) {
        if (!is_null_searching_ && !is_checked && (depth
        >= shared_st_ptr_->search_params_ptr_->probcut_limit_depth())) {
          // 手を作る。
          maker.RegenMoves();

          // 浅読みパラメータ。
          int prob_beta =
          beta + shared_st_ptr_->search_params_ptr_->probcut_margin();
          int prob_depth = depth
          - shared_st_ptr_->search_params_ptr_->probcut_search_reduction();

          // 探索。
          for (Move move = maker.PickMove(); move; move = maker.PickMove()) {
            // 次のノードへの準備。
            Hash next_hash = GetNextHash(pos_hash, move);
            int next_my_material = GetNextMyMaterial(material, move);

            MakeMove(move);

            // 合法手じゃなければ次の手へ。
            if (IsAttacked(king_[side], enemy_side)) {
              UnmakeMove(move);
              continue;
            }

            int score = -Search<NodeType::NON_PV>(next_hash, prob_depth - 1,
            level + 1, -prob_beta, -(prob_beta - 1), -next_my_material, table);

            UnmakeMove(move);

            // ベータカット。
            if (score >= prob_beta) {
              // PVライン。
              pv_line_table_[level].SetMove(move);
              pv_line_table_[level].Insert(pv_line_table_[level + 1]);

              // 取らない手。
              if (!(move & CAPTURED_PIECE_MASK)) {
                // 手の情報を得る。
                Square from = move_from(move);
                Square to = move_to(move);

                // キラームーブ。
                if (shared_st_ptr_->search_params_ptr_->enable_killer()) {
                  shared_st_ptr_->killer_stack_[level][0] = move;
                  if (shared_st_ptr_->search_params_ptr_->enable_killer_2()) {
                    shared_st_ptr_->killer_stack_[level + 2][1] = move;
                  }
                }

                // ヒストリー。
                if (shared_st_ptr_->search_params_ptr_->enable_history()) {
                  shared_st_ptr_->history_[side][from][to] += depth * depth;
                  if (shared_st_ptr_->history_[side][from][to]
                  > shared_st_ptr_->history_max_) {
                    shared_st_ptr_->history_max_ =
                    shared_st_ptr_->history_[side][from][to];
                  }
                }
              }

              // トランスポジションテーブルに登録。
              // Null Move Reductionされていた場合、容量節約のため登録しない。
              if (shared_st_ptr_->search_params_ptr_->enable_ttable()) {
                if (!null_reduction && !ShouldBeStopped()) {
                  table.Add(pos_hash, depth, beta, ScoreType::BETA,
                  pv_line_table_[level][0], pv_line_table_[level].mate_in());
                }
              }

              return beta;
            }
          }
        }
      }
    }

    // --- Check Extension --- //
    if (is_checked) {
      depth += 1;
    }

    // 準備。
    int num_all_moves = maker.RegenMoves();
    int num_moves = 0;
    ScoreType score_type = ScoreType::ALPHA;
    bool has_legal_move = false;
    int margin = GetMargin(depth);

    // 仕事の生成。
    std::mutex mutex;
    PositionRecord record(*this, pos_hash);
    Job& job = job_table_[level];
    job.Init(maker);
    job.mutex_ptr_ = &mutex;
    job.record_ptr_ = &record;
    job.node_type_ = Type;
    job.pos_hash_ = pos_hash;
    job.depth_ = depth;
    job.level_ = level;
    job.alpha_ptr_ = &alpha;
    job.beta_ptr_ = &beta;
    job.table_ptr_ = &table;
    job.pv_line_ptr_ = &pv_line_table_[level];
    job.is_null_searching_ = is_null_searching_;
    job.null_reduction_ = null_reduction;
    job.score_type_ptr_ = &score_type;
    job.material_ = material;
    job.is_checked_ = is_checked;
    job.num_all_moves_ = num_all_moves;
    job.has_legal_move_ptr_ = &has_legal_move;

    // パラメータ保存。
    // YBWC。
    int ybwc_after_moves =
    shared_st_ptr_->search_params_ptr_->ybwc_after_moves();
    int ybwc_limit_depth =
    shared_st_ptr_->search_params_ptr_->ybwc_limit_depth();

    // History Pruning。
    bool enable_history_pruning =
    shared_st_ptr_->search_params_ptr_->enable_history_pruning();

    int history_pruning_limit_depth =
    shared_st_ptr_->search_params_ptr_->history_pruning_limit_depth();

    int history_pruning_move_threshold = num_all_moves *
    shared_st_ptr_->search_params_ptr_->history_pruning_move_threshold();
    int history_pruning_after_moves =
    shared_st_ptr_->search_params_ptr_->history_pruning_after_moves();
    history_pruning_move_threshold =
    history_pruning_move_threshold < history_pruning_after_moves 
    ? history_pruning_after_moves : history_pruning_move_threshold;

    std::uint64_t history_pruning_threshold = shared_st_ptr_->history_max_
    * shared_st_ptr_->search_params_ptr_->history_pruning_threshold();

    int history_pruning_reduction =
    shared_st_ptr_->search_params_ptr_->history_pruning_reduction();

    // Late Move Reduction。
    bool enable_lmr = shared_st_ptr_->search_params_ptr_->enable_lmr();

    int lmr_limit_depth =
    shared_st_ptr_->search_params_ptr_->lmr_limit_depth();

    int lmr_threshold = num_all_moves
    * shared_st_ptr_->search_params_ptr_->lmr_threshold();
    int lmr_after_moves =
    shared_st_ptr_->search_params_ptr_->lmr_after_moves();
    lmr_threshold =
    lmr_threshold < lmr_after_moves ? lmr_after_moves : lmr_threshold;

    int lmr_search_reduction =
    shared_st_ptr_->search_params_ptr_->lmr_search_reduction();

    // Futility Pruning。
    bool enable_futility_pruning =
    shared_st_ptr_->search_params_ptr_->enable_futility_pruning();

    int futility_pruning_depth =
    shared_st_ptr_->search_params_ptr_->futility_pruning_depth();

    for (Move move = maker.PickMove(); move; move = maker.PickMove()) {
      // すでにベータカットされていればループを抜ける。
      if (alpha >= beta) {
        break;
      }

      // 別スレッドに助けを求める。(YBWC)
      if ((depth >= ybwc_limit_depth) && (num_moves > ybwc_after_moves)) {
        shared_st_ptr_->helper_queue_ptr_->Help(job);
      }

      // 次のハッシュ。
      Hash next_hash = GetNextHash(pos_hash, move);

      // 次の自分のマテリアル。
      int next_my_material = GetNextMyMaterial(material, move);

      MakeMove(move);

      // 合法手じゃなければ次の手へ。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // 合法手があったのでフラグを立てる。
      has_legal_move = true;

      num_moves = job.Count();

      // -- Futility Pruning --- //
      if (enable_futility_pruning) {
        if (depth <= futility_pruning_depth) {
          if ((next_my_material + margin) <= alpha) {
            UnmakeMove(move);
            continue;
          }
        }
      }

      // 手の情報を得る。
      Square from = move_from(move);
      Square to = move_to(move);

      // 探索。
      int score = 0;
      int temp_alpha = alpha;
      int temp_beta = beta;
      int new_depth = depth;

      // History PruningとLate Move Reductionの共通条件。
      bool is_hp_or_lmr_ok = false;
      if (!is_checked && !null_reduction
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
          new_depth - lmr_search_reduction - 1, level + 1, -(temp_alpha + 1),
          -temp_alpha, -next_my_material, table);
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
          score = -Search<Type>(next_hash, new_depth - 1, level + 1,
          -temp_beta, -temp_alpha, -next_my_material, table);
        } else {
          // PV発見後のPVノード。
          // ゼロウィンドウ探索。
          score = -Search<NodeType::NON_PV>(next_hash, new_depth - 1,
          level + 1, -(temp_alpha + 1), -temp_alpha, -next_my_material, table);

          if (score > temp_alpha) {
            // fail lowならず。
            // フルウィンドウで再探索。
            score = -Search<NodeType::PV>(next_hash, new_depth - 1, level + 1,
            -temp_beta, -temp_alpha, -next_my_material, table);
          }
        }
      }

      UnmakeMove(move);

      mutex.lock();  // ロック。

      // アルファ値を更新。
      if (score > alpha) {
        // 評価値のタイプをセット。
        if (score_type == ScoreType::ALPHA) {
          score_type = ScoreType::EXACT;
        }

        // PVライン。
        pv_line_table_[level].SetMove(move);
        pv_line_table_[level].Insert(pv_line_table_[level + 1]);

        alpha = score;
      }

      // ベータ値を調べる。
      if (score >= beta) {
        // 評価値の種類をセット。
        score_type = ScoreType::BETA;

        // 取らない手。
        if (!(move & CAPTURED_PIECE_MASK)) {
          // キラームーブ。
          if (shared_st_ptr_->search_params_ptr_->enable_killer()) {
            shared_st_ptr_->killer_stack_[level][0] = move;
            if (shared_st_ptr_->search_params_ptr_->enable_killer_2()) {
              shared_st_ptr_->killer_stack_[level + 2][1] = move;
            }
          }

          // ヒストリー。
          if (shared_st_ptr_->search_params_ptr_->enable_history()) {
            shared_st_ptr_->history_[side][from][to] += depth * depth;
            if (shared_st_ptr_->history_[side][from][to]
            > shared_st_ptr_->history_max_) {
              shared_st_ptr_->history_max_ =
              shared_st_ptr_->history_[side][from][to];
            }
          }
        }

        // ベータカット。
        mutex.unlock();  // ロック解除。
        alpha = beta;
        break;
      }

      mutex.unlock();  // ロック解除。
    }

    // スレッドを合流。
    job.WaitForHelpers();


    // このノードでゲーム終了だった場合。
    if (!has_legal_move) {
      if (is_checked) {
        // チェックメイト。
        int score = SCORE_LOSE;
        if (score < alpha) score = alpha;
        if (score > beta) score = beta;
        pv_line_table_[level].score(score);
        pv_line_table_[level].mate_in(level);
        return score;
      } else {
        // ステールメイト。
        int score = SCORE_DRAW;
        if (score < alpha) score = alpha;
        if (score > beta) score = beta;
        pv_line_table_[level].score(score);
        return score;
      }
    }

    // トランスポジションテーブルに登録。
    // Null Move探索中の局面は登録しない。
    // Null Move Reductionされていた場合、容量節約のため登録しない。
    if (shared_st_ptr_->search_params_ptr_->enable_ttable()) {
      if (!is_null_searching_ && !null_reduction && !ShouldBeStopped()) {
        table.Add(pos_hash, depth, alpha, score_type,
        pv_line_table_[level][0], pv_line_table_[level].mate_in());
      }
    }

    // 探索結果を返す。
    pv_line_table_[level].score(alpha);
    return alpha;
  }
  // 実体化。
  template int ChessEngine::Search<NodeType::PV>(Hash pos_hash,
  int depth, std::uint32_t level, int alpha, int beta, int material,
  TranspositionTable& table);
  template int ChessEngine::Search<NodeType::NON_PV>(Hash pos_hash,
  int depth, std::uint32_t level, int alpha, int beta, int material,
  TranspositionTable& table);

  // 探索のルート。
  PVLine ChessEngine::SearchRoot(TranspositionTable& table,
  const std::vector<Move>& moves_to_search, UCIShell& shell) {
    // --- 初期化 --- //
    searched_level_ = 0;
    shared_st_ptr_->num_searched_nodes_ = 0;
    shared_st_ptr_->start_time_ = SysClock::now();
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Square from = 0; from < NUM_SQUARES; from++) {
        for (Square to = 0; to < NUM_SQUARES; to++) {
          shared_st_ptr_->history_[side][from][to] = 0;
        }
      }
    }
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      position_memo_[i] = 0;
      shared_st_ptr_->iid_stack_[i] = 0;
      shared_st_ptr_->killer_stack_[i][0] = 0;
      shared_st_ptr_->killer_stack_[i][1] = 0;
      shared_st_ptr_->killer_stack_[i + 2][0] = 0;
      shared_st_ptr_->killer_stack_[i + 2][1] = 0;
      maker_table_[i].ResetStack();
      pv_line_table_[i].ResetLine();
      job_table_[i] = Job();
    }
    shared_st_ptr_->history_max_ = 1;
    shared_st_ptr_->stop_now_ = false;
    shared_st_ptr_->i_depth_ = 1;
    is_null_searching_ = false;

    // スレッドの準備。
    shared_st_ptr_->helper_queue_ptr_.reset(new HelperQueue());
    for (auto& thread : thread_vec_) {
      thread =
      std::thread(&ChessEngine::ThreadYBWC, this, std::ref(shell));
    }

    // --- Iterative Deepening --- //
    int level = 0;
    Hash pos_hash = position_memo_[level] = GetCurrentHash();
    int material = GetMaterial(to_move_);
    int alpha = -MAX_VALUE;
    int beta = MAX_VALUE;
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    TimePoint now = SysClock::now();
    TimePoint next_print_info_time = now + Chrono::milliseconds(1000);
    MoveMaker& maker = maker_table_[level];
    bool is_checked = IsAttacked(king_[side], enemy_side);
    bool found_mate = false;
    for (shared_st_ptr_->i_depth_ = 1; shared_st_ptr_->i_depth_ <= MAX_PLYS;
    shared_st_ptr_->i_depth_++) {
      // 探索終了。
      if (ShouldBeStopped()) break;

      // ノードを加算。
      shared_st_ptr_->num_searched_nodes_++;

      // メイトをすでに見つけていたら探索しない。
      if (found_mate) {
        Chrono::milliseconds time =
        Chrono::duration_cast<Chrono::milliseconds>
        (SysClock::now() - shared_st_ptr_->start_time_);

        shell.PrintPVInfo(shared_st_ptr_->i_depth_, 0,
        pv_line_table_[level].score(), time,
        shared_st_ptr_->num_searched_nodes_, pv_line_table_[level]);

        continue;
      }

      // --- Aspiration Windows --- //
      int delta =
      shared_st_ptr_->search_params_ptr_->aspiration_windows_delta();
      // 探索窓の設定。
      if (shared_st_ptr_->search_params_ptr_->enable_aspiration_windows()
      && (shared_st_ptr_->i_depth_ >= shared_st_ptr_->search_params_ptr_->
      aspiration_windows_limit_depth())) {
        beta = alpha + delta;
        alpha -= delta;
      } else {
        alpha = -MAX_VALUE;
        beta = MAX_VALUE;
      }

      // 標準出力に深さ情報を送る。
      shell.PrintDepthInfo(shared_st_ptr_->i_depth_);

      // 前回の繰り返しの最善手を得る。
      Move prev_best = 0;
      if (shared_st_ptr_->search_params_ptr_->enable_ttable()) {
        table.Lock();
        const TTEntry& prev_entry =
        table.GetEntry(pos_hash, shared_st_ptr_->i_depth_ - 1);
        if (prev_entry && (prev_entry.score_type() != ScoreType::ALPHA)) {
          prev_best = prev_entry.best_move();
        }
        table.Unlock();
      }

      // --- Check Extension --- //
      int depth = shared_st_ptr_->i_depth_;
      if (is_checked) {
        depth += 1;
      }

      // 仕事を作る。
      std::mutex mutex;
      PositionRecord record(*this, pos_hash);
      int num_all_moves = maker.GenMoves<GenMoveType::ALL>(prev_best,
      shared_st_ptr_->iid_stack_[level],
      shared_st_ptr_->killer_stack_[level][0],
      shared_st_ptr_->killer_stack_[level][1]);
      ScoreType score_type = ScoreType::EXACT;
      bool has_legal_move = false;
      Job& job = job_table_[level];
      job.Init(maker);
      job.mutex_ptr_ = &mutex;
      job.record_ptr_ = &record;
      job.node_type_ = NodeType::PV;
      job.pos_hash_ = pos_hash;
      job.depth_ = depth;
      job.level_ = level;
      job.alpha_ptr_ = &alpha;
      job.beta_ptr_ = &beta;
      job.delta_ptr_ = &delta;
      job.table_ptr_ = &table;
      job.pv_line_ptr_ = &pv_line_table_[level];
      job.is_null_searching_ = is_null_searching_;
      job.null_reduction_ = 0;
      job.score_type_ptr_ = &score_type;
      job.material_ = material;
      job.is_checked_ = is_checked;
      job.num_all_moves_ = num_all_moves;
      job.has_legal_move_ptr_ = &has_legal_move;
      job.moves_to_search_ptr_ = &moves_to_search;
      job.next_print_info_time_ptr_ = &next_print_info_time;

      // ヘルプして待つ。
      shared_st_ptr_->helper_queue_ptr_->HelpRoot(job);
      job.WaitForHelpers();

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
    now = SysClock::now();
    shell.PrintOtherInfo
    (Chrono::duration_cast<Chrono::milliseconds>
    (now - (shared_st_ptr_->start_time_)),
    shared_st_ptr_->num_searched_nodes_, table.GetUsedPermill());


    // 探索終了したけど、まだ思考を止めてはいけない場合、関数を終了しない。
    while (!ShouldBeStopped()) continue;

    return pv_line_table_[level];
  }

  // YBWC探索用スレッド。
  void ChessEngine::ThreadYBWC(UCIShell& shell) {
    // 子エンジンを作る。
    mutex_.lock();
    std::unique_ptr<ChessEngine> child_ptr(new ChessEngine());
    child_ptr->shared_st_ptr_ = shared_st_ptr_;
    mutex_.unlock();

    // 仕事ループ。
    while (true) {
      if (ShouldBeStopped()) break;

      // 仕事を拾う。
      Job* job_ptr = child_ptr->shared_st_ptr_->helper_queue_ptr_->GetJob();

      if (!job_ptr) {
        break;
      } else {
        // 駒の配置を読み込む。
        child_ptr->LoadRecord(*(job_ptr->record_ptr_));

        // Null Move探索中かどうかをセット。
        child_ptr->is_null_searching_ = job_ptr->is_null_searching_;

        if (job_ptr->level_ <= 0) {
          // ルートノード。
          child_ptr->SearchRootParallel(*job_ptr, shell);
        } else {
          // ルートではないノード。
          if (job_ptr->node_type_ == NodeType::PV) {
            child_ptr->SearchParallel<NodeType::PV>(*job_ptr);
          } else {
            child_ptr->SearchParallel<NodeType::NON_PV>(*job_ptr);
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
    Side enemy_side = side ^ 0x3;
    int num_moves = 0;
    int margin = GetMargin(job.depth_);

    // パラメータ保存。
    // YBWC。
    int ybwc_after_moves =
    shared_st_ptr_->search_params_ptr_->ybwc_after_moves();
    int ybwc_limit_depth =
    shared_st_ptr_->search_params_ptr_->ybwc_limit_depth();

    // History Pruning。
    bool enable_history_pruning =
    shared_st_ptr_->search_params_ptr_->enable_history_pruning();

    int history_pruning_limit_depth =
    shared_st_ptr_->search_params_ptr_->history_pruning_limit_depth();

    int history_pruning_move_threshold = job.num_all_moves_
    * shared_st_ptr_->search_params_ptr_->history_pruning_move_threshold();
    int history_pruning_after_moves =
    shared_st_ptr_->search_params_ptr_->history_pruning_after_moves();
    history_pruning_move_threshold =
    history_pruning_move_threshold < history_pruning_after_moves
    ? history_pruning_after_moves : history_pruning_move_threshold;

    std::uint64_t history_pruning_threshold = shared_st_ptr_->history_max_
    * shared_st_ptr_->search_params_ptr_->history_pruning_threshold();

    int history_pruning_reduction =
    shared_st_ptr_->search_params_ptr_->history_pruning_reduction();

    // Late Move Reduction。
    bool enable_lmr = shared_st_ptr_->search_params_ptr_->enable_lmr();

    int lmr_limit_depth =
    shared_st_ptr_->search_params_ptr_->lmr_limit_depth();

    int lmr_threshold = job.num_all_moves_
    * shared_st_ptr_->search_params_ptr_->lmr_threshold();
    int lmr_after_moves =
    shared_st_ptr_->search_params_ptr_->lmr_after_moves();
    lmr_threshold =
    lmr_threshold < lmr_after_moves ? lmr_after_moves : lmr_threshold;

    int lmr_search_reduction =
    shared_st_ptr_->search_params_ptr_->lmr_search_reduction();

    // Futility Pruning。
    bool enable_futility_pruning =
    shared_st_ptr_->search_params_ptr_->enable_futility_pruning();

    int futility_pruning_depth =
    shared_st_ptr_->search_params_ptr_->futility_pruning_depth();

    for (Move move = job.PickMove(); move; move = job.PickMove()) {
      // すでにベータカットされていれば仕事をしない。
      if (*(job.alpha_ptr_) >= *(job.beta_ptr_)) {
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
      *(job.has_legal_move_ptr_) = true;

      // --- Futility Pruning --- //
      if (enable_futility_pruning) {
        if (job.depth_ <= futility_pruning_depth) {
          if ((next_my_material + margin) <= *(job.alpha_ptr_)) {
            UnmakeMove(move);
            continue;
          }
        }
      }

      // 手の情報を得る。
      Square from = move_from(move);
      Square to = move_to(move);

      // 探索。
      int score = 0;
      int temp_alpha = *(job.alpha_ptr_);
      int temp_beta = *(job.beta_ptr_);
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

          if (score > temp_alpha) {
            // fail lowならず。
            score = -Search<NodeType::PV>(next_hash, new_depth - 1,
            job.level_ + 1, -temp_beta, -temp_alpha, -next_my_material,
            *(job.table_ptr_));
          }
        }
      }

      UnmakeMove(move);

      job.mutex_ptr_->lock();  // ロック。

      // 探索した深さを更新。
      if (searched_level_ > job.client_ptr_->searched_level_) {
        job.client_ptr_->searched_level_ = searched_level_;
      }

      // アルファ値を更新。
      if (score > *(job.alpha_ptr_)) {
        // 評価値のタイプをセット。
        if (*(job.score_type_ptr_) == ScoreType::ALPHA) {
          *(job.score_type_ptr_) = ScoreType::EXACT;
        }

        // PVラインをセット。
        job.pv_line_ptr_->SetMove(move);
        job.pv_line_ptr_->Insert(pv_line_table_[job.level_ + 1]);

        *(job.alpha_ptr_) = score;
      }

      // ベータ値を調べる。
      if (score >= *(job.beta_ptr_)) {
        // 評価値の種類をセット。
        *(job.score_type_ptr_) = ScoreType::BETA;

        // 取らない手。
        if (!(move & CAPTURED_PIECE_MASK)) {
          // キラームーブ。
          if (shared_st_ptr_->search_params_ptr_->enable_killer()) {
            shared_st_ptr_->killer_stack_[job.level_][0] = move;
            if (shared_st_ptr_->search_params_ptr_->enable_killer_2()) {
              shared_st_ptr_->killer_stack_[job.level_ + 2][1] = move;
            }
          }

          // ヒストリー。
          if (shared_st_ptr_->search_params_ptr_->enable_history()) {
            shared_st_ptr_->history_[side][from][to] +=
            job.depth_ * job.depth_;
            if (shared_st_ptr_->history_[side][from][to]
            > shared_st_ptr_->history_max_) {
              shared_st_ptr_->history_max_ =
              shared_st_ptr_->history_[side][from][to];
            }
          }
        }

        // ベータカット。
        *(job.alpha_ptr_) = *(job.beta_ptr_);
        job.mutex_ptr_->unlock();  // ロック解除。
        break;
      }

      job.mutex_ptr_->unlock();  // ロック解除。
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
    Side enemy_side = side ^ 0x3;
    int num_moves = 0;

    // パラメータを保存。
    // YBWC。
    int ybwc_after_moves =
    shared_st_ptr_->search_params_ptr_->ybwc_after_moves();
    int ybwc_limit_depth =
    shared_st_ptr_->search_params_ptr_->ybwc_limit_depth();

    // Late Move Reduction。
    bool enable_lmr = shared_st_ptr_->search_params_ptr_->enable_lmr();

    int lmr_limit_depth =
    shared_st_ptr_->search_params_ptr_->lmr_limit_depth();

    int lmr_threshold = job.num_all_moves_
    * shared_st_ptr_->search_params_ptr_->lmr_threshold();
    int lmr_after_moves =
    shared_st_ptr_->search_params_ptr_->lmr_after_moves();
    lmr_threshold =
    lmr_threshold < lmr_after_moves ? lmr_after_moves : lmr_threshold;

    int lmr_search_reduction =
    shared_st_ptr_->search_params_ptr_->lmr_search_reduction();

    for (Move move = job.PickMove(); move; move = job.PickMove()) {
      if (ShouldBeStopped()) break;

      // 定時(1秒)報告の情報を送る。
      job.mutex_ptr_->lock();  // ロック。
      TimePoint now = SysClock::now();
      if (now > *(job.next_print_info_time_ptr_)) {
        shell.PrintOtherInfo(Chrono::duration_cast<Chrono::milliseconds>
        (now - shared_st_ptr_->start_time_),
        shared_st_ptr_->num_searched_nodes_, job.table_ptr_->GetUsedPermill());

        *(job.next_print_info_time_ptr_) = now + Chrono::milliseconds(1000);
      }
      job.mutex_ptr_->unlock();  // ロック解除。

      // 探索すべき手が指定されていれば、今の手がその手かどうか調べる。
      if (!(job.moves_to_search_ptr_->empty())) {
        bool hit = false;
        const std::vector<Move>& vec = *(job.moves_to_search_ptr_);
        for (auto move_2 : vec) {
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

      // 探索したレベルをリセット。
      searched_level_ = 0;

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
      job.mutex_ptr_->lock();  // ロック。
      shell.PrintCurrentMoveInfo(move, num_moves);
      job.mutex_ptr_->unlock();  // ロック解除。

      // --- PVSearch --- //
      int score = 0;
      int temp_alpha = *(job.alpha_ptr_);
      int temp_beta = *(job.beta_ptr_);
      if (num_moves <= 1) {
        while (true) {
          // 探索終了。
          if (ShouldBeStopped()) break;

          // フルでPVを探索。
          score = -Search<NodeType::PV> (next_hash, job.depth_ - 1,
          job.level_ + 1, -temp_beta, -temp_alpha, -next_my_material,
          *(job.table_ptr_));

          // アルファ値、ベータ値を調べる。
          job.mutex_ptr_->lock();  // ロック。
          if (score >= temp_beta) {
            // 探索失敗。
            // ベータ値を広げる。
            if ((pv_line_table_[job.level_ + 1].mate_in() >= 0)
            && ((pv_line_table_[job.level_ + 1].mate_in() % 2) == 1)) {
              // メイトっぽかった場合。
              *(job.beta_ptr_) = MAX_VALUE;
            } else {
              *(job.delta_ptr_) *= 2;
              *(job.beta_ptr_) += *(job.delta_ptr_);
            }
            temp_beta = *(job.beta_ptr_);
            job.mutex_ptr_->unlock();  // ロック解除。
            continue;
          } else if (score <= temp_alpha) {
            // 探索失敗。
            // アルファ値を広げる。
            if ((pv_line_table_[job.level_ + 1].mate_in() >= 0)
            && ((pv_line_table_[job.level_ + 1].mate_in() % 2) == 0)) {
              // メイトされていたかもしれない場合。
              *(job.alpha_ptr_) = -MAX_VALUE;
            } else {
              *(job.delta_ptr_) *= 2;
              *(job.alpha_ptr_) -= *(job.delta_ptr_);
            }
            temp_alpha = *(job.alpha_ptr_);
            job.mutex_ptr_->unlock();  // ロック解除。
            continue;
          } else {
            job.mutex_ptr_->unlock();  // ロック解除。
            break;
          }
          job.mutex_ptr_->unlock();  // ロック解除。
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
              job.mutex_ptr_->lock();  // ロック。
              if (score >= temp_beta) {
                // 探索失敗。
                // ベータ値を広げる。
                if ((pv_line_table_[job.level_ + 1].mate_in() >= 0)
                && ((pv_line_table_[job.level_ + 1].mate_in() % 2) == 1)) {
                  // メイトっぽかった場合。
                  *(job.beta_ptr_) = MAX_VALUE;
                  job.mutex_ptr_->unlock();
                } else {
                  *(job.delta_ptr_) *= 2;
                  *(job.beta_ptr_) += *(job.delta_ptr_);
                }
                temp_beta = *(job.beta_ptr_);
                job.mutex_ptr_->unlock();  // ロック解除。
                continue;
              } else {
                job.mutex_ptr_->unlock();  // ロック解除。
                break;
              }
              job.mutex_ptr_->unlock();  // ロック解除。
            }
          }
        }
      }

      UnmakeMove(move);

      // ストップがかかっていたらループを抜ける。
      if (ShouldBeStopped()) break;

      // 最善手を見つけた。
      job.mutex_ptr_->lock();  // ロック。
      if (score > *(job.alpha_ptr_)) {
        // PVラインにセット。
        job.pv_line_ptr_->SetMove(move);
        job.pv_line_ptr_->Insert(pv_line_table_[job.level_ + 1]);
        job.pv_line_ptr_->score(score);

        // トランスポジションテーブルに登録。
        if (shared_st_ptr_->search_params_ptr_->enable_ttable()) {
          job.table_ptr_->Add(job.pos_hash_, job.depth_, score,
          ScoreType::EXACT, (*(job.pv_line_ptr_))[0],
          job.pv_line_ptr_->mate_in());
        }

        // 標準出力にPV情報を表示。
        now = SysClock::now();
        Chrono::milliseconds time =
        Chrono::duration_cast<Chrono::milliseconds>
        (now - shared_st_ptr_->start_time_);

        shell.PrintPVInfo(job.depth_, searched_level_, score,
        time, shared_st_ptr_->num_searched_nodes_, *(job.pv_line_ptr_));

        *(job.alpha_ptr_) = score;
      }
      job.mutex_ptr_->unlock();  // ロック解除。
    }

    // 仕事終了。
    job.FinishMyJob();
  }

  // SEEで候補手を評価する。
  int ChessEngine::SEE(Move move) const {
    int score = 0;
    if (!(shared_st_ptr_->search_params_ptr_->enable_see())) return score;

    if (move) {
      // 手の情報を得る。
      Square to = move_to(move);
      MoveType move_type = move_move_type(move);

      // キングを取る手なら無視。
      if (piece_board_[to] == KING) {
        return score;
      }

      // 取る駒の価値を得る。
      int capture_value = 0;
      if (move_type == EN_PASSANT) {
        // アンパッサン。
        capture_value = shared_st_ptr_->search_params_ptr_->material()[PAWN];
      } else {
        capture_value =
        shared_st_ptr_->search_params_ptr_->material()[piece_board_[to]];
      }

      // ポーンの昇格。
      if ((move & PROMOTION_MASK)) {
        capture_value +=
        shared_st_ptr_->search_params_ptr_->material()[move_promotion(move)]
        - shared_st_ptr_->search_params_ptr_->material()[PAWN];
      }

      Side side = to_move_;

      ChessEngine* self = const_cast<ChessEngine*>(this);
      self->MakeMove(move);

      // 違法な手なら計算しない。
      if (!(IsAttacked(king_[side], side ^ 0x3))) {
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
    if (target == king_[to_move_ ^ 0x3]) {
      return 0;
    }

    // 価値の低いものから調べる。
    for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
      Bitboard attackers = 0;
      Piece promotion = EMPTY;
      switch (piece_type) {
        case PAWN:
          attackers = Util::GetPawnAttack(target, to_move_ ^ 0x3)
          & position_[to_move_][PAWN];
          if (((to_move_ == WHITE) && (Util::GetRank(target) == RANK_8))
          || ((to_move_ == BLACK) && (Util::GetRank(target) == RANK_1))) {
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
          throw SayuriError("駒の種類が不正です。");
          break;
      }
      if (attackers) {
        Move move = 0;
        move_from(move, Util::GetSquare(attackers));
        move_to(move, target);
        move_promotion(move, promotion);
        move_move_type(move, NORMAL);
        return move;
      }
    }

    return 0;
  }
  // Futility Pruningのマージンを計算する。
  int ChessEngine::GetMargin(int depth) {
    if (depth <= 1) {
      return shared_st_ptr_->search_params_ptr_->futility_pruning_margin();
    }

    return shared_st_ptr_->search_params_ptr_->futility_pruning_margin()
    * depth;
  }

  // 探索のストップ条件を設定する。
  void ChessEngine::SetStopper(std::uint32_t max_depth,
  std::uint64_t max_nodes, Chrono::milliseconds thinking_time,
  bool infinite_thinking) {
    shared_st_ptr_->max_depth_ = max_depth <= MAX_PLYS ? max_depth : MAX_PLYS;
    shared_st_ptr_->max_nodes_ =
    max_nodes <= MAX_NODES ? max_nodes : MAX_NODES;
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
    if (shared_st_ptr_->num_searched_nodes_ >= shared_st_ptr_->max_nodes_) {
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
