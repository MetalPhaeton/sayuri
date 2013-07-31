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
#include "error.h"
#include "fen.h"

namespace Sayuri {
  /******************/
  /* テスト用関数。 */
  /******************/
  extern void PrintBitboard(Bitboard bitboard);
  extern void PrintPosition(Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]);
  extern void PrintMove(Move move);

  void ChessEngine::Test() {
    std::string fen_str = "8/p7/3p1k2/P2P1p1p/5K2/1R4PP/1p3P2/r7 b -";
    Fen fen(fen_str);
    LoadFen(fen);

    Evaluator eval(this);
    int value = eval.Evaluate();

    PrintPosition(position_);
    std::cout << "Phase: " << eval.GetPhase() << std::endl;
    std::cout << "Value: " << value << std::endl;
    PrintEvaluator(eval);
  }

  // 各評価値をプリント。
  void ChessEngine::PrintEvaluator(Evaluator& eval) {
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Material: " << GetMaterial(to_move_) << std::endl;
    std::cout << "EvalMobility: " << eval.EvalMobility() << std::endl;
    std::cout << "EvalAttackCenter: " << eval.EvalAttackCenter() << std::endl;
    std::cout << "EvalDevelopment: " << eval.EvalDevelopment() << std::endl;
    std::cout << "EvalAttackAroundKing: " << eval.EvalAttackAroundKing() << std::endl;
    std::cout << "EvalPawnPosition: " << eval.EvalPawnPosition() << std::endl;
    std::cout << "EvalKnightPosition: " << eval.EvalKnightPosition() << std::endl;
    std::cout << "EvalRookPosition: " << eval.EvalRookPosition() << std::endl;
    std::cout << "EvalKingPositionMiddle: " << eval.EvalKingPositionMiddle() << std::endl;
    std::cout << "EvalKingPositionEnding: " << eval.EvalKingPositionEnding() << std::endl;
    std::cout << "EvalPassPawn: " << eval.EvalPassPawn() << std::endl;
    std::cout << "EvalDoublePawn: " << eval.EvalDoublePawn() << std::endl;
    std::cout << "EvalIsoPawn: " << eval.EvalIsoPawn() << std::endl;
    std::cout << "EvalBishopPair: " << eval.EvalBishopPair() << std::endl;
    std::cout << "EvalEarlyQueenLaunched: " << eval.EvalEarlyQueenLaunched() << std::endl;
    std::cout << "EvalPawnShield: " << eval.EvalPawnShield() << std::endl;
    std::cout << "EvalCastling: " << eval.EvalCastling() << std::endl;
    std::cout << "----------------------------------------" << std::endl;
  }
}  // namespace Sayuri
