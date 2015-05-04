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

      /** 評価関数で使う、ポジショナル評価値。 */
      int score_;

      // ==================== //
      // 評価関数用キャッシュ //
      // ==================== //
      static constexpr unsigned int MAX_ATTACKS = 7 * 4;
      static constexpr unsigned int NUM_CENTER = 4 * 4;
      static constexpr unsigned int NUM_SWEET_CENTER = 2 * 2;
      static constexpr unsigned int NUM_AROUND_KING = 8;
      enum : unsigned int {
        WHITE_2, BLACK_2
      };

      /**
       * キャッシュを初期化する。
       */
      void InitCache();

      /** キャッシュ構造体。 (9,924 Bytes) */
      struct Cache {
        /** キャッシュ - オープニングの配置。 */
        int opening_position_cache_[NUM_PIECE_TYPES][NUM_SQUARES];
        /** キャッシュ - エンディングの配置。 */
        int ending_position_cache_[NUM_PIECE_TYPES][NUM_SQUARES];
        /** キャッシュ - 機動力。 */
        int mobility_cache_[NUM_PIECE_TYPES][MAX_ATTACKS + 1];
        /** キャッシュ - センターコントロール。 */
        int center_control_cache_[NUM_PIECE_TYPES][NUM_CENTER + 1];
        /** キャッシュ - スウィートセンターコントロール。 */
        int sweet_center_control_cache_[NUM_PIECE_TYPES][NUM_SWEET_CENTER + 1];
        /** キャッシュ - 駒の展開。 */
        int development_cache_[NUM_PIECE_TYPES][NUM_SQUARES + 1];
        /** キャッシュ - 攻撃。 */
        int attack_cache_[NUM_PIECE_TYPES][NUM_PIECE_TYPES];
        /** キャッシュ - 防御。 */
        int defense_cache_[NUM_PIECE_TYPES][NUM_PIECE_TYPES];
        /** キャッシュ - ピン。 */
        int pin_cache_[NUM_PIECE_TYPES][NUM_PIECE_TYPES][NUM_PIECE_TYPES];
        /** キャッシュ - キング周辺への攻撃。 */
        int attack_around_king_cache_[NUM_PIECE_TYPES][NUM_AROUND_KING + 1];
        /** キャッシュ - パスポーン。 */
        int pass_pawn_cache_;
        /** キャッシュ - 守られたパスポーン。 */
        int protected_pass_pawn_cache_;
        /** キャッシュ - ダブルポーン。 */
        int double_pawn_cache_;
        /** キャッシュ - 孤立ポーン。 */
        int iso_pawn_cache_;
        /** キャッシュ - ポーンの盾。 */
        int pawn_shield_cache_[NUM_SQUARES];
        /** キャッシュ - ビショップペア。 */
        int bishop_pair_cache_;
        /** キャッシュ - バッドビショップ。 */
        int bad_bishop_cache_[NUM_SQUARES + 1];
        /** キャッシュ - ルークペア。 */
        int rook_pair_cache_;
        /** キャッシュ - セミオープンファイルのルーク。 */
        int rook_semiopen_fyle_cache_;
        /** キャッシュ - オープンファイルのルーク。 */
        int rook_open_fyle_cache_;
        /** キャッシュ - 早すぎるクイーンの始動。 */
        int early_queen_starting_cache_[NUM_SQUARES + 1];
        /** キャッシュ - キング周りの弱いマス。 */
        int weak_square_cache_[NUM_SQUARES + 1];
        /** キャッシュ - キャスリング。 */
        int castling_cache_;
        /** キャッシュ - キャスリングの権利の放棄。 */
        int abandoned_castling_cache_;
      };
      /** キャッシュの配列。 */
      Cache cache_[NUM_SQUARES + 1];
      /** 現在の配列のポインタ。 */
      Cache* cache_ptr_;
  };
}  // namespace Sayuri

#endif
