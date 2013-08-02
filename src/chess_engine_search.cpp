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
    // ノード数を加算。
    num_searched_nodes_++;

    // stand_pad。
    Evaluator eval(this);
    int stand_pad = eval.Evaluate();

    // アルファ値、ベータ値を調べる。
    if (stand_pad >= beta) return beta;
    if (stand_pad > alpha) alpha = stand_pad;

    // 探索できる限界を超えているか。
    // 超えていればこれ以上探索しない。
    if ((level >= MAX_PLY) || (num_searched_nodes_ >= max_nodes_)) {
      return alpha;
    }

    // サイド。
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;

    // 候補手を作る。
    MoveMaker maker(this);
    if (IsAttacked(king_[side], enemy_side)) {
      maker.GenMoves<GenMoveType::ALL>(pos_key, depth, level, table);
    } else {
      maker.GenMoves<GenMoveType::CAPTURE>(pos_key, depth, level, table);
    }

    // 探索する。
    int score;
    for (Move move = maker.PickMove(); move.all_; move = maker.PickMove()) {
      MakeMove(move);

      // 合法手かどうか調べる。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // 次の手を探索。
      score = -Quiesce(GetNextKey(pos_key, move),
      depth - 1, level + 1, -beta, -alpha, table);

      UnmakeMove(move);

      // アルファ値、ベータ値を調べる。
      if (score >= beta) return beta;
      if (score > alpha) alpha = score;
    }

    return alpha;
  }
}  // namespace Sayuri
