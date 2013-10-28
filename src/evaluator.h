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
      // 後処理。
      static void PostprocessEvaluator();
      // 現在の局面の評価値を返す。
      // [戻り値]
      // 評価値。
      int Evaluate();

    private:
      /******************************/
      /* 評価のウェイト定数クラス。 */
      /******************************/
      class Weight {
        public:
          /**************************/
          /* コンストラクタと代入。 */
          /**************************/
          // [引数]
          // openint_weight: ピース(ポーン以外の駒)が14個の時のウェイト。
          // ending_weight: ピース(ポーン以外の駒)が0個の時のウェイト。
          Weight(double opening_weight, double ending_weight) :
          modulus_((opening_weight - ending_weight) / 14.0),
          shift_(ending_weight) {}

          // その他のコンストラクタと代入。
          Weight() = delete;
          Weight(const Weight&) = delete;
          Weight(Weight&&) = delete;
          Weight& operator=(const Weight&) = delete;
          Weight& operator=(Weight&&) = delete;

          /********************/
          /* パブリック関数。 */
          /********************/
          // 評価値を局面のフェーズの一次関数で計算して返す。
          // [引数]
          // num_pieces: ピース(ポーン以外の駒)の数。
          // value: ウェイトに掛ける評価値の元になる値。
          // [戻り値]
          // 評価値。
          double GetScore(double num_pieces, int value) {
            return ((modulus_ * num_pieces) + shift_)
            * static_cast<double>(value);
          }

        private:
          double modulus_;
          double shift_;
      };

      /******************************/
      /* 駒の配置の重要度テーブル。 */
      /******************************/
      // 各駒の配置の価値。
      static constexpr int POSITION_TABLE[NUM_PIECE_TYPES][NUM_SQUARES] {
        {  // 何もなし。
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // ポーン。
          0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0,
          1, 2, 3, 4, 4, 3, 2, 1,
          2, 3, 4, 5, 5, 4, 3, 2,
          3, 4, 5, 6, 6, 5, 4, 3,
          4, 5, 6, 7, 7, 6, 5, 4,
          5, 6, 7, 8, 8, 7, 6, 5,
          6, 7, 8, 9, 9, 8, 7, 6
        },
        {  // ナイト。
          -3, -2, -1, -1, -1, -1, -2, -3,
          -2, -1,  0,  0,  0,  0, -1, -2,
          -1,  0,  1,  1,  1,  1,  0, -1,
           0,  1,  2,  2,  2,  2,  1,  0,
           1,  2,  3,  3,  3,  3,  2,  1,
           2,  3,  4,  4,  4,  4,  3,  2,
           1,  2,  3,  3,  3,  3,  2,  1,
           0,  1,  2,  2,  2,  2,  1,  0
        },
        {  // ビショップ。
          1, 0, 0, 0, 0, 0, 0, 1,
          0, 3, 0, 2, 2, 0, 3, 0,
          0, 1, 2, 1, 1, 2, 1, 0,
          0, 0, 3, 2, 2, 3, 0, 0,
          0, 3, 2, 3, 3, 2, 3, 0,
          1, 2, 2, 2, 2, 2, 2, 1,
          0, 1, 1, 1, 1, 1, 1, 0,
          1, 0, 0, 0, 0, 0, 0, 1
        },
        {  // ルーク。
          0, 1, 2, 3, 3, 2, 1, 0,
          0, 1, 2, 3, 3, 2, 1, 0,
          0, 1, 2, 3, 3, 2, 1, 0,
          0, 1, 2, 3, 3, 2, 1, 0,
          0, 1, 2, 3, 3, 2, 1, 0,
          0, 1, 2, 3, 3, 2, 1, 0,
          4, 4, 4, 4, 4, 4, 4, 4,
          4, 4, 4, 4, 4, 4, 4, 4
        },
        {  // クイーン。
          -3, -2, -2, -1, -1, -2, -2, -3,
          -2,  0,  0,  0,  0,  0,  0, -2,
          -2,  0,  1,  1,  1,  1,  0, -2,
          -1,  0,  1,  2,  2,  1,  0, -1,
          -1,  0,  1,  2,  2,  1,  0, -1,
          -2,  0,  1,  1,  1,  1,  0, -2,
          -2,  0,  0,  0,  0,  0,  0, -2,
          -3, -2, -2, -1, -1, -2, -2, -3
        },
        {  // キング。
           1,  1,  0, -1, -1,  0,  1,  1,
           0,  0, -1, -2, -2, -1,  0,  0,
          -1, -1, -2, -3, -3, -2, -1, -1,
          -2, -2, -3, -4, -4, -3, -2, -2,
          -2, -2, -3, -4, -4, -3, -2, -2,
          -1, -1, -2, -3, -3, -2, -1, -1,
           0,  0, -1, -2, -2, -1,  0,  0,
           1,  1,  0, -1, -1,  0,  1,  1
        }
      };

      // ポーンの終盤の駒の配置の価値。
      static constexpr int PAWN_POSITION_ENDING_TABLE[NUM_SQUARES] {
        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 1, 1, 
        2, 2, 2, 2, 2, 2, 2, 2, 
        3, 3, 3, 3, 3, 3, 3, 3, 
        4, 4, 4, 4, 4, 4, 4, 4, 
        5, 5, 5, 5, 5, 5, 5, 5, 
        6, 6, 6, 6, 6, 6, 6, 6 
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
      /* その他のテーブル。 */
      /**********************/
      // 駒への攻撃の価値テーブル。
      // ATTACK_VALUE_TABLE[攻撃側の駒の種類][ターゲットの駒の種類]。
      static constexpr int ATTACK_VALUE_TABLE
      [NUM_PIECE_TYPES][NUM_PIECE_TYPES] {
        {0, 0, 0, 0, 0, 0, 0},  // 攻撃側: Empty。
        {0, 0, 2, 2, 3, 4, 3},  // 攻撃側: ポーン。
        {0, 1, 0, 2, 3, 4, 3},  // 攻撃側: ナイト。
        {0, 1, 2, 0, 4, 4, 3},  // 攻撃側: ビショップ。
        {0, 1, 2, 2, 0, 4, 4},  // 攻撃側: ルーク。
        {0, 1, 2, 2, 3, 0, 4},  // 攻撃側: クイーン。
        {0, 3, 1, 1, 1, 1, 0}   // 攻撃側: キング。
      };

      /************************/
      /* 価値を計算する関数。 */
      /************************/
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
      bool HasEnoughPieces(Side side) const;
      // 局面の進行状況を得る。
      // [戻り値]
      // 進行状況。0.0以上、1.0以下。
      // 値が小さいほどエンディングに近い。
      double GetPhase() const;

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

      /************************************************/
      /* 各ウェイト。                                 */
      /* 設定はevaluator.cppのInitEvaluator()でやる。 */
      /************************************************/
      // ポーンの配置。
      static Weight* weight_pawn_position_;
      // ナイトの配置。
      static Weight* weight_knight_position_;
      // ビショップの配置。
      static Weight* weight_bishop_position_;
      // ルークの配置。
      static Weight* weight_rook_position_;
      // クイーンの配置。
      static Weight* weight_queen_position_;
      // キングの配置。
      static Weight* weight_king_position_;
      // 終盤のポーンの配置。
      static Weight* weight_pawn_position_ending_;
      // 終盤のキングの配置。
      static Weight* weight_king_position_ending_;
      // 機動力。
      static Weight* weight_mobility_;
      // センターコントロール。
      static Weight* weight_center_control_;
      // スウィートセンターのコントロール。
      static Weight* weight_sweet_center_control_;
      // 駒の展開。
      static Weight* weight_development_;
      // 攻撃。
      static Weight* weight_attack_;
      // キングによる攻撃。
      static Weight* weight_attack_by_king_;
      // パスポーン。
      static Weight* weight_pass_pawn_;
      // 守られたパスポーン。
      static Weight* weight_protected_pass_pawn_;
      // ダブルポーン。
      static Weight* weight_double_pawn_;
      // 孤立ポーン。
      static Weight* weight_iso_pawn_;
      // ビショップペア。
      static Weight* weight_bishop_pair_;
      // 早すぎるクイーンの始動。
      static Weight* weight_early_queen_launched_;
      // ポーンの盾。
      static Weight* weight_pawn_shield_;
      // キャスリング。(これの2倍が評価値。)
      static Weight* weight_castling_;

      /****************/
      /* メンバ変数。 */
      /****************/
      // 使用するチェスエンジン。
      ChessEngine* engine_ptr_;
      // 価値の変数。
      int position_value_[NUM_PIECE_TYPES];  // 各駒の配置。
      int pawn_position_ending_value_;  //  ポーンの終盤の配置。
      int king_position_ending_value_;  //  キングの終盤の配置。
      int mobility_value_;  // 駒の動きやすさ。
      int center_control_value_;  // センターコントロール。
      int sweet_center_control_value_;  // スウィートセンターのコントロール。
      int development_value_;  // 駒の展開。
      int attack_value_[NUM_PIECE_TYPES];  // 攻撃。
      int pass_pawn_value_;  // パスポーン。
      int protected_pass_pawn_value_;  // 守られたパスポーン。
      int double_pawn_value_;  // ダブルポーン。
      int iso_pawn_value_;  // 孤立ポーン。
      int bishop_pair_value_;  // ビショップペア。
      int early_queen_launched_value_;  // 早すぎるクイーンの始動。
      int pawn_shield_value_;  // ポーンの盾。
      int castling_value_;  // キャスリング。
  };
}  // namespace Sayuri

#endif
