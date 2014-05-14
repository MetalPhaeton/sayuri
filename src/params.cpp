/*
   params.h: 探索アルゴリズム、評価関数を変更するパラメータのクラスの実装。

   The MIT License (MIT)

   Copyright (c) 2014 Hironori Ishibashi

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

# include "params.h"

#include <iostream>

namespace Sayuri {
  /****************/
  /* EvalParams。 */
  /****************/
  // コンストラクタ。
  EvalParams::EvalParams() :
  weight_mobility_(1.0, 1.0),
  weight_center_control_(0.5, 0.0),
  weight_sweet_center_control_(0.5, 0.0),
  weight_development_(2.5, 0.0),
  weight_attack_(2.0, 0.0),
  weight_attack_around_king_(0.0, 3.0),
  weight_pass_pawn_(7.0, 14.0),
  weight_protected_pass_pawn_(2.5, 2.5),
  weight_double_pawn_(-2.5, -5.0),
  weight_iso_pawn_(-5.0, -2.5),
  weight_pawn_shield_(15.0, 0.0),
  weight_bishop_pair_(10.0, 60.0),
  weight_bad_bishop_(-0.7, 0.0),
  weight_pin_knight_(10.0, 0.0),
  weight_rook_pair_(10.0, 20.0),
  weight_rook_semiopen_fyle_(3.5, 3.5),
  weight_rook_open_fyle_(3.5, 3.5),
  weight_early_queen_launched_(-20.0, 0.0),
  weight_weak_square_(-5.0, 0.0),
  weight_castling_(90.0, 0.0),
  weight_abandoned_castling_(-45.0, 0.0) {
    // オープニング時の駒の配置の価値テーブルの初期化。
    constexpr double TABLE_1[NUM_PIECE_TYPES][NUM_SQUARES] {
      {  // EMPTY。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // ポーン。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0,
        2.0, 3.0, 4.0, 5.0, 5.0, 4.0, 3.0, 2.0,
        3.0, 4.0, 5.0, 6.0, 6.0, 5.0, 4.0, 3.0,
        4.0, 5.0, 6.0, 7.0, 7.0, 6.0, 5.0, 4.0,
        5.0, 6.0, 7.0, 8.0, 8.0, 7.0, 6.0, 5.0,
        6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0
      },
      {  // ナイト。
        -3.0, -2.0, -1.0, -1.0, -1.0, -1.0, -2.0, -3.0,
        -2.0, -1.0,  0.0,  0.0,  0.0,  0.0, -1.0, -2.0,
        -1.0,  0.0,  1.0,  1.0,  1.0,  1.0,  0.0, -1.0,
         0.0,  1.0,  2.0,  2.0,  2.0,  2.0,  1.0,  0.0,
         1.0,  2.0,  3.0,  3.0,  3.0,  3.0,  2.0,  1.0,
         2.0,  3.0,  4.0,  4.0,  4.0,  4.0,  3.0,  2.0,
         1.0,  2.0,  3.0,  3.0,  3.0,  3.0,  2.0,  1.0,
         0.0,  1.0,  2.0,  2.0,  2.0,  2.0,  1.0,  0.0
      },
      {  // ビショップ。
        2.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0,
        1.0, 3.0, 1.0, 2.0, 2.0, 1.0, 3.0, 1.0,
        1.0, 2.0, 3.0, 2.0, 2.0, 3.0, 2.0, 1.0,
        0.0, 1.0, 3.0, 3.0, 3.0, 3.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 2.0, 2.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // ルーク。
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0,
        4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0
      },
      {  // クイーン。
        -3.0, -2.0, -2.0, -1.0, -1.0, -2.0, -2.0, -3.0,
        -2.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -2.0,
        -2.0,  0.0,  1.0,  1.0,  1.0,  1.0,  0.0, -2.0,
        -1.0,  0.0,  1.0,  2.0,  2.0,  1.0,  0.0, -1.0,
        -1.0,  0.0,  1.0,  2.0,  2.0,  1.0,  0.0, -1.0,
        -2.0,  0.0,  1.0,  1.0,  1.0,  1.0,  0.0, -2.0,
        -2.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -2.0,
        -3.0, -2.0, -2.0, -1.0, -1.0, -2.0, -2.0, -3.0
      },
      {  // キング。
         1.0,  1.0,  0.0, -1.0, -1.0,  0.0,  1.0,  1.0,
         0.0,  0.0, -1.0, -2.0, -2.0, -1.0,  0.0,  0.0,
        -1.0, -1.0, -2.0, -3.0, -3.0, -2.0, -1.0, -1.0,
        -2.0, -2.0, -3.0, -4.0, -4.0, -3.0, -2.0, -2.0,
        -2.0, -2.0, -3.0, -4.0, -4.0, -3.0, -2.0, -2.0,
        -1.0, -1.0, -2.0, -3.0, -3.0, -2.0, -1.0, -1.0,
         0.0,  0.0, -1.0, -2.0, -2.0, -1.0,  0.0,  0.0,
         1.0,  1.0,  0.0, -1.0, -1.0,  0.0,  1.0,  1.0
      }
    };
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        opening_position_value_table_[piece_type][square] =
        TABLE_1[piece_type][square];
      }
    }

    // エンディング時の駒の配置の価値テーブルを初期化。
    constexpr double TABLE_2[NUM_PIECE_TYPES][NUM_SQUARES] {
      {  // 何もなし。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // ポーン。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 
        3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 
        4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 
        5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 
        6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0 
      },
      {  // ナイト。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // ビショップ。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // ルーク。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // クイーン。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // キング。
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0,
        2.0, 3.0, 4.0, 5.0, 5.0, 4.0, 3.0, 2.0,
        3.0, 4.0, 5.0, 6.0, 6.0, 5.0, 4.0, 3.0,
        3.0, 4.0, 5.0, 6.0, 6.0, 5.0, 4.0, 3.0,
        2.0, 3.0, 4.0, 5.0, 5.0, 4.0, 3.0, 2.0,
        1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0
      }
    };
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        ending_position_value_table_[piece_type][square] =
        TABLE_2[piece_type][square];
      }
    }

    // 駒への攻撃の価値テーブルを初期化。
    constexpr double TABLE_3[NUM_PIECE_TYPES][NUM_PIECE_TYPES] {
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 攻撃側: Empty。
      {0.0, 4.0, 5.0, 5.0, 6.0, 7.0, 7.0},  // 攻撃側: ポーン。
      {0.0, 3.0, 4.0, 4.0, 5.0, 6.0, 6.0},  // 攻撃側: ナイト。
      {0.0, 3.0, 4.0, 4.0, 5.0, 6.0, 6.0},  // 攻撃側: ビショップ。
      {0.0, 2.0, 3.0, 3.0, 4.0, 5.0, 5.0},  // 攻撃側: ルーク。
      {0.0, 1.0, 2.0, 2.0, 3.0, 4.0, 4.0},  // 攻撃側: クイーン。
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}   // 攻撃側: キング。
    };
    for (Piece type_1 = 0; type_1 < NUM_PIECE_TYPES; type_1++) {
      for (Piece type_2 = 0; type_2 < NUM_PIECE_TYPES; type_2++) {
        attack_value_table_[type_1][type_2] = TABLE_3[type_1][type_2];
      }
    }

    // ポーンの盾の配置の価値テーブルを初期化。
    constexpr double TABLE_4[NUM_SQUARES] {
      7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0,
      6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0,
      5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
      4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0,
      3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0,
      2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
    };
    for (Square square = 0; square < NUM_SQUARES; square++) {
      pawn_shield_value_table_[square] = TABLE_4[square];
    }

    // オープニング時の駒の配置のウェイトを初期化。
    weight_opening_position_[EMPTY] = Weight(0.0, 0.0);  // EMPTY。
    weight_opening_position_[PAWN] = Weight(2.0, 0.0);  // ポーン。
    weight_opening_position_[KNIGHT] = Weight(2.5, 0.0);  // ナイト。
    weight_opening_position_[BISHOP] = Weight(3.5, 0.0);  // ビショップ。
    weight_opening_position_[ROOK] = Weight(2.5, 0.0);  // ルーク。
    weight_opening_position_[QUEEN] = Weight(2.5, 0.0);  // クイーン。
    weight_opening_position_[KING] = Weight(10.0, 0.0);  // キング。

    // エンディング時の駒の配置のウェイトを初期化。
    weight_ending_position_[EMPTY] = Weight(0.0, 0.0);  // EMPTY。
    weight_ending_position_[PAWN] = Weight(0.0, 20.0);  // ポーン。
    weight_ending_position_[KNIGHT] = Weight(0.0, 0.0);  // ナイト。
    weight_ending_position_[BISHOP] = Weight(0.0, 0.0);  // ビショップ。
    weight_ending_position_[ROOK] = Weight(0.0, 0.0);  // ルーク。
    weight_ending_position_[QUEEN] = Weight(0.0, 0.0);  // クイーン。
    weight_ending_position_[KING] = Weight(0.0, 15.0);  // キング。
  }

  // コピーコンストラクタ。
  EvalParams::EvalParams(const EvalParams& params) {
    ScanMember(params);
  }

  // ムーブコンストラクタ。
  EvalParams::EvalParams(EvalParams&& params) {
    ScanMember(params);
  }

  // コピー代入。
  EvalParams& EvalParams::operator=(const EvalParams& params) {
    ScanMember(params);
    return *this;
  }

  // ムーブ代入。
  EvalParams& EvalParams::operator=(EvalParams&& params) {
    ScanMember(params);
    return *this;
  }

  // オープニング時の駒の配置の価値テーブルのミューテータ。
  void EvalParams::opening_position_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]) {
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        opening_position_value_table_[piece_type][square] =
        table[piece_type][square];
      }
    }
  }

  // エンディング時の駒の配置の価値テーブルのミューテータ。
  void EvalParams::ending_position_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]) {
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        ending_position_value_table_[piece_type][square] =
        table[piece_type][square];
      }
    }
  }

  // 駒への攻撃の価値テーブルのミューテータ。
  void EvalParams::attack_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES]) {
    for (Piece type_1 = 0; type_1 < NUM_PIECE_TYPES; type_1++) {
      for (Piece type_2 = 0; type_2 < NUM_PIECE_TYPES; type_2++) {
        attack_value_table_[type_1][type_2] = table[type_1][type_2];
      }
    }
  }

  // ポーンの盾の配置の価値テーブル。
  void EvalParams::pawn_shield_value_table
  (const double (& table)[NUM_SQUARES]) {
    for (Square square = 0; square < NUM_SQUARES; square++) {
      pawn_shield_value_table_[square] = table[square];
    }
  }

  // メンバをコピーする。
  void EvalParams::ScanMember(const EvalParams& params) {
    // 価値テーブルをコピー。
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        opening_position_value_table_[piece_type][square] =
        params.opening_position_value_table_[piece_type][square];
      }
    }
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        ending_position_value_table_[piece_type][square] =
        params.ending_position_value_table_[piece_type][square];
      }
    }
    for (Piece type_1 = 0; type_1 < NUM_PIECE_TYPES; type_1++) {
      for (Piece type_2 = 0; type_2 < NUM_PIECE_TYPES; type_2++) {
        attack_value_table_[type_1][type_2] =
        params.attack_value_table_[type_1][type_2];
      }
    }
    for (Square square = 0; square < NUM_SQUARES; square++) {
      pawn_shield_value_table_[square] =
      params.pawn_shield_value_table_[square];
    }

    // ウェイトをコピー。
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      weight_opening_position_[piece_type] =
      params.weight_opening_position_[piece_type];
    }
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      weight_ending_position_[piece_type] =
      params.weight_ending_position_[piece_type];
    }
    weight_mobility_ = params.weight_mobility_;
    weight_center_control_ = params.weight_center_control_;
    weight_sweet_center_control_ = params.weight_sweet_center_control_;
    weight_development_ = params.weight_development_;
    weight_attack_ = params.weight_attack_;
    weight_attack_around_king_ = params.weight_attack_around_king_;
    weight_pass_pawn_ = params.weight_pass_pawn_;
    weight_protected_pass_pawn_ = params.weight_protected_pass_pawn_;
    weight_double_pawn_ = params.weight_double_pawn_;
    weight_iso_pawn_ = params.weight_iso_pawn_;
    weight_pawn_shield_ = params.weight_pawn_shield_;
    weight_bishop_pair_ = params.weight_bishop_pair_;
    weight_bad_bishop_ = params.weight_bad_bishop_;
    weight_pin_knight_ = params.weight_pin_knight_;
    weight_rook_pair_ = params.weight_rook_pair_;
    weight_rook_semiopen_fyle_ = params.weight_rook_semiopen_fyle_;
    weight_rook_open_fyle_ = params.weight_rook_open_fyle_;
    weight_early_queen_launched_ = params.weight_early_queen_launched_;
    weight_weak_square_ = params.weight_weak_square_;
    weight_castling_ = params.weight_castling_;
    weight_abandoned_castling_ = params.weight_abandoned_castling_;
  }
}  // namespace Sayuri
