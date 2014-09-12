/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file evaluator.h
 * @author Hironori Ishibashi
 * @brief 評価関数クラス。
 */

#ifndef EVALUATOR_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define EVALUATOR_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <cstring>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;

  /** 評価関数クラス。 */
  class Evaluator {
    private:
      // --- クラス内で使う定数 --- //
      /** 価値テーブルのインデックス - オープニングの配置。 */
      constexpr static unsigned int OPENING_POSITION = 0;
      /** 価値テーブルのインデックス - エンディングの配置。 */
      constexpr static unsigned int ENDING_POSITION = 1;
      /** 価値テーブルのインデックス - 機動力。 */
      constexpr static unsigned int MOBILITY = 2;
      /** 価値テーブルのインデックス - センターコントロール。 */
      constexpr static unsigned int CENTER_CONTROL = 3;
      /** 価値テーブルのインデックス - スウィートセンターのコントロール。 */
      constexpr static unsigned int SWEET_CENTER_CONTROL = 4;
      /** 価値テーブルのインデックス - ピースの展開。 */
      constexpr static unsigned int DEVELOPMENT = 5;
      /** 価値テーブルのインデックス - 攻撃。 */
      constexpr static unsigned int ATTACK = 6;
      /** 価値テーブルのインデックス - 防御。 */
      constexpr static unsigned int DEFENSE = 7;
      /** 価値テーブルのインデックス - ピン。 */
      constexpr static unsigned int PIN = 8;
      /** 価値テーブルのインデックス - キング周辺への攻撃。 */
      constexpr static unsigned int ATTACK_AROUND_KING = 9;
      /** 価値テーブルのインデックス - パスポーン。 */
      constexpr static unsigned int PASS_PAWN = 10;
      /** 価値テーブルのインデックス - 守られたパスポーン。 */
      constexpr static unsigned int PROTECTED_PASS_PAWN = 11;
      /** 価値テーブルのインデックス - ダブルポーン。 */
      constexpr static unsigned int DOUBLE_PAWN = 12;
      /** 価値テーブルのインデックス - 孤立ポーン。 */
      constexpr static unsigned int ISO_PAWN = 13;
      /** 価値テーブルのインデックス - ポーンの盾。 */
      constexpr static unsigned int PAWN_SHIELD = 14;
      /** 価値テーブルのインデックス - ビショップペア。 */
      constexpr static unsigned int BISHOP_PAIR = 15;
      /** 価値テーブルのインデックス - バッドビショップ。 */
      constexpr static unsigned int BAD_BISHOP = 16;
      /** 価値テーブルのインデックス - ルークペア。 */
      constexpr static unsigned int ROOK_PAIR = 17;
      /** 価値テーブルのインデックス - セミオープンファイルのルーク。 */
      constexpr static unsigned int ROOK_SEMIOPEN_FYLE = 18;
      /** 価値テーブルのインデックス - オープンファイルのルーク。 */
      constexpr static unsigned int ROOK_OPEN_FYLE = 19;
      /** 価値テーブルのインデックス - 早すぎるクイーンの始動。 */
      constexpr static unsigned int EARLY_QUEEN_LAUNCHED = 20;
      /** 価値テーブルのインデックス - キング周りの弱いマス。 */
      constexpr static unsigned int WEAK_SQUARE = 21;
      /** 価値テーブルのインデックス - キャスリング。 */
      constexpr static unsigned int CASTLING = 22;
      /** 価値テーブルのインデックス - キャスリングの放棄。 */
      constexpr static unsigned int ABANDONED_CASTLING = 23;

      /** 価値テーブルのサイズ。 */
      constexpr static std::size_t TABLE_SIZE = ABANDONED_CASTLING + 1;

    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param engine 評価したいエンジン。
       */
      Evaluator(const ChessEngine& engine);
      /** 
       * コピーコンストラクタ。
       * @param eval コピー元。
       */
      Evaluator(const Evaluator& eval);
      /** 
       * ムーブコンストラクタ。
       * @param eval ムーブ元。
       */
      Evaluator(Evaluator&& eval);
      /** 
       * コピー代入演算子。
       * @param eval コピー元。
       */
      Evaluator& operator=(const Evaluator& eval);
      /** 
       * ムーブ代入演算子。
       * @param eval ムーブ元。
       */
      Evaluator& operator=(Evaluator&& eval);
      /** コンストラクタ。 (削除) */
      Evaluator() = delete;
      /** デストラクタ。 */
      virtual ~Evaluator() {}

      // ======================= //
      // Evaluatorクラスの初期化 //
      // ======================= //
      /** static変数の初期化。 */
      static void InitEvaluator();

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 現在の局面の評価値を計算する。
       * @param material 現在のマテリアル。
       * @return 評価値。
       */
      int Evaluate(int material);

    private:
      /** フレンドのデバッグ用関数。 */
      friend int DebugMain(int argc, char* argv[]);

      /** フレンドのデバッグ用関数。 */
      friend void PrintValueTable(const Evaluator& evaluator);

      // ================== //
      // 価値を計算する関数 //
      // ================== //
      /**
       * 各駒の価値を計算する。
       * @param <Side> 駒のサイド。
       * @param <Type> 駒の種類。
       * @param piece_square 駒のいるマス。
       * @param piece_side 駒のサイド。
       */
      template<Side PSide, Piece PType>
      void CalValue(Square piece_square);

      // --- テンプレート部品 --- //
      template<Side PSide>
      friend struct AddOrSub;
      template<Side PSide, Piece PType>
      friend struct GenBitboards;
      template<Side PSide, Piece PType>
      friend struct CalPosition;
      template<Side PSide, Piece PType>
      friend struct CalMobility;
      template<Side PSide, Piece PType>
      friend struct GenPinTargets;
      template<Side PSide, Piece PType>
      friend struct CalSpecial;

      // ========== //
      // 局面分析用 //
      // ========== //
      /** 駒の初期位置のビットボード。 [サイド][駒の種類] */
      static Bitboard start_position_[NUM_SIDES][NUM_PIECE_TYPES];
      /** start_position_[][]を初期化。 */
      static void InitStartPosition();

      /** センターのマスク。 */
      static Bitboard center_mask_;
      /** スウィートセンターのマスク。 */
      static Bitboard sweet_center_mask_;
      /** center_mask_、sweet_center_mask_を初期化する。 */
      static void InitCenterMask();

      /** パスポーンの前方3列のマスク。 [サイド][マス] */
      static Bitboard pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
      /** pass_pawn_mask_[][]を初期化する。 */
      static void InitPassPawnMask();

      /** 孤立ポーンの両脇ファイルのマスク。 [マス] */
      static Bitboard iso_pawn_mask_[NUM_SQUARES];
      /** iso_pawn_mask_[]を初期化する。 */
      static void InitIsoPawnMask();

      /** ポーン盾の位置のマスク。 [サイド][キングの位置] */
      static Bitboard pawn_shield_mask_[NUM_SIDES][NUM_SQUARES];
      /** pawn_shield_mask_[][]を初期化する。 */
      static void InitPawnShieldMask();

      /** 弱いマスのマスク。 [サイド][キングの位置] */
      static Bitboard weak_square_mask_[NUM_SIDES][NUM_SQUARES];
      /** weak_square_mask_[][]を初期化する。 */
      static void InitWeakSquareMask();

      // ========== //
      // メンバ変数 //
      // ========== //
      /** 使用するチェスエンジン。 */
      const ChessEngine* engine_ptr_;

      /** 評価関数で使う価値テーブル。 */
      double value_table_[TABLE_SIZE][NUM_PIECE_TYPES];
  };
}  // namespace Sayuri

#endif
