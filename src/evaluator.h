/* The MIT License (MIT)
 *
 * Copyright (c) 2015 Hironori Ishibashi
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
      /** 価値テーブル 1 のインデックス。 */
      enum {
        /** 価値のインデックス - オープニングの配置。 */
        OPENING_POSITION,
        /** 価値のインデックス - エンディングの配置。 */
        ENDING_POSITION,
        /** 価値のインデックス - 機動力。 */
        MOBILITY,
        /** 価値のインデックス - センターコントロール。 */
        CENTER_CONTROL,
        /** 価値のインデックス - スウィートセンターのコントロール。 */
        SWEET_CENTER_CONTROL,
        /** 価値のインデックス - ピースの展開。 */
        DEVELOPMENT,
        /** 価値のインデックス - 攻撃。 */
        ATTACK,
        /** 価値のインデックス - 防御。 */
        DEFENSE,
        /** 価値のインデックス - ピン。 */
        PIN,
        /** 価値のインデックス - キング周辺への攻撃。 */
        ATTACK_AROUND_KING,
      };
      /** 価値テーブル 1 のサイズ。 */
      constexpr static std::size_t TABLE_SIZE_1 = ATTACK_AROUND_KING + 1;

      /** 価値テーブル 2 のインデックス。 */
      enum {
        /** 価値のインデックス - パスポーン。 */
        PASS_PAWN,
        /** 価値のインデックス - 守られたパスポーン。 */
        PROTECTED_PASS_PAWN,
        /** 価値のインデックス - ダブルポーン。 */
        DOUBLE_PAWN,
        /** 価値のインデックス - 孤立ポーン。 */
        ISO_PAWN,
        /** 価値のインデックス - ポーンの盾。 */
        PAWN_SHIELD,
        /** 価値のインデックス - ビショップペア。 */
        BISHOP_PAIR,
        /** 価値のインデックス - バッドビショップ。 */
        BAD_BISHOP,
        /** 価値のインデックス - ルークペア。 */
        ROOK_PAIR,
        /** 価値のインデックス - セミオープンファイルのルーク。 */
        ROOK_SEMIOPEN_FYLE,
        /** 価値のインデックス - オープンファイルのルーク。 */
        ROOK_OPEN_FYLE,
        /** 価値のインデックス - 早すぎるクイーンの始動。 */
        EARLY_QUEEN_LAUNCHED,
        /** 価値のインデックス - キング周りの弱いマス。 */
        WEAK_SQUARE,
        /** 価値のインデックス - キャスリング。 */
        CASTLING,
        /** 価値のインデックス - キャスリングの放棄。 */
        ABANDONED_CASTLING,
      };
      /** 価値テーブル 2 のサイズ。 */
      constexpr static std::size_t TABLE_SIZE_2 = ABANDONED_CASTLING + 1;

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

      /**
       * EvalParamsをキャッシュする。
       */
      void CacheEvalParams();

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
      template<Side PSide, PieceType PType>
      void CalValue(Square piece_square);

      // --- テンプレート部品 --- //
      /** 評価関数で使うテンプレート部品。 */
      template<Side PSide, PieceType PType>
      friend struct GenBitboards;
      /** 評価関数で使うテンプレート部品。 */
      template<Side PSide, PieceType PType>
      friend struct CalPosition;
      /** 評価関数で使うテンプレート部品。 */
      template<Side PSide, PieceType PType>
      friend struct CalMobility;
      /** 評価関数で使うテンプレート部品。 */
      template<Side PSide, PieceType PType>
      friend struct GenPinTargets;
      /** 評価関数で使うテンプレート部品。 */
      template<Side PSide, PieceType PType>
      friend struct CalSpecial;

      // ========== //
      // 局面分析用 //
      // ========== //
      /** 駒の初期位置のビットボード。 [サイド][駒の種類] */
      static Bitboard start_position_[NUM_SIDES][NUM_PIECE_TYPES];
      /** 駒の初期位置のビットボードの論理否定。 [サイド][駒の種類] */
      static Bitboard not_start_position_[NUM_SIDES][NUM_PIECE_TYPES];
      /** start_position_[][]とnot_start_position_[][]を初期化。 */
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

      /** 評価関数で使う、現在の駒の数。 */
      int num_pieces_;

      /** 評価関数で使う、ポジショナル評価値。 */
      int score_;

      // ==================== //
      // 評価関数用キャッシュ //
      // ==================== //
      static constexpr unsigned int MAX_ATTACKS = 7 * 4;
      static constexpr unsigned int NUM_CENTER = 4 * 4;
      static constexpr unsigned int NUM_SWEET_CENTER = 2 * 2;
      static constexpr unsigned int NUM_AROUND_KING = 8;
      /**
       * キャッシュを初期化する。
       */
      void InitCache();
      int opening_position_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_PIECE_TYPES]
      [NUM_SQUARES];
      int ending_position_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_PIECE_TYPES]
      [NUM_SQUARES];
      int mobility_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_PIECE_TYPES]
      [MAX_ATTACKS + 1];
      int center_control_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_PIECE_TYPES]
      [NUM_CENTER + 1];
      int sweet_center_control_cache_[NUM_SIDES][NUM_SQUARES + 1]
      [NUM_PIECE_TYPES][NUM_SWEET_CENTER + 1];
      int development_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_PIECE_TYPES]
      [NUM_SQUARES + 1];
      int attack_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_PIECE_TYPES]
      [NUM_PIECE_TYPES];
      int defense_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_PIECE_TYPES]
      [NUM_PIECE_TYPES];
      int pin_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_PIECE_TYPES]
      [NUM_PIECE_TYPES][NUM_PIECE_TYPES];
      int attack_around_king_cache_[NUM_SIDES][NUM_SQUARES + 1]
      [NUM_PIECE_TYPES][NUM_AROUND_KING + 1];
      int pass_pawn_cache_[NUM_SIDES][NUM_SQUARES + 1];
      int protected_pass_pawn_cache_[NUM_SIDES][NUM_SQUARES + 1];
      int double_pawn_cache_[NUM_SIDES][NUM_SQUARES + 1];
      int iso_pawn_cache_[NUM_SIDES][NUM_SQUARES + 1];
      int pawn_shield_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_SQUARES];
      int bishop_pair_cache_[NUM_SIDES][NUM_SQUARES + 1];
      int bad_bishop_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_SQUARES + 1];
      int rook_pair_cache_[NUM_SIDES][NUM_SQUARES + 1];
      int rook_semiopen_fyle_cache_[NUM_SIDES][NUM_SQUARES + 1];
      int rook_open_fyle_cache_[NUM_SIDES][NUM_SQUARES + 1];
      int early_queen_launched_cache_[NUM_SIDES][NUM_SQUARES + 1]
      [NUM_SQUARES + 1];
      int weak_square_cache_[NUM_SIDES][NUM_SQUARES + 1][NUM_SQUARES + 1];
      int castling_cache_[NUM_SIDES][NUM_SQUARES + 1];
      int abandoned_castling_cache_[NUM_SIDES][NUM_SQUARES + 1];
  };
}  // namespace Sayuri

#endif
