/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Hironori Ishibashi
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
 * @file cache.h
 * @author Hironori Ishibashi
 * @brief 探索関数、評価関数で使うパラメータのキャッシュ。
 */

#ifndef CACHE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define CACHE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <cstdint>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class SearchParams;
  class EvalParams;

  /** 評価関数用キャッシュ構造体。 */
  struct EvalCache {
    // --- 定数 --- //
    /** 1駒あたりの攻撃の最大数。 */
    static constexpr unsigned int MAX_ATTACKS = 7 * 4;
    /** センターのマスの数。 */
    static constexpr unsigned int NUM_CENTER = 4 * 4;
    /** スウィートセンターのマスの数。 */
    static constexpr unsigned int NUM_SWEET_CENTER = 2 * 2;
    /** キング周辺のマスの数。 */
    static constexpr unsigned int NUM_AROUND_KING = 8;

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

  /** 探索関数、評価関数で使うキャッシュの構造体。*/
  class Cache {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      Cache();
      /**
       * コピーコンストラクタ。
       * @param cache コピー元。
       */
      Cache(const Cache& cache);
      /**
       * ムーブコンストラクタ。
       * @param cache ムーブ元。
       */
      Cache(Cache&& cache);
      /**
       * コピー代入演算子。
       * @param cache コピー元。
       */
      Cache& operator=(const Cache& cache);
      /**
       * ムーブ代入演算子。
       * @param cache ムーブ元。
       */
      Cache& operator=(Cache&& cache);
      /** デストラクタ。 */
      virtual ~Cache() {}

      // ============== //
      // 公開メンバ変数 //
      // ============== //
      /** 
       * 探索関数用キャッシュ。
       * マテリアル。
       */
      int material_[NUM_PIECE_TYPES];
      /**
       * 探索関数用キャッシュ。
       * Quiescence Search - 有効無効。
       */
      bool enable_quiesce_search_;
      /**
       * 探索関数用キャッシュ。
       * 繰り返しチェック - 有効無効。
       */
      bool enable_repetition_check_;
      /**
       * 探索関数用キャッシュ。
       * Check Extension - 有効無効。
       */
      bool enable_check_extension_;
      /**
       * 探索関数用キャッシュ。
       * YBWC - 残り深さ制限。
       */
      int ybwc_limit_depth_;
      /**
       * 探索関数用キャッシュ。
       * YBWC - 無効にする先頭の候補手の数。
       */
      int ybwc_invalid_moves_;
      /**
       * 探索関数用キャッシュ。
       * Aspiration Windows - 有効無効。
       */
      bool enable_aspiration_windows_;
      /**
       * 探索関数用キャッシュ。
       * Aspiration Windows - 残り深さ制限。
       */
      int aspiration_windows_limit_depth_;
      /**
       * 探索関数用キャッシュ。
       * Aspiration Windows - デルタ値。
       */
      int aspiration_windows_delta_;
      /**
       * 探索関数用キャッシュ。
       * SEE - 有効無効。
       */
      bool enable_see_;
      /**
       * 探索関数用キャッシュ。
       * ヒストリー - 有効無効。
       */
      bool enable_history_;
      /**
       * 探索関数用キャッシュ。
       * キラームーブ - 有効無効。
       */
      bool enable_killer_;
      /**
       * 探索関数用キャッシュ。
       * トランスポジションテーブル - 有効無効。
       */
      bool enable_ttable_;
      /**
       * 探索関数用キャッシュ。
       * Internal Iterative Deepening - 有効無効。
       */
      bool enable_iid_;
      /**
       * 探索関数用キャッシュ。
       * Internal Iterative Deepening - 残り深さ制限。
       */
      int iid_limit_depth_;
      /**
       * 探索関数用キャッシュ。
       * Internal Iterative Deepening - 探索の深さ。
       */
      int iid_search_depth_;
      /**
       * 探索関数用キャッシュ。
       * Null Move Reduction - 有効無効。
       */
      bool enable_nmr_;
      /**
       * 探索関数用キャッシュ。
       * Null Move Reduction - 残り深さ制限。
       */
      int nmr_limit_depth_;
      /**
       * 探索関数用キャッシュ。
       * Null Move Reduction - 何プライ浅く探索するか。
       */
      int nmr_search_reduction_;
      /**
       * 探索関数用キャッシュ。
       * Null Move Reduction - リダクションする深さ。
       */
      int nmr_reduction_;
      /**
       * 探索関数用キャッシュ。
       * ProbCut - 有効無効。
       */
      bool enable_probcut_;
      /**
       * 探索関数用キャッシュ。
       * ProbCut - 残り深さ制限。
       */
      int probcut_limit_depth_;
      /**
       * 探索関数用キャッシュ。
       * ProbCut - ベータ値の増分。
       */
      int probcut_margin_;
      /**
       * 探索関数用キャッシュ。
       * ProbCut - 何プライ浅く探索するか。
       */
      int probcut_search_reduction_;
      /**
       * 探索関数用キャッシュ。
       * History Pruning - 有効無効。
       */
      bool enable_history_pruning_;
      /**
       * 探索関数用キャッシュ。
       * History Pruning - 残り深さ制限。
       */
      int history_pruning_limit_depth_;
      /**
       * 探索関数用キャッシュ。
       * History Pruning - 無効にする先頭の候補手の数。
       */
      int history_pruning_invalid_moves_[MAX_CANDIDATES + 1];
      /**
       * 探索関数用キャッシュ。
       * History Pruning - 最大ヒストリー値に対する閾値。
       */
      std::uint64_t history_pruning_threshold_;
      /**
       * 探索関数用キャッシュ。
       * History Pruning - リダクションする深さ。
       */
      int history_pruning_reduction_;
      /**
       * 探索関数用キャッシュ。
       * Late Move Reduction - 有効無効。
       */
      bool enable_lmr_;
      /**
       * 探索関数用キャッシュ。
       * Late Move Reduction - 残り深さ制限。
       */
      int lmr_limit_depth_;
      /**
       * 探索関数用キャッシュ。
       * Late Move Reduction - 無効にする先頭の候補手の数。
       */
      int lmr_invalid_moves_[MAX_CANDIDATES + 1];
      /**
       * 探索関数用キャッシュ。
       * Late Move Reduction - リダクションする深さ。
       */
      int lmr_search_reduction_;
      /**
       * 探索関数用キャッシュ。
       * Futility Pruning - 有効無効。
       */
      bool enable_futility_pruning_;
      /**
       * 探索関数用キャッシュ。
       * Futility Pruning - 有効にする残り深さ。
       */
      int futility_pruning_depth_;
      /**
       * 探索関数用キャッシュ。
       * Futility Pruning - 残り深さ1プライあたりのマージン。
       */
      int futility_pruning_margin_[MAX_PLYS + 1];
      /**
       * 探索関数用キャッシュ。
       * 駒の情報のハッシュ値のテーブル。
       */
      Hash piece_hash_value_table_[NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
      /**
       * 探索関数用キャッシュ。
       * 手番のハッシュ値のテーブル。
       */
      Hash to_move_hash_value_table_[NUM_SIDES];
      /**
       * 探索関数用キャッシュ。
       * キャスリングの権利のハッシュ値のテーブル。
       */
      Hash castling_hash_value_table_[16];
      /**
       * 探索関数用キャッシュ。
       * アンパッサンのハッシュ値のテーブル。
       */
      Hash en_passant_hash_value_table_[NUM_SQUARES];
      /**
       * 探索関数用キャッシュ。
       * 探索ストップ条件 - 最大探索ノード数。
       */
      std::uint64_t max_nodes_;
      /**
       * 探索関数用キャッシュ。
       * 探索ストップ条件 - 最大探索深さ。
       */
      std::uint32_t max_depth_;
      /**
       * 探索関数用キャッシュ。
       * 探索ストップ条件 - 思考時間。
       */
      Chrono::milliseconds thinking_time_;

      /** キャッシュの配列。 */
      EvalCache eval_cache_[NUM_SQUARES + 1];

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * SearchParamsをキャッシュする。
       * @param params キャッシュするSearchParams。
       */
      void CacheSearchParams(const SearchParams& params);
      /**
       * EvalParamsをキャッシュする。
       * @param params キャッシュするEvalParams。
       */
      void CacheEvalParams(const EvalParams& params);

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * メンバをコピーする。
       * @param cache コピー元。
       */
      void ScanMember(const Cache& cache);
  };
}  // namespace Sayuri

#endif
