/*
   params.h: 探索アルゴリズム、評価関数を変更するパラメータのクラスの実装。

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

# include "params.h"

#include <iostream>
#include <cstdint>

namespace Sayuri {
  /******************/
  /* SearchParams。 */
  /******************/
  // コンストラクタ。
  SearchParams::SearchParams() :
  ybwc_after_(3),
  enable_aspiration_windows_(true),
  aspiration_windows_limit_depth_(5),
  aspiration_windows_delta_(15),
  enable_see_(true),
  enable_history_(true),
  enable_killer_(true),
  enable_killer_2_(true),
  enable_ttable_(true),
  enable_iid_(true),
  iid_limit_depth_(5),
  iid_search_depth_(4),
  enable_nmr_(true),
  nmr_limit_depth_(4),
  nmr_search_reduction_(3),
  nmr_reduction_(3),
  enable_probcut_(true),
  probcut_limit_depth_(5),
  probcut_margin_(200),
  probcut_search_reduction_(4),
  enable_history_pruning_(true),
  history_pruning_limit_depth_(4),
  history_pruning_move_threshold_(0.5),
  history_pruning_after_(10),
  history_pruning_threshold_(0.5),
  history_pruning_reduction_(1),
  enable_lmr_(true),
  lmr_limit_depth_(4),
  lmr_threshold_(0.20),
  lmr_after_(4),
  lmr_search_reduction_(1),
  enable_futility_pruning_(true),
  futility_pruning_depth_(3),
  futility_pruning_margin_(300) {
    // ヒストリーをチェック。
    if (!enable_history_) enable_history_pruning_ = false;
    if (!enable_killer_) enable_killer_2_ = false;
  }

  // コピーコンストラクタ。
  SearchParams::SearchParams(const SearchParams& params) {
    ScanMember(params);
  }

  // ムーブコンストラクタ。
  SearchParams::SearchParams(SearchParams&& params) {
    ScanMember(params);
  }

  // コピー代入。
  SearchParams& SearchParams::operator=(const SearchParams& params) {
    ScanMember(params);
    return *this;
  }

  // ムーブ代入。
  SearchParams& SearchParams::operator=(SearchParams&& params) {
    ScanMember(params);
    return *this;
  }

  // メンバをコピーする。
  void SearchParams::ScanMember(const SearchParams& params) {
    ybwc_after_ = params.ybwc_after_;
    enable_aspiration_windows_ = params.enable_aspiration_windows_;
    aspiration_windows_limit_depth_ = params.aspiration_windows_limit_depth_;
    aspiration_windows_delta_ = params.aspiration_windows_delta_;
    enable_see_ = params.enable_see_;
    enable_history_ = params.enable_history_;
    enable_killer_ = params.enable_killer_;
    enable_killer_2_ = params.enable_killer_2_;
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
    history_pruning_after_ = params.history_pruning_after_;
    history_pruning_threshold_ = params.history_pruning_threshold_;
    history_pruning_reduction_ = params.history_pruning_reduction_;
    enable_lmr_ = params.enable_lmr_;
    lmr_limit_depth_ = params.lmr_limit_depth_;
    lmr_threshold_ = params.lmr_threshold_;
    lmr_after_ =params.lmr_after_;
    lmr_search_reduction_ = params.lmr_search_reduction_;
    enable_futility_pruning_ = params.enable_futility_pruning_;
    futility_pruning_depth_ = params.futility_pruning_depth_;
    futility_pruning_margin_ = params.futility_pruning_margin_;
  }

  /****************/
  /* EvalParams。 */
  /****************/
  // コンストラクタ。
  EvalParams::EvalParams() :
  weight_mobility_(1.0, 1.0),
  weight_center_control_(0.5, 0.0),
  weight_sweet_center_control_(0.5, 0.0),
  weight_development_(2.5, 0.0),
  weight_attack_around_king_(0.0, 3.0),
  weight_pass_pawn_(7.0, 14.0),
  weight_protected_pass_pawn_(2.5, 2.5),
  weight_double_pawn_(-2.5, -5.0),
  weight_iso_pawn_(-5.0, -2.5),
  weight_pawn_shield_(15.0, 0.0),
  weight_bishop_pair_(10.0, 60.0),
  weight_bad_bishop_(-0.7, 0.0),
  weight_pin_knight_(10.0, 0.0),
  weight_rook_pair_(10.0, 20.0),
  weight_rook_semiopen_fyle_(3.5, 3.5),
  weight_rook_open_fyle_(3.5, 3.5),
  weight_early_queen_launched_(-20.0, 0.0),
  weight_weak_square_(-5.0, 0.0),
  weight_castling_(90.0, 0.0),
  weight_abandoned_castling_(-45.0, 0.0) {
    // オープニング時の駒の配置の価値テーブルの初期化。
    constexpr double TABLE_1[NUM_PIECE_TYPES][NUM_SQUARES] {
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
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0,
        2.0, 3.0, 4.0, 5.0, 5.0, 4.0, 3.0, 2.0,
        3.0, 4.0, 5.0, 6.0, 6.0, 5.0, 4.0, 3.0,
        4.0, 5.0, 6.0, 7.0, 7.0, 6.0, 5.0, 4.0,
        5.0, 6.0, 7.0, 8.0, 8.0, 7.0, 6.0, 5.0,
        6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0
      },
      {  // ナイト。
        -3.0, -2.0, -1.0, -1.0, -1.0, -1.0, -2.0, -3.0,
        -2.0, -1.0,  0.0,  0.0,  0.0,  0.0, -1.0, -2.0,
        -1.0,  0.0,  1.0,  1.0,  1.0,  1.0,  0.0, -1.0,
         0.0,  1.0,  2.0,  2.0,  2.0,  2.0,  1.0,  0.0,
         1.0,  2.0,  3.0,  3.0,  3.0,  3.0,  2.0,  1.0,
         2.0,  3.0,  4.0,  4.0,  4.0,  4.0,  3.0,  2.0,
         1.0,  2.0,  3.0,  3.0,  3.0,  3.0,  2.0,  1.0,
         0.0,  1.0,  2.0,  2.0,  2.0,  2.0,  1.0,  0.0
      },
      {  // ビショップ。
        2.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0,
        1.0, 3.0, 1.0, 2.0, 2.0, 1.0, 3.0, 1.0,
        1.0, 2.0, 3.0, 2.0, 2.0, 3.0, 2.0, 1.0,
        0.0, 1.0, 3.0, 3.0, 3.0, 3.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 2.0, 2.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
      },
      {  // ルーク。
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0,
        4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0
      },
      {  // クイーン。
        -3.0, -2.0, -2.0, -1.0, -1.0, -2.0, -2.0, -3.0,
        -2.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -2.0,
        -2.0,  0.0,  1.0,  1.0,  1.0,  1.0,  0.0, -2.0,
        -1.0,  0.0,  1.0,  2.0,  2.0,  1.0,  0.0, -1.0,
        -1.0,  0.0,  1.0,  2.0,  2.0,  1.0,  0.0, -1.0,
        -2.0,  0.0,  1.0,  1.0,  1.0,  1.0,  0.0, -2.0,
        -2.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -2.0,
        -3.0, -2.0, -2.0, -1.0, -1.0, -2.0, -2.0, -3.0
      },
      {  // キング。
         1.0,  1.0,  0.0, -1.0, -1.0,  0.0,  1.0,  1.0,
         0.0,  0.0, -1.0, -2.0, -2.0, -1.0,  0.0,  0.0,
        -1.0, -1.0, -2.0, -3.0, -3.0, -2.0, -1.0, -1.0,
        -2.0, -2.0, -3.0, -4.0, -4.0, -3.0, -2.0, -2.0,
        -2.0, -2.0, -3.0, -4.0, -4.0, -3.0, -2.0, -2.0,
        -1.0, -1.0, -2.0, -3.0, -3.0, -2.0, -1.0, -1.0,
         0.0,  0.0, -1.0, -2.0, -2.0, -1.0,  0.0,  0.0,
         1.0,  1.0,  0.0, -1.0, -1.0,  0.0,  1.0,  1.0
      }
    };
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        opening_position_value_table_[piece_type][square] =
        TABLE_1[piece_type][square];
      }
    }

    // エンディング時の駒の配置の価値テーブルを初期化。
    constexpr double TABLE_2[NUM_PIECE_TYPES][NUM_SQUARES] {
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
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
        2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 
        3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 
        4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 
        5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 
        6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0 
      },
      {  // ナイト。
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
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
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0,
        1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0,
        2.0, 3.0, 4.0, 5.0, 5.0, 4.0, 3.0, 2.0,
        3.0, 4.0, 5.0, 6.0, 6.0, 5.0, 4.0, 3.0,
        3.0, 4.0, 5.0, 6.0, 6.0, 5.0, 4.0, 3.0,
        2.0, 3.0, 4.0, 5.0, 5.0, 4.0, 3.0, 2.0,
        1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0,
        0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 0.0
      }
    };
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        ending_position_value_table_[piece_type][square] =
        TABLE_2[piece_type][square];
      }
    }

    // 駒への攻撃の価値テーブルを初期化。
    constexpr double TABLE_3[NUM_PIECE_TYPES][NUM_PIECE_TYPES] {
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 攻撃側: Empty。
      {0.0, 4.0, 5.0, 5.0, 6.0, 7.0, 7.0},  // 攻撃側: ポーン。
      {0.0, 3.0, 4.0, 4.0, 5.0, 6.0, 6.0},  // 攻撃側: ナイト。
      {0.0, 3.0, 4.0, 4.0, 5.0, 6.0, 6.0},  // 攻撃側: ビショップ。
      {0.0, 2.0, 3.0, 3.0, 4.0, 5.0, 5.0},  // 攻撃側: ルーク。
      {0.0, 1.0, 2.0, 2.0, 3.0, 4.0, 4.0},  // 攻撃側: クイーン。
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}   // 攻撃側: キング。
    };
    for (Piece type_1 = 0; type_1 < NUM_PIECE_TYPES; type_1++) {
      for (Piece type_2 = 0; type_2 < NUM_PIECE_TYPES; type_2++) {
        attack_value_table_[type_1][type_2] = TABLE_3[type_1][type_2];
      }
    }

    // ポーンの盾の配置の価値テーブルを初期化。
    constexpr double TABLE_4[NUM_SQUARES] {
      7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0,
      6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0,
      5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
      4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0,
      3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0,
      2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0,
      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
    };
    for (Square square = 0; square < NUM_SQUARES; square++) {
      pawn_shield_value_table_[square] = TABLE_4[square];
    }

    // オープニング時の駒の配置のウェイトを初期化。
    weight_opening_position_[EMPTY] = Weight(0.0, 0.0);  // EMPTY。
    weight_opening_position_[PAWN] = Weight(2.0, 0.0);  // ポーン。
    weight_opening_position_[KNIGHT] = Weight(2.5, 0.0);  // ナイト。
    weight_opening_position_[BISHOP] = Weight(3.5, 0.0);  // ビショップ。
    weight_opening_position_[ROOK] = Weight(2.5, 0.0);  // ルーク。
    weight_opening_position_[QUEEN] = Weight(2.5, 0.0);  // クイーン。
    weight_opening_position_[KING] = Weight(10.0, 0.0);  // キング。

    // エンディング時の駒の配置のウェイトを初期化。
    weight_ending_position_[EMPTY] = Weight(0.0, 0.0);  // EMPTY。
    weight_ending_position_[PAWN] = Weight(0.0, 20.0);  // ポーン。
    weight_ending_position_[KNIGHT] = Weight(0.0, 0.0);  // ナイト。
    weight_ending_position_[BISHOP] = Weight(0.0, 0.0);  // ビショップ。
    weight_ending_position_[ROOK] = Weight(0.0, 0.0);  // ルーク。
    weight_ending_position_[QUEEN] = Weight(0.0, 0.0);  // クイーン。
    weight_ending_position_[KING] = Weight(0.0, 15.0);  // キング。

    // 駒への攻撃のウェイトを初期化。
    weight_attack_[EMPTY] = Weight(0.0, 0.0);  // EMPTY。
    weight_attack_[PAWN] = Weight(2.0, 0.0);  // PAWN。
    weight_attack_[KNIGHT] = Weight(2.0, 0.0);  // KNIGHT。
    weight_attack_[BISHOP] = Weight(2.0, 0.0);  // BISHOP。
    weight_attack_[ROOK] = Weight(2.0, 0.0);  // ROOK。
    weight_attack_[QUEEN] = Weight(2.0, 0.0);  // QUEEN。
    weight_attack_[KING] = Weight(2.0, 0.0);  // KING。
  }

  // コピーコンストラクタ。
  EvalParams::EvalParams(const EvalParams& params) {
    ScanMember(params);
  }

  // ムーブコンストラクタ。
  EvalParams::EvalParams(EvalParams&& params) {
    ScanMember(params);
  }

  // コピー代入。
  EvalParams& EvalParams::operator=(const EvalParams& params) {
    ScanMember(params);
    return *this;
  }

  // ムーブ代入。
  EvalParams& EvalParams::operator=(EvalParams&& params) {
    ScanMember(params);
    return *this;
  }

  // オープニング時の駒の配置の価値テーブルのミューテータ。
  void EvalParams::opening_position_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]) {
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        opening_position_value_table_[piece_type][square] =
        table[piece_type][square];
      }
    }
  }

  // エンディング時の駒の配置の価値テーブルのミューテータ。
  void EvalParams::ending_position_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]) {
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        ending_position_value_table_[piece_type][square] =
        table[piece_type][square];
      }
    }
  }

  // 駒への攻撃の価値テーブルのミューテータ。
  void EvalParams::attack_value_table
  (const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES]) {
    for (Piece type_1 = 0; type_1 < NUM_PIECE_TYPES; type_1++) {
      for (Piece type_2 = 0; type_2 < NUM_PIECE_TYPES; type_2++) {
        attack_value_table_[type_1][type_2] = table[type_1][type_2];
      }
    }
  }

  // ポーンの盾の配置の価値テーブルのミューテータ。
  void EvalParams::pawn_shield_value_table
  (const double (& table)[NUM_SQUARES]) {
    for (Square square = 0; square < NUM_SQUARES; square++) {
      pawn_shield_value_table_[square] = table[square];
    }
  }

  // オープニング時の駒の配置のウェイトのミューテータ。
  void EvalParams::weight_opening_position
  (const Weight (& weights)[NUM_PIECE_TYPES]) {
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      weight_opening_position_[piece_type] = weights[piece_type];
    }
  }

  // オープニング時の駒の配置のウェイトのミューテータ。
  void EvalParams::weight_ending_position
  (const Weight (& weights)[NUM_PIECE_TYPES]) {
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      weight_ending_position_[piece_type] = weights[piece_type];
    }
  }

  // 駒への攻撃のウェイトのミューテータ。
  void EvalParams::weight_attack(const Weight (& weights)[NUM_PIECE_TYPES]) {
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      weight_attack_[piece_type] = weights[piece_type];
    }
  }

  // メンバをコピーする。
  void EvalParams::ScanMember(const EvalParams& params) {
    // 価値テーブルをコピー。
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        opening_position_value_table_[piece_type][square] =
        params.opening_position_value_table_[piece_type][square];
      }
    }
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        ending_position_value_table_[piece_type][square] =
        params.ending_position_value_table_[piece_type][square];
      }
    }
    for (Piece type_1 = 0; type_1 < NUM_PIECE_TYPES; type_1++) {
      for (Piece type_2 = 0; type_2 < NUM_PIECE_TYPES; type_2++) {
        attack_value_table_[type_1][type_2] =
        params.attack_value_table_[type_1][type_2];
      }
    }
    for (Square square = 0; square < NUM_SQUARES; square++) {
      pawn_shield_value_table_[square] =
      params.pawn_shield_value_table_[square];
    }

    // ウェイトをコピー。
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      weight_opening_position_[piece_type] =
      params.weight_opening_position_[piece_type];
    }
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      weight_ending_position_[piece_type] =
      params.weight_ending_position_[piece_type];
    }
    weight_mobility_ = params.weight_mobility_;
    weight_center_control_ = params.weight_center_control_;
    weight_sweet_center_control_ = params.weight_sweet_center_control_;
    weight_development_ = params.weight_development_;
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      weight_attack_[piece_type] = params.weight_attack_[piece_type];
    }
    weight_attack_around_king_ = params.weight_attack_around_king_;
    weight_pass_pawn_ = params.weight_pass_pawn_;
    weight_protected_pass_pawn_ = params.weight_protected_pass_pawn_;
    weight_double_pawn_ = params.weight_double_pawn_;
    weight_iso_pawn_ = params.weight_iso_pawn_;
    weight_pawn_shield_ = params.weight_pawn_shield_;
    weight_bishop_pair_ = params.weight_bishop_pair_;
    weight_bad_bishop_ = params.weight_bad_bishop_;
    weight_pin_knight_ = params.weight_pin_knight_;
    weight_rook_pair_ = params.weight_rook_pair_;
    weight_rook_semiopen_fyle_ = params.weight_rook_semiopen_fyle_;
    weight_rook_open_fyle_ = params.weight_rook_open_fyle_;
    weight_early_queen_launched_ = params.weight_early_queen_launched_;
    weight_weak_square_ = params.weight_weak_square_;
    weight_castling_ = params.weight_castling_;
    weight_abandoned_castling_ = params.weight_abandoned_castling_;
  }
}  // namespace Sayuri
