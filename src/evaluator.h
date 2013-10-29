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

      /*****************************/
      /* Evaluatorクラスの初期化。 */
      /*****************************/
      // static変数の初期化。
      static void InitEvaluator();

      /********************/
      /* パブリック関数。 */
      /********************/
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
          double GetScore(double num_pieces, double value) const {
            return ((modulus_ * num_pieces) + shift_) * value;
          }

        private:
          double modulus_;
          double shift_;
      };

      /****************/
      /* 各ウェイト。 */
      /****************/
      // ポーンの配置。
      static const Weight WEIGHT_PAWN_POSITION;
      // ナイトの配置。
      static const Weight WEIGHT_KNIGHT_POSITION;
      // ビショップの配置。
      static const Weight WEIGHT_BISHOP_POSITION;
      // ルークの配置。
      static const Weight WEIGHT_ROOK_POSITION;
      // クイーンの配置。
      static const Weight WEIGHT_QUEEN_POSITION;
      // キングの配置。
      static const Weight WEIGHT_KING_POSITION;
      // 終盤のポーンの配置。
      static const Weight WEIGHT_PAWN_POSITION_ENDING;
      // 終盤のキングの配置。
      static const Weight WEIGHT_KING_POSITION_ENDING;
      // 機動力。
      static const Weight WEIGHT_MOBILITY;
      // センターコントロール。
      static const Weight WEIGHT_CENTER_CONTROL;
      // スウィートセンターのコントロール。
      static const Weight WEIGHT_SWEET_CENTER_CONTROL;
      // 駒の展開。
      static const Weight WEIGHT_DEVELOPMENT;
      // 攻撃。
      static const Weight WEIGHT_ATTACK;
      // キングによる攻撃。
      static const Weight WEIGHT_ATTACK_BY_KING;
      // パスポーン。
      static const Weight WEIGHT_PASS_PAWN;
      // 守られたパスポーン。
      static const Weight WEIGHT_PROTECTED_PASS_PAWN;
      // ダブルポーン。
      static const Weight WEIGHT_DOUBLE_PAWN;
      // 孤立ポーン。
      static const Weight WEIGHT_ISO_PAWN;
      // ビショップペア。
      static const Weight WEIGHT_BISHOP_PAIR;
      // 早すぎるクイーンの始動。
      static const Weight WEIGHT_EARLY_QUEEN_LAUNCHED;
      // ポーンの盾。
      static const Weight WEIGHT_PAWN_SHIELD;
      // キャスリング。(これの2倍が評価値。)
      static const Weight WEIGHT_CASTLING;

      /******************************/
      /* 駒の配置の重要度テーブル。 */
      /******************************/
      // 各駒の配置の価値。
      static constexpr double POSITION_TABLE[NUM_PIECE_TYPES][NUM_SQUARES] {
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
          1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0,
          0.0, 3.0, 0.0, 2.0, 2.0, 0.0, 3.0, 0.0,
          0.0, 1.0, 2.0, 1.0, 1.0, 2.0, 1.0, 0.0,
          0.0, 0.0, 3.0, 2.0, 2.0, 3.0, 0.0, 0.0,
          0.0, 3.0, 2.0, 3.0, 3.0, 2.0, 3.0, 0.0,
          1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 1.0,
          0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,
          1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0
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

      // ポーンの終盤の駒の配置の価値。
      static constexpr double PAWN_POSITION_ENDING_TABLE[NUM_SQUARES] {
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 
        3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 
        4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 
        5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 
        6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0 
      };

      // キングの終盤の駒の配置の価値。
      static constexpr double KING_POSITION_ENDING_TABLE[NUM_SQUARES] {
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0,
        2.0, 3.0, 4.0, 5.0, 5.0, 4.0, 3.0, 2.0,
        3.0, 4.0, 5.0, 6.0, 6.0, 5.0, 4.0, 3.0,
        3.0, 4.0, 5.0, 6.0, 6.0, 5.0, 4.0, 3.0,
        2.0, 3.0, 4.0, 5.0, 5.0, 4.0, 3.0, 2.0,
        1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0
      };

      /**********************/
      /* その他のテーブル。 */
      /**********************/
      // 駒への攻撃の価値テーブル。
      // ATTACK_VALUE_TABLE[攻撃側の駒の種類][ターゲットの駒の種類]。
      static constexpr double ATTACK_VALUE_TABLE
      [NUM_PIECE_TYPES][NUM_PIECE_TYPES] {
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 攻撃側: Empty。
        {0.0, 0.0, 2.0, 2.0, 3.0, 4.0, 3.0},  // 攻撃側: ポーン。
        {0.0, 1.0, 0.0, 2.0, 3.0, 4.0, 3.0},  // 攻撃側: ナイト。
        {0.0, 1.0, 2.0, 0.0, 4.0, 4.0, 3.0},  // 攻撃側: ビショップ。
        {0.0, 1.0, 2.0, 2.0, 0.0, 4.0, 4.0},  // 攻撃側: ルーク。
        {0.0, 1.0, 2.0, 2.0, 3.0, 0.0, 4.0},  // 攻撃側: クイーン。
        {0.0, 3.0, 1.0, 1.0, 1.0, 1.0, 0.0}   // 攻撃側: キング。
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

      /****************/
      /* メンバ変数。 */
      /****************/
      // 使用するチェスエンジン。
      ChessEngine* engine_ptr_;
      // 価値の変数。
      double position_value_[NUM_PIECE_TYPES];  // 各駒の配置。
      double pawn_position_ending_value_;  //  ポーンの終盤の配置。
      double king_position_ending_value_;  //  キングの終盤の配置。
      double mobility_value_;  // 駒の動きやすさ。
      double center_control_value_;  // センターコントロール。
      double sweet_center_control_value_;  // スウィートセンターのコントロール。
      double development_value_;  // 駒の展開。
      double attack_value_[NUM_PIECE_TYPES];  // 攻撃。
      double pass_pawn_value_;  // パスポーン。
      double protected_pass_pawn_value_;  // 守られたパスポーン。
      double double_pawn_value_;  // ダブルポーン。
      double iso_pawn_value_;  // 孤立ポーン。
      double bishop_pair_value_;  // ビショップペア。
      double early_queen_launched_value_;  // 早すぎるクイーンの始動。
      double pawn_shield_value_;  // ポーンの盾。
      double castling_value_;  // キャスリング。
  };
}  // namespace Sayuri

#endif
