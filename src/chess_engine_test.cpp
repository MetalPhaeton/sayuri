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
    std::string fen_str = "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq";
    Fen fen(fen_str);
    LoadFen(fen);

    Evaluator eval(this);

    PrintPosition(position_);
    std::cout << "Phase: " << eval.GetPhase() << std::endl;
    std::cout << "Advantage: " << eval.Evaluate() << std::endl;
    PrintEvaluator(eval);
  }

  // 評価値をプリント。改造版。
  void ChessEngine::PrintEvaluator(Evaluator& eval) {
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "material_value: " << eval.material_value_ << std::endl;
    std::cout << "mobility_value: " << eval.mobility_value_ << std::endl;
    std::cout << "center_control_value: " << eval.center_control_value_ << std::endl;
    std::cout << "development_value: " << eval.development_value_ << std::endl;
    std::cout << "attack_around_king_value: " << eval.attack_around_king_value_ << std::endl;
    std::cout << "pawn_position_value: " << eval.pawn_position_value_ << std::endl;
    std::cout << "knight_position_value: " << eval.knight_position_value_ << std::endl;
    std::cout << "rook_position_value: " << eval.rook_position_value_ << std::endl;
    std::cout << "king_position_middle_value: " << eval.king_position_middle_value_ << std::endl;
    std::cout << "king_position_ending_value: " << eval.king_position_ending_value_ << std::endl;
    std::cout << "pass_pawn_value: " << eval.pass_pawn_value_ << std::endl;
    std::cout << "protected_pass_pawn_value: " << eval.protected_pass_pawn_value_ << std::endl;
    std::cout << "double_pawn_value: " << eval.double_pawn_value_ << std::endl;
    std::cout << "iso_pawn_value: " << eval.iso_pawn_value_ << std::endl;
    std::cout << "bishop_pair_value: " << eval.bishop_pair_value_ << std::endl;
    std::cout << "early_queen_launched_value: " << eval.early_queen_launched_value_ << std::endl;
    std::cout << "pawn_shield_value: " << eval.pawn_shield_value_ << std::endl;
    std::cout << "castling_value: " << eval.castling_value_ << std::endl;
    std::cout << "----------------------------------------" << std::endl;
  }
}  // namespace Sayuri
