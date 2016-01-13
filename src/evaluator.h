/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Hironori Ishibashi
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
#include "evaluator_extra.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;
  struct EvalCache;

  /** Evaluator用メタ関数名前空間。 */
  namespace MetaEvaluator {
    // ========================= //
    // Evaluator定数初期化用関数 //
    // ========================= //
    /** 
     * Evaluator::ISO_PAWN_MASKを初期化する。
     * @param fyle そのマスのファイル。
     * @return そのマスに対応したISO_PAWN_MASK。
     */
    inline constexpr Bitboard INIT_ISO_PAWN_MASK(Fyle fyle) {
      return fyle == FYLE_A ? Util::FYLE[FYLE_B]
      : (fyle == FYLE_H ? Util::FYLE[FYLE_G]
      : (Util::FYLE[fyle + 1] | Util::FYLE[fyle - 1]));
    }

    inline constexpr Bitboard TEMP_FUNC_1(Fyle fyle) {
      return Util::FYLE[fyle] | INIT_ISO_PAWN_MASK(fyle);
    }
    inline constexpr Bitboard TEMP_FUNC_2(Rank rank) {
      return rank == RANK_1 ? Util::RANK[RANK_1]
     : (Util::RANK[rank] | TEMP_FUNC_2(rank - 1));
    }
    inline constexpr Bitboard TEMP_FUNC_3(Rank rank) {
      return rank == RANK_8 ? Util::RANK[RANK_8]
     : (Util::RANK[rank] | TEMP_FUNC_3(rank + 1));
    }
    /**
     * PASS_PAWN_MASKを初期化する。
     * @param side サイド。
     * @param square マス。
     * @return パスポーン判定用マスク。
     */
    inline constexpr Bitboard INIT_PASS_PAWN_MASK(Side side, Square square) {
      return side == WHITE
      ? (TEMP_FUNC_1(Util::SquareToFyle(square))
      & ~TEMP_FUNC_2(Util::SquareToRank(square)))

      : (side == BLACK
      ? (TEMP_FUNC_1(Util::SquareToFyle(square))
      & ~TEMP_FUNC_3(Util::SquareToRank(square)))

      : 0);
    }
  }

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
      static constexpr Bitboard PASS_PAWN_MASK [NUM_SIDES][NUM_SQUARES] {
        {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, A1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, B1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, C1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, D1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, E1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, F1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, G1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, H1),

          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, A2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, B2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, C2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, D2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, E2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, F2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, G2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, H2),

          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, A3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, B3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, C3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, D3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, E3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, F3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, G3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, H3),

          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, A4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, B4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, C4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, D4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, E4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, F4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, G4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, H4),

          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, A5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, B5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, C5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, D5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, E5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, F5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, G5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, H5),

          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, A6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, B6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, C6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, D6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, E6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, F6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, G6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, H6),

          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, A7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, B7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, C7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, D7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, E7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, F7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, G7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, H7),

          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, A8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, B8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, C8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, D8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, E8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, F8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, G8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(WHITE, H8)
        },
        {
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, A1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, B1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, C1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, D1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, E1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, F1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, G1),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, H1),

          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, A2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, B2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, C2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, D2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, E2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, F2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, G2),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, H2),

          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, A3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, B3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, C3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, D3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, E3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, F3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, G3),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, H3),

          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, A4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, B4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, C4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, D4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, E4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, F4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, G4),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, H4),

          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, A5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, B5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, C5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, D5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, E5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, F5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, G5),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, H5),

          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, A6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, B6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, C6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, D6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, E6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, F6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, G6),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, H6),

          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, A7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, B7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, C7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, D7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, E7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, F7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, G7),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, H7),

          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, A8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, B8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, C8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, D8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, E8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, F8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, G8),
          MetaEvaluator::INIT_PASS_PAWN_MASK(BLACK, H8)
        }
      };

      /** 孤立ポーンの両脇ファイルのマスク。 [マス] */
      static constexpr Bitboard ISO_PAWN_MASK[NUM_SQUARES] {
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_A),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_B),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_C),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_D),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_E),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_F),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_G),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_H),

        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_A),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_B),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_C),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_D),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_E),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_F),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_G),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_H),

        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_A),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_B),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_C),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_D),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_E),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_F),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_G),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_H),

        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_A),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_B),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_C),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_D),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_E),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_F),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_G),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_H),

        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_A),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_B),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_C),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_D),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_E),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_F),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_G),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_H),

        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_A),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_B),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_C),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_D),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_E),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_F),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_G),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_H),

        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_A),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_B),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_C),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_D),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_E),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_F),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_G),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_H),

        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_A),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_B),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_C),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_D),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_E),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_F),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_G),
        MetaEvaluator::INIT_ISO_PAWN_MASK(FYLE_H)
      };

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
