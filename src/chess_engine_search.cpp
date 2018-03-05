/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Hironori Ishibashi
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
#include <functional>
#include <queue>
#include <system_error>
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
#include "cache.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // クイース探索。
  int ChessEngine::Quiesce(u32 level, int alpha, int beta, int material) {
    // ジョブの準備。
    Job& job = job_table_[level];
    job.Init(maker_table_[level]);

    // キャッシュ。
    Cache& cache = shared_st_ptr_->cache_;

    // 探索中止。
    if (JudgeToStop(job)) return ReturnProcess(alpha, level);

    // ノード数を加算。
    ++(shared_st_ptr_->searched_nodes_);

    // 最大探索数。
    Util::UpdateMax(shared_st_ptr_->searched_level_, level);

    // 勝負に十分な駒がなければ0点。
    if (!HasSufficientMaterial()) {
      return ReturnProcess(SCORE_DRAW, level);
    }

    // サイド。
    Side side = basic_st_.to_move_;
    Side enemy_side = Util::GetOppositeSide(side);

    // stand_pad。
    int stand_pad = evaluator_.Evaluate(material);

    // アルファ値、ベータ値を調べる。
    if (stand_pad >= beta) {
      return ReturnProcess(stand_pad, level);
    }
    Util::UpdateMax(alpha, stand_pad);

    // 探索できる限界を超えているか。
    // 超えていればこれ以上探索しない。
    if (level >= MAX_PLYS) {
      return ReturnProcess(alpha, level);
    }

    // 候補手を作る。
    // 駒を取る手だけ。
    MoveMaker& maker = maker_table_[level];
    if (IsAttacked(basic_st_.king_[side], enemy_side)) {
      maker.GenMoves<GenMoveType::ALL>(0, 0, 0, 0);
    } else {
      maker.GenMoves<GenMoveType::CAPTURE>(0, 0, 0, 0);
    }

    // 探索する。
    // --- Futility Pruning --- //
    int margin = cache.futility_pruning_margin_[0];

    for (Move move = maker.PickMove(); move; move = maker.PickMove()) {
      if (JudgeToStop(job)) break;

      // 次の自分のマテリアル。
      int next_material = GetNextMaterial(material, move);

      MakeMove(move);

      // 合法手かどうか調べる。
      if (IsAttacked(basic_st_.king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // Futility Pruning。
      if ((-next_material + margin) <= alpha) {
        UnmakeMove(move);
        continue;
      }

      // 次の手を探索。
      int score = -Quiesce(level + 1, -beta, -alpha, next_material);

      UnmakeMove(move);

      // アルファ値、ベータ値を調べる。
      Util::UpdateMax(alpha, score);
      if (alpha >= beta) {
        break;
      }
    }

    return ReturnProcess(alpha, level);
  }

  // 通常の探索。
  int ChessEngine::Search(NodeType node_type, Hash pos_hash, int depth,
  u32 level, int alpha, int beta, int material) {
    // ジョブの準備。
    Job& job = job_table_[level];
    job.Init(maker_table_[level]);

    // キャッシュ。
    Cache& cache = shared_st_ptr_->cache_;

    // 探索中止。
    if (JudgeToStop(job)) return ReturnProcess(alpha, level);

    // ノード数を加算。
    ++(shared_st_ptr_->searched_nodes_);

    // 最大探索数。
    Util::UpdateMax(shared_st_ptr_->searched_level_, level);

    // サイドとチェックされているか。
    Side side = basic_st_.to_move_;
    Side enemy_side = Util::GetOppositeSide(side);
    bool is_checked = IsAttacked(basic_st_.king_[side], enemy_side);

    // PVLineをリセット。
    pv_line_table_[level].ResetLine();

    // 勝負に十分な駒がなければ0点。
    if (!HasSufficientMaterial()) {
      return ReturnProcess(SCORE_DRAW, level);
    }

    // --- 繰り返しチェック (繰り返しなら0点。) --- //
    basic_st_.position_memo_[level] = pos_hash;
    if (cache.enable_repetition_check_) {
      if (level <= 4) {
        // お互いの2手目。
        int index = (shared_st_ptr_->position_history_.size() - 5) + level;
        if ((index >= 0)
        && (shared_st_ptr_->position_history_[index].pos_hash()
        == pos_hash)) {
          return ReturnProcess(SCORE_DRAW, level);
        }
      } else {
        // お互いの3手目以降。
        if (basic_st_.position_memo_[level - 4] == pos_hash) {
          return ReturnProcess(SCORE_DRAW, level);
        }
      }
    }

    // --- トランスポジションテーブル --- //
    Move prev_best = 0;
    if (cache.enable_ttable_) {
      table_ptr_->Lock();  // ロック。

      // 前回の繰り返しの最善手を含めたエントリーを得る。
      const TTEntry& tt_entry = table_ptr_->GetEntry(pos_hash);
      if (tt_entry) {
        ScoreType score_type = tt_entry.score_type();

        // 前回の最善手を得る。
        if (score_type != ScoreType::ALPHA) {
          prev_best = tt_entry.best_move();
        }

        // 探索省略のための探索済み局面をチェック。
        // (注 1) 局面の繰り返し対策などのため、
        // 自分の初手と相手の初手の場合(level < 2の場合)は探索省略しない。
        // (注 2) チェックメイトを見つけていた時
        // (SCORE_WIN以上、SCORE_LOSE以下)は、
        // 「チェックメイト遠回り問題」を避けるため、探索省略しない。。
        int score = tt_entry.score();
        if ((level >= 2) && (tt_entry.depth() >= depth)
        && (score < SCORE_WIN) && (score > SCORE_LOSE)) {
          Move best_move = prev_best;

          if (tt_entry.score_type() == ScoreType::EXACT) {
            // エントリーが正確な値。
            if (!(tt_entry.best_move() & MASK[CAPTURED_PIECE])
            && (level < MAX_PLYS)) {
              // キラームーブをセット。
              if (cache.enable_killer_) {
                shared_st_ptr_->killer_stack_[level][0] = best_move;
                shared_st_ptr_->killer_stack_[level + 2][1] = best_move;
              }
            }

            pv_line_table_[level].score(score);
            table_ptr_->Unlock();  // ロック解除。
            return ReturnProcess(score, level);
          } else if (tt_entry.score_type() == ScoreType::ALPHA) {
            // エントリーがアルファ値。
            if (score <= alpha) {
              // アルファ値以下が確定。
              pv_line_table_[level].score(score);
              table_ptr_->Unlock();  // ロック解除。
              return ReturnProcess(score, level);
            }

            // ベータ値を下げられる。
            if (score < beta) beta = score + 1;
          } else {
            // エントリーがベータ値。
            Move best_move = tt_entry.best_move();
            if (!(tt_entry.best_move() & MASK[CAPTURED_PIECE])
            && (level < MAX_PLYS)) {
              // キラームーブをセット。
              if (cache.enable_killer_) {
                shared_st_ptr_->killer_stack_[level][0] = best_move;
                shared_st_ptr_->killer_stack_[level + 2][1] = best_move;
              }
            }

            if (score >= beta) {
              // ベータ値以上が確定。
              pv_line_table_[level].score(score);
              table_ptr_->Unlock();  // ロック解除。
              return ReturnProcess(score, level);
            }

            // アルファ値を上げられる。
            if (score > alpha) alpha = score - 1;
          }
        }
      }
      table_ptr_->Unlock();  // ロック解除。
    }

    // 深さが0ならクイース。 (無効なら評価値を返す。)
    // 限界探索数を超えていてもクイース。
    if ((depth <= 0) || (level >= MAX_PLYS)) {
      if (cache.enable_quiesce_search_) {
        // クイース探索ノードに移行するため、ノード数を減らしておく。
        --(shared_st_ptr_->searched_nodes_);
        return ReturnProcess(Quiesce(level, alpha, beta, material), level);
      } else {
        return ReturnProcess(evaluator_.Evaluate(material), level);
      }
    }

    // --- Internal Iterative Deepening --- //
    if (cache.enable_iid_ && (node_type == NodeType::PV)) {
      // 前回の繰り返しの最善手があればIIDしない。
      if (prev_best) {
        shared_st_ptr_->iid_stack_[level] = prev_best;
      } else {
        if (!is_checked
        && (depth >= cache.iid_limit_depth_)) {
          // Internal Iterative Deepening。
          Search(NodeType::PV, pos_hash, cache.iid_search_depth_,
          level, alpha, beta, material);

          shared_st_ptr_->iid_stack_[level] =
          pv_line_table_[level][0];
        }
      }
    }

    // --- Null Move Reduction --- //
    int null_reduction = 0;
    if (cache.enable_nmr_ && (node_type == NodeType::NON_PV)) {
      if (!(is_null_searching_ || is_checked)
      && (depth >= cache.nmr_limit_depth_)) {
        // Null Move Reduction。
        Move null_move = 0;

        is_null_searching_ = true;
        MakeNullMove(null_move);

        // Null Move Search。
        int score = -(Search(NodeType::NON_PV, pos_hash,
        depth - cache.nmr_search_reduction_ - 1, level + 1, -(beta),
        -(beta - 1), -material));

        UnmakeNullMove(null_move);
        is_null_searching_ = false;

        if (score >= beta) {
          null_reduction = cache.nmr_reduction_;
          depth = depth - null_reduction;
          if (depth <= 0) {
            if (cache.enable_quiesce_search_) {
              // クイース探索ノードに移行するため、ノード数を減らしておく。
              --(shared_st_ptr_->searched_nodes_);
              return ReturnProcess
              (Quiesce(level, alpha, beta, material), level);
            } else {
              return ReturnProcess(evaluator_.Evaluate(material), level);
            }
          }
        }
      }
    }

    // 手を作る。
    maker_table_[level].GenMoves<GenMoveType::ALL>(prev_best,
    shared_st_ptr_->iid_stack_[level],
    shared_st_ptr_->killer_stack_[level][0],
    shared_st_ptr_->killer_stack_[level][1]);

    ScoreType score_type = ScoreType::ALPHA;

    // --- ProbCut --- //
    if (cache.enable_probcut_ && (node_type == NodeType::NON_PV)) {
      if (!(is_null_searching_ || is_checked)
      && (depth >= cache.probcut_limit_depth_)) {
        // 手を作る。
        MoveMaker& maker = maker_table_[level];
        maker.RegenMoves();

        // 浅読みパラメータ。
        int prob_beta = beta + cache.probcut_margin_;
        int prob_depth = depth - cache.probcut_search_reduction_;

        // 探索。
        for (Move move = maker.PickMove(); move; move = maker.PickMove()) {
          if (JudgeToStop(job)) return ReturnProcess(alpha, level);

          // 次のノードへの準備。
          Hash next_hash = GetNextHash(pos_hash, move);
          int next_material = GetNextMaterial(material, move);

          MakeMove(move);

          // 合法手じゃなければ次の手へ。
          if (IsAttacked(basic_st_.king_[side], enemy_side)) {
            UnmakeMove(move);
            continue;
          }

          int score = -Search(NodeType::NON_PV, next_hash,
          prob_depth - 1, level + 1, -prob_beta, -(prob_beta - 1),
          next_material);

          UnmakeMove(move);

          if (JudgeToStop(job)) return ReturnProcess(alpha, level);

          // ベータカット。
          if (score >= prob_beta) {
            // PVライン。
            pv_line_table_[level].SetMove(move);
            pv_line_table_[level].Insert(pv_line_table_[level + 1]);

            // 取らない手。
            if (!(move & MASK[CAPTURED_PIECE])) {
              // 手の情報を得る。
              Square from = Get<FROM>(move);
              Square to = Get<TO>(move);

              // キラームーブ。
              if (cache.enable_killer_) {
                shared_st_ptr_->killer_stack_[level][0] = move;
                shared_st_ptr_->killer_stack_[level + 2][1] = move;
              }

              // ヒストリー。
              if (cache.enable_history_) {
                shared_st_ptr_->history_[side][from][to] +=
                Util::DepthToHistory(depth);

                Util::UpdateMax
                (shared_st_ptr_->history_[side][from][to],
                shared_st_ptr_->history_max_);
              }
            }

            // トランスポジションテーブルに登録。
            if (!null_reduction) {
              table_ptr_->Add(pos_hash, depth, beta, ScoreType::BETA, move);
            }

            return ReturnProcess(beta, level);
          }
        }
      }
    }

    // --- Check Extension --- //
    if (is_checked && cache.enable_check_extension_) {
      depth += 1;
    }

    // 準備。
    int num_all_moves = maker_table_[level].RegenMoves();
    int move_number = 0;
    int margin = cache.futility_pruning_margin_[depth];

    // 仕事の生成。
    job.Lock();
    job.Init(maker_table_[level]);
    job.node_type_ = node_type;
    job.pos_hash_ = pos_hash;
    job.depth_ = depth;
    job.alpha_ = alpha;
    job.beta_ = beta;
    job.pv_line_ptr_ = &pv_line_table_[level];
    job.is_null_searching_ = is_null_searching_;
    job.null_reduction_ = null_reduction;
    job.score_type_ = score_type;
    job.material_ = material;
    job.is_checked_ = is_checked;
    job.num_all_moves_ = num_all_moves;
    job.has_legal_move_ = false;
    job.Unlock();

    // パラメータ準備。
    int history_pruning_move_number =
    cache.history_pruning_invalid_moves_[num_all_moves];

    u64 history_pruning_threshold =
    (shared_st_ptr_->history_max_ * cache.history_pruning_threshold_) >> 8;

    // Late Move Reduction。
    int lmr_move_number = cache.lmr_invalid_moves_[num_all_moves];

    for (Move move = job.PickMove(); move; move = job.PickMove()) {
      // 探索終了ならループを抜ける。
      if (JudgeToStop(job)) break;

      // 別スレッドに助けを求める。(YBWC)
      if ((job.depth_ >= cache.ybwc_limit_depth_)
      && (move_number > cache.ybwc_invalid_moves_)) {
        shared_st_ptr_->helper_queue_ptr_->Help(job);
      }

      // 次のハッシュ。
      Hash next_hash = GetNextHash(job.pos_hash_, move);

      // 次の自分のマテリアル。
      int next_material = GetNextMaterial(job.material_, move);

      MakeMove(move);

      // 合法手じゃなければ次の手へ。
      if (IsAttacked(basic_st_.king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // 合法手があったのでフラグを立てる。
      job.has_legal_move_ = true;

      move_number = job.Count();

      // -- Futility Pruning --- //
      if ((-next_material + margin) <= job.alpha_) {
        UnmakeMove(move);
        continue;
      }

      // 手の情報を得る。
      Square from = Get<FROM>(move);
      Square to = Get<TO>(move);

      // 探索。
      int temp_alpha = job.alpha_;
      int temp_beta = job.beta_;
      int score = temp_alpha;

      // --- Late Move Reduction --- //
      if (cache.enable_lmr_) {
        if (!(is_checked || null_reduction)
        && (depth >= cache.lmr_limit_depth_)
        && (move_number > lmr_move_number)
        && !((move & (MASK[CAPTURED_PIECE] | MASK[PROMOTION]))
        || EqualMove(move, shared_st_ptr_->killer_stack_[level][0])
        || EqualMove(move, shared_st_ptr_->killer_stack_[level][1]))) {
          score = -Search(NodeType::NON_PV, next_hash,
          depth - cache.lmr_search_reduction_ - 1, level + 1,
          -(temp_alpha + 1), -temp_alpha, next_material);
        } else {
          ++score;
        }
      } else {
        ++score;
      }

      // Late Move Reduction失敗。
      // PVノードの探索とNON_PVノードの探索に分ける。
      if (score > temp_alpha) {
        if (node_type == NodeType::PV) {
          // PV。
          // --- PVSearch --- //
          if (move_number <= 1) {
            // フルウィンドウで探索。
            score = -Search(node_type, next_hash, depth - 1, level + 1,
            -temp_beta, -temp_alpha, next_material);
          } else {
            // PV発見後のPVノード。
            // ゼロウィンドウ探索。
            score = -Search(NodeType::NON_PV, next_hash, depth - 1, level + 1,
            -(temp_alpha + 1), -temp_alpha, next_material);

            if ((score > temp_alpha) && (score < temp_beta)) {
              // Fail Lowならず。
              // Fail-Softなので、Beta値以上も探索しない。
              // フルウィンドウで再探索。
              score = -Search(NodeType::PV, next_hash, depth - 1, level + 1,
              -temp_beta, -temp_alpha, next_material);
            }
          }
        } else {
          // NON_PV。
          // --- History Pruning --- //
          int new_depth = depth;
          if (cache.enable_history_pruning_) {
            if (!(is_checked || null_reduction)
            && (depth >= cache.history_pruning_limit_depth_)
            && (move_number > history_pruning_move_number)
            && (shared_st_ptr_->history_[side][from][to]
            < history_pruning_threshold)
            && !((move & (MASK[CAPTURED_PIECE] | MASK[PROMOTION]))
            || EqualMove(move, shared_st_ptr_->killer_stack_[level][0])
            || EqualMove(move, shared_st_ptr_->killer_stack_[level][1]))) {
              new_depth -= cache.history_pruning_reduction_;
            }
          }

          // 探索。 NON_PVノードなので、ゼロウィンドウになる。
          score = -Search(node_type, next_hash, new_depth - 1, level + 1,
          -temp_beta, -temp_alpha, next_material);
        }
      }

      UnmakeMove(move);

      // 探索終了ならループを抜ける。
      if (JudgeToStop(job)) break;

      job.Lock();  // ロック。

      // アルファ値を更新。
      if (score > job.alpha_) {
        // 評価値のタイプをセット。
        job.score_type_ = ScoreType::EXACT;

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

        // ベータカット。
        job.NotifyBetaCut(*this);
        job.Unlock();  // ロック解除。
        break;
      }

      job.Unlock();  // ロック解除。
    }

    // スレッドを合流。
    if (job.depth_ >= cache.ybwc_limit_depth_) {
      job.WaitForHelpers();
    }

    // このノードでゲーム終了だった場合。
    if (!(job.has_legal_move_)) {
      if (is_checked) {
        // チェックメイト。
        pv_line_table_[level].score(SCORE_LOSE);
        pv_line_table_[level].mate_in(level);
        return ReturnProcess(SCORE_LOSE, level);
      } else {
        // ステールメイト。
        pv_line_table_[level].score(SCORE_DRAW);
        return ReturnProcess(SCORE_DRAW, level);
      }
    }

    // --- 後処理 --- //
    {
      Move best_move = pv_line_table_[level][0];
      pv_line_table_[level].score(job.alpha_);

      // キラームーブ、ヒストリーを記録。
      if (job.score_type_ != ScoreType::ALPHA) {
        if (!(best_move & (MASK[CAPTURED_PIECE] | MASK[PROMOTION]))) {
          // キラームーブ。
          if (cache.enable_killer_) {
            shared_st_ptr_->killer_stack_[level][0] = best_move;
            shared_st_ptr_->killer_stack_[level + 2][1] = best_move;
          }

          // ヒストリー。
          if (cache.enable_history_) {
            Square from = Get<FROM>(best_move);
            Square to = Get<TO>(best_move);

            shared_st_ptr_->history_[side][from][to] += 
            Util::DepthToHistory(job.depth_);

            Util::UpdateMax(shared_st_ptr_->history_max_,
            shared_st_ptr_->history_[side][from][to]);
          }
        }
      }

      // トランスポジションテーブルに登録。
      // Null Move探索中の局面は登録しない。
      // Null Move Reductionされていた場合、容量節約のため登録しない。
      if (cache.enable_ttable_) {
        if (!(is_null_searching_ || null_reduction)) {
          table_ptr_->Add(pos_hash, depth, job.alpha_, job.score_type_,
          best_move);
        }
      }
    }

    return ReturnProcess(job.alpha_, level);
  }

  // 探索のルート。
  PVLine ChessEngine::SearchRoot(const std::vector<Move>& moves_to_search,
  UCIShell& shell) {
    constexpr int level = 0;

    // --- 初期化 --- //
    // ジョブ。
    Job& job = job_table_[level];
    job.Init(maker_table_[level]);

    // カット通知。
    notice_cut_level_ = MAX_PLYS + 1;

    // キャッシュ。
    table_ptr_ = shared_st_ptr_->table_ptr_;
    shared_st_ptr_->CacheParams();
    Cache& cache = shared_st_ptr_->cache_;

    shared_st_ptr_->searched_nodes_ = 0;
    shared_st_ptr_->searched_level_ = 0;
    shared_st_ptr_->is_time_over_ = false;
    FOR_SIDES(side) {
      FOR_SQUARES(from) {
        FOR_SQUARES(to) {
          shared_st_ptr_->history_[side][from][to] = 0;
        }
      }
    }
    for (u32 i = 0; i < (MAX_PLYS + 1); ++i) {
      basic_st_.position_memo_[i] = 0;
      shared_st_ptr_->iid_stack_[i] = 0;
      shared_st_ptr_->killer_stack_[i][0] = 0;
      shared_st_ptr_->killer_stack_[i][1] = 0;
      shared_st_ptr_->killer_stack_[i + 2][0] = 0;
      shared_st_ptr_->killer_stack_[i + 2][1] = 0;
      maker_table_[i].ResetStack();
      pv_line_table_[i].ResetLine();
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

      ChessEngine* child_ptr = child_vec.back().get();
      child_ptr->shared_st_ptr_ = shared_st_ptr_;

      thread_vec_[i] = std::thread([child_ptr, &shell]() {
          child_ptr->ThreadYBWC(shell);
      });
    }

    // 定期処理開始。
    std::thread time_thread([this, &shell]() {
      this->shared_st_ptr_->ThreadPeriodicProcess(shell);
    });

    // --- Iterative Deepening --- //
    Move prev_best = 0;
    Hash pos_hash = basic_st_.position_memo_[level] = GetCurrentHash();
    int material = GetMaterial(basic_st_.to_move_);
    int alpha = -MAX_VALUE;
    int beta = MAX_VALUE;
    Side side = basic_st_.to_move_;
    Side enemy_side = Util::GetOppositeSide(side);
    bool is_checked = IsAttacked(basic_st_.king_[side], enemy_side);
    bool found_mate = false;

    for (shared_st_ptr_->i_depth_ = 1; shared_st_ptr_->i_depth_ <= MAX_PLYS;
    ++(shared_st_ptr_->i_depth_)) {
      // 探索終了。
      if (JudgeToStop(job)) {
        --(shared_st_ptr_->i_depth_);
        break;
      }

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
        shared_st_ptr_->searched_nodes_, table_ptr_->GetUsedPermill(),
        pv_line_table_[level]);

        continue;
      }

      // メイトをすでに見つけていたら探索しない。
      if (found_mate) {
        Chrono::milliseconds time =
        Chrono::duration_cast<Chrono::milliseconds>
        (SysClock::now() - shared_st_ptr_->start_time_);

        shell.PrintPVInfo(depth, 0, pv_line_table_[level].score(), time,
        shared_st_ptr_->searched_nodes_, table_ptr_->GetUsedPermill(),
        pv_line_table_[level]);

        continue;
      }

      // --- Aspiration Windows --- //
      int delta = cache.aspiration_windows_delta_;
      // 探索窓の設定。
      if (cache.enable_aspiration_windows_
      && (depth >= cache.aspiration_windows_limit_depth_)) {
        beta = alpha + delta;
        alpha -= delta;
      } else {
        alpha = -MAX_VALUE;
        beta = MAX_VALUE;
      }

      // 標準出力に深さ情報を送る。
      shell.PrintDepthInfo(depth);

      // --- Check Extension --- //
      if (is_checked && cache.enable_check_extension_) {
        depth += 1;
      }

      // 仕事を作る。
      int num_all_moves =
      maker_table_[level].GenMoves<GenMoveType::ALL>(prev_best,
      shared_st_ptr_->iid_stack_[level],
      shared_st_ptr_->killer_stack_[level][0],
      shared_st_ptr_->killer_stack_[level][1]);
      job.Lock();
      job.Init(maker_table_[level]);
      job.node_type_ = NodeType::PV;
      job.pos_hash_ = pos_hash;
      job.depth_ = depth;
      job.alpha_ = alpha;
      job.beta_ = beta;
      job.delta_ = delta;
      job.pv_line_ptr_ = &pv_line_table_[level];
      job.is_null_searching_ = is_null_searching_;
      job.null_reduction_ = 0;
      job.score_type_ = ScoreType::EXACT;
      job.material_ = material;
      job.is_checked_ = is_checked;
      job.num_all_moves_ = num_all_moves;
      job.has_legal_move_ = false;
      job.moves_to_search_ptr_ = &moves_to_search;
      job.Unlock();

      // ヘルプして待つ。
      shared_st_ptr_->helper_queue_ptr_->HelpRoot(job);
      job.WaitForHelpers();

      // 最善手を記録する。
      prev_best = pv_line_table_[level][0];

      // アルファ、ベータ、デルタを更新。
      alpha = job.alpha_;
      beta = job.beta_;
      delta = job.delta_;

      // 最善手が取らない手の場合、ヒストリー、キラームーブをセット。
      if (!(prev_best & MASK[CAPTURED_PIECE])) {
        // キラームーブ。
        if (cache.enable_killer_) {
          shared_st_ptr_->killer_stack_[level][0] = prev_best;
          shared_st_ptr_->killer_stack_[level + 2][1] = prev_best;
        }

        // ヒストリー。
        if (cache.enable_history_) {
          // 手の情報を得る。
          Square from = Get<FROM>(prev_best);
          Square to = Get<TO>(prev_best);

          shared_st_ptr_->history_[side][from][to] +=
          Util::DepthToHistory(job.depth_);

          Util::UpdateMax(shared_st_ptr_->history_[side][from][to],
          shared_st_ptr_->history_max_);
        }
      }

      // 最善手をトランスポジションテーブルに登録。
      if (cache.enable_ttable_) {
        table_ptr_->Add(job.pos_hash_, job.depth_, job.alpha_,
        ScoreType::EXACT, prev_best);
      }

      // メイトを見つけたらフラグを立てる。
      // 直接ループを抜けない理由は、depth等の終了条件対策。
      if (pv_line_table_[level].mate_in() >= 0) {
        found_mate = true;
      }
    }

    // スレッドをジョイン。
    shared_st_ptr_->helper_queue_ptr_->ReleaseHelpers();
    for (auto& thread : thread_vec_) {
      try {
        thread.join();
      } catch (std::system_error err) {
        // 無視。
      }
    }

    // 探索終了したけど、まだ思考を止めてはいけない場合、関数を終了しない。
    while (!JudgeToStop(job)) {
      std::this_thread::sleep_for(Chrono::milliseconds(1));
    }

    // 定期処理スレッドを待つ。
    try {
      time_thread.join();
    } catch (std::system_error err) {
        // 無視。
    }

    // 最後に情報を送る。
    shell.PrintFinalInfo(shared_st_ptr_->i_depth_,
    Chrono::duration_cast<Chrono::milliseconds>
    (SysClock::now() - (shared_st_ptr_->start_time_)),
    shared_st_ptr_->searched_nodes_, table_ptr_->GetUsedPermill(),
    pv_line_table_[level].score(), pv_line_table_[level]);

    return pv_line_table_[level];
  }

  // YBWC探索用スレッド。
  void ChessEngine::ThreadYBWC(UCIShell& shell) {
    // 準備。
    notice_cut_level_ = MAX_PLYS + 1;
    table_ptr_ = shared_st_ptr_->table_ptr_;
    is_null_searching_ = false;

    // 仕事ループ。
    while (true) {
      // 準備。
      Job* job_ptr = nullptr;
      notice_cut_level_ = MAX_PLYS + 1;

      // 仕事を拾う。
      job_ptr = shared_st_ptr_->helper_queue_ptr_->GetJob(this);

      if (job_ptr) {
        if (job_ptr->level_ <= 0) {
          // ルートノード。
          SearchRootParallel(*job_ptr, shell);
        } else {
          // ルートではないノード。
          SearchParallel(job_ptr->node_type_, *job_ptr);
        }

        job_ptr->ReleaseHelper(*this);
      } else {
        break;
      }
    }
  }

  // 並列探索。
  void ChessEngine::SearchParallel(NodeType node_type, Job& job) {
    // キャッシュ。
    Cache& cache = shared_st_ptr_->cache_;

    // 仕事ループ。
    Side side = basic_st_.to_move_;
    Side enemy_side = Util::GetOppositeSide(side);
    int move_number = 0;
    int margin = cache.futility_pruning_margin_[job.depth_];

    // パラメータ準備。
    int history_pruning_move_number =
    cache.history_pruning_invalid_moves_[job.num_all_moves_];

    u64 history_pruning_threshold =
    (shared_st_ptr_->history_max_ * cache.history_pruning_threshold_) >> 8;

    // Late Move Reduction。
    int lmr_move_number = cache.lmr_invalid_moves_[job.num_all_moves_];

    for (Move move = job.PickMove(); move; move = job.PickMove()) {
      // 探索終了ならループを抜ける。
      if (JudgeToStop(job)) break;

      // 次のハッシュ。
      Hash next_hash = GetNextHash(job.pos_hash_, move);

      // 次の局面のマテリアルを得る。
      int next_material = GetNextMaterial(job.material_, move);

      MakeMove(move);

      // 合法手じゃなければ次の手へ。
      if (IsAttacked(basic_st_.king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      move_number = job.Count();

      // 合法手が見つかったのでフラグを立てる。
      job.has_legal_move_ = true;

      // --- Futility Pruning --- //
      if ((-next_material + margin) <= job.alpha_) {
        UnmakeMove(move);
        continue;
      }

      // 手の情報を得る。
      Square from = Get<FROM>(move);
      Square to = Get<TO>(move);

      // 探索。
      int temp_alpha = job.alpha_;
      int temp_beta = job.beta_;
      int score = temp_alpha;

      // --- Late Move Reduction --- //
      if (cache.enable_lmr_) {
        if (!(job.is_checked_ || job.null_reduction_)
        && (job.depth_ >= cache.lmr_limit_depth_)
        && (move_number > lmr_move_number)
        && !((move & (MASK[CAPTURED_PIECE] | MASK[PROMOTION]))
        || EqualMove(move, shared_st_ptr_->killer_stack_[job.level_][0])
        || EqualMove(move, shared_st_ptr_->killer_stack_[job.level_][1]))) {
          score = -Search(NodeType::NON_PV, next_hash,
          job.depth_ - cache.lmr_search_reduction_ - 1, job.level_ + 1,
          -(temp_alpha + 1), -temp_alpha, next_material);
        } else {
          ++score;
        }
      } else {
        ++score;
      }

      // Late Move Reduction失敗時。
      // PVノードの探索とNON_PVノードの探索に分ける。
      if (score > temp_alpha) {
        if (node_type == NodeType::PV) {
          // PV。
          // --- PVSearch --- //
          if (move_number <= 1) {
            // フルウィンドウで探索。
            score = -Search(node_type, next_hash, job.depth_ - 1,
            job.level_ + 1, -temp_beta, -temp_alpha, next_material);
          } else {
            // PV発見後のPVノード。
            // ゼロウィンドウ探索。
            score = -Search(NodeType::NON_PV, next_hash, job.depth_ - 1,
            job.level_ + 1, -(temp_alpha + 1), -temp_alpha, next_material);

            if ((score > temp_alpha) && (score < temp_beta)) {
              // Fail Lowならず。
              // Fail-Softなので、Beta値以上も探索しない。
              // フルウィンドウで再探索。
              score = -Search(NodeType::PV, next_hash, job.depth_ - 1,
              job.level_ + 1, -temp_beta, -temp_alpha, next_material);
            }
          }
        } else {
          // NON_PV。
          // --- History Pruning --- //
          int new_depth = job.depth_;
          if (cache.enable_history_pruning_) {
            if (!(job.is_checked_ || job.null_reduction_)
            && (job.depth_ >= cache.history_pruning_limit_depth_)
            && (move_number > history_pruning_move_number)
            && (shared_st_ptr_->history_[side][from][to]
            < history_pruning_threshold)
            && !((move & (MASK[CAPTURED_PIECE] | MASK[PROMOTION]))
            || EqualMove(move, shared_st_ptr_->killer_stack_[job.level_][0])
            || EqualMove
            (move, shared_st_ptr_->killer_stack_[job.level_][1]))) {
              new_depth -= cache.history_pruning_reduction_;
            }
          }

          // 探索。 NON_PVノードなので、ゼロウィンドウになる。
          score = -Search(node_type, next_hash, new_depth - 1, job.level_ + 1,
          -temp_beta, -temp_alpha, next_material);
        }
      }

      UnmakeMove(move);

      // 探索終了ならループを抜ける。
      if (JudgeToStop(job)) break;

      job.Lock();  // ロック。

      // アルファ値を更新。
      if (score > job.alpha_) {
        // 評価値のタイプをセット。
        job.score_type_ = ScoreType::EXACT;

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

        // ベータカット。
        job.NotifyBetaCut(*this);
        job.Unlock();  // ロック解除。
        break;
      }

      job.Unlock();  // ロック解除。
    }
  }

  // ルートノードで呼び出される、別スレッド用探索関数。
  void ChessEngine::SearchRootParallel(Job& job, UCIShell& shell) {
    // キャッシュ。
    Cache& cache = shared_st_ptr_->cache_;

    // 仕事ループ。
    Side side = basic_st_.to_move_;
    Side enemy_side = Util::GetOppositeSide(side);
    int move_number = 0;

    // パラメータを準備。
    int lmr_move_number = cache.lmr_invalid_moves_[job.num_all_moves_];

    for (Move move = job.PickMove(); move; move = job.PickMove()) {
      if (JudgeToStop(job)) break;

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

      // 次のハッシュ。
      Hash next_hash = GetNextHash(job.pos_hash_, move);

      // 次の局面のマテリアル。
      int next_material = GetNextMaterial(job.material_, move);

      MakeMove(move);

      // 合法手じゃなければ次の手へ。
      if (IsAttacked(basic_st_.king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      move_number = job.Count();

      // 現在探索している手の情報を表示。
      job.Lock();  // ロック。
      shell.PrintCurrentMoveInfo(move, move_number);
      job.Unlock();  // ロック解除。

      // --- PVSearch --- //
      int temp_alpha = job.alpha_;
      int temp_beta = job.beta_;
      int score = temp_alpha;
      if (move_number <= 1) {
        while (true) {
          // 探索終了。
          if (JudgeToStop(job)) break;

          // フルでPVを探索。
          score = -Search(NodeType::PV, next_hash,
          job.depth_ - 1, job.level_ + 1, -temp_beta, -temp_alpha,
          next_material);
          
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
        if (cache.enable_lmr_) {
          if (!(job.is_checked_)
          && (job.depth_ >= cache.lmr_limit_depth_)
          && (move_number > lmr_move_number)
          && !(move & (MASK[CAPTURED_PIECE] | MASK[PROMOTION]))) {
            // ゼロウィンドウ探索。
            score = -Search(NodeType::NON_PV, next_hash,
            job.depth_ - cache.lmr_search_reduction_ - 1, job.level_ + 1,
            -(temp_alpha + 1), -temp_alpha, next_material);
          } else {
            ++score;
          }
        } else {
          ++score;
        }

        // 普通の探索。
        if (score > temp_alpha) {
          // ゼロウィンドウ探索。
          score = -Search(NodeType::NON_PV, next_hash,
          job.depth_ - 1, job.level_ + 1, -(temp_alpha + 1), -temp_alpha,
          next_material);

          if (score > temp_alpha) {
            while (true) {
              // 探索終了。
              if (JudgeToStop(job)) break;

              // フルウィンドウで再探索。
              score = -Search(NodeType::PV, next_hash,
              job.depth_ - 1, job.level_ + 1, -temp_beta, -temp_alpha,
              next_material);

              // ベータ値を調べる。
              job.Lock();  // ロック。
              // if (score >= temp_beta) {
              if (score >= job.beta_) {
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
      if (JudgeToStop(job)) break;

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
        time, shared_st_ptr_->searched_nodes_, table_ptr_->GetUsedPermill(),
        *(job.pv_line_ptr_));

        job.alpha_ = score;
      }
      job.Unlock();  // ロック解除。
    }
  }

  // SEEで候補手を評価する。
  u32 ChessEngine::SEE(Move move) const {

    Cache& cache = shared_st_ptr_->cache_;

    // SEEが無効かどうか。
    if (!(cache.enable_see_)) return 1;

    return Get<MOVE_TYPE>(move) == EN_PASSANT ? cache.material_[PAWN]
    : cache.material_[basic_st_.piece_board_[Get<TO>(move)]];
  }

  // 探索のストップ条件を設定する。
  void ChessEngine::SetStopper(u32 max_depth, u64 max_nodes,
  const Chrono::milliseconds& thinking_time, bool infinite_thinking) {
    shared_st_ptr_->max_depth_ = Util::GetMin(max_depth, MAX_PLYS);
    shared_st_ptr_->max_nodes_ = Util::GetMin(max_nodes, MAX_NODES);
    shared_st_ptr_->start_time_ = SysClock::now();
    // 時間を10ミリ秒(100分の1秒)余裕を見る。
    shared_st_ptr_->end_time_ =
    shared_st_ptr_->start_time_ + thinking_time - Chrono::milliseconds(10);
    shared_st_ptr_->infinite_thinking_ = infinite_thinking;
  }

  // 無限に思考するかどうかのフラグをセットする。
  void ChessEngine::EnableInfiniteThinking(bool enable) {
    shared_st_ptr_->infinite_thinking_ = enable;
  }

  // 現在のノードの探索を中止すべきかどうか判断する。
  bool ChessEngine::JudgeToStop(Job& job) {
    // キャッシュ。
    Cache& cache = shared_st_ptr_->cache_;

    // 今すぐ終了すべきかどうか。
    if (shared_st_ptr_->stop_now_) {
      return true;
    }

    // ベータカット通知で終了すべきかどうか。
    if (notice_cut_level_ <= job.level_) {
      if ((job.client_ptr_ == this) && (job.level_ != notice_cut_level_)) {
        // 自分のヘルパーに「もう意味ないよ」と通知する。
        job.NotifyBetaCut(*this);
      }
      return true;
    }

    // --- 思考ストップ条件から判断 --- //
    // ストップするべきではない。
    if ((shared_st_ptr_->i_depth_ <= 1)
    || shared_st_ptr_->infinite_thinking_) {
      return false;
    }

    // ストップするべき。
    if ((shared_st_ptr_->is_time_over_)
    || (shared_st_ptr_->searched_nodes_ >= cache.max_nodes_)
    || (shared_st_ptr_->i_depth_ > cache.max_depth_)) {
      shared_st_ptr_->stop_now_ = true;
      return true;
    }

    return false;
  }
}  // namespace Sayuri
