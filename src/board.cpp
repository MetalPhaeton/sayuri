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
 * @file board.cpp
 * @author Hironori Ishibashi
 * @brief チェスボードの構造体の実装。
 */

#include "board.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  std::string Board::ToString(const Board& board) {
    std::ostringstream oss;

    // 先ずボードを描く。
    Rank rank = RANK_8;
    oss << " +-----------------+\n";
    std::vector<std::string> piece_rank_vec;
    for (Rank i = 0; i < NUM_RANKS; ++i, --rank) {
      oss << std::to_string(rank + 1) << "| ";
      FOR_FYLES(fyle) {
        Square square = Util::CoordToSquare(fyle, rank);
        char c = '.';
        switch (board.piece_board_[square]) {
          case PAWN: c = 'P'; break;
          case KNIGHT: c = 'N'; break;
          case BISHOP: c = 'B'; break;
          case ROOK: c = 'R'; break;
          case QUEEN: c = 'Q'; break;
          case KING: c = 'K'; break;
        }
        if (board.side_board_[square] == BLACK) c = c - 'A' + 'a';

        oss << c << " ";
      }
      oss << "|\n";
    }
    oss << " +-----------------+\n   a b c d e f g h\n";

    // 手番、クロック、手数を描く。
    oss << "To Move: ";
    if (board.to_move_ == WHITE) oss << "w";
    else oss << "b";
    oss << " | Clock: " << std::to_string(board.clock_);
    oss << " | Ply: " << std::to_string(board.ply_) << std::endl;

    // アンパッサンの場所を描く。
    oss << "En Passant Square: ";
    Square en_passant_square = board.en_passant_square_;
    if (en_passant_square) {
      oss << static_cast<char>(Util::SquareToFyle(en_passant_square) + 'a');
      oss << static_cast<char>(Util::SquareToRank(en_passant_square) + '1');
    } else {
      oss << "-";
    }
    oss << std::endl;

    // キャスリングの権利を描く。
    oss << "Castling Rights : ";
    Castling rights = board.castling_rights_;
    if (rights) {
      if ((rights & WHITE_SHORT_CASTLING)) oss << "K";
      if ((rights & WHITE_LONG_CASTLING)) oss << "Q";
      if ((rights & BLACK_SHORT_CASTLING)) oss << "k";
      if ((rights & BLACK_LONG_CASTLING)) oss << "q";
    } else {
      oss << "-";
    }
    oss << std::endl;

    return oss.str();
  }
}  // namespace Sayuri
