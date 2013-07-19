/*
   chess_engine_test.cpp: チェスボードのテスト用プログラム。

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
#include <string>
#include <cstddef>
#include <memory>
#include "chess_def.h"
#include "chess_util.h"
#include "sayuri_error.h"
#include "fen.h"

namespace Sayuri {
  /******************/
  /* テスト用関数。 */
  /******************/
  extern void PrintBitboard(Bitboard bitboard);
  extern void PrintPosition(Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]);
  extern void PrintMove(Move move);

  void ChessEngine::Test() {
    std::string fen_str = "6qk/8/2p5/3B4/2P5/4N3/8/7K b -";

    Fen fen(fen_str);
    LoadFen(fen);

    HashKey pos_key = GetCurrentKey();
    int depth = 3;
    int level = 3;
    search_stack_[level].current_pos_key_ = pos_key;
    Move best_move;
    best_move.from_ = C6;
    best_move.to_ = D5;
    best_move.move_type_ = NORMAL;

    std::size_t bytes = 50000000;
    std::unique_ptr<TranspositionTable> table(new TranspositionTable(bytes));
    table->Add(pos_key, depth - 1, level, to_move_,
    SCORE_WIN, TTValueFlag::EXACT, best_move);

    MoveMaker maker(this);
    maker.GenMoves<GenMoveType::CAPTURE>(depth, level, *table);
    maker.GenMoves<GenMoveType::NON_CAPTURE>(depth, level, *table);
    std::cout << "Move Size: " << maker.GetSize() << std::endl;

    PrintPosition(position_);

    MoveMaker::MoveSlot* ptr = maker.begin_;
    while (ptr < maker.last_) {
      std::cout << "----------------------------------------" << std::endl;
      std::cout << std::endl;
      PrintMove(ptr->move_);
      std::cout << "Score: " << ptr->score_ << std::endl;
      std::cout << std::endl;
      std::cout << "----------------------------------------" << std::endl;
      ptr++;
    }

    Move first_move = maker.PickMove();
    std::cout << "First Move is " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    PrintMove(first_move);
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
  }
}  // namespace Sayuri
