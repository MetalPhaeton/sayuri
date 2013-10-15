/*
   evaluator.h: 局面を評価するクラスのヘッダ。

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

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <iostream>
#include "chess_engine.h"

namespace Sayuri {
  class Evaluator {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      Evaluator(ChessEngine* engine_ptr);
      Evaluator() = delete;
      Evaluator(const Evaluator& eval);
      Evaluator(Evaluator&& eval);
      Evaluator& operator=(const Evaluator& eval);
      Evaluator& operator=(Evaluator&& eval);

      /********************/
      /* パブリック関数。 */
      /********************/
      // static変数の初期化。
      static void InitEvaluator();
      // 現在の局面の評価値を返す。
      // [戻り値]
      // 評価値。
      int Evaluate();

    private:
      /******************************/
      /* 駒の配置の重要度テーブル。 */
      /******************************/
      // 各駒の配置の価値。
      static constexpr int POSITION_TABLE[NUM_PIECE_TYPES][NUM_SQUARES] {
        {
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0
        },
        {
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          1, 2, 3, 4, 4, 3, 2, 1,
          2, 3, 4, 5, 5, 4, 3, 2,
          3, 4, 5, 6, 6, 5, 4, 3,
          4, 5, 6, 7, 7, 6, 5, 4,
          5, 6, 7, 8, 8, 7, 6, 5,
          6, 7, 8, 9, 9, 8, 7, 6
        },
        {
          -3, -2, -1, -1, -1, -1, -2, -3,
          -2, -1,  0,  0,  0,  0, -1, -2,
          -1,  0,  1,  1,  1,  1,  0, -1,
           0,  1,  2,  2,  2,  2,  1,  0,
           1,  2,  3,  3,  3,  3,  2,  1,
           2,  3,  4,  4,  4,  4,  3,  2,
           1,  2,  3,  3,  3,  3,  2,  1,
           0,  1,  2,  2,  2,  2,  1,  0
        },
        {
          -2, -1, -1, -1, -1, -1, -1, -2,
          -1,  0,  0,  0,  0,  0,  0, -1,
          -1,  1,  2,  2,  2,  2,  1, -1,
          -1,  0,  1,  2,  2,  1,  0, -1,
          -1,  0,  1,  2,  2,  1,  0, -1,
          -1,  0,  1,  2,  2,  1,  0, -1,
          -1,  0,  0,  0,  0,  0,  0, -1,
          -2, -1, -1, -1, -1, -1, -1, -2
        },
        {
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1
        },
        {
          -3, -2, -2, -1, -1, -2, -2, -3,
          -2,  0,  0,  0,  0,  0,  0, -2,
          -2,  0,  1,  1,  1,  1,  0, -2,
          -1,  0,  1,  2,  2,  1,  0, -1,
          -1,  0,  1,  2,  2,  1,  0, -1,
          -2,  0,  1,  1,  1,  1,  0, -2,
          -2,  0,  0,  0,  0,  0,  0, -2,
          -3, -2, -2, -1, -1, -2, -2, -3
        },
        {
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0
        }
      };
      // キングの中盤の駒の配置の価値。
      static constexpr int KING_POSITION_MIDDLE_TABLE[NUM_SQUARES] {
         1,  1,  0, -1, -1,  0,  1,  1,
         0,  0, -1, -2, -2, -1,  0,  0,
        -1, -1, -2, -3, -3, -2, -1, -1,
        -2, -2, -3, -4, -4, -3, -2, -2,
        -2, -2, -3, -4, -4, -3, -2, -2,
        -1, -1, -2, -3, -3, -2, -1, -1,
         0,  0, -1, -2, -2, -1,  0,  0,
         1,  1,  0, -1, -1,  0,  1,  1
      };
      // キングの終盤の駒の配置の価値。
      static constexpr int KING_POSITION_ENDING_TABLE[NUM_SQUARES] {
        0, 1, 2, 3, 3, 2, 1, 0,
        1, 2, 3, 4, 4, 3, 2, 1,
        2, 3, 4, 5, 5, 4, 3, 2,
        3, 4, 5, 6, 6, 5, 4, 3,
        3, 4, 5, 6, 6, 5, 4, 3,
        2, 3, 4, 5, 5, 4, 3, 2,
        1, 2, 3, 4, 4, 3, 2, 1,
        0, 1, 2, 3, 3, 2, 1, 0
      };

      /**********************/
      /* 全駒の評価の重さ。 */
      /**********************/
      // 機動力の重さ。
      static constexpr int WEIGHT_MOBILITY = 2;
      // センター攻撃の重さ。
      static constexpr int WEIGHT_CENTER_CONTROL = 5;
      // スウィートセンター攻撃の重さ。
      static constexpr int WEIGHT_ATTACK_SWEET_CENTER = 5;
      // 展開の重さ。
      static constexpr int WEIGHT_DEVELOPMENT = 30;
      // 敵を攻撃している重さ。
      static constexpr int WEIGHT_ATTACK_ENEMY = 10;
      // キングの周囲への攻撃の重さ。
      static constexpr int WEIGHT_ATTACK_AROUND_KING = 10;

      /********************/
      /* 駒の配置の重さ。 */
      /********************/
      // ポーンの配置の重さ。
      static constexpr int WEIGHT_PAWN_POSITION = 10;
      // ナイトの配置の重さ。
      static constexpr int WEIGHT_KNIGHT_POSITION = 10;
      // ビショップの配置の重さ。
      static constexpr int WEIGHT_BISHOP_POSITION = 20;
      // ルークの配置の重さ。
      static constexpr int WEIGHT_ROOK_POSITION = 30;
      // クイーンの配置の重さ。
      static constexpr int WEIGHT_QUEEN_POSITION = 20;
      // キングの中盤の配置の重さ。
      static constexpr int WEIGHT_KING_POSITION_MIDDLE = 30;
      // キングの終盤の配置の重さ。
      static constexpr int WEIGHT_KING_POSITION_ENDING = 10;

      /********************/
      /* それ以外の重さ。 */
      /********************/
      // パスポーンの重さ。
      static constexpr int WEIGHT_PASS_PAWN = 50;
      // 守られたパスポーンの重さ。
      static constexpr int WEIGHT_PROTECTED_PASS_PAWN = 20;
      // ダブルポーンの重さ。
      static constexpr int WEIGHT_DOUBLE_PAWN = -5;
      // 孤立ポーンの重さ。
      static constexpr int WEIGHT_ISO_PAWN = -5;
      // ビショップペアの重さ。
      static constexpr int WEIGHT_BISHOP_PAIR = 25;
      // 早すぎるクイーンの出動の重さ。
      static constexpr int WEIGHT_EARLY_QUEEN_LAUNCHED = -30;
      // ポーンの盾の重さ。
      static constexpr int WEIGHT_PAWN_SHIELD = 30;
      // キャスリングの重さ。
      static constexpr int WEIGHT_CASTLING = 50;

      /************************/
      /* 価値を計算する関数。 */
      /************************/
      // 価値の変数。
      int material_value_;  // マテリアル。
      int mobility_value_;  // 駒の動きやすさ。
      int center_control_value_;  // センター支配。
      int sweet_center_control_value_;  // センター支配。
      int development_value_;  // 駒の展開。
      int attack_enemy_value_;  // 敵を攻撃。
      int attack_around_king_value_;  // キング周辺への攻撃。
      int position_value_[NUM_PIECE_TYPES];  // 各駒の配置。
      int king_position_middle_value_;  // キングの中盤の配置。
      int king_position_ending_value_;  // キングの終盤の配置。
      int pass_pawn_value_;  // パスポーン。
      int protected_pass_pawn_value_;  // 守られたパスポーン。
      int double_pawn_value_;  // ダブルポーン。
      int iso_pawn_value_;  // 孤立ポーン。
      int bishop_pair_value_;  // ビショップペア。
      int early_queen_launched_value_;  // 早すぎるクイーンの始動。
      int pawn_shield_value_;  // ポーンの盾。
      int castling_value_;  // キャスリング。
      // 各駒での価値を計算する。
      // [引数]
      // <Type>: 計算したい駒。
      // piece_square: 駒の位置。
      // piece_side: 駒のサイド。
      template<Piece Type>
      void CalValue(Square piece_square, Side piece_side);

      /****************************/
      /* 局面評価に使用する関数。 */
      /****************************/
      // 勝つのに十分な駒があるかどうか調べる。
      // side: 調べるサイド。
      // [戻り値]
      // 十分な駒があればtrue。
      bool HasEnoughPieces(Side side);
      // 局面の進行状況を得る。
      // [戻り値]
      // 進行状況。0.0以上、1.0以下。
      // 値が小さいほどエンディングに近い。
      double GetPhase();

      /****************/
      /* 局面分析用。 */
      /****************/
      // 駒の初期位置。
      static Bitboard start_position_[NUM_SIDES][NUM_PIECE_TYPES];
      // 駒の初期位置を初期化。
      static void InitStartPosition();

      // センターを判定するときに使用するマスク。
      static Bitboard center_mask_;
      static Bitboard sweet_center_mask_;
      // センターマスクを初期化する。
      static void InitCenterMask();

      // パスポーンを判定するときに使用するマスク。
      static Bitboard pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
      // pass_pawn_mask_[][]を初期化する。
      static void InitPassPawnMask();

      // 孤立ポーンを判定するときに使用するマスク。
      static Bitboard iso_pawn_mask_[NUM_SQUARES];
      // iso_pawn_mask_[]を初期化する。
      static void InitIsoPawnMask();

      // ポーン盾の位置のマスク。
      static Bitboard pawn_shield_mask_[NUM_SIDES][NUM_SQUARES];
      // pawn_shield_mask_[][]を初期化する。
      static void InitPawnShieldMask();

      /****************/
      /* メンバ変数。 */
      /****************/
      // 使用するチェスエンジン。
      ChessEngine* engine_ptr_;
  };
}  // namespace Sayuri

#endif
