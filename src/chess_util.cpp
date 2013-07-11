/* chess_util.cpp: チェスの便利ツール。
   Copyright (c) 2011 Ishibashi Hironori

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

#include "chess_util.h"

#include <iostream>
#include "chess_def.h"
#include "sayuri_debug.h"

namespace Sayuri {
  /************************
   * ビットボードの配列。 *
   ************************/
  // ビットボードの配列。
  const bitboard_t ChessUtil::BIT[NUM_SQUARES] = {
    0x1ULL, 0x1ULL << 1, 0x1ULL << 2, 0x1ULL << 3,
    0x1ULL << 4, 0x1ULL << 5, 0x1ULL << 6, 0x1ULL << 7,
    0x1ULL << 8, 0x1ULL << 9, 0x1ULL << 10, 0x1ULL << 11,
    0x1ULL << 12, 0x1ULL << 13, 0x1ULL << 14, 0x1ULL << 15,
    0x1ULL << 16, 0x1ULL << 17, 0x1ULL << 18, 0x1ULL << 19,
    0x1ULL << 20, 0x1ULL << 21, 0x1ULL << 22, 0x1ULL << 23,
    0x1ULL << 24, 0x1ULL << 25, 0x1ULL << 26, 0x1ULL << 27,
    0x1ULL << 28, 0x1ULL << 29, 0x1ULL << 30, 0x1ULL << 31,
    0x1ULL << 32, 0x1ULL << 33, 0x1ULL << 34, 0x1ULL << 35,
    0x1ULL << 36, 0x1ULL << 37, 0x1ULL << 38, 0x1ULL << 39,
    0x1ULL << 40, 0x1ULL << 41, 0x1ULL << 42, 0x1ULL << 43,
    0x1ULL << 44, 0x1ULL << 45, 0x1ULL << 46, 0x1ULL << 47,
    0x1ULL << 48, 0x1ULL << 49, 0x1ULL << 50, 0x1ULL << 51,
    0x1ULL << 52, 0x1ULL << 53, 0x1ULL << 54, 0x1ULL << 55,
    0x1ULL << 56, 0x1ULL << 57, 0x1ULL << 58, 0x1ULL << 59,
    0x1ULL << 60, 0x1ULL << 61, 0x1ULL << 62, 0x1ULL << 63
  };
  // ファイルのビットボードの配列。
  const bitboard_t ChessUtil::FYLE[NUM_FYLES] = {
    0x0101010101010101ULL,
    0x0101010101010101ULL << 1,
    0x0101010101010101ULL << 2,
    0x0101010101010101ULL << 3,
    0x0101010101010101ULL << 4,
    0x0101010101010101ULL << 5,
    0x0101010101010101ULL << 6,
    0x0101010101010101ULL << 7
  };
  // ランクのビットボードの配列。
  const bitboard_t ChessUtil::RANK[NUM_RANKS] = {
    0xffULL,
    0xffULL << (1 * 8),
    0xffULL << (2 * 8),
    0xffULL << (3 * 8),
    0xffULL << (4 * 8),
    0xffULL << (5 * 8),
    0xffULL << (6 * 8),
    0xffULL << (7 * 8)
  };

  /**********************
   * 回転座標変換配列。 *
   **********************/
  // 通常から左に45度。
  const square_t ChessUtil::ROT45[NUM_SQUARES] = {
    E4, F3, H2, C2, G1, D1, B1, A1,
    E5, F4, G3, A3, D2, H1, E1, C1,
    D6, F5, G4, H3, B3, E2, A2, F1,
    B7, E6, G5, H4, A4, C3, F2, B2,
    G7, C7, F6, H5, A5, B4, D3, G2,
    C8, H7, D7, G6, A6, B5, C4, E3,
    F8, D8, A8, E7, H6, B6, C5, D4,
    H8, G8, E8, B8, F7, A7, C6, D5
  };
  // 通常から左に90度。
  const square_t ChessUtil::ROT90[NUM_SQUARES] = {
    H1, H2, H3, H4, H5, H6, H7, H8,
    G1, G2, G3, G4, G5, G6, G7, G8,
    F1, F2, F3, F4, F5, F6, F7, F8,
    E1, E2, E3, E4, E5, E6, E7, E8,
    D1, D2, D3, D4, D5, D6, D7, D8,
    C1, C2, C3, C4, C5, C6, C7, C8,
    B1, B2, B3, B4, B5, B6, B7, B8,
    A1, A2, A3, A4, A5, A6, A7, A8
  };
  // 通常から左に135度。
  const square_t ChessUtil::ROT135[NUM_SQUARES] = {
    A1, C1, F1, B2, G2, E3, D4, D5,
    B1, E1, A2, F2, D3, C4, C5, C6,
    D1, H1, E2, C3, B4, B5, B6, A7,
    G1, D2, B3, A4, A5, A6, H6, F7,
    C2, A3, H3, H4, H5, G6, E7, B8,
    H2, G3, G4, G5, F6, D7, A8, E8,
    F3, F4, F5, E6, C7, H7, D8, G8,
    E4, E5, D6, B7, G7, C8, F8, H8
  };
  // 逆変換。
  // 左に45度から通常へ。
  const square_t ChessUtil::R_ROT45[NUM_SQUARES] = {
    H1,
    G1, H2,
    F1, G2, H3,
    E1, F2, G3, H4,
    D1, E2, F3, G4, H5,
    C1, D2, E3, F4, G5, H6,
    B1, C2, D3, E4, F5, G6, H7,
    A1, B2, C3, D4, E5, F6, G7, H8,
    A2, B3, C4, D5, E6, F7, G8,
    A3, B4, C5, D6, E7, F8,
    A4, B5, C6, D7, E8,
    A5, B6, C7, D8,
    A6, B7, C8,
    A7, B8,
    A8
  };
  // 左に90度から通常へ。
  const square_t ChessUtil::R_ROT90[NUM_SQUARES] = {
    A8, A7, A6, A5, A4, A3, A2, A1,
    B8, B7, B6, B5, B4, B3, B2, B1,
    C8, C7, C6, C5, C4, C3, C2, C1,
    D8, D7, D6, D5, D4, D3, D2, D1,
    E8, E7, E6, E5, E4, E3, E2, E1,
    F8, F7, F6, F5, F4, F3, F2, F1,
    G8, G7, G6, G5, G4, G3, G2, G1,
    H8, H7, H6, H5, H4, H3, H2, H1
  };
  // 左に135度から通常へ。
  const square_t ChessUtil::R_ROT135[NUM_SQUARES] = {
    A1,
    A2, B1,
    A3, B2, C1,
    A4, B3, C2, D1,
    A5, B4, C3, D2, E1,
    A6, B5, C4, D3, E2, F1,
    A7, B6, C5, D4, E3, F2, G1,
    A8, B7, C6, D5, E4, F3, G2, H1,
    B8, C7, D6, E5, F4, G3, H2,
    C8, D7, E6, F5, G4, H3,
    D8, E7, F6, G5, H4,
    E8, F7, G6, H5,
    F8, G7, H6,
    G8, H7,
    H8
  };

  /**********************************
   * ビットを数えるときに使うもの。 *
   **********************************/
  // 16ビットのビットの個数が入った配列。
  // 引数には16ビットのパターンを入れる。
  int ChessUtil::num_bit16_array_[BIT16_PATTERN];
  // num_bit16_array_[]を初期化する。
  void ChessUtil::InitNumBit16Array() {
    unsigned int index;
    for (index = 0; index <= 0xffff; index++) {
      num_bit16_array_[index] = GetNumBits(index);
    }
  }
  // ビットの個数を得る。
  int ChessUtil::GetNumBits(unsigned int bits) {
    int num = 0;
    for (; bits; bits &= bits - 1) {
      num++;
    }
    return num;
  }

  /**************
   * マジック。 *
   **************/
  // マジックのシフト。
  // 0度と90度用。
  const int ChessUtil::magic_shift_v_[NUM_SQUARES] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    8, 8, 8, 8, 8, 8, 8, 8,
    16, 16, 16, 16, 16, 16, 16, 16,
    24, 24, 24, 24, 24, 24, 24, 24,
    32, 32, 32, 32, 32, 32, 32, 32,
    40, 40, 40, 40, 40, 40, 40, 40,
    48, 48, 48, 48, 48, 48, 48, 48,
    56, 56, 56, 56, 56, 56, 56, 56
  };
  // 45度と135度用。
  const int ChessUtil::magic_shift_d_[NUM_SQUARES] = {
    0,
    1, 1,
    3, 3, 3,
    6, 6, 6, 6,
    10, 10, 10, 10, 10,
    15, 15, 15, 15, 15, 15,
    21, 21, 21, 21, 21, 21, 21,
    28, 28, 28, 28, 28, 28, 28, 28,
    36, 36, 36, 36, 36, 36, 36,
    43, 43, 43, 43, 43, 43,
    49, 49, 49, 49, 49,
    54, 54, 54, 54,
    58, 58, 58,
    61, 61,
    63
  };
  // マジックのマスク。
  // 0度と90度用。
  const bitboard_t ChessUtil::magic_mask_v_[NUM_SQUARES] = {
    0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
    0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
    0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
    0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
    0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
    0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
    0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
    0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL
  };
  // 45度と135度用。
  const bitboard_t ChessUtil::magic_mask_d_[NUM_SQUARES] = {
    0x1ULL,
    0x3ULL, 0x3ULL,
    0x7ULL, 0x7ULL, 0x7ULL,
    0xfULL, 0xfULL, 0xfULL, 0xfULL,
    0x1fULL, 0x1fULL, 0x1fULL, 0x1fULL, 0x1fULL,
    0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL,
    0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL,
    0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
    0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL,
    0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL,
    0x1fULL, 0x1fULL, 0x1fULL, 0x1fULL, 0x1fULL,
    0xfULL, 0xfULL, 0xfULL, 0xfULL,
    0x7ULL, 0x7ULL, 0x7ULL,
    0x3ULL, 0x3ULL,
    0x1ULL
  };
  // 各方向の攻撃の配列。
  bitboard_t ChessUtil::attack_array0_[NUM_SQUARES][BLOCKER_MAP];  // 0度。
  bitboard_t ChessUtil::attack_array45_[NUM_SQUARES][BLOCKER_MAP];  // 45度。
  bitboard_t ChessUtil::attack_array90_[NUM_SQUARES][BLOCKER_MAP];  // 90度。
  bitboard_t ChessUtil::attack_array135_[NUM_SQUARES][BLOCKER_MAP];  // 135度。
  // attack_array***_[][]を初期化する。
  void ChessUtil::InitAttackArray() {
    // 位置のビットボード。
    bitboard_t point;

    // 利き筋を入れるビットボード。
    bitboard_t attack;

    // 障害物を入れるビットボード。
    bitboard_t temp;

    // 0度のマップを作成。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];
      point >>= magic_shift_v_[square];
      for (int map = 0; map < BLOCKER_MAP; map++) {
        attack = 0;
        // 右側を作る。
        temp = point;
        while (temp = GetRightBitboard(temp)) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while (temp = GetLeftBitboard(temp)) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= magic_mask_v_[square];
        // 利き筋をシフトする。
        attack <<= magic_shift_v_[square];
        // 利き筋をマップに入れる。
        attack_array0_[square][map] = attack;
      }
    }
    // 45度のマップを作成。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[ROT45[square]];
      point >>= magic_shift_d_[ROT45[square]];
      for (int map = 0; map < BLOCKER_MAP; map++) {
        attack = 0;
        // 右側を作る。
        temp = point;
        while (temp = GetRightBitboard(temp)) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while (temp = GetLeftBitboard(temp)) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= magic_mask_d_[ROT45[square]];
        // 利き筋をシフトする。
        attack <<= magic_shift_d_[ROT45[square]];
        // 利き筋を通常の座標にする。
        attack = Reverse45(attack);
        // 利き筋をマップに入れる。
        attack_array45_[square][map] = attack;
      }
    }
    // 90度のマップを作成。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[ROT90[square]];
      point >>= magic_shift_v_[ROT90[square]];
      for (int map = 0; map < BLOCKER_MAP; map++) {
        attack = 0;
        // 右側を作る。
        temp = point;
        while (temp = GetRightBitboard(temp)) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while (temp = GetLeftBitboard(temp)) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= magic_mask_v_[ROT90[square]];
        // 利き筋をシフトする。
        attack <<= magic_shift_v_[ROT90[square]];
        // 利き筋を通常の座標にする。
        attack = Reverse90(attack);
        // 利き筋をマップに入れる。
        attack_array90_[square][map] = attack;
      }
    }
    // 135度のマップを作成。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[ROT135[square]];
      point >>= magic_shift_d_[ROT135[square]];
      for (int map = 0; map < BLOCKER_MAP; map++) {
        attack = 0;
        // 右側を作る。
        temp = point;
        while (temp = GetRightBitboard(temp)) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while (temp = GetLeftBitboard(temp)) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= magic_mask_d_[ROT135[square]];
        // 利き筋をシフトする。
        attack <<= magic_shift_d_[ROT135[square]];
        // 利き筋を通常の座標にする。
        attack = Reverse135(attack);
        // 利き筋をマップに入れる。
        attack_array135_[square][map] = attack;
      }
    }
  }
  // 45度座標のビットボードを通常の座標に戻す。
  bitboard_t ChessUtil::Reverse45(bitboard_t bitboard45) {
    // 通常座標のビットボード。
    bitboard_t bitboard = 0;

    // 45度座標の位置。
    square_t square45;

    // 通常座標に変換する。
    for (;bitboard45; bitboard45 &= bitboard45 - 1) {
      // 45度座標の位置を得る。
      square45 = static_cast<square_t>(CountZero(bitboard45));
      // 変換して追加。
      bitboard |= BIT[R_ROT45[square45]];
    }

    // 返す。
    return bitboard;
  }
  // 90度座標のビットボードを通常の座標に戻す。
  bitboard_t ChessUtil::Reverse90(bitboard_t bitboard90) {
    // 通常座標のビットボード。
    bitboard_t bitboard = 0;

    // 90度座標の位置。
    square_t square90;

    // 通常座標に変換する。
    for (;bitboard90; bitboard90 &= bitboard90 - 1) {
      // 90度座標の位置を得る。
      square90 = static_cast<square_t>(CountZero(bitboard90));
      // 変換して追加。
      bitboard |= BIT[R_ROT90[square90]];
    }

    // 返す。
    return bitboard;
  }
  // 135度座標のビットボードを通常の座標に戻す。
  bitboard_t ChessUtil::Reverse135(bitboard_t bitboard135) {
    // 通常座標のビットボード。
    bitboard_t bitboard = 0;

    // 135度座標の位置。
    square_t square135;

    // 通常座標に変換する。
    for (;bitboard135; bitboard135 &= bitboard135 - 1) {
      // 135度座標の位置を得る。
      square135 = static_cast<square_t>(CountZero(bitboard135));
      // 変換して追加。
      bitboard |= BIT[R_ROT135[square135]];
    }

    // 返す。
    return bitboard;
  }

  /************************
   * ビットボードの配列。 *
   ************************/
  // 直線の入った配列。
  bitboard_t ChessUtil::line_[NUM_SQUARES][NUM_SQUARES];
  // ポーンの通常の動きの配列。
  bitboard_t ChessUtil::pawn_move_[NUM_SIDES][NUM_SQUARES];
  // ポーンの2歩の動きの配列。
  bitboard_t ChessUtil::pawn_2step_move_[NUM_SIDES][NUM_SQUARES];
  // ポーンの攻撃筋の配列。
  bitboard_t ChessUtil::pawn_attack_[NUM_SIDES][NUM_SQUARES];
  // ナイトの動きの配列。
  bitboard_t ChessUtil::knight_move_[NUM_SQUARES];
  // ビショップの動きの配列。
  bitboard_t ChessUtil::bishop_move_[NUM_SQUARES];
  // ルークの動きの配列。
  bitboard_t ChessUtil::rook_move_[NUM_SQUARES];
  // キングの動きの配列。
  bitboard_t ChessUtil::king_move_[NUM_SQUARES];
  // line_[][]を初期化する。
  void ChessUtil::InitLine() {
    // 端点のビットボード。
    bitboard_t point1;
    bitboard_t point2;
    // 端点間のビットボード。
    bitboard_t between;
    // 調べるビットボード。
    bitboard_t temp;

    // line_を作る。
    for (int square1 = 0; square1 < NUM_SQUARES; square1++) {
      for (int square2 = 0; square2 < NUM_SQUARES; square2++) {
        // 端点を入手する。
        point1 = BIT[square1];
        point2 = BIT[square2];

        // 右から調べていく。
        temp = point1;
        between = 0;
        while (temp = GetRightBitboard(temp)) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point2) {
            line_[square1][square2] = point1 | between;
            break;
          }
        }
        // 左から調べていく。
        temp = point1;
        between = 0;
        while (temp = GetLeftBitboard(temp)) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point2) {
            line_[square1][square2] = point1 | between;
            break;
          }
        }
        // 上から調べていく。
        temp = point1;
        between = 0;
        while (temp = GetUpBitboard(temp)) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point2) {
            line_[square1][square2] = point1 | between;
            break;
          }
        }
        // 下から調べていく。
        temp = point1;
        between = 0;
        while (temp = GetDownBitboard(temp)) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point2) {
            line_[square1][square2] = point1 | between;
            break;
          }
        }
        // 右上から調べていく。
        temp = point1;
        between = 0;
        while (temp = GetRightUpBitboard(temp)) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point2) {
            line_[square1][square2] = point1 | between;
            break;
          }
        }
        // 右下から調べていく。
        temp = point1;
        between = 0;
        while (temp = GetRightDownBitboard(temp)) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point2) {
            line_[square1][square2] = point1 | between;
            break;
          }
        }
        // 左上から調べていく。
        temp = point1;
        between = 0;
        while (temp = GetLeftUpBitboard(temp)) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point2) {
            line_[square1][square2] = point1 | between;
            break;
          }
        }
        // 左下から調べていく。
        temp = point1;
        between = 0;
        while (temp = GetLeftDownBitboard(temp)) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point2) {
            line_[square1][square2] = point1 | between;
            break;
          }
        }
      }
    }
  }
  // pawn_move_[][]を初期化する。
  void ChessUtil::InitPawnMove() {
    // 位置を入れるビットボード。
    bitboard_t point;

    // ポーンの動きを作る。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];
      // どちらのサイドでもない。
      pawn_move_[NO_SIDE][square] = 0;
      // 白。
      pawn_move_[WHITE][square] = GetUpBitboard(point);
      // 黒。
      pawn_move_[BLACK][square] = GetDownBitboard(point);
    }
  }
  // pawn_2step_move_[][]を初期化する。
  void ChessUtil::InitPawn2StepMove() {
    // 位置を入れるビットボード。
    bitboard_t point;

    // ポーンの2歩の動きを作る。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];

      // とりあえず0で初期化する。
      pawn_2step_move_[NO_SIDE][square] = 0;
      pawn_2step_move_[WHITE][square] = 0;
      pawn_2step_move_[BLACK][square] = 0;

      // ランク2の時は白に2歩の動きを入れる。
      // ランク7の時は黒に2歩の動きを入れる。
      if (point & RANK[RANK_2]) {
        pawn_2step_move_[WHITE][square] =
        GetUpBitboard(GetUpBitboard(point));
      } else if (point & RANK[RANK_7]) {
        pawn_2step_move_[BLACK][square] =
        GetDownBitboard(GetDownBitboard(point));
      }
    }
  }
  // pawn_attack_[][]を初期化する。
  void ChessUtil::InitPawnAttack() {
    // 位置を入れるビットボード。
    bitboard_t point;

    // 攻撃筋を入れる。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];

      // どちらでもない。
      pawn_attack_[NO_SIDE][square] = 0;
      // 白。
      pawn_attack_[WHITE][square] =
      GetRightUpBitboard(point) | GetLeftUpBitboard(point);
      // 黒。
      pawn_attack_[BLACK][square] =
      GetRightDownBitboard(point) | GetLeftDownBitboard(point);
    }
  }
  // knight_move_[]を初期化する。
  void ChessUtil::InitKnightMove() {
    // 位置を入れるビットボード。
    bitboard_t point;

    // 動きを入れる。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];

      knight_move_[square] = GetRightRightUpBitboard(point)
      | GetRightUpUpBitboard(point)
      | GetRightRightDownBitboard(point)
      | GetRightDownDownBitboard(point)
      | GetLeftLeftUpBitboard(point)
      | GetLeftUpUpBitboard(point)
      | GetLeftLeftDownBitboard(point)
      | GetLeftDownDownBitboard(point);
    }
  }
  // bishop_move_[]を初期化する。
  void ChessUtil::InitBishopMove() {
    // 位置を入れるビットボード。
    bitboard_t point;
    // 動きを調べるビットボード。
    bitboard_t temp;

    // 動きを入れる。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];
      bishop_move_[square] = 0;

      // 右上の動きを入れる。
      temp = point;
      while (temp = GetRightUpBitboard(temp)) {
        bishop_move_[square] |= temp;
      }
      // 右下の動きを入れる。
      temp = point;
      while (temp = GetRightDownBitboard(temp)) {
        bishop_move_[square] |= temp;
      }
      // 左上の動きを入れる。
      temp = point;
      while (temp = GetLeftUpBitboard(temp)) {
        bishop_move_[square] |= temp;
      }
      // 左下の動きを入れる。
      temp = point;
      while (temp = GetLeftDownBitboard(temp)) {
        bishop_move_[square] |= temp;
      }
    }
  }
  // rook_move_[]を初期化する。
  void ChessUtil::InitRookMove() {
    // 位置を入れるビットボード。
    bitboard_t point;
    // 動きを調べるビットボード。
    bitboard_t temp;

    // 動きを入れる。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];
      rook_move_[square] = 0;

      // 右の動きを入れる。
      temp = point;
      while (temp = GetRightBitboard(temp)) {
        rook_move_[square] |= temp;
      }
      // 左の動きを入れる。
      temp = point;
      while (temp = GetLeftBitboard(temp)) {
        rook_move_[square] |= temp;
      }
      // 上の動きを入れる。
      temp = point;
      while (temp = GetUpBitboard(temp)) {
        rook_move_[square] |= temp;
      }
      // 下の動きを入れる。
      temp = point;
      while (temp = GetDownBitboard(temp)) {
        rook_move_[square] |= temp;
      }
    }
  }
  // king_move_[]を初期化する。
  void ChessUtil::InitKingMove() {
    // 位置のビットボード。
    bitboard_t point;

    // 動きを入れる。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];

      // 動きを入れる。
      king_move_[square] = GetRightBitboard(point)
      | GetLeftBitboard(point)
      | GetUpBitboard(point)
      | GetDownBitboard(point)
      | GetRightUpBitboard(point)
      | GetRightDownBitboard(point)
      | GetLeftUpBitboard(point)
      | GetLeftDownBitboard(point);
    }
  }
}  // namespace Sayuri
