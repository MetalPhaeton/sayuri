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
  class ChessEngine;

  class Evaluator {
    public:
      /********************/
      /* コンストラクタ。 */
      /********************/
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
      // デバッグ用。
      friend class ChessEngine;

      /******************************/
      /* 駒の配置の重要度テーブル。 */
      /******************************/
      // ポーンの配置の重要度。
      static constexpr int PAWN_POSITION_TABLE[NUM_SQUARES] {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6
      };
      // ナイトの駒の配置の重要度。
      static constexpr int KNIGHT_POSITION_TABLE[NUM_SQUARES] {
        -3, -2, -1, -1, -1, -1, -2, -3,
        -2, -1,  0,  0,  0,  0, -1, -2,
        -1,  0,  1,  1,  1,  1,  0, -1,
         0,  1,  2,  2,  2,  2,  1,  0,
         1,  2,  3,  3,  3,  3,  2,  1,
         2,  3,  4,  4,  4,  4,  3,  2,
         1,  2,  3,  3,  3,  3,  2,  1,
         0,  1,  2,  2,  2,  2,  1,  0
      };
      // ルークの駒の配置の重要度。
      static constexpr int ROOK_POSITION_TABLE[NUM_SQUARES] {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1
      };
      // キングの中盤の駒の配置の重要度。
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
      // キングの終盤の駒の配置の重要度。
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
      static constexpr int WEIGHT_ATTACK_CENTER = 5;
      // スウィートセンター攻撃の重さ。
      static constexpr int WEIGHT_ATTACK_SWEET_CENTER = 5;
      // 展開の重さ。
      static constexpr int WEIGHT_DEVELOPMENT = 30;
      // キングの周囲への攻撃の重さ。
      static constexpr int WEIGHT_ATTACK_AROUND_KING = 10;

      /********************/
      /* 駒の配置の重さ。 */
      /********************/
      // ポーンの配置の重さ。
      static constexpr int WEIGHT_PAWN_POSITION = 10;
      // ナイトの配置の重さ。
      static constexpr int WEIGHT_KNIGHT_POSITION = 20;
      // ルークの配置の重さ。
      static constexpr int WEIGHT_ROOK_POSITION = 30;
      // キングの中盤の配置の重さ。
      static constexpr int WEIGHT_KING_POSITION_MIDDLE = 50;
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
      static constexpr int WEIGHT_BISHOP_PAIR = 50;
      // 早すぎるクイーンの出動の重さ。
      static constexpr int WEIGHT_EARLY_QUEEN_LAUNCHED = -30;
      // ポーンの盾の重さ。
      static constexpr int WEIGHT_PAWN_SHIELD = 30;
      // キャスリングの重さ。
      static constexpr int WEIGHT_CASTLING = 50;

      /************************/
      /* 局面を評価する関数。 */
      /************************/
      // 全てを評価する。
      // [戻り値]
      // 評価値。
      int EvalAll();
      // 機動力を評価する。
      // [戻り値]
      // 評価値。
      int EvalMobility();
      // センター攻撃を評価する。
      // [戻り値]
      // 評価値。
      int EvalAttackCenter();
      // 展開を評価する。
      // [戻り値]
      // 評価値。
      int EvalDevelopment();
      // キングの周囲への攻撃を評価する。
      // [戻り値]
      // 評価値。
      int EvalAttackAroundKing();
      // ポーンの配置を評価する。
      // [戻り値]
      // 評価値。
      int EvalPawnPosition();
      // ナイトの配置を評価する。
      // [戻り値]
      // 評価値。
      int EvalKnightPosition();
      // ルークの配置を評価する。
      // [戻り値]
      // 評価値。
      int EvalRookPosition();
      // キングの中盤の配置を評価する。
      // [戻り値]
      // 評価値。
      int EvalKingPositionMiddle();
      // キングの終盤の配置を評価する。
      // [戻り値]
      // 評価値。
      int EvalKingPositionEnding();
      // パスポーンを評価する。
      // [戻り値]
      // 評価値。
      int EvalPassPawn();
      // ダブルポーンを評価する。
      // [戻り値]
      // 評価値。
      int EvalDoublePawn();
      // 孤立ポーンを評価する。
      // [戻り値]
      // 評価値。
      int EvalIsoPawn();
      // ビショップペアを評価する。
      // [戻り値]
      // 評価値。
      int EvalBishopPair();
      // 早すぎるクイーンの出動を評価する。
      // [戻り値]
      // 評価値。
      int EvalEarlyQueenLaunched();
      // ポーンの盾を評価する。
      // [戻り値]
      // 評価値。
      int EvalPawnShield();
      // キャスリングの評価する。
      // [戻り値]
      // 評価値。
      int EvalCastling();

      /****************************/
      /* 局面評価に使用する関数。 */
      /****************************/
      // 勝つのに十分な駒があるかどうか調べる。
      // side: 調べるサイド。
      // [戻り値]
      // 十分な駒があればtrue。
      bool HasEnoughPieces(Side side);
      // 動ける位置の数を得る。
      // [引数]
      // piece_square: 調べたい駒の位置。
      // [戻り値]
      // 動ける位置の数。
      int GetMobility(Square piece_square);
      // パスポーンの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // パスポーンの位置のビットボード。
      Bitboard GetPassPawns(Side side);
      // ダブルポーンの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // ダブルポーンの位置のビットボード。
      Bitboard GetDoublePawns(Side side);
      // 孤立ポーンの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // 孤立ポーンの位置のビットボード。
      Bitboard GetIsoPawns(Side side);

      /****************/
      /* 局面分析用。 */
      /****************/
      // テーブルを計算する。
      // [引数]
      // table: 計算するテーブル。
      // side: 計算したいサイド。
      // bitboard: 位置のビットボード。
      // [戻り値]
      // テーブルを計算した結果の値。
      static int GetTableValue(const int (& table)[NUM_SQUARES], Side side,
      Bitboard bitboard);

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
