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
#include <utility>
#include "chess_def.h"
#include "chess_util.h"
#include "transposition_table.h"
#include "move_maker.h"
#include "pv_line.h"
#include "evaluator.h"
#include "error.h"

namespace Sayuri {
  // クイース探索。
  int ChessEngine::Quiesce(HashKey pos_key, int depth, int level,
  int alpha, int beta, TranspositionTable& table) {
    // 探索中止の時。
    if (ShouldBeStopped()) return alpha;

    // ノード数を加算。
    searched_nodes_++;

    // 最大探索数。
    if (level > searched_level_) {
      searched_level_ = level;
    }

    // stand_pad。
    Evaluator eval(this);
    int stand_pad = eval.Evaluate();

    // アルファ値、ベータ値を調べる。
    if (stand_pad >= beta) return beta;
    if (stand_pad > alpha) alpha = stand_pad;

    // 探索できる限界を超えているか。
    // 超えていればこれ以上探索しない。
    if (level >= MAX_PLYS) {
      return alpha;
    }

    // サイド。
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;

    // 候補手を作る。
    // 駒を取る手だけ。
    MoveMaker maker(this);
    if (IsAttacked(king_[side], enemy_side)) {
      maker.GenMoves<GenMoveType::ALL>(pos_key, depth, level, table);
    } else {
      maker.GenMoves<GenMoveType::CAPTURE>(pos_key, depth, level, table);
    }

    // マテリアルを得る。
    int material = GetMaterial(side);

    // 探索する。
    int score;
    int margin;
    for (Move move = maker.PickMove(); move.all_; move = maker.PickMove()) {
      // マージン。
      margin = GetMargin(move, depth);

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
      score = -Quiesce(GetNextKey(pos_key, move), depth - 1, level + 1,
      -beta, -alpha, table);

      UnmakeMove(move);

      // アルファ値、ベータ値を調べる。
      if (score > alpha) alpha = score;
      if (score >= beta) return beta;
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

    // サイドとスコアとチェックされているか。
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    int score;
    bool is_checked = IsAttacked(king_[side], enemy_side);

    // トランスポジションテーブルを調べる。
    TTEntry* entry_ptr =
    table.GetFulfiledEntry(pos_key, depth, side);
    if (entry_ptr) {
      score = entry_ptr->value();
      if (entry_ptr->value_flag() == TTValueFlag::EXACT) {
        // エントリーが正確な値。
        if (score >= beta) return beta;
        if (score <= alpha) return alpha;
        return score;
      } else if (entry_ptr->value_flag() == TTValueFlag::ALPHA) {
        // エントリーがアルファ値。
        // アルファ値以下が確定。
        if (score <= alpha) return alpha;
        // ベータ値を下げられる。
        if (score < beta) beta = score + 1;
      } else {
        // エントリーがベータ値。
        // ベータ値以上が確定。
        if (score >= beta) return beta;
        // アルファ値を上げられる。
        if (score > alpha) alpha = score - 1;
      }
    }

    // 深さが0ならクイース。
    if (depth <= 0) {
      return Quiesce(pos_key, depth, level, alpha, beta, table);
    }

    // ゲーム終了した場合。
    if (!HasLegalMove(side)) {
      if (is_checked) {
        // チェックメイト。
        pv_line.MarkCheckmated();
        score = SCORE_LOSE;
        if (score >= beta) return beta;
        if (score <= alpha) return alpha;
        return score;
      } else {
        // ステールメイト。
        score = SCORE_DRAW;
        if (score >= beta) return beta;
        if (score <= alpha) return alpha;
        return score;
      }
    }

    // PVノードの時はIID、そうじゃないノードならNull Move Reduction。
    PVLine temp_line;
    if (Type == NodeType::PV) {
      // 前回の繰り返しの最善手があればIIDしない。
      TTEntry* temp_entry = table.GetFulfiledEntry(pos_key, depth - 1, side);
      if (temp_entry && (temp_entry->value_flag() == TTValueFlag::EXACT)) {
        iid_stack_[level] = temp_entry->best_move();
      } else {
        if (depth >= 5) {
          // Internal Iterative Deepening。
          score = Search<NodeType::PV>(pos_key, depth - 3, level, alpha, beta,
          table, temp_line);

          iid_stack_[level] = temp_line.line()[0].move();
        }
      }
    } else {
      if (!is_null_searching_ && !is_checked && (depth >= 4)) {
        // Null Move Reduction。
        Move null_move;
        null_move.all_ = 0;
        null_move.move_type_ = NULL_MOVE;  // Null Move。

        is_null_searching_ = true;
        MakeMove(null_move);

        // Null Move Search。
        int red = depth > 6 ? 4 : 3;
        score = -Search<NodeType::NON_PV>(pos_key, depth - red - 1, level + 1,
        -(beta), -(beta - 1), table, temp_line);

        UnmakeMove(null_move);
        is_null_searching_ = false;

        if (score >= beta) {
          depth -= 4;
          if (depth <= 0) {
            return Quiesce(pos_key, depth, level, alpha, beta, table);
          }
        }
      }
    }

    /**************/
    /* PVSearch。 */
    /**************/
    // 手を作る。
    MoveMaker maker(this);
    maker.GenMoves<GenMoveType::ALL>(pos_key, depth, level, table);

    // マテリアルを得る。
    int material = GetMaterial(side);

    // ループ。
    TTValueFlag value_flag = TTValueFlag::ALPHA;
    bool is_searching_pv = true;
    HashKey next_key;
    PVLine next_line;
    int margin;
    int num_searched_moves = 0;
    for (Move move = maker.PickMove(); move.all_; move = maker.PickMove()) {
      // 次のハッシュキー。
      next_key = GetNextKey(pos_key, move);

      // マージン。
      margin = GetMargin(move, depth);

      MakeMove(move);

      // 合法手じゃなければ次の手へ。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // Futility Pruning。
      if (depth <= 2) {
        if ((material + margin) <= alpha) {
          UnmakeMove(move);
          continue;
        }
      }

      // 探索。
      // Late Move Reduction。
      if ((depth >= 3) && (num_searched_moves >= 4)) {
        int red = 1;
        if (!is_checked && (Type == NodeType::NON_PV)) {
          // History Puruning。
          if (!move.captured_piece_
          && (history_[side][move.from_][move.to_] < (history_max_ * 0.5))) {
            red++;
          }
        }
        score = -Search<NodeType::NON_PV>(next_key, depth - red - 1,
        level + 1, -(alpha + 1), -alpha, table, next_line);
      } else {
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

  namespace {
    // 手を文字列にする。
    std::string BeMoveToString(Move move) {
      char str[6];
      str[0] = static_cast<char>(Util::GetFyle(move.from_) + 'a');
      str[1] = static_cast<char>(Util::GetRank(move.from_) + '1');
      str[2] = static_cast<char>(Util::GetFyle(move.to_) + 'a');
      str[3] = static_cast<char>(Util::GetRank(move.to_) + '1');
      switch (move.promotion_) {
        case KNIGHT:
          str[4] = 'n';
          break;
        case BISHOP:
          str[4] = 'b';
          break;
        case ROOK:
          str[4] = 'r';
          break;
        case QUEEN:
          str[4] = 'q';
          break;
        default:
          str[4] = '\0';
          break;
      }
      str[5] = '\0';
      return std::string(str);
    }
  }

  // 探索のルート。
  void ChessEngine::SearchRoot(PVLine& pv_line) {
    // 初期化。
    searched_nodes_ = 0;
    searched_level_ = 0;
    is_null_searching_ = false;
    start_time_ = SysClock::now();
    Move null;
    null.all_ = 0;
    for (int i = 0; i < MAX_PLYS; i++) {
      iid_stack_[i] = null;
      killer_stack_[i] = null;
    }
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_SQUARES; j++) {
        for (int k = 0; k < NUM_SQUARES; k++) {
          history_[i][j][k] = 0;
        }
      }
    }
    history_max_ = 1;

    // 使う変数。
    HashKey pos_key = GetCurrentKey();
    int level = 0;
    int alpha = -MAX_VALUE;
    int beta = MAX_VALUE;
    std::unique_ptr<TranspositionTable>
    table_ptr(new TranspositionTable(table_size_));
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;

    // ゲーム終了した場合。
    if (!HasLegalMove(side)) {
      if (IsAttacked(king_[side], enemy_side)) {
        // チェックメイト。
        pv_line.MarkCheckmated();
        pv_line.score(SCORE_LOSE);
        return;
      } else {
        // ステールメイト。
        return;
      }
    }

    // Iterative Deepening。
    HashKey next_key;
    int score;
    PVLine next_line;
    bool is_searching_pv;
    MoveMaker maker(this);
    TTEntry* entry_ptr = nullptr;
    int delta = 15;
    int num_searched_moves;
    for (int depth = 1; depth <= stopper_.max_depth_; depth++) {
      // 準備。
      is_searching_pv = true;
      entry_ptr = nullptr;
      delta = 15;
      num_searched_moves = 0;

      if (depth < 5) {
        alpha = -MAX_VALUE;
        beta = MAX_VALUE;
      } else {
        beta = alpha + delta;
        alpha -= delta;
      }

      // 手を作る。
      maker.GenMoves<GenMoveType::ALL>(pos_key, depth, level,
      *(table_ptr.get()));

      for (Move move = maker.PickMove(); move.all_;
      move = maker.PickMove()) {
        // 探索したレベルをリセット。
        searched_level_ = 0;

        // 次のハッシュキー。
        next_key = GetNextKey(pos_key, move);

        MakeMove(move);

        // 合法手じゃなければ次の手へ。
        if (IsAttacked(king_[side], enemy_side)) {
          UnmakeMove(move);
          continue;
        }

        // PVSearch。
        if (is_searching_pv) {
          while (true) {
            // 探索終了。
            if (ShouldBeStopped()) break;

            // フルでPVを探索。
            score = -Search<NodeType::PV>(next_key, depth - 1, level + 1,
            -beta, -alpha, *(table_ptr.get()), next_line);
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
          if ((depth >= 3) && (num_searched_moves >= 4)) {
            // ゼロウィンドウ探索。
            score = -Search<NodeType::NON_PV>(next_key, depth - 2, level + 1,
            -(alpha + 1), -alpha, *(table_ptr.get()), next_line);
          } else {
            // 普通に探索するためにscoreをalphaより大きくしておく。
            score = alpha + 1;
          }

          // 普通の探索。
          if (score > alpha) {
            // ゼロウィンドウ探索。
            score = -Search<NodeType::NON_PV>(next_key, depth - 1, level + 1,
            -(alpha + 1), -alpha, *(table_ptr.get()), next_line);
            if (score > alpha) {
              while (true) {
                // 探索終了。
                if (ShouldBeStopped()) break;

                // フルウィンドウで再探索。
                score = -Search<NodeType::PV>(next_key, depth - 1, level + 1,
                -beta, -alpha, *(table_ptr.get()), next_line);
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
            table_ptr->Add(pos_key, depth, side, score,
            TTValueFlag::EXACT, move);

            entry_ptr = table_ptr->GetFulfiledEntry(pos_key, depth, side);
          } else {
            entry_ptr->Update(score, TTValueFlag::EXACT, move);
          }

          alpha = score;
        }
      }

      // ストップがかかっていたらループを抜ける。
      if (ShouldBeStopped()) break;
    }
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
    margin += depth <= 1 ? 200 : 400;

    return margin;
  }

  // ストップ条件を設定する。
  void ChessEngine::SetStopper(std::size_t max_nodes, int max_depth,
  Chrono::milliseconds thinking_time, bool infinite_thinking) {
    stopper_.max_nodes_ = max_nodes <= MAX_NODES ? max_nodes : MAX_NODES;
    stopper_.max_depth_ = max_depth <= MAX_PLYS ? max_depth : MAX_PLYS;
    stopper_.thinking_time_ = thinking_time;
    stopper_.infinite_thinking_ = infinite_thinking;
  }

  // 探索中止しなければいけないかどうか。
  bool ChessEngine::ShouldBeStopped() {
    if (stopper_.stop_now_) return true;
    if (stopper_.infinite_thinking_) return false;
    if (searched_nodes_ >= stopper_.max_nodes_) {
      stopper_.stop_now_ = true;
      return true;
    }
    TimePoint now_time = SysClock::now();
    if ((now_time - start_time_)
    >= stopper_.thinking_time_) {
      stopper_.stop_now_ = true;
      return true;
    }
    return false;
  }
}  // namespace Sayuri
