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
 * @file cache.cpp
 * @author Hironori Ishibashi
 * @brief 探索関数、評価関数で使うパラメータのキャッシュの実装。
 */

#include "cache.h"

#include <iostream>
#include <cstdint>
#include "common.h"
#include "params.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  Cache::Cache() :
  enable_quiesce_search_(false),
  enable_repetition_check_(false),
  enable_check_extension_(false),
  ybwc_limit_depth_(0),
  ybwc_invalid_moves_(0),
  enable_aspiration_windows_(false),
  aspiration_windows_limit_depth_(0),
  aspiration_windows_delta_(0),
  enable_see_(false),
  enable_history_(false),
  enable_killer_(false),
  enable_ttable_(false),
  enable_iid_(false),
  iid_limit_depth_(0),
  iid_search_depth_(0),
  enable_nmr_(false),
  nmr_limit_depth_(0),
  nmr_search_reduction_(0),
  nmr_reduction_(0),
  enable_probcut_(false),
  probcut_limit_depth_(0),
  probcut_margin_(0),
  probcut_search_reduction_(0),
  enable_history_pruning_(false),
  history_pruning_limit_depth_(0),
  history_pruning_threshold_(0),
  history_pruning_reduction_(0),
  enable_lmr_(false),
  lmr_limit_depth_(0),
  lmr_search_reduction_(0),
  enable_futility_pruning_(false),
  futility_pruning_depth_(0),
  max_nodes_(0),
  max_depth_(0),
  thinking_time_(0) {
    INIT_ARRAY(material_);
    INIT_ARRAY(history_pruning_invalid_moves_);
    INIT_ARRAY(lmr_invalid_moves_);
    INIT_ARRAY(futility_pruning_margin_);
    INIT_ARRAY(piece_hash_value_table_);
    INIT_ARRAY(to_move_hash_value_table_);
    INIT_ARRAY(castling_hash_value_table_);
    INIT_ARRAY(en_passant_hash_value_table_);
    INIT_ARRAY(eval_cache_);
  }
  // コピーコンストラクタ。
  Cache::Cache(const Cache& cache) {
    ScanMember(cache);
  }
  // ムーブコンストラクタ。
  Cache::Cache(Cache&& cache) {
    ScanMember(cache);
  }
  // コピー代入演算子。
  Cache& Cache::operator=(const Cache& cache) {
    ScanMember(cache);
    return *this;
  }
  // ムーブ代入演算子。
  Cache& Cache::operator=(Cache&& cache) {
    ScanMember(cache);
    return *this;
  }

  // SearchParamsをキャッシュする。
  void Cache::CacheSearchParams(const SearchParams& params) {
    COPY_ARRAY(material_, params.material());
    enable_quiesce_search_ = params.enable_quiesce_search();
    enable_repetition_check_ = params.enable_repetition_check();
    enable_check_extension_ = params.enable_check_extension();
    ybwc_limit_depth_ = params.ybwc_limit_depth();
    ybwc_invalid_moves_ = params.ybwc_invalid_moves();
    enable_aspiration_windows_ = params.enable_aspiration_windows();
    aspiration_windows_limit_depth_ = params.aspiration_windows_limit_depth();
    aspiration_windows_delta_ = params.aspiration_windows_delta();
    enable_see_ = params.enable_see();
    enable_history_ = params.enable_history();
    enable_killer_ = params.enable_killer();
    enable_ttable_ = params.enable_ttable();
    enable_iid_ = params.enable_iid();
    iid_limit_depth_ = params.iid_limit_depth();
    iid_search_depth_ = params.iid_search_depth();
    enable_nmr_ = params.enable_nmr();
    nmr_limit_depth_ = params.nmr_limit_depth();
    nmr_search_reduction_ = params.nmr_search_reduction();
    nmr_reduction_ = params.nmr_reduction();
    enable_probcut_ = params.enable_probcut();
    probcut_limit_depth_ = params.probcut_limit_depth();
    probcut_margin_ = params.probcut_margin();
    probcut_search_reduction_ = params.probcut_search_reduction();
    if (params.enable_history()) {
      enable_history_pruning_ = params.enable_history_pruning();
    } else {
      enable_history_pruning_ = false;
    }
    history_pruning_limit_depth_ = params.history_pruning_limit_depth();
    for (unsigned int num_moves = 0; num_moves < (MAX_CANDIDATES + 1);
    ++num_moves) {
      history_pruning_invalid_moves_[num_moves] =
      Util::GetMax(params.history_pruning_invalid_moves(),
      static_cast<int>(params.history_pruning_move_threshold() * num_moves));
    }
    history_pruning_threshold_ = params.history_pruning_threshold() * 256.0;
    history_pruning_reduction_ = params.history_pruning_reduction();
    enable_lmr_ = params.enable_lmr();
    lmr_limit_depth_ = params.lmr_limit_depth();
    for (unsigned int num_moves = 0; num_moves < (MAX_CANDIDATES + 1);
    ++num_moves) {
      lmr_invalid_moves_[num_moves] = Util::GetMax(params.lmr_invalid_moves(),
      static_cast<int>(params.lmr_move_threshold() * num_moves));
    }
    lmr_search_reduction_ = params.lmr_search_reduction();
    enable_futility_pruning_ = params.enable_futility_pruning();
    futility_pruning_depth_ = params.futility_pruning_depth();
    for (int depth = 0; depth < static_cast<int>(MAX_PLYS + 1); ++depth) {
      if (enable_futility_pruning_) {
        if (depth <= futility_pruning_depth_) {
          if (depth <= 0) {
            futility_pruning_margin_[depth] = params.futility_pruning_margin();
          } else {
            futility_pruning_margin_[depth] =
            params.futility_pruning_margin() * depth;
          }
        } else {
          futility_pruning_margin_[depth] = 3 * SCORE_WIN;
        }
      } else {
        futility_pruning_margin_[depth] = 3 * SCORE_WIN;
      }
    }
  }

  // EvalParamsをキャッシュする。
  void Cache::CacheEvalParams(const EvalParams& params) {
    for (unsigned int num_pieces = 0; num_pieces < (NUM_SQUARES + 1);
    ++num_pieces) {
      EvalCache* ptr = &(eval_cache_[num_pieces]);

      FOR_PIECE_TYPES(piece_type) {
        FOR_SQUARES(square) {
          ptr->opening_position_cache_[piece_type][square] =
          256.0 * params.opening_position_value_table()[piece_type][square]
          * params.weight_opening_position()[piece_type](num_pieces);

          ptr->ending_position_cache_[piece_type][square] =
          256.0 * params.ending_position_value_table()[piece_type][square]
          * params.weight_ending_position()[piece_type](num_pieces);
        }

        for (unsigned int num_attacks = 0;
        num_attacks < (EvalCache::MAX_ATTACKS + 1); ++num_attacks) {
          ptr->mobility_cache_[piece_type][num_attacks] =
          256.0 * params.weight_mobility()[piece_type](num_pieces);
        }

        for (unsigned int num_center = 0;
        num_center < (EvalCache::NUM_CENTER + 1); ++num_center) {
          ptr->center_control_cache_[piece_type][num_center] =
          256.0 * num_center
          * params.weight_center_control()[piece_type](num_pieces);
        }

        for (unsigned int num_sweet_center = 0;
        num_sweet_center < (EvalCache::NUM_SWEET_CENTER + 1);
        ++num_sweet_center) {
          ptr->sweet_center_control_cache_[piece_type]
          [num_sweet_center] =
          256.0 * num_sweet_center
          * params.weight_sweet_center_control()[piece_type](num_pieces);
        }

        for (unsigned int num_pieces_2 = 0; num_pieces_2 < (NUM_SQUARES + 1);
        ++num_pieces_2) {
          ptr->development_cache_[piece_type][num_pieces_2] =
          256.0 * num_pieces_2
          * params.weight_development()[piece_type](num_pieces);
        }

        FOR_PIECE_TYPES(piece_type_2) {
          ptr->attack_cache_[piece_type][piece_type_2] =
          256.0 * params.attack_value_table()[piece_type][piece_type_2]
          * params.weight_attack()[piece_type](num_pieces);

          ptr->defense_cache_[piece_type][piece_type_2] =
          256.0 * params.defense_value_table()[piece_type][piece_type_2]
          * params.weight_defense()[piece_type](num_pieces);

          FOR_PIECE_TYPES(piece_type_3) {
            ptr->pin_cache_[piece_type][piece_type_2]
            [piece_type_3] =
            256.0 * params.pin_value_table()[piece_type][piece_type_2]
            [piece_type_3]
            * params.weight_pin()[piece_type](num_pieces);
          }
        }

        for (unsigned int num_around_king = 0;
        num_around_king < (EvalCache::NUM_AROUND_KING + 1);
        ++num_around_king) {
          ptr->attack_around_king_cache_[piece_type]
          [num_around_king] = 256.0 * num_around_king
          * params.weight_attack_around_king()[piece_type](num_pieces);
        }
      }

      ptr->pass_pawn_cache_ = 256.0 * params.weight_pass_pawn()(num_pieces);

      ptr->protected_pass_pawn_cache_ =
      256.0 * params.weight_protected_pass_pawn()(num_pieces);

      ptr->double_pawn_cache_ =
      256.0 * params.weight_double_pawn()(num_pieces);

      ptr->iso_pawn_cache_ =
      256.0 * params.weight_iso_pawn()(num_pieces);

      FOR_SQUARES(square) {
        ptr->pawn_shield_cache_[square] =
        256.0 * params.pawn_shield_value_table()[square]
        * params.weight_pawn_shield()(num_pieces);
      }

      ptr->bishop_pair_cache_ =
      256.0 * params.weight_bishop_pair()(num_pieces);

      for (unsigned int num_pawn = 0; num_pawn < (NUM_SQUARES + 1);
      ++num_pawn) {
        ptr->bad_bishop_cache_[num_pawn] =
        256.0 * num_pawn * params.weight_bad_bishop()(num_pieces);
      }

      ptr->rook_pair_cache_ =
      256.0 * params.weight_rook_pair()(num_pieces);

      ptr->rook_open_fyle_cache_ =
      256.0 * params.weight_rook_open_fyle()(num_pieces);

      ptr->rook_semiopen_fyle_cache_ =
      256.0 * params.weight_rook_semiopen_fyle()(num_pieces);

      for (unsigned int num_minor = 0; num_minor < (NUM_SQUARES + 1);
      ++num_minor) {
        ptr->early_queen_starting_cache_[num_minor] =
        256.0 * num_minor * params.weight_early_queen_starting()(num_pieces);
      }

      for (unsigned int num_square = 0; num_square < (NUM_SQUARES + 1);
      ++num_square) {
        ptr->weak_square_cache_[num_square] =
        256.0 * num_square * params.weight_weak_square()(num_pieces);
      }

      ptr->castling_cache_ =
      256.0 * params.weight_castling()(num_pieces);

      ptr->abandoned_castling_cache_ =
      256.0 * params.weight_abandoned_castling()(num_pieces);
    }
  }

  // メンバをコピーする。
  void Cache::ScanMember(const Cache& cache) {
    COPY_ARRAY(material_, cache.material_);
    enable_quiesce_search_ = cache.enable_quiesce_search_;
    enable_repetition_check_ = cache.enable_repetition_check_;
    enable_check_extension_ = cache.enable_check_extension_;
    ybwc_limit_depth_ = cache.ybwc_limit_depth_;
    ybwc_invalid_moves_ = cache.ybwc_invalid_moves_;
    enable_aspiration_windows_ = cache.enable_aspiration_windows_;
    aspiration_windows_limit_depth_ = cache.aspiration_windows_limit_depth_;
    aspiration_windows_delta_ = cache.aspiration_windows_delta_;
    enable_see_ = cache.enable_see_;
    enable_history_ = cache.enable_history_;
    enable_killer_ = cache.enable_killer_;
    enable_ttable_ = cache.enable_ttable_;
    enable_iid_ = cache.enable_iid_;
    iid_limit_depth_ = cache.iid_limit_depth_;
    iid_search_depth_ = cache.iid_search_depth_;
    enable_nmr_ = cache.enable_nmr_;
    nmr_limit_depth_ = cache.nmr_limit_depth_;
    nmr_search_reduction_ = cache.nmr_search_reduction_;
    nmr_reduction_ = cache.nmr_reduction_;
    enable_probcut_ = cache.enable_probcut_;
    probcut_limit_depth_ = cache.probcut_limit_depth_;
    probcut_margin_ = cache.probcut_margin_;
    probcut_search_reduction_ = cache.probcut_search_reduction_;
    enable_history_pruning_ = cache.enable_history_pruning_;
    history_pruning_limit_depth_ = cache.history_pruning_limit_depth_;
    COPY_ARRAY(history_pruning_invalid_moves_,
    cache.history_pruning_invalid_moves_);
    history_pruning_threshold_ = cache.history_pruning_threshold_;
    history_pruning_reduction_ = cache.history_pruning_reduction_;
    enable_lmr_ = cache.enable_lmr_;
    lmr_limit_depth_ = cache.lmr_limit_depth_;
    COPY_ARRAY(lmr_invalid_moves_, cache.lmr_invalid_moves_);
    lmr_search_reduction_ = cache.lmr_search_reduction_;
    enable_futility_pruning_ = cache.enable_futility_pruning_;
    futility_pruning_depth_ = cache.futility_pruning_depth_;
    COPY_ARRAY(futility_pruning_margin_, cache.futility_pruning_margin_);
    COPY_ARRAY(piece_hash_value_table_, cache.piece_hash_value_table_);
    COPY_ARRAY(to_move_hash_value_table_, cache.to_move_hash_value_table_);
    COPY_ARRAY(castling_hash_value_table_, cache.castling_hash_value_table_);
    COPY_ARRAY(en_passant_hash_value_table_,
    cache.en_passant_hash_value_table_);
    max_nodes_ = cache.max_nodes_;
    max_depth_ = cache.max_depth_;
    thinking_time_ = cache.thinking_time_;
    COPY_ARRAY(eval_cache_, cache.eval_cache_);
  }
}  // namespace Sayuri
