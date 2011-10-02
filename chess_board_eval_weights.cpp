/* chess_board_eval_weights.cpp: 評価の重み。
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

#include "chess_board.h"

#include <iostream>
#include "chess_def.h"
#include "chess_util.h"

namespace Misaki {
  /********************************
   * チェスの評価の重みの構造体。 *
   ********************************/
  // コンストラクタ。
  EvalWeights::EvalWeights() {
    // 全駒の評価の重さ。
    mobility_weight_ = 2;  // 機動力の重さ。
    attack_center_weight_ = 5;  // センター攻撃の重さ。
    development_weight_ = 30;  // 展開の重さ。
    attack_around_king_weight_ = 10;  // キングの周囲への攻撃の重さ。

    // 駒の配置の重要度テーブルを初期化。
    // ポーンの駒の配置の重要度。
    static const int pawn_position_table[NUM_SQUARES] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3,
      4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5,
      0, 0, 0, 0, 0, 0, 0, 0
    };
    for (int index = 0; index < NUM_SQUARES; index++) {
      pawn_position_table_[index] = pawn_position_table[index];
    }
    // ナイトの駒の配置の重要度。
    static const int knight_position_table[NUM_SQUARES] = {
      -3, -2, -1, -1, -1, -1, -2, -3,
      -2, -1,  0,  0,  0,  0, -1, -2,
      -1,  0,  1,  1,  1,  1,  0, -1,
       0,  1,  2,  2,  2,  2,  1,  0,
       1,  2,  3,  3,  3,  3,  2,  1,
       2,  3,  4,  4,  4,  4,  3,  2,
       1,  2,  3,  3,  3,  3,  2,  1,
       0,  1,  2,  2,  2,  2,  1,  0
    };
    for (int index = 0; index < NUM_SQUARES; index++) {
      knight_position_table_[index] = knight_position_table[index];
    }
    // ルークの駒の配置の重要度。
    static const int rook_position_table[NUM_SQUARES] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1
    };
    for (int index = 0; index < NUM_SQUARES; index++) {
      rook_position_table_[index] = rook_position_table[index];
    }
    // キングの中盤の駒の配置の重要度。
    static const int king_position_middle_table[NUM_SQUARES] = {
       1,  1,  0, -1, -1,  0,  1,  1,
       0,  0, -1, -2, -2, -1,  0,  0,
      -1, -1, -2, -3, -3, -2, -1, -1,
      -2, -2, -3, -4, -4, -3, -2, -2,
      -2, -2, -3, -4, -4, -3, -2, -2,
      -1, -1, -2, -3, -3, -2, -1, -1,
       0,  0, -1, -2, -2, -1,  0,  0,
       1,  1,  0, -1, -1,  0,  1,  1
    };
    for (int index = 0; index < NUM_SQUARES; index++) {
      king_position_middle_table_[index] = king_position_middle_table[index];
    }
    // キングの終盤の駒の配置の重要度。
    static const int king_position_ending_table[NUM_SQUARES] = {
      0, 1, 2, 3, 3, 2, 1, 0,
      1, 2, 3, 4, 4, 3, 2, 1,
      2, 3, 4, 5, 5, 4, 3, 2,
      3, 4, 5, 6, 6, 5, 4, 3,
      3, 4, 5, 6, 6, 5, 4, 3,
      2, 3, 4, 5, 5, 4, 3, 2,
      1, 2, 3, 4, 4, 3, 2, 1,
      0, 1, 2, 3, 3, 2, 1, 0
    };
    for (int index = 0; index < NUM_SQUARES; index++) {
      king_position_ending_table_[index] = king_position_ending_table[index];
    }

    // 駒の配置の重さ。
    pawn_position_weight_ = 10;  // ポーンの配置の重さ。
    knight_position_weight_ = 20;  // ナイトの配置の重さ。
    rook_position_weight_ = 30;  // ルークの配置の重さ。
    king_position_middle_weight_ = 50;  // キングの中盤の配置の重さ。
    king_position_ending_weight_ = 10;  // キングの終盤の配置の重さ。

    // それ以外の重さ。
    pass_pawn_weight_ = 50;  // パスポーンの重さ。
    protected_pass_pawn_weight_ = 20;  // 守られたパスポーンの重さ。
    double_pawn_weight_ = -5;  // ダブルポーンの重さ。
    iso_pawn_weight_ = -5;  // 孤立ポーンの重さ。
    bishop_pair_weight_ = 50;  // ビショップペアの重さ。
    rook_7th_weight_ = 30;  // 第7ランクのルークの重さ。
    early_queen_launched_weight_ = -30;  // 早すぎるクイーンの出動の重さ。
    pawn_shield_weight_ = 30;  // ポーンの盾の重さ。
    canceled_castling_weight_ = -50;  // キャスリングの破棄の重さ。
  }
}  // Misaki
