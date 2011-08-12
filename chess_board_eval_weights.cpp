/* chess_board_eval_weights.cpp: 評価の重み。
   copyright (c) 2011 石橋宏之利
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
    early_king_launched_weight_ = -50;  // 早すぎるキングの出動の重さ。
  }
}  // Misaki
