/*
   chess_util.cpp: チェスの便利ツール。

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

#include "chess_util.h"

#include <iostream>
#include <cstddef>
#include "chess_def.h"

namespace Sayuri {
  namespace {
    // ビットの個数を得る。
    int GetNumBits(unsigned int bits) {
      int num = 0;
      for (; bits; bits &= bits - 1) {
        num++;
      }
      return num;
    }
  }

  /****************/
  /* static定数。 */
  /****************/
  constexpr Bitboard Util::BIT[NUM_SQUARES];
  constexpr Bitboard Util::FYLE[NUM_FYLES];
  constexpr Bitboard Util::RANK[NUM_RANKS];
  constexpr Square Util::ROT45[NUM_SQUARES];
  constexpr Square Util::ROT90[NUM_SQUARES];
  constexpr Square Util::ROT135[NUM_SQUARES];
  constexpr Square Util::R_ROT45[NUM_SQUARES];
  constexpr Square Util::R_ROT90[NUM_SQUARES];
  constexpr Square Util::R_ROT135[NUM_SQUARES];
  constexpr std::size_t Util::MAGIC_SHIFT_V[NUM_SQUARES];
  constexpr std::size_t Util::MAGIC_SHIFT_D[NUM_SQUARES];
  constexpr Bitboard Util::MAGIC_MASK_V[NUM_SQUARES];
  constexpr Bitboard Util::MAGIC_MASK_D[NUM_SQUARES];
  constexpr int Util::BLOCKER_MAP;

  /**********************************/
  /* ビットを数えるときに使うもの。 */
  /**********************************/
  // 16ビットのビットの個数が入った配列。
  // 引数には16ビットのパターンを入れる。
  int Util::num_bit16_array_[0xffff + 1];
  // num_bit16_array_[]を初期化する。
  void Util::InitNumBit16Array() {
    unsigned int index;
    for (index = 0; index <= 0xffff; index++) {
      num_bit16_array_[index] = GetNumBits(index);
    }
  }

  /**************/
  /* マジック。 */
  /**************/
  // マジックのシフト。
  // 0度と90度用。
  // 45度と135度用。
  // マジックのマスク。
  // 0度と90度用。
  // 45度と135度用。
  // 各方向の攻撃の配列。
  Bitboard Util::attack_array0_[NUM_SQUARES][BLOCKER_MAP + 1];  // 0度。
  Bitboard Util::attack_array45_[NUM_SQUARES][BLOCKER_MAP + 1];  // 45度。
  Bitboard Util::attack_array90_[NUM_SQUARES][BLOCKER_MAP + 1];  // 90度。
  Bitboard Util::attack_array135_[NUM_SQUARES][BLOCKER_MAP + 1];  // 135度。
  // attack_array***_[][]を初期化する。
  void Util::InitAttackArray() {
    // 位置のビットボード。
    Bitboard point;

    // 利き筋を入れるビットボード。
    Bitboard attack;

    // 障害物を入れるビットボード。
    Bitboard temp;

    // 0度のマップを作成。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];
      point >>= MAGIC_SHIFT_V[square];
      for (int map = 0; map <= BLOCKER_MAP; map++) {
        attack = 0;
        // 右側を作る。
        temp = point;
        while ((temp = GetRightBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while ((temp = GetLeftBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= MAGIC_MASK_V[square];
        // 利き筋をシフトする。
        attack <<= MAGIC_SHIFT_V[square];
        // 利き筋をマップに入れる。
        attack_array0_[square][map] = attack;
      }
    }
    // 45度のマップを作成。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[ROT45[square]];
      point >>= MAGIC_SHIFT_D[ROT45[square]];
      for (int map = 0; map <= BLOCKER_MAP; map++) {
        attack = 0;
        // 右側を作る。
        temp = point;
        while ((temp = GetRightBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while ((temp = GetLeftBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= MAGIC_MASK_D[ROT45[square]];
        // 利き筋をシフトする。
        attack <<= MAGIC_SHIFT_D[ROT45[square]];
        // 利き筋を通常の座標にする。
        attack = Reverse45(attack);
        // 利き筋をマップに入れる。
        attack_array45_[square][map] = attack;
      }
    }
    // 90度のマップを作成。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[ROT90[square]];
      point >>= MAGIC_SHIFT_V[ROT90[square]];
      for (int map = 0; map <= BLOCKER_MAP; map++) {
        attack = 0;
        // 右側を作る。
        temp = point;
        while ((temp = GetRightBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while ((temp = GetLeftBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= MAGIC_MASK_V[ROT90[square]];
        // 利き筋をシフトする。
        attack <<= MAGIC_SHIFT_V[ROT90[square]];
        // 利き筋を通常の座標にする。
        attack = Reverse90(attack);
        // 利き筋をマップに入れる。
        attack_array90_[square][map] = attack;
      }
    }
    // 135度のマップを作成。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[ROT135[square]];
      point >>= MAGIC_SHIFT_D[ROT135[square]];
      for (int map = 0; map <= BLOCKER_MAP; map++) {
        attack = 0;
        // 右側を作る。
        temp = point;
        while ((temp = GetRightBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while ((temp = GetLeftBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= MAGIC_MASK_D[ROT135[square]];
        // 利き筋をシフトする。
        attack <<= MAGIC_SHIFT_D[ROT135[square]];
        // 利き筋を通常の座標にする。
        attack = Reverse135(attack);
        // 利き筋をマップに入れる。
        attack_array135_[square][map] = attack;
      }
    }
  }
  // 45度座標のビットボードを通常の座標に戻す。
  Bitboard Util::Reverse45(Bitboard bitboard45) {
    // 通常座標のビットボード。
    Bitboard bitboard = 0;

    // 45度座標の位置。
    Square square45;

    // 通常座標に変換する。
    for (;bitboard45; bitboard45 &= bitboard45 - 1) {
      // 45度座標の位置を得る。
      square45 = CountZero(bitboard45);
      // 変換して追加。
      bitboard |= BIT[R_ROT45[square45]];
    }

    // 返す。
    return bitboard;
  }
  // 90度座標のビットボードを通常の座標に戻す。
  Bitboard Util::Reverse90(Bitboard bitboard90) {
    // 通常座標のビットボード。
    Bitboard bitboard = 0;

    // 90度座標の位置。
    Square square90;

    // 通常座標に変換する。
    for (;bitboard90; bitboard90 &= bitboard90 - 1) {
      // 90度座標の位置を得る。
      square90 = CountZero(bitboard90);
      // 変換して追加。
      bitboard |= BIT[R_ROT90[square90]];
    }

    // 返す。
    return bitboard;
  }
  // 135度座標のビットボードを通常の座標に戻す。
  Bitboard Util::Reverse135(Bitboard bitboard135) {
    // 通常座標のビットボード。
    Bitboard bitboard = 0;

    // 135度座標の位置。
    Square square135;

    // 通常座標に変換する。
    for (;bitboard135; bitboard135 &= bitboard135 - 1) {
      // 135度座標の位置を得る。
      square135 = CountZero(bitboard135);
      // 変換して追加。
      bitboard |= BIT[R_ROT135[square135]];
    }

    // 返す。
    return bitboard;
  }

  /************************/
  /* ビットボードの配列。 */
  /************************/
  // 直線の入った配列。
  Bitboard Util::line_[NUM_SQUARES][NUM_SQUARES];
  // ポーンの通常の動きの配列。
  Bitboard Util::pawn_move_[NUM_SIDES][NUM_SQUARES];
  // ポーンの2歩の動きの配列。
  Bitboard Util::pawn_2step_move_[NUM_SIDES][NUM_SQUARES];
  // ポーンの攻撃筋の配列。
  Bitboard Util::pawn_attack_[NUM_SIDES][NUM_SQUARES];
  // ナイトの動きの配列。
  Bitboard Util::knight_move_[NUM_SQUARES];
  // ビショップの動きの配列。
  Bitboard Util::bishop_move_[NUM_SQUARES];
  // ルークの動きの配列。
  Bitboard Util::rook_move_[NUM_SQUARES];
  // キングの動きの配列。
  Bitboard Util::king_move_[NUM_SQUARES];
  // line_[][]を初期化する。
  void Util::InitLine() {
    // 端点のビットボード。
    Bitboard point1;
    Bitboard point2;
    // 端点間のビットボード。
    Bitboard between;
    // 調べるビットボード。
    Bitboard temp;

    // line_を作る。
    for (int square1 = 0; square1 < NUM_SQUARES; square1++) {
      for (int square2 = 0; square2 < NUM_SQUARES; square2++) {
        // 端点を入手する。
        point1 = BIT[square1];
        point2 = BIT[square2];

        // 右から調べていく。
        temp = point1;
        between = 0;
        while ((temp = GetRightBitboard(temp))) {
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
        while ((temp = GetLeftBitboard(temp))) {
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
        while ((temp = GetUpBitboard(temp))) {
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
        while ((temp = GetDownBitboard(temp))) {
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
        while ((temp = GetRightUpBitboard(temp))) {
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
        while ((temp = GetRightDownBitboard(temp))) {
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
        while ((temp = GetLeftUpBitboard(temp))) {
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
        while ((temp = GetLeftDownBitboard(temp))) {
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
  void Util::InitPawnMove() {
    // 位置を入れるビットボード。
    Bitboard point;

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
  void Util::InitPawn2StepMove() {
    // 位置を入れるビットボード。
    Bitboard point;

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
  void Util::InitPawnAttack() {
    // 位置を入れるビットボード。
    Bitboard point;

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
  void Util::InitKnightMove() {
    // 位置を入れるビットボード。
    Bitboard point;

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
  void Util::InitBishopMove() {
    // 位置を入れるビットボード。
    Bitboard point;
    // 動きを調べるビットボード。
    Bitboard temp;

    // 動きを入れる。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];
      bishop_move_[square] = 0;

      // 右上の動きを入れる。
      temp = point;
      while ((temp = GetRightUpBitboard(temp))) {
        bishop_move_[square] |= temp;
      }
      // 右下の動きを入れる。
      temp = point;
      while ((temp = GetRightDownBitboard(temp))) {
        bishop_move_[square] |= temp;
      }
      // 左上の動きを入れる。
      temp = point;
      while ((temp = GetLeftUpBitboard(temp))) {
        bishop_move_[square] |= temp;
      }
      // 左下の動きを入れる。
      temp = point;
      while ((temp = GetLeftDownBitboard(temp))) {
        bishop_move_[square] |= temp;
      }
    }
  }
  // rook_move_[]を初期化する。
  void Util::InitRookMove() {
    // 位置を入れるビットボード。
    Bitboard point;
    // 動きを調べるビットボード。
    Bitboard temp;

    // 動きを入れる。
    for (int square = 0; square < NUM_SQUARES; square++) {
      point = BIT[square];
      rook_move_[square] = 0;

      // 右の動きを入れる。
      temp = point;
      while ((temp = GetRightBitboard(temp))) {
        rook_move_[square] |= temp;
      }
      // 左の動きを入れる。
      temp = point;
      while ((temp = GetLeftBitboard(temp))) {
        rook_move_[square] |= temp;
      }
      // 上の動きを入れる。
      temp = point;
      while ((temp = GetUpBitboard(temp))) {
        rook_move_[square] |= temp;
      }
      // 下の動きを入れる。
      temp = point;
      while ((temp = GetDownBitboard(temp))) {
        rook_move_[square] |= temp;
      }
    }
  }
  // king_move_[]を初期化する。
  void Util::InitKingMove() {
    // 位置のビットボード。
    Bitboard point;

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
