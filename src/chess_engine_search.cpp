/* 
   chess_engine_search.cpp: 探索の実装ファイル。

   The MIT License (MIT)

   Copyright (c) 2013 Ishibashi Hironori

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

#include "chess_engine.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstddef>
#include "chess_def.h"
#include "chess_util.h"
#include "transposition_table.h"
#include "move_maker.h"
#include "pv_line.h"
#include "evaluator.h"
#include "error.h"
#include "uci_shell.h"

namespace Sayuri {
  // クイース探索。
  int ChessEngine::Quiesce(int depth, int level, int alpha, int beta,
  TranspositionTable& table) {
    // 探索中止の時。
    if (ShouldBeStopped()) return alpha;

    // ノード数を加算。
    searched_nodes_++;

    // 最大探索数。
    if (level > searched_level_) {
      searched_level_ = level;
    }

    // サイド。
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;

    // stand_pad。
    Evaluator eval(this);
    int stand_pad = eval.Evaluate();

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
    MoveMaker maker(this);
    Move prev_best;
    if (IsAttacked(king_[side], enemy_side)) {
      maker.GenMoves<GenMoveType::ALL>(prev_best, iid_stack_[level],
      killer_stack_[level]);
    } else {
      maker.GenMoves<GenMoveType::CAPTURE>(prev_best, iid_stack_[level],
      killer_stack_[level]);
    }

    // マテリアルを得る。
    int material = GetMaterial(side);

    // 探索する。
    for (Move move = maker.PickMove(); move.all_; move = maker.PickMove()) {
      // マージン。
      int margin = GetMargin(move, depth);

      MakeMove(move);

      // 合法手かどうか調べる。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // Futility Pruning。
      if ((material + margin) <= alpha) {
        UnmakeMove(move);
        continue;
      }

      // 次の手を探索。
      int score = -Quiesce(depth - 1, level + 1, -beta, -alpha, table);

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

  // 探索する。
  template<NodeType Type>
  int ChessEngine::Search(HashKey pos_key, int depth, int level,
  int alpha, int beta, TranspositionTable& table, PVLine& pv_line) {
    // 探索中止の時。
    if (ShouldBeStopped()) return alpha;

    // ノード数を加算。
    searched_nodes_++;

    // 最大探索数。
    if (level > searched_level_) {
      searched_level_ = level;
    }

    // サイドとチェックされているか。
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    bool is_checked = IsAttacked(king_[side], enemy_side);

    // トランスポジションテーブルを調べる。
    TTEntry* entry_ptr =
    table.GetFulfiledEntry(pos_key, depth, side);
    if (entry_ptr) {
      int score = entry_ptr->value();
      if (entry_ptr->value_flag() == TTValueFlag::EXACT) {
        // エントリーが正確な値。
        if (score >= beta) return beta;
        if (score <= alpha) return alpha;
        return score;
      } else if (entry_ptr->value_flag() == TTValueFlag::ALPHA) {
        // エントリーがアルファ値。
        // アルファ値以下が確定。
        if (score <= alpha) {
          return alpha;
        }
        // ベータ値を下げられる。
        if (score < beta) beta = score + 1;
      } else {
        // エントリーがベータ値。
        // ベータ値以上が確定。
        if (score >= beta) {
          return beta;
        }
        // アルファ値を上げられる。
        if (score > alpha) alpha = score - 1;
      }
    }

    // 深さが0ならクイース。
    // 限界探索数を超えていてもクイース。
    if ((depth <= 0) || (level >= MAX_PLYS)) {
      // クイース探索ノードに移行するため、ノード数を減らしておく。
      searched_nodes_--;
      return Quiesce(depth, level, alpha, beta, table);
    }

    // 前回の繰り返しの最善手を得る。
    TTEntry* prev_entry = table.GetFulfiledEntry(pos_key, depth - 1, side);
    Move prev_best;
    if (prev_entry && (prev_entry->value_flag() != TTValueFlag::ALPHA)) {
      prev_best = prev_entry->best_move();
    }

    // PVノードの時はIID、そうじゃないノードならNull Move Reduction。
    if (Type == NodeType::PV) {
      // 前回の繰り返しの最善手があればIIDしない。
      if (prev_best.all_) {
        iid_stack_[level] = prev_best;
      } else {
        if (depth >= 5) {
          // Internal Iterative Deepening。
          PVLine temp_line;
          Search<NodeType::PV> (pos_key, depth - 3, level, alpha, beta,
          table, temp_line);

          iid_stack_[level] = temp_line.line()[0].move();
        }
      }
    } else {
      if (!is_null_searching_ && !is_checked && (depth >= 4)) {
        // Null Move Reduction。
        Move null_move;
        null_move.move_type_ = NULL_MOVE;  // Null Move。

        is_null_searching_ = true;
        MakeMove(null_move);

        // Null Move Search。
        int reduction = (depth / 2) + 1;
        PVLine temp_line;
        int score = -Search<NodeType::NON_PV>(pos_key, depth - reduction - 1,
        level + 1, -(beta), -(beta - 1), table, temp_line);

        UnmakeMove(null_move);
        is_null_searching_ = false;

        if (score >= beta) {
          depth -= 4;
          if (depth <= 0) {
            // クイース探索ノードに移行するため、ノード数を減らしておく。
            searched_nodes_--;
            return Quiesce(depth, level, alpha, beta, table);
          }
        }
      }
    }

    /**************/
    /* PVSearch。 */
    /**************/
    // 手を作る。
    MoveMaker maker(this);
    maker.GenMoves<GenMoveType::ALL>(prev_best, iid_stack_[level],
    killer_stack_[level]);

    // Futility Pruningの準備。
    bool do_futility_pruning = false;
    int material;
    if (depth <= 2) {
      material = GetMaterial(side);
      do_futility_pruning = true;
    }

    // ループ。
    TTValueFlag value_flag = TTValueFlag::ALPHA;
    bool is_searching_pv = true;
    int num_searched_moves = 0;
    bool has_legal_move = false;
    for (Move move = maker.PickMove(); move.all_; move = maker.PickMove()) {
      // 次のハッシュキー。
      HashKey next_key = GetNextKey(pos_key, move);

      // マージン。
      int margin;
      if (do_futility_pruning) {
        margin = GetMargin(move, depth);
      }

      MakeMove(move);

      // 合法手じゃなければ次の手へ。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // 合法手があったのでフラグを立てる。
      has_legal_move = true;

      // Futility Pruning。
      if (do_futility_pruning) {
        if ((material + margin) <= alpha) {
          UnmakeMove(move);
          continue;
        }
      }

      // 探索。
      // Late Move Reduction。
      int score;
      PVLine next_line;
      if ((depth >= 3) && (num_searched_moves >= 4)) {
        int reduction = depth / 3;
        if (!is_checked && (Type == NodeType::NON_PV)) {
          // History Puruning。
          if (!move.captured_piece_
          && (history_[side][move.from_][move.to_] < (history_max_ / 2))) {
            reduction++;
          }
        }
        score = -Search<NodeType::NON_PV>(next_key, depth - reduction - 1,
        level + 1, -(alpha + 1), -alpha, table, next_line);
      } else {
        // PVSearchをするためにalphaより大きくしておく。
        score = alpha + 1;
      }
      if (score > alpha) {
        // PVSearch。
        if (is_searching_pv || (Type == NodeType::NON_PV)) {
          // フルウィンドウで探索。
          score = -Search<Type>(next_key, depth - 1, level + 1,
          -beta, -alpha, table, next_line);
        } else {
          // PV発見後のPVノード。
          // ゼロウィンドウ探索。
          score = -Search<NodeType::NON_PV>(next_key, depth - 1, level + 1,
          -(alpha + 1), -alpha, table, next_line);
          if (score > alpha) {
            // fail lowならず。
            // フルウィンドウで再探索。
            score = -Search<NodeType::PV>(next_key, depth - 1, level + 1,
            -beta, -alpha, table, next_line);
          }
        }
      }


      UnmakeMove(move);
      num_searched_moves++;

      // ベータ値を調べる。
      // アルファ値を更新。
      if (score > alpha) {
        // PVを見つけた。
        is_searching_pv = false;

        // トランスポジション用。
        value_flag = TTValueFlag::EXACT;

        // PVライン。
        pv_line.SetMove(move);
        pv_line.Insert(next_line);
        pv_line.score(score);

        alpha = score;
      }

      if (score >= beta) {
        // トランスポジション用。
        value_flag = TTValueFlag::BETA;

        // キラームーブ。
        killer_stack_[level] = move;

        // ヒストリー。
        if (!(move.captured_piece_)) {
          history_[side][move.from_][move.to_] += depth * depth;
          if (history_[side][move.from_][move.to_] > history_max_) {
            history_max_ = history_[side][move.from_][move.to_];
          }
        }

        // ベータカット。
        alpha = beta;
        break;
      }
    }

    // このノードでゲーム終了だった場合。
    if (!has_legal_move) {
      if (is_checked) {
        // チェックメイト。
        pv_line.MarkCheckmated();
        int score = SCORE_LOSE;
        if (score >= beta) return beta;
        if (score <= alpha) return alpha;
        return score;
      } else {
        // ステールメイト。
        int score = SCORE_DRAW;
        if (score >= beta) return beta;
        if (score <= alpha) return alpha;
        return score;
      }
    }

    // トランスポジションテーブルに登録。
    if (!entry_ptr) {
      table.Add(pos_key, depth, side, alpha, value_flag,
      pv_line.line()[0].move());
    } else if (entry_ptr->depth() == depth) {
      entry_ptr->Update(alpha, value_flag, pv_line.line()[0].move());
    }

    // 探索結果を返す。
    return alpha;
  }
  // 実体化。
  template int ChessEngine::Search<NodeType::PV>(HashKey pos_key,
  int depth, int level, int alpha, int beta,
  TranspositionTable& table, PVLine& pv_line);
  template int ChessEngine::Search<NodeType::NON_PV>(HashKey pos_key,
  int depth, int level, int alpha, int beta,
  TranspositionTable& table, PVLine& pv_line);

  // 探索のルート。
  PVLine ChessEngine::SearchRoot(TranspositionTable& table,
  std::vector<Move>* moves_to_search_ptr) {
    // 初期化。
    searched_nodes_ = 0;
    searched_level_ = 0;
    is_null_searching_ = false;
    start_time_ = SysClock::now();
    for (int i = 0; i < MAX_PLYS; i++) {
      iid_stack_[i] = Move();
      killer_stack_[i] = Move();
    }
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_SQUARES; j++) {
        for (int k = 0; k < NUM_SQUARES; k++) {
          history_[i][j][k] = 0;
        }
      }
    }
    history_max_ = 1;
    stopper_.stop_now_ = false;

    // Iterative Deepening。
    HashKey pos_key = GetCurrentKey();
    int level = 0;
    int alpha = -MAX_VALUE;
    int beta = MAX_VALUE;
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    PVLine pv_line;
    TimePoint now = SysClock::now();
    TimePoint next_send_info_time = now + Chrono::milliseconds(1000);
    std::vector<Move> move_vec;
    MoveMaker maker(this);
    for (i_depth_ = 1; i_depth_ <= MAX_PLYS; i_depth_++) {
      // 探索終了。
      if (ShouldBeStopped()) break;

      // 準備。
      int delta = 15;
      // 探索窓の設定。
      if (i_depth_ < 5) {
        alpha = -MAX_VALUE;
        beta = MAX_VALUE;
      } else {
        beta = alpha + delta;
        alpha -= delta;
      }
      bool is_searching_pv = true;
      TTEntry* entry_ptr = nullptr;


      // ノードを加算。
      searched_nodes_++;

      // 標準出力に深さ情報を送る。
      UCIShell::SendDepthInfo(i_depth_);

      // 前回の繰り返しの最善手を得る。
      TTEntry* prev_entry =
      table.GetFulfiledEntry(pos_key, i_depth_ - 1, side);
      Move prev_best;
      if (prev_entry && (prev_entry->value_flag() != TTValueFlag::ALPHA)) {
        prev_best = prev_entry->best_move();
      }

      // 手を作る。
      maker.GenMoves<GenMoveType::ALL>(prev_best, iid_stack_[level],
      killer_stack_[level]);

      // 手を探索。
      int num_searched_moves = 0;
      int move_num = 0;
      for (Move move = maker.PickMove(); move.all_;
      move = maker.PickMove()) {
        // 情報を送る。
        now = SysClock::now();
        if (now > next_send_info_time) {
          UCIShell::SendOtherInfo
          (Chrono::duration_cast<Chrono::milliseconds>(now - start_time_),
          searched_nodes_, table.GetUsedPermill());

          next_send_info_time = now + Chrono::milliseconds(1000);
        }

        // 探索したレベルをリセット。
        searched_level_ = 0;

        // 次のハッシュキー。
        HashKey next_key = GetNextKey(pos_key, move);

        MakeMove(move);

        // 合法手じゃなければ次の手へ。
        if (IsAttacked(king_[side], enemy_side)) {
          UnmakeMove(move);
          continue;
        }

        // 探索すべき手が指定されていれば、今の手がその手かどうか調べる。
        if (moves_to_search_ptr) {
          bool hit = false;
          for (auto& move_2 : *(moves_to_search_ptr)) {
            if (move_2 == move) {
              // 探索すべき手だった。
              hit = true;
              break;
            }
          }
          if (!hit) {
            // 探索すべき手ではなかった。
            // 次の手へ。
            UnmakeMove(move);
            continue;
          }
        }

        // 現在探索している手の情報を送る。
        if (i_depth_ <= 1) {
          // 最初の探索。
          UCIShell::SendCurrentMoveInfo(move, move_num);
          move_vec.push_back(move);
          move_num++;
        } else {
          // 2回目以降の探索。
          move_num = 0;
          for (auto& move_2 : move_vec) {
            if (move_2 == move) {
              UCIShell::SendCurrentMoveInfo(move, move_num);
              break;
            }
            move_num++;
          }
        }

        // PVSearch。
        int score;
        PVLine next_line;
        if (is_searching_pv) {
          while (true) {
            // 探索終了。
            if (ShouldBeStopped()) break;

            // フルでPVを探索。
            score = -Search<NodeType::PV>
            (next_key, i_depth_ - 1, level + 1, -beta, -alpha,
            table, next_line);
            // アルファ値、ベータ値を調べる。
            if (score >= beta) {
              // 探索失敗。
              // ベータ値を広げる。
              delta *= 2;
              beta += delta;
              continue;
            } else if (score <= alpha) {
              // 探索失敗。
              // アルファ値を広げる。
              delta *= 2;
              alpha -= delta;
              continue;
            } else {
              break;
            }
          }
        } else {
          // PV発見後。
          // Late move Reduction。
          if ((i_depth_ >= 3) && (num_searched_moves >= 4)) {
            int reduction = i_depth_ / 3;
            // ゼロウィンドウ探索。
            score = -Search<NodeType::NON_PV>(next_key,
            i_depth_ - reduction - 1, level + 1, -(alpha + 1),
            -alpha, table, next_line);
          } else {
            // 普通に探索するためにscoreをalphaより大きくしておく。
            score = alpha + 1;
          }

          // 普通の探索。
          if (score > alpha) {
            // ゼロウィンドウ探索。
            score = -Search<NodeType::NON_PV>(next_key, i_depth_ - 1,
            level + 1, -(alpha + 1), -alpha, table, next_line);
            if (score > alpha) {
              while (true) {
                // 探索終了。
                if (ShouldBeStopped()) break;

                // フルウィンドウで再探索。
                score = -Search<NodeType::PV>(next_key, i_depth_ - 1,
                level + 1, -beta, -alpha, table, next_line);
                // アルファ値、ベータ値を調べる。
                if (score >= beta) {
                  // 探索失敗。
                  // ベータ値を広げる。
                  delta *= 2;
                  beta += delta;
                  continue;
                } else {
                  break;
                }
              }
            }
          }
        }

        UnmakeMove(move);
        num_searched_moves++;

        // ストップがかかっていたらループを抜ける。
        if (ShouldBeStopped()) break;

        // 最善手を見つけた。
        if (score > alpha) {
          is_searching_pv = false;

          // PVラインにセット。
          pv_line.SetMove(move);
          pv_line.Insert(next_line);
          pv_line.score(score);

          // トランスポジションテーブルに登録。
          if (!entry_ptr) {
            table.Add(pos_key, i_depth_, side, score,
            TTValueFlag::EXACT, move);

            entry_ptr = table.GetFulfiledEntry(pos_key, i_depth_, side);
          } else {
            entry_ptr->Update(score, TTValueFlag::EXACT, move);
          }

          // 標準出力にPV情報を送る。
          TimePoint now = SysClock::now();
          Chrono::milliseconds time =
          Chrono::duration_cast<Chrono::milliseconds>(now - start_time_);
          UCIShell::SendPVInfo(i_depth_, searched_level_, score, time,
          searched_nodes_, pv_line);

          alpha = score;
        }
      }

      // ストップがかかっていたらループを抜ける。
      if (ShouldBeStopped()) break;
    }

    // 探索終了したけど、まだ思考を止めてはいけない場合、関数を終了しない。
    while (!ShouldBeStopped()) continue;

    return pv_line;
  }

  // Futility Pruningのマージンを計算する。
  int ChessEngine::GetMargin(Move move, int depth) {
    // マテリアルの変動。
    int margin = MATERIAL[piece_board_[move.to_]];
    // 昇格の値を加算。
    if (piece_board_[move.from_] == PAWN) {
      if ((Util::GetRank(move.to_) == RANK_8)
      || (Util::GetRank(move.to_) == RANK_1)) {
        margin += MATERIAL[QUEEN] - MATERIAL[PAWN];
      }
    }

    // ポジションの評価の最大値。
    margin += depth <= 1 ? 300 : 400;

    return margin;
  }

  // ストップ条件を設定する。
  void ChessEngine::SetStopper(int max_depth, std::size_t max_nodes,
  Chrono::milliseconds thinking_time, bool infinite_thinking) {
    stopper_.max_depth_ = max_depth <= MAX_PLYS ? max_depth : MAX_PLYS;
    stopper_.max_nodes_ = max_nodes <= MAX_NODES ? max_nodes : MAX_NODES;
    stopper_.thinking_time_ = thinking_time.count();
    stopper_.infinite_thinking_ = infinite_thinking;
  }

  // 思考の無限時間フラグを設定する。
  void ChessEngine::EnableInfiniteThinking(bool enable) {
    stopper_.infinite_thinking_ = enable;
  }

  // 探索中止しなければいけないかどうか。
  bool ChessEngine::ShouldBeStopped() {
    if (stopper_.stop_now_) return true;
    if (stopper_.infinite_thinking_) return false;
    if (i_depth_ > stopper_.max_depth_) {
      stopper_.stop_now_ = true;
      return true;
    }
    if (searched_nodes_ >= stopper_.max_nodes_) {
      stopper_.stop_now_ = true;
      return true;
    }
    TimePoint now = SysClock::now();
    if ((Chrono::duration_cast<Chrono::milliseconds>
    (now - start_time_)).count() >= stopper_.thinking_time_) {
      stopper_.stop_now_ = true;
      return true;
    }
    return false;
  }
}  // namespace Sayuri
