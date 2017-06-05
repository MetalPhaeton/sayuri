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
 * @file params.cpp
 * @author Hironori Ishibashi
 * @brief 探索アルゴリズム、評価関数を変更するパラメータの実装。
 */

# include "params.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ============ //
  // SearchParams //
  // ============ //
  // コンストラクタ。
  SearchParams::SearchParams() :
  enable_quiesce_search_(true),
  enable_repetition_check_(true),
  enable_check_extension_(true),
  ybwc_limit_depth_(4),
  ybwc_invalid_moves_(3),
  enable_aspiration_windows_(true),
  aspiration_windows_limit_depth_(5),
  aspiration_windows_delta_(15),
  enable_see_(true),
  enable_history_(true),
  enable_killer_(true),
  enable_ttable_(true),
  enable_iid_(true),
  iid_limit_depth_(5),
  iid_search_depth_(4),
  enable_nmr_(true),
  nmr_limit_depth_(4),
  nmr_search_reduction_(4),
  nmr_reduction_(3),
  enable_probcut_(false),
  probcut_limit_depth_(4),
  probcut_margin_(400),
  probcut_search_reduction_(3),
  enable_history_pruning_(false),
  history_pruning_limit_depth_(4),
  history_pruning_move_threshold_(0.6),
  history_pruning_invalid_moves_(10),
  history_pruning_threshold_(0.5),
  history_pruning_reduction_(1),
  enable_lmr_(true),
  lmr_limit_depth_(4),
  lmr_move_threshold_(0.3),
  lmr_invalid_moves_(4),
  lmr_search_reduction_(1),
  enable_futility_pruning_(true),
  futility_pruning_depth_(3),
  futility_pruning_margin_(400) {
    // マテリアルの初期化。
    material_[EMPTY] = 0;  // 何もなし。
    material_[PAWN] = 100;  // ポーン。
    material_[KNIGHT] = 400;  // ナイト。
    material_[BISHOP] = 400;  // ビショップ。
    material_[ROOK] = 600;  // ルーク。
    material_[QUEEN] = 1200;  // クイーン。
    material_[KING] = SCORE_WIN;  // キング。

    // ヒストリーをチェック。
    if (!enable_history_) enable_history_pruning_ = false;
  }

  // コピーコンストラクタ。
  SearchParams::SearchParams(const SearchParams& params) {
    ScanMember(params);
  }

  // ムーブコンストラクタ。
  SearchParams::SearchParams(SearchParams&& params) {
    ScanMember(params);
  }

  // コピー代入演算子。
  SearchParams& SearchParams::operator=(const SearchParams& params) {
    ScanMember(params);
    return *this;
  }

  // ムーブ代入演算子。
  SearchParams& SearchParams::operator=(SearchParams&& params) {
    ScanMember(params);
    return *this;
  }

  // メンバをコピーする。
  void SearchParams::ScanMember(const SearchParams& params) {
    FOR_PIECE_TYPES(piece_type) {
      material_[piece_type] = params.material_[piece_type];
    }
    enable_quiesce_search_ = params.enable_quiesce_search_;
    enable_repetition_check_= params.enable_repetition_check_;
    enable_check_extension_ = params.enable_check_extension_;
    ybwc_limit_depth_ = params.ybwc_limit_depth_;
    ybwc_invalid_moves_ = params.ybwc_invalid_moves_;
    enable_aspiration_windows_ = params.enable_aspiration_windows_;
    aspiration_windows_limit_depth_ = params.aspiration_windows_limit_depth_;
    aspiration_windows_delta_ = params.aspiration_windows_delta_;
    enable_see_ = params.enable_see_;
    enable_history_ = params.enable_history_;
    enable_killer_ = params.enable_killer_;
    enable_ttable_ = params.enable_ttable_;
    enable_iid_ = params.enable_iid_;
    iid_limit_depth_ = params.iid_limit_depth_;
    iid_search_depth_ = params.iid_search_depth_;
    enable_nmr_ = params.enable_nmr_;
    nmr_limit_depth_ = params.nmr_limit_depth_;
    nmr_search_reduction_ = params.nmr_search_reduction_;
    nmr_reduction_ = params.nmr_reduction_;
    enable_probcut_ = params.enable_probcut_;
    probcut_limit_depth_ = params.probcut_limit_depth_;
    probcut_margin_ = params.probcut_margin_;
    probcut_search_reduction_ = params.probcut_search_reduction_;
    enable_history_pruning_ = params.enable_history_pruning_;
    history_pruning_limit_depth_ = params.history_pruning_limit_depth_;
    history_pruning_move_threshold_ = params.history_pruning_move_threshold_;
    history_pruning_invalid_moves_ = params.history_pruning_invalid_moves_;
    history_pruning_threshold_ = params.history_pruning_threshold_;
    history_pruning_reduction_ = params.history_pruning_reduction_;
    enable_lmr_ = params.enable_lmr_;
    lmr_limit_depth_ = params.lmr_limit_depth_;
    lmr_move_threshold_ = params.lmr_move_threshold_;
    lmr_invalid_moves_ =params.lmr_invalid_moves_;
    lmr_search_reduction_ = params.lmr_search_reduction_;
    enable_futility_pruning_ = params.enable_futility_pruning_;
    futility_pruning_depth_ = params.futility_pruning_depth_;
    futility_pruning_margin_ = params.futility_pruning_margin_;
  }

  // マテリアルのミューテータ。
  void SearchParams::material(const int (& table)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      material_[piece_type] = table[piece_type];
    }
  }

  // ========== //
  // EvalParams //
  // ========== //
  // コンストラクタ。
  EvalParams::EvalParams() :
  weight_pass_pawn_(Weight::CreateWeight(22.6, 45.3)),
  weight_protected_pass_pawn_(Weight::CreateWeight(11.3, 16.0)),
  weight_double_pawn_(Weight::CreateWeight(-6.7, -13.5)),
  weight_iso_pawn_(Weight::CreateWeight(-13.5, -8.0)),
  weight_pawn_shield_(Weight::CreateWeight(1.0, 0.0)),
  weight_bishop_pair_(Weight::CreateWeight(16.0, 32.0)),
  weight_bad_bishop_(Weight::CreateWeight(-4.0, 0.0)),
  weight_rook_pair_(Weight::CreateWeight(8.0, 16.0)),
  weight_rook_semiopen_fyle_(Weight::CreateWeight(16.0, 8.0)),
  weight_rook_open_fyle_(Weight::CreateWeight(8.0, 0.0)),
  weight_early_queen_starting_(Weight::CreateWeight(-26.9, 0.0)),
  weight_weak_square_(Weight::CreateWeight(-1.0, 0.0)),
  weight_castling_(Weight::CreateWeight(16.0, 0.0)),
  weight_abandoned_castling_(Weight::CreateWeight(-90.5, 0.0)) {
    // オープニング時の駒の配置の価値テーブルの初期化。
    static const double OPENING_POSITION[NUM_PIECE_TYPES][NUM_SQUARES] {
      {  // EMPTY。
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
         0.0,  0.0,  0.0,   0.0,   0.0,  0.0,  0.0,  0.0,
         0.0,  0.0,  0.0,   0.0,   0.0,  0.0,  0.0,  0.0,
         4.0,  9.5, 16.0,  19.0,  19.0, 16.0,  9.5,  4.0,
         8.0, 19.0, 32.0,  38.0,  38.0, 32.0, 19.0,  8.0,
        12.0, 28.5, 48.0,  57.0,  57.0, 48.0, 28.5, 12.0,
        16.0, 38.0, 64.0,  76.0,  76.0, 64.0, 38.0, 16.0,
        20.0, 47.5, 80.0,  95.0,  95.0, 80.0, 47.5, 20.0,
        24.0, 57.0, 96.0, 114.0, 114.0, 96.0, 57.0, 24.0
      },
      {  // ナイト。
        -13.5, -11.2, -9.0, -6.7, -6.7, -9.0, -11.2, -13.5,
         -8.9,  -6.5, -4.0, -1.6, -1.6, -4.0,  -6.5,  -8.9,
         -4.3,  -1.7,  1.0,  3.6,  3.6,  1.0,  -1.7,  -4.3,
          0.3,   3.1,  5.9,  8.7,  8.7,  5.9,   3.1,   0.3,
          4.9,   7.9, 10.9, 13.9, 13.9, 10.9,   7.9,   4.9,
          9.5,  12.7, 15.8, 19.0, 19.0, 15.8,  12.7,   9.5,
          4.9,   7.9, 10.9, 13.9, 13.9, 10.9,   7.9,   4.9,
          0.3,   3.1,  5.9,  8.7,  8.7,  5.9,   3.1,   0.3
      },
      {  // ビショップ。
         8.0,  5.3,  2.7,  0.0,  0.0,  2.7,  5.3,  8.0,
         6.9,  9.6,  6.9,  4.3,  4.3,  6.9,  9.6,  6.9,
         5.9,  8.5, 11.2,  8.5,  8.5, 11.2,  8.5,  5.9,
         4.8,  7.5, 10.1, 12.8, 12.8, 10.1,  7.5,  4.8,
         6.4,  9.1, 11.7, 14.4, 14.4, 11.7,  9.1,  6.4,
        10.7, 13.3, 16.0, 13.3, 13.3, 16.0, 13.3, 10.7,
        11.7, 14.4, 11.7,  9.1,  9.1, 11.7, 14.4, 11.7,
        12.8, 10.1,  7.5,  4.8,  4.8,  7.5, 10.1, 12.8
      },
      {  // ルーク。
         1.0,  1.9,  3.6,  6.7,  6.7,  3.6,  1.9,  1.0,
         1.0,  1.9,  3.6,  6.7,  6.7,  3.6,  1.9,  1.0,
         1.0,  1.9,  3.6,  6.7,  6.7,  3.6,  1.9,  1.0,
         1.0,  1.9,  3.6,  6.7,  6.7,  3.6,  1.9,  1.0,
         1.0,  1.9,  3.6,  6.7,  6.7,  3.6,  1.9,  1.0,
         1.0,  1.9,  3.6,  6.7,  6.7,  3.6,  1.9,  1.0,
        22.6, 22.6, 22.6, 22.6, 22.6, 22.6, 22.6, 22.6,
        22.6, 22.6, 22.6, 22.6, 22.6, 22.6, 22.6, 22.6
      },
      {  // クイーン。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 0.0,
        0.0, 4.0, 8.0, 8.0, 8.0, 8.0, 4.0, 0.0,
        0.0, 4.0, 8.0, 8.0, 8.0, 8.0, 4.0, 0.0,
        0.0, 4.0, 8.0, 8.0, 8.0, 8.0, 4.0, 0.0,
        0.0, 4.0, 8.0, 8.0, 8.0, 8.0, 4.0, 0.0,
        0.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // キング。
         32.0,  32.0,  16.0, -22.6, -22.6,  16.0,  32.0,  32.0,
          8.0,   8.0,   4.0, -22.6, -22.6,   4.0,   8.0,   8.0,
        -32.0, -32.0, -32.0, -32.0, -32.0, -32.0, -32.0, -32.0,
        -38.1, -38.1, -38.1, -38.1, -38.1, -38.1, -38.1, -38.1,
        -45.3, -45.3, -45.3, -45.3, -45.3, -45.3, -45.3, -45.3,
        -53.8, -53.8, -53.8, -53.8, -53.8, -53.8, -53.8, -53.8,
        -64.0, -64.0, -64.0, -64.0, -64.0, -64.0, -64.0, -64.0,
        -76.1, -76.1, -76.1, -76.1, -76.1, -76.1, -76.1, -76.1
      }
    };
    COPY_ARRAY(opening_position_value_table_, OPENING_POSITION);

    // エンディング時の駒の配置の価値テーブルを初期化。
    static const double ENDING_POSITION[NUM_PIECE_TYPES][NUM_SQUARES] {
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
          0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0, 
          0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0, 
         32.0,  32.0,  32.0,  32.0,  32.0,  32.0,  32.0,  32.0,
         64.0,  64.0,  64.0,  64.0,  64.0,  64.0,  64.0,  64.0,
         96.0,  96.0,  96.0,  96.0,  96.0,  96.0,  96.0,  96.0,
        128.0, 128.0, 128.0, 128.0, 128.0, 128.0, 128.0, 128.0,
        160.0, 160.0, 160.0, 160.0, 160.0, 160.0, 160.0, 160.0,
        192.0, 192.0, 192.0, 192.0, 192.0, 192.0, 192.0, 192.0
      },
      {  // ナイト。
        0.0, 0.0,  0.0,  0.0,  0.0,  0.0, 0.0, 0.0,
        0.0, 0.0,  0.0,  0.0,  0.0,  0.0, 0.0, 0.0,
        0.0, 0.0,  8.0,  8.0,  8.0,  8.0, 0.0, 0.0,
        0.0, 0.0,  8.0,  8.0,  8.0,  8.0, 0.0, 0.0,
        0.0, 0.0,  8.0,  8.0,  8.0,  8.0, 0.0, 0.0,
        0.0, 0.0,  8.0,  8.0,  8.0,  8.0, 0.0, 0.0,
        0.0, 0.0,  0.0,  0.0,  0.0,  0.0, 0.0, 0.0,
        0.0, 0.0,  0.0,  0.0,  0.0,  0.0, 0.0, 0.0
      },
      {  // ビショップ。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // ルーク。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // クイーン。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // キング。
        -16.0, -10.7, -5.3,  0.0,  0.0, -5.3, -10.7, -16.0,
        -10.7,  -5.3,  0.0,  5.3,  5.3,  0.0,  -5.3, -10.7,
         -5.3,   0.0,  5.3, 10.7, 10.7,  5.3,   0.0,  -5.3,
          0.0,   5.3, 10.7, 16.0, 16.0, 10.7,   5.3,   0.0,
          0.0,   5.3, 10.7, 16.0, 16.0, 10.7,   5.3,   0.0,
         -5.3,   0.0,  5.3, 10.7, 10.7,  5.3,   0.0,  -5.3,
        -10.7,  -5.3,  0.0,  5.3,  5.3,  0.0,  -5.3, -10.7,
        -16.0, -10.7, -5.3,  0.0,  0.0, -5.3, -10.7, -16.0
      }
    };
    COPY_ARRAY(ending_position_value_table_, ENDING_POSITION);

    // 相手への攻撃の価値テーブルを初期化。
    static const double ATTACK[NUM_PIECE_TYPES][NUM_PIECE_TYPES] {
      // 相手側:{Empty, ポーン, ナイト, ビショップ, ルーク, クイーン, キング}
      {0.0, 0.0, 0.0, 0.0,  0.0,  0.0,  0.0},  // 攻撃側: Empty。
      {0.0, 0.0, 8.0, 8.0,  8.0,  8.0,  8.0},  // 攻撃側: ポーン。
      {0.0, 2.0, 0.0, 2.8,  8.0,  8.0,  8.0},  // 攻撃側: ナイト。
      {0.0, 4.0, 5.7, 0.0, 16.0, 16.0, 16.0},  // 攻撃側: ビショップ。
      {0.0, 4.0, 8.0, 8.0,  0.0, 16.0, 16.0},  // 攻撃側: ルーク。
      {0.0, 4.0, 8.0, 8.0,  8.0,  0.0, 16.0},  // 攻撃側: クイーン。
      {0.0, 0.0, 0.0, 0.0,  0.0,  0.0,  0.0}   // 攻撃側: キング。
    };
    COPY_ARRAY(attack_value_table_, ATTACK);

    // 味方への防御の価値テーブルを初期化。
    static const double DEFENSE[NUM_PIECE_TYPES][NUM_PIECE_TYPES] {
      // 相手側:{Empty, ポーン, ナイト, ビショップ, ルーク, クイーン, キング}
      {0.0,  0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 守り側: Empty。
      {0.0,  5.7, 4.0, 0.0, 0.0, 0.0, 0.0},  // 守り側: ポーン。
      {0.0,  6.7, 3.4, 0.0, 0.0, 0.0, 0.0},  // 守り側: ナイト。
      {0.0,  4.8, 0.0, 0.0, 0.0, 0.0, 0.0},  // 守り側: ビショップ。
      {0.0,  4.0, 0.0, 0.0, 0.0, 1.0, 0.0},  // 守り側: ルーク。
      {0.0,  2.8, 1.0, 1.0, 1.0, 0.0, 0.0},  // 守り側: クイーン。
      {0.0, 19.0, 0.0, 0.0, 0.0, 0.0, 0.0}   // 守り側: キング。
    };
    COPY_ARRAY(defense_value_table_, DEFENSE);

    // ピンの価値テーブルを初期化。
    static const double
    PIN[NUM_PIECE_TYPES][NUM_PIECE_TYPES][NUM_PIECE_TYPES] {
      {  // Emptyでピン。
        // バック:{Empty, ポーン, ナイト, ビショップ, ルーク, クイーン, キング}
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: Empty。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ポーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ナイト。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ビショップ。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ルーク。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: クイーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}   // ターゲット: キング。
      },
      {  // ポーンでピン。
        // バック:{Empty, ポーン, ナイト, ビショップ, ルーク, クイーン, キング}
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: Empty。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ポーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ナイト。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ビショップ。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ルーク。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: クイーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}   // ターゲット: キング。
      },
      {  // ナイトでピン。
        // バック:{Empty, ポーン, ナイト, ビショップ, ルーク, クイーン, キング}
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: Empty。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ポーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ナイト。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ビショップ。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ルーク。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: クイーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}   // ターゲット: キング。
      },
      {  // ビショップでピン。
        // バック:{Empty, ポーン, ナイト, ビショップ, ルーク, クイーン, キング}
        {0.0, 0.0, 0.0, 0.0,  0.0,  0.0,  0.0},  // ターゲット: Empty。
        {0.0, 0.0, 0.0, 0.0,  4.0,  4.0,  4.0},  // ターゲット: ポーン。
        {0.0, 0.0, 0.0, 0.0, 16.0, 16.0, 16.0},  // ターゲット: ナイト。
        {0.0, 0.0, 0.0, 0.0,  0.0,  0.0,  0.0},  // ターゲット: ビショップ。
        {0.0, 0.0, 0.0, 0.0, 22.6, 22.6, 22.6},  // ターゲット: ルーク。
        {0.0, 0.0, 0.0, 0.0, 22.6, 45.3, 45.3},  // ターゲット: クイーン。
        {0.0, 0.0, 0.0, 0.0, 22.6, 45.3,  0.0}   // ターゲット: キング。
      },
      {  // ルークでピン。
        // バック:{Empty, ポーン, ナイト, ビショップ, ルーク, クイーン, キング}
        {0.0, 0.0, 0.0, 0.0, 0.0,  0.0,  0.0},  // ターゲット: Empty。
        {0.0, 0.0, 0.0, 0.0, 0.0,  4.0,  4.0},  // ターゲット: ポーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 16.0, 16.0},  // ターゲット: ナイト。
        {0.0, 0.0, 0.0, 0.0, 0.0, 16.0, 16.0},  // ターゲット: ビショップ。
        {0.0, 0.0, 0.0, 0.0, 0.0,  0.0,  0.0},  // ターゲット: ルーク。
        {0.0, 0.0, 0.0, 0.0, 0.0, 45.3, 45.3},  // ターゲット: クイーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 45.3,  0.0}   // ターゲット: キング。
      },
      {  // クイーンでピン。
        // バック:{Empty, ポーン, ナイト, ビショップ, ルーク, クイーン, キング}
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.0},  // ターゲット: Empty。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  4.0},  // ターゲット: ポーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 16.0},  // ターゲット: ナイト。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 16.0},  // ターゲット: ビショップ。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 22.6},  // ターゲット: ルーク。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.0},  // ターゲット: クイーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.0}   // ターゲット: キング。
      },
      {  // キングでピン。
        // バック:{Empty, ポーン, ナイト, ビショップ, ルーク, クイーン, キング}
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: Empty。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ポーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ナイト。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ビショップ。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: ルーク。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // ターゲット: クイーン。
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}   // ターゲット: キング。
      }
    };
    COPY_ARRAY(pin_value_table_, PIN);

    // ポーンの盾の配置の価値テーブルを初期化。
    static const double PAWN_SHIELD[NUM_SQUARES] {
         0.0,    0.0,    0.0,    0.0,    0.0,    0.0,    0.0,    0.0,
        32.0,   32.0,   32.0,   32.0,   32.0,   32.0,   38.1,   32.0,
         0.0,    0.0,    0.0,    0.0,    0.0,    0.0,    0.0,    0.0,
       -22.6,  -22.6,  -22.6,  -22.6,  -22.6,  -22.6,  -22.6,  -22.6,
       -45.2,  -45.2,  -45.2,  -45.2,  -45.2,  -45.2,  -45.2,  -45.2,
       -67.8,  -67.8,  -67.8,  -67.8,  -67.8,  -67.8,  -67.8,  -67.8,
       -90.4,  -90.4,  -90.4,  -90.4,  -90.4,  -90.4,  -90.4,  -90.4,
      -113.0, -113.0, -113.0, -113.0, -113.0, -113.0, -113.0, -113.0
    };
    COPY_ARRAY(pawn_shield_value_table_, PAWN_SHIELD);

    // オープニング時の駒の配置のウェイトを初期化。
    weight_opening_position_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_opening_position_[PAWN] = Weight::CreateWeight(1.0, 0.0);
    weight_opening_position_[KNIGHT] = Weight::CreateWeight(1.0, 0.0);
    weight_opening_position_[BISHOP] = Weight::CreateWeight(1.0, 0.0);
    weight_opening_position_[ROOK] = Weight::CreateWeight(1.0, 0.0);
    weight_opening_position_[QUEEN] = Weight::CreateWeight(1.0, 0.0);
    weight_opening_position_[KING] = Weight::CreateWeight(1.0, 0.0);

    // エンディング時の駒の配置のウェイトを初期化。
    weight_ending_position_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_ending_position_[PAWN] = Weight::CreateWeight(0.0, 1.0);
    weight_ending_position_[KNIGHT] = Weight::CreateWeight(0.0, 1.0);
    weight_ending_position_[BISHOP] = Weight::CreateWeight(0.0, 1.0);
    weight_ending_position_[ROOK] = Weight::CreateWeight(0.0, 1.0);
    weight_ending_position_[QUEEN] = Weight::CreateWeight(0.0, 1.0);
    weight_ending_position_[KING] = Weight::CreateWeight(0.0, 1.0);

    // 機動力のウェイトを初期化。
    weight_mobility_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_mobility_[PAWN] = Weight::CreateWeight(1.2, 2.4);
    weight_mobility_[KNIGHT] = Weight::CreateWeight(0.8, 1.7);
    weight_mobility_[BISHOP] = Weight::CreateWeight(9.5, 13.5);
    weight_mobility_[ROOK] = Weight::CreateWeight(4.8, 8.0);
    weight_mobility_[QUEEN] = Weight::CreateWeight(2.8, 6.7);
    weight_mobility_[KING] = Weight::CreateWeight(0.0, 1.4);

    // センターコントロールのウェイトを初期化。
    weight_center_control_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_center_control_[PAWN] = Weight::CreateWeight(4.8, 1.0);
    weight_center_control_[KNIGHT] = Weight::CreateWeight(8.0, 1.2);
    weight_center_control_[BISHOP] = Weight::CreateWeight(4.8, 1.0);
    weight_center_control_[ROOK] = Weight::CreateWeight(2.0, 1.7);
    weight_center_control_[QUEEN] = Weight::CreateWeight(1.4, 1.2);
    weight_center_control_[KING] = Weight::CreateWeight(0.0, 6.7);

    // スウィートセンターのコントロールのウェイトを初期化。
    weight_sweet_center_control_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_sweet_center_control_[PAWN] = Weight::CreateWeight(4.0, 0.0);
    weight_sweet_center_control_[KNIGHT] = Weight::CreateWeight(3.4, 0.0);
    weight_sweet_center_control_[BISHOP] = Weight::CreateWeight(1.4, 0.0);
    weight_sweet_center_control_[ROOK] = Weight::CreateWeight(0.6, 1.0);
    weight_sweet_center_control_[QUEEN] = Weight::CreateWeight(1.0, 0.0);
    weight_sweet_center_control_[KING] = Weight::CreateWeight(0.0, 2.8);

    // 展開のウェイトを初期化。
    weight_development_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_development_[PAWN] = Weight::CreateWeight(1.2, 0.0);
    weight_development_[KNIGHT] = Weight::CreateWeight(38.1, 0.0);
    weight_development_[BISHOP] = Weight::CreateWeight(32.0, 0.0);
    weight_development_[ROOK] = Weight::CreateWeight(0.0, 0.0);
    weight_development_[QUEEN] = Weight::CreateWeight(0.0, 0.0);
    weight_development_[KING] = Weight::CreateWeight(0.0, 0.0);

    // 相手への攻撃のウェイトを初期化。
    weight_attack_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_attack_[PAWN] = Weight::CreateWeight(1.0, 1.0);
    weight_attack_[KNIGHT] = Weight::CreateWeight(1.0, 1.0);
    weight_attack_[BISHOP] = Weight::CreateWeight(0.8, 1.0);
    weight_attack_[ROOK] = Weight::CreateWeight(1.0, 1.0);
    weight_attack_[QUEEN] = Weight::CreateWeight(1.0, 1.0);
    weight_attack_[KING] = Weight::CreateWeight(1.0, 1.0);

    // 味方への防御のウェイトを初期化。
    weight_defense_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_defense_[PAWN] = Weight::CreateWeight(1.0, 1.0);
    weight_defense_[KNIGHT] = Weight::CreateWeight(1.0, 1.0);
    weight_defense_[BISHOP] = Weight::CreateWeight(1.0, 1.0);
    weight_defense_[ROOK] = Weight::CreateWeight(1.0, 1.0);
    weight_defense_[QUEEN] = Weight::CreateWeight(1.0, 1.0);
    weight_defense_[KING] = Weight::CreateWeight(0.5, 1.0);

    // ピンのウェイトを初期化。
    weight_pin_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_pin_[PAWN] = Weight::CreateWeight(0.0, 0.0);
    weight_pin_[KNIGHT] = Weight::CreateWeight(0.0, 0.0);
    weight_pin_[BISHOP] = Weight::CreateWeight(1.0, 1.0);
    weight_pin_[ROOK] = Weight::CreateWeight(1.0, 1.0);
    weight_pin_[QUEEN] = Weight::CreateWeight(1.0, 1.0);
    weight_pin_[KING] = Weight::CreateWeight(0.0, 0.0);

    // 相手キング周辺への攻撃のウェイトを初期化。
    weight_attack_around_king_[EMPTY] = Weight::CreateWeight(0.0, 0.0);
    weight_attack_around_king_[PAWN] = Weight::CreateWeight(0.0, 0.0);
    weight_attack_around_king_[KNIGHT] = Weight::CreateWeight(0.0, 0.0);
    weight_attack_around_king_[BISHOP] = Weight::CreateWeight(8.0, 4.0);
    weight_attack_around_king_[ROOK] = Weight::CreateWeight(8.0, 4.8);
    weight_attack_around_king_[QUEEN] = Weight::CreateWeight(8.0, 4.0);
    weight_attack_around_king_[KING] = Weight::CreateWeight(0.0, 4.0);
  }

  // コピーコンストラクタ。
  EvalParams::EvalParams(const EvalParams& params) {
    ScanMember(params);
  }

  // ムーブコンストラクタ。
  EvalParams::EvalParams(EvalParams&& params) {
    ScanMember(params);
  }

  // コピー代入演算子。
  EvalParams& EvalParams::operator=(const EvalParams& params) {
    ScanMember(params);
    return *this;
  }

  // ムーブ代入演算子。
  EvalParams& EvalParams::operator=(EvalParams&& params) {
    ScanMember(params);
    return *this;
  }

  // オープニング時の駒の配置の価値テーブルのミューテータ。
  void EvalParams::opening_position_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]) {
    COPY_ARRAY(opening_position_value_table_, table);
  }

  // エンディング時の駒の配置の価値テーブルのミューテータ。
  void EvalParams::ending_position_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]) {
    COPY_ARRAY(ending_position_value_table_, table);
  }

  // 相手への攻撃の価値テーブルのミューテータ。
  void EvalParams::attack_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES]) {
    COPY_ARRAY(attack_value_table_, table);
  }

  // 味方への防御の価値テーブルのミューテータ。
  void EvalParams::defense_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES]) {
    COPY_ARRAY(defense_value_table_, table);
  }

  // ピンの価値テーブルのミューテータ。
  void EvalParams::pin_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES][NUM_PIECE_TYPES]) {
    COPY_ARRAY(pin_value_table_, table);
  }

  // ポーンの盾の配置の価値テーブルのミューテータ。
  void EvalParams::pawn_shield_value_table
  (const double (& table)[NUM_SQUARES]) {
    COPY_ARRAY(pawn_shield_value_table_, table);
  }

  // オープニング時の駒の配置のウェイトのミューテータ。
  void EvalParams::weight_opening_position
  (const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_opening_position_[piece_type] = weights[piece_type];
    }
  }

  // オープニング時の駒の配置のウェイトのミューテータ。
  void EvalParams::weight_ending_position
  (const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_ending_position_[piece_type] = weights[piece_type];
    }
  }

  // 機動力のウェイトのミューテータ。
  void EvalParams::weight_mobility
  (const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_mobility_[piece_type] = weights[piece_type];
    }
  }

  // センターコントロールのウェイトのミューテータ。
  void EvalParams::weight_center_control
  (const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_center_control_[piece_type] = weights[piece_type];
    }
  }

  // スウィートセンターのコントロールのウェイトのミューテータ。
  void EvalParams::weight_sweet_center_control
  (const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_sweet_center_control_[piece_type] = weights[piece_type];
    }
  }

  // 展開のウェイトのミューテータ。
  void EvalParams::weight_development
  (const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_development_[piece_type] = weights[piece_type];
    }
  }

  // 相手への攻撃のウェイトのミューテータ。
  void EvalParams::weight_attack(const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_attack_[piece_type] = weights[piece_type];
    }
  }

  // 味方への防御のウェイトのミューテータ。
  void EvalParams::weight_defense(const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_defense_[piece_type] = weights[piece_type];
    }
  }

  // ピンのウェイトのミューテータ。
  void EvalParams::weight_pin(const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_pin_[piece_type] = weights[piece_type];
    }
  }

  // 相手キング周辺への攻撃のウェイトのミューテータ。
  void EvalParams::weight_attack_around_king
  (const Weight (& weights)[NUM_PIECE_TYPES]) {
    FOR_PIECE_TYPES(piece_type) {
      weight_attack_around_king_[piece_type] = weights[piece_type];
    }
  }

  // メンバをコピーする。
  void EvalParams::ScanMember(const EvalParams& params) {
    // 価値テーブルをコピー。
    COPY_ARRAY(opening_position_value_table_,
    params.opening_position_value_table_);

    COPY_ARRAY(ending_position_value_table_,
    params.ending_position_value_table_);

    COPY_ARRAY(attack_value_table_,
    params.attack_value_table_);

    COPY_ARRAY(defense_value_table_,
    params.defense_value_table_);

    COPY_ARRAY(pin_value_table_,
    params.pin_value_table_);

    COPY_ARRAY(pawn_shield_value_table_,
    params.pawn_shield_value_table_);

    // ウェイトをコピー。
    FOR_PIECE_TYPES(piece_type) {
      weight_opening_position_[piece_type] =
      params.weight_opening_position_[piece_type];

      weight_ending_position_[piece_type] =
      params.weight_ending_position_[piece_type];

      weight_mobility_[piece_type] = params.weight_mobility_[piece_type];

      weight_center_control_[piece_type] =
      params.weight_center_control_[piece_type];

      weight_sweet_center_control_[piece_type] =
      params.weight_sweet_center_control_[piece_type];

      weight_development_[piece_type] = params.weight_development_[piece_type];

      weight_attack_[piece_type] = params.weight_attack_[piece_type];

      weight_attack_around_king_[piece_type] =
      params.weight_attack_around_king_[piece_type];
    }
    weight_pass_pawn_ = params.weight_pass_pawn_;
    weight_protected_pass_pawn_ = params.weight_protected_pass_pawn_;
    weight_double_pawn_ = params.weight_double_pawn_;
    weight_iso_pawn_ = params.weight_iso_pawn_;
    weight_pawn_shield_ = params.weight_pawn_shield_;
    weight_bishop_pair_ = params.weight_bishop_pair_;
    weight_bad_bishop_ = params.weight_bad_bishop_;
    weight_rook_pair_ = params.weight_rook_pair_;
    weight_rook_semiopen_fyle_ = params.weight_rook_semiopen_fyle_;
    weight_rook_open_fyle_ = params.weight_rook_open_fyle_;
    weight_early_queen_starting_ = params.weight_early_queen_starting_;
    weight_weak_square_ = params.weight_weak_square_;
    weight_castling_ = params.weight_castling_;
    weight_abandoned_castling_ = params.weight_abandoned_castling_;
  }
}  // namespace Sayuri
