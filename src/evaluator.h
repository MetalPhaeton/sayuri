/*
   evaluator.h: 局面を評価するクラスのヘッダ。

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

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <iostream>
#include "chess_engine.h"
#include "params.h"

namespace Sayuri {
  class Evaluator {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // [引数]
      // engine: 評価したいエンジン。
      Evaluator(const ChessEngine& engine);
      Evaluator(const Evaluator& eval);
      Evaluator(Evaluator&& eval);
      Evaluator& operator=(const Evaluator& eval);
      Evaluator& operator=(Evaluator&& eval);
      Evaluator() = delete;
      virtual ~Evaluator() {}

      /*****************************/
      /* Evaluatorクラスの初期化。 */
      /*****************************/
      // static変数の初期化。
      static void InitEvaluator();

      /********************/
      /* パブリック関数。 */
      /********************/
      // 現在の局面の評価値を返す。
      // [引数]
      // material: 現在の局面のマテリアル。
      // [戻り値]
      // 評価値。
      int Evaluate(int material);

    private:
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

      // 弱いマスのマスク。
      static Bitboard weak_square_mask_[NUM_SIDES][NUM_SQUARES];
      // weak_square_mask_[][]を初期化する。
      static void InitWeakSquareMask();

      /****************/
      /* メンバ変数。 */
      /****************/
      // 使用するチェスエンジン。
      const ChessEngine* engine_ptr_;
      // 価値の変数。
      // オープニング時の駒の配置。
      double opening_position_value_[NUM_PIECE_TYPES];
      // エンディング時の駒の配置。
      double ending_position_value_[NUM_PIECE_TYPES];
      double mobility_value_;  // 機動力。
      double center_control_value_;  // センターコントロール。
      // スウィートセンターのコントロール。
      double sweet_center_control_value_;
      double development_value_;  // 駒の展開。
      double attack_value_[NUM_PIECE_TYPES];  // 攻撃。
      double attack_around_king_value_;  // 相手キング周辺への攻撃。
      double pass_pawn_value_;  // パスポーン。
      double protected_pass_pawn_value_;  // 守られたパスポーン。
      double double_pawn_value_;  // ダブルポーン。
      double iso_pawn_value_;  // 孤立ポーン。
      double pawn_shield_value_;  // ポーンの盾。
      double bishop_pair_value_;  // ビショップペア。
      double bad_bishop_value_;  // バッドビショップ。
      double pin_knight_value_;  // ナイトをピン。
      double rook_pair_value_;  // ルークペア。
      double rook_semiopen_fyle_value_;  // セミオープンファイルのルーク。
      double rook_open_fyle_value_;  // オープンファイルのルーク。
      double early_queen_launched_value_;  // 早すぎるクイーンの始動。
      double weak_square_value_;  // キング周りの弱いマス。
      double castling_value_;  // キャスリング。
      double abandoned_castling_value_;  // キャスリングの放棄。
  };
}  // namespace Sayuri

#endif
