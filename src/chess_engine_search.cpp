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

    // サイドと得点。
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    int score;

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
    if (IsAttacked(king_[side], enemy_side)) {
      maker.GenMoves<GenMoveType::ALL>(pos_key, depth, level, table);
    } else {
      maker.GenMoves<GenMoveType::CAPTURE>(pos_key, depth, level, table);
    }

    // マテリアルを得る。
    int material = GetMaterial(side);

    // 探索する。
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

    // サイドとスコアとチェックされているか。
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    int score;
    bool is_checked = IsAttacked(king_[side], enemy_side);

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
    // 限界探索数を超えていてもクイース。
    if ((depth <= 0) || (level >= MAX_PLYS)) {
      // クイース探索ノードに移行するため、ノード数を減らしておく。
      searched_nodes_--;
      return Quiesce(pos_key, depth, level, alpha, beta, table);
    }

    // PVノードの時はIID、そうじゃないノードならNull Move Reduction。
    PVLine temp_line;
    int reduction;
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
        reduction = (depth / 2) + 1;
        score = -Search<NodeType::NON_PV>(pos_key, depth - reduction - 1,
        level + 1, -(beta), -(beta - 1), table, temp_line);

        UnmakeMove(null_move);
        is_null_searching_ = false;

        if (score >= beta) {
          depth -= 4;
          if (depth <= 0) {
            // クイース探索ノードに移行するため、ノード数を減らしておく。
            searched_nodes_--;
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
    HashKey next_key;
    bool is_searching_pv = true;
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
        reduction = depth / 3;
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
    stopper_.stop_now_ = false;

    // 使う変数。
    HashKey pos_key = GetCurrentKey();
    int level = 0;
    int alpha = -MAX_VALUE;
    int beta = MAX_VALUE;
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    PVLine pv_line;
    PVLine cur_line;
    PVLine next_line;
    TimePoint now = SysClock::now();
    TimePoint next_send_info_time = now + Chrono::milliseconds(1000);
    std::vector<Move> move_vec;
    int move_num;

    // ゲーム終了した場合。
    if (!HasLegalMove(side)) {
      if (IsAttacked(king_[side], enemy_side)) {
        // チェックメイト。
        pv_line.MarkCheckmated();
        pv_line.score(SCORE_LOSE);
        return std::move(pv_line);
      } else {
        // ステールメイト。
        return std::move(pv_line);
      }
    }

    // Iterative Deepening。
    HashKey next_key;
    int score;
    bool is_searching_pv;
    MoveMaker maker(this);
    TTEntry* entry_ptr = nullptr;
    int delta = 15;
    int num_searched_moves;
    int reduction;
    for (i_depth_ = 1; i_depth_ <= MAX_PLYS; i_depth_++) {
      // 探索終了。
      if (ShouldBeStopped()) break;

      // 準備。
      is_searching_pv = true;
      entry_ptr = nullptr;
      delta = 15;
      num_searched_moves = 0;
      move_num = 0;

      if (i_depth_ < 5) {
        alpha = -MAX_VALUE;
        beta = MAX_VALUE;
      } else {
        beta = alpha + delta;
        alpha -= delta;
      }

      // ノードを加算。
      searched_nodes_++;

      // 標準出力に深さ情報を送る。
      UCIShell::SendDepthInfo(i_depth_);

      // 手を作る。
      maker.GenMoves<GenMoveType::ALL>(pos_key, i_depth_, level, table);

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
        next_key = GetNextKey(pos_key, move);

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
            if ((move_2.to_ == move.to_)
            && (move_2.from_ == move.from_)
            && (move_2.promotion_ == move.promotion_)) {
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
            if ((move.from_ == move_2.from_)
            && (move.to_ == move_2.to_)
            && (move.promotion_ == move_2.promotion_)) {
              UCIShell::SendCurrentMoveInfo(move, move_num);
              break;
            }
            move_num++;
          }
        }

        // PVSearch。
        if (is_searching_pv) {
          while (true) {
            // 探索終了。
            if (ShouldBeStopped()) break;

            // フルでPVを探索。
            score = -Search<NodeType::PV>(next_key, i_depth_ - 1, level + 1,
            -beta, -alpha, table, next_line);
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
            reduction = i_depth_ / 3;
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
          cur_line.SetMove(move);
          cur_line.Insert(next_line);
          cur_line.score(score);

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
          searched_nodes_, cur_line);

          alpha = score;
        }
      }

      // ストップがかかっていたらループを抜ける。
      if (ShouldBeStopped()) break;

      // PVラインをセット。
      pv_line = cur_line;
      // チェックメイトならもう探索しない。
      if (pv_line.line()[pv_line.length() - 1].has_checkmated()) {
        stopper_.stop_now_ = true;
        break;
      }
    }

    // 探索終了したけど、まだ思考を止めてはいけない場合、関数を終了しない。
    while (!ShouldBeStopped()) continue;

    return std::move(pv_line);
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
