/* chess_def.h: チェスの定数の定義。
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

#ifndef CHESS_DEF_H
#define CHESS_DEF_H

#include <cstdint>

namespace Sayuri {
  // ビットボードの型。
  using Bitboard = std::uint64_t;

  // マスの型。
  using Square = int;
  constexpr Square A1 = 0;
  constexpr Square A2 = 1;
  constexpr Square A3 = 2;
  constexpr Square A4 = 3;
  constexpr Square A5 = 4;
  constexpr Square A6 = 5;
  constexpr Square A7 = 6;
  constexpr Square A8 = 7;
  constexpr Square B1 = 8;
  constexpr Square B2 = 9;
  constexpr Square B3 = 10;
  constexpr Square B4 = 11;
  constexpr Square B5 = 12;
  constexpr Square B6 = 13;
  constexpr Square B7 = 14;
  constexpr Square B8 = 15;
  constexpr Square C1 = 16;
  constexpr Square C2 = 17;
  constexpr Square C3 = 18;
  constexpr Square C4 = 19;
  constexpr Square C5 = 20;
  constexpr Square C6 = 21;
  constexpr Square C7 = 22;
  constexpr Square C8 = 23;
  constexpr Square D1 = 24;
  constexpr Square D2 = 25;
  constexpr Square D3 = 26;
  constexpr Square D4 = 27;
  constexpr Square D5 = 28;
  constexpr Square D6 = 29;
  constexpr Square D7 = 30;
  constexpr Square D8 = 31;
  constexpr Square E1 = 32;
  constexpr Square E2 = 33;
  constexpr Square E3 = 34;
  constexpr Square E4 = 35;
  constexpr Square E5 = 36;
  constexpr Square E6 = 37;
  constexpr Square E7 = 38;
  constexpr Square E8 = 39;
  constexpr Square F1 = 40;
  constexpr Square F2 = 41;
  constexpr Square F3 = 42;
  constexpr Square F4 = 43;
  constexpr Square F5 = 44;
  constexpr Square F6 = 45;
  constexpr Square F7 = 46;
  constexpr Square F8 = 47;
  constexpr Square G1 = 48;
  constexpr Square G2 = 49;
  constexpr Square G3 = 50;
  constexpr Square G4 = 51;
  constexpr Square G5 = 52;
  constexpr Square G6 = 53;
  constexpr Square G7 = 54;
  constexpr Square G8 = 55;
  constexpr Square H1 = 56;
  constexpr Square H2 = 57;
  constexpr Square H3 = 58;
  constexpr Square H4 = 59;
  constexpr Square H5 = 60;
  constexpr Square H6 = 61;
  constexpr Square H7 = 62;
  constexpr Square H8 = 63;

  // ファイルの型。
  using Fyle = int;
  constexpr Fyle FYLE_A = 0;
  constexpr Fyle FYLE_B = 1;
  constexpr Fyle FYLE_C = 2;
  constexpr Fyle FYLE_D = 3;
  constexpr Fyle FYLE_E = 4;
  constexpr Fyle FYLE_F = 5;
  constexpr Fyle FYLE_G = 6;
  constexpr Fyle FYLE_H = 7;

  // ランクの型。
  using Rank = int;
  constexpr Rank RANK_1 = 0;
  constexpr Rank RANK_2 = 1;
  constexpr Rank RANK_3 = 2;
  constexpr Rank RANK_4 = 3;
  constexpr Rank RANK_5 = 4;
  constexpr Rank RANK_6 = 5;
  constexpr Rank RANK_7 = 6;
  constexpr Rank RANK_8 = 7;

  // サイドの型。
  using Side = int;
  constexpr Side NO_SIDE = 0;
  constexpr Side WHITE = 1;
  constexpr Side BLACK = 2;

  // 駒の型。
  using Piece = int;
  constexpr Piece EMPTY = 0;
  constexpr Piece PAWN = 1;
  constexpr Piece KNIGHT = 2;
  constexpr Piece BISHOP = 3;
  constexpr Piece ROOK = 4;
  constexpr Piece QUEEN = 5;
  constexpr Piece KING = 6;

  // 数の定義。
  constexpr int NUM_SQUARES = 64;
  constexpr int NUM_FYLES = 8;
  constexpr int NUM_RANKS = 8;
  constexpr int NUM_SIDES = 3;
  constexpr int NUM_PIECE_TYPES = 7;
  constexpr int INFINITE = 9999999;

  // キャスリングのフラグの型。
  using Castling = std::uint8_t;
  // 白のショートキャスリング。
  constexpr Castling WHITE_SHORT_CASTLING = 1;
  // 白のロングキャスリング。
  constexpr Castling WHITE_LONG_CASTLING = 1 << 1;
  // 黒のショートキャスリング。
  constexpr Castling BLACK_SHORT_CASTLING = 1 << 2;
  // 黒のロングキャスリング。
  constexpr Castling BLACK_LONG_CASTLING = 1 << 3;
  // 白のキャスリング。
  constexpr Castling WHITE_CASTLING =
  WHITE_SHORT_CASTLING | WHITE_LONG_CASTLING;
  // 黒のキャスリング。
  constexpr Castling BLACK_CASTLING =
  BLACK_SHORT_CASTLING | BLACK_LONG_CASTLING;
  // 全キャスリング。
  constexpr Castling ALL_CASTLING = WHITE_CASTLING | BLACK_CASTLING;

  // 手の種類の定数。
  using MoveType = int;
  constexpr MoveType NULL_MOVE = 0;
  constexpr MoveType NORMAL = 1;
  constexpr MoveType CASTLING = 2;
  constexpr MoveType EN_PASSANT = 3;

  // 手の型。
  union Move {
    std::uint32_t all_ : 31;
    struct {
      unsigned int piece_square_ : 6;  // 駒の位置。
      unsigned int goal_square_ : 6;  // 移動先の位置。
      unsigned int captured_piece_ : 3;  // 取った駒の種類。
      unsigned int promotion_ : 3;  // 昇格する駒の種類。
      // 動かす前のキャスリングのフラグ。
      unsigned int last_castling_rights_ : 4;
      // 動かす前のアンパッサンできるかどうか。
      bool last_can_en_passant_ : 1;
      // 動かす前のアンパッサンのターゲットの位置。
      unsigned int last_en_passant_target_ : 6;
      unsigned int move_type_ : 2;  // 手の種類。
    };
  };

  // ハッシュの型。
  using HashKey = std::uint64_t;
}  // namespace Sayuri

#endif
