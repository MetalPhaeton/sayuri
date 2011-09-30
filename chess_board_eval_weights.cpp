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

    // 駒の配置の重さ。
    pawn_position_weight_ = 10;  // ポーンの配置の重さ。
    knight_position_weight_ = 20;  // ナイトの配置の重さ。
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
