/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Hironori Ishibashi
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
  struct EvalCache;

  /** 評価関数クラス。 */
  class Evaluator {
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
      template<Side SIDE, PieceType TYPE>
      void CalValue(Square piece_square);

      // --- テンプレート部品 --- //
      /** 評価関数で使うテンプレート部品。 */
      template<Side SIDE, PieceType TYPE>
      friend struct GenBitboards;
      /** 評価関数で使うテンプレート部品。 */
      template<Side SIDE, PieceType TYPE>
      friend struct CalPosition;
      /** 評価関数で使うテンプレート部品。 */
      template<Side SIDE, PieceType TYPE>
      friend struct CalMobility;
      /** 評価関数で使うテンプレート部品。 */
      template<Side SIDE, PieceType TYPE>
      friend struct GenPinTargets;
      /** 評価関数で使うテンプレート部品。 */
      template<Side SIDE, PieceType TYPE>
      friend struct CalSpecial;

      // ========== //
      // 局面分析用 //
      // ========== //
      /** 駒の初期位置のビットボード。 [サイド][駒の種類] */
      static constexpr Bitboard START_POSITION[NUM_SIDES][NUM_PIECE_TYPES] {
        {0, 0, 0, 0, 0, 0, 0},
        {
          0, Util::RANK[RANK_2],
          Util::SQUARE[B1][R0] | Util::SQUARE[G1][R0],
          Util::SQUARE[C1][R0] | Util::SQUARE[F1][R0],
          Util::SQUARE[A1][R0] | Util::SQUARE[H1][R0],
          Util::SQUARE[D1][R0], Util::SQUARE[E1][R0]
        },
        {
          0, Util::RANK[RANK_7],
          Util::SQUARE[B8][R0] | Util::SQUARE[G8][R0],
          Util::SQUARE[C8][R0] | Util::SQUARE[F8][R0],
          Util::SQUARE[A8][R0] | Util::SQUARE[H8][R0],
          Util::SQUARE[D8][R0], Util::SQUARE[E8][R0]
        }
      };
      static constexpr Bitboard NOT_START_POSITION
      [NUM_SIDES][NUM_PIECE_TYPES] {
        {0, 0, 0, 0, 0, 0, 0},
        {
          0, ~START_POSITION[WHITE][PAWN],
          ~START_POSITION[WHITE][KNIGHT],
          ~START_POSITION[WHITE][BISHOP],
          ~START_POSITION[WHITE][ROOK],
          ~START_POSITION[WHITE][QUEEN],
          ~START_POSITION[WHITE][KING]
        },
        {
          0, ~START_POSITION[BLACK][PAWN],
          ~START_POSITION[BLACK][KNIGHT],
          ~START_POSITION[BLACK][BISHOP],
          ~START_POSITION[BLACK][ROOK],
          ~START_POSITION[BLACK][QUEEN],
          ~START_POSITION[BLACK][KING]
        }
      };

      /** センターのマスク。 */
      static constexpr Bitboard CENTER_MASK = 
      Util::SQUARE[C3][R0] | Util::SQUARE[C4][R0]
      | Util::SQUARE[C5][R0] | Util::SQUARE[C6][R0]
      | Util::SQUARE[D3][R0] | Util::SQUARE[D4][R0]
      | Util::SQUARE[D5][R0] | Util::SQUARE[D6][R0]
      | Util::SQUARE[E3][R0] | Util::SQUARE[E4][R0]
      | Util::SQUARE[E5][R0] | Util::SQUARE[E6][R0]
      | Util::SQUARE[F3][R0] | Util::SQUARE[F4][R0]
      | Util::SQUARE[F5][R0] | Util::SQUARE[F6][R0];
      /** スウィートセンターのマスク。 */
      static constexpr Bitboard SWEET_CENTER_MASK =
      Util::SQUARE[D4][R0] | Util::SQUARE[D5][R0]
      | Util::SQUARE[E4][R0] | Util::SQUARE[E5][R0];

      /** パスポーンの前方3列のマスク。 [サイド][マス] */
      static Bitboard pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
      /** pass_pawn_mask_[][]を初期化する。 */
      static void InitPassPawnMask();

      /** 孤立ポーンの両脇ファイルのマスク。 [マス] */
      static Bitboard iso_pawn_mask_[NUM_SQUARES];
      /** iso_pawn_mask_[]を初期化する。 */
      static void InitIsoPawnMask();

      /** クイーンサイドのポーンシールドのマスク。 */
      static constexpr Bitboard QUEEN_SIDE_SHIELD_MASK =
      Util::FYLE[FYLE_A] | Util::FYLE[FYLE_B] | Util::FYLE[FYLE_C];
      /** キングサイドのポーンシールドのマスク。 */
      static constexpr Bitboard KING_SIDE_SHIELD_MASK =
      Util::FYLE[FYLE_F] | Util::FYLE[FYLE_G] | Util::FYLE[FYLE_H];
      /** ポーンシールドのマスク。 */
      static constexpr Bitboard PAWN_SHIELD_MASK[NUM_SIDES][NUM_SQUARES] {
        {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {
          QUEEN_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          0, 0,
          KING_SIDE_SHIELD_MASK,
          KING_SIDE_SHIELD_MASK,
          KING_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          0, 0,
          KING_SIDE_SHIELD_MASK,
          KING_SIDE_SHIELD_MASK,
          KING_SIDE_SHIELD_MASK,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          QUEEN_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          0, 0,
          KING_SIDE_SHIELD_MASK,
          KING_SIDE_SHIELD_MASK,
          KING_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          QUEEN_SIDE_SHIELD_MASK,
          0, 0,
          KING_SIDE_SHIELD_MASK,
          KING_SIDE_SHIELD_MASK,
          KING_SIDE_SHIELD_MASK
        }
      };

      /** 白のクイーンサイドの弱いマスのマスク。 */
      static constexpr Bitboard WHITE_QUEEN_SIDE_WEAK_MASK =
      (Util::FYLE[FYLE_A] | Util::FYLE[FYLE_B] | Util::FYLE[FYLE_C])
      & (Util::RANK[RANK_2] | Util::RANK[RANK_3]);
      /** 白のキングサイドの弱いマスのマスク。 */
      static constexpr Bitboard WHITE_KING_SIDE_WEAK_MASK =
      (Util::FYLE[FYLE_F] | Util::FYLE[FYLE_G] | Util::FYLE[FYLE_H])
      & (Util::RANK[RANK_2] | Util::RANK[RANK_3]);
      /** 黒のクイーンサイドの弱いマスのマスク。 */
      static constexpr Bitboard BLACK_QUEEN_SIDE_WEAK_MASK =
      (Util::FYLE[FYLE_A] | Util::FYLE[FYLE_B] | Util::FYLE[FYLE_C])
      & (Util::RANK[RANK_7] | Util::RANK[RANK_6]);
      /** 黒のキングサイドの弱いマスのマスク。 */
      static constexpr Bitboard BLACK_KING_SIDE_WEAK_MASK =
      (Util::FYLE[FYLE_F] | Util::FYLE[FYLE_G] | Util::FYLE[FYLE_H])
      & (Util::RANK[RANK_7] | Util::RANK[RANK_6]);
      /** 弱いマスのマスク。 [サイド][キングの位置] */
      static constexpr Bitboard WEAK_SQUARE_MASK[NUM_SIDES][NUM_SQUARES] {
        {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {
          WHITE_QUEEN_SIDE_WEAK_MASK,
          WHITE_QUEEN_SIDE_WEAK_MASK,
          WHITE_QUEEN_SIDE_WEAK_MASK,
          0, 0,
          WHITE_KING_SIDE_WEAK_MASK,
          WHITE_KING_SIDE_WEAK_MASK,
          WHITE_KING_SIDE_WEAK_MASK,
          WHITE_QUEEN_SIDE_WEAK_MASK,
          WHITE_QUEEN_SIDE_WEAK_MASK,
          WHITE_QUEEN_SIDE_WEAK_MASK,
          0, 0,
          WHITE_KING_SIDE_WEAK_MASK,
          WHITE_KING_SIDE_WEAK_MASK,
          WHITE_KING_SIDE_WEAK_MASK,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          BLACK_QUEEN_SIDE_WEAK_MASK,
          BLACK_QUEEN_SIDE_WEAK_MASK,
          BLACK_QUEEN_SIDE_WEAK_MASK,
          0, 0,
          BLACK_KING_SIDE_WEAK_MASK,
          BLACK_KING_SIDE_WEAK_MASK,
          BLACK_KING_SIDE_WEAK_MASK,
          BLACK_QUEEN_SIDE_WEAK_MASK,
          BLACK_QUEEN_SIDE_WEAK_MASK,
          BLACK_QUEEN_SIDE_WEAK_MASK,
          0, 0,
          BLACK_KING_SIDE_WEAK_MASK,
          BLACK_KING_SIDE_WEAK_MASK,
          BLACK_KING_SIDE_WEAK_MASK
        }
      };

      // --- ピン用便利ツール --- //
      /**
       * ピン用のテーブル。
       * マス、パターン、方向を指定すると、
       * ピンバックのビットボードが得られる。 [マス][マジックパターン][角度]
       */
      static Bitboard pin_back_table_[NUM_SQUARES][0xff + 1][NUM_ROTS];
      /** pin_table[][][][]を初期化する。 */
      static void InitPinBackTable();

      // ========== //
      // メンバ変数 //
      // ========== //
      /** 使用するチェスエンジン。 */
      const ChessEngine* engine_ptr_;

      /** 評価関数で使う、ポジショナル評価値。 */
      int score_;

      /** 現在のキャッシュポインタ。 */
      EvalCache* cache_ptr_;
  };
}  // namespace Sayuri

#endif
