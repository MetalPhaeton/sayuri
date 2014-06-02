/*
   params.h: 探索アルゴリズム、評価関数を変更するパラメータのクラス。

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

#ifndef PARAMS_H
#define PARAMS_H

#include <iostream>
#include <cstdint>
#include "common.h"

namespace Sayuri {
  // 探索関数用パラメータのクラス。
  class SearchParams {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      SearchParams();
      SearchParams(const SearchParams& params);
      SearchParams(SearchParams&& params);
      SearchParams& operator=(const SearchParams& params);
      SearchParams& operator=(SearchParams&& params);
      virtual ~SearchParams() {}

      /**************/
      /* アクセサ。 */
      /**************/
      // マテリアル。
      const int (& material() const)[NUM_PIECE_TYPES] {return material_;}

      // YBWC。
      // 深さ制限。
      int ybwc_limit_depth() const {return ybwc_limit_depth_;}
      // 指定の候補手数以上で実行する。
      int ybwc_after() const {return ybwc_after_;}

      // Aspiration Windows。
      // 有効かどうか。
      bool enable_aspiration_windows() const {
        return enable_aspiration_windows_;
      }
      // 残り深さの制限。
      std::uint32_t aspiration_windows_limit_depth() const {
        return aspiration_windows_limit_depth_;
      }
      // デルタ値。
      int aspiration_windows_delta() const {return aspiration_windows_delta_;}

      // SEE。
      // 有効かどうか。
      bool enable_see() const {return enable_see_;}

      // ヒストリー。
      // 有効かどうか。
      bool enable_history() const {return enable_history_;}

      // キラームーブ。
      // 有効かどうか。
      bool enable_killer() const {return enable_killer_;}
      // 2プライ先のキラームーブが有効かどうか。
      bool enable_killer_2() const {return enable_killer_2_;}

      // トランスポジションテーブル。
      // 有効かどうか。
      bool enable_ttable() const {return enable_ttable_;}

      // Internal Iterative Deepening。
      // 有効かどうか。
      bool enable_iid() const {return enable_iid_;}
      // 残り深さの制限。
      int iid_limit_depth() const {return iid_limit_depth_;}
      // IIDで読む深さ。
      int iid_search_depth() const {return iid_search_depth_;}

      // Null Move Reduction。
      // 有効かどうか。
      bool enable_nmr() const {return enable_nmr_;}
      // 残り深さの制限。
      int nmr_limit_depth() const {return nmr_limit_depth_;}
      // 何プライ浅く読むか。
      int nmr_search_reduction() const {return nmr_search_reduction_;}
      // リダクションする深さ。
      int nmr_reduction() const {return nmr_reduction_;}

      // ProbCut。
      // 有効かどうか。
      bool enable_probcut() const {return enable_probcut_;}
      // 残り深さの制限。
      int probcut_limit_depth() const {return probcut_limit_depth_;}
      // ベータ値増加分。
      int probcut_margin() const {return probcut_margin_;}
      // 何プライ浅く読むか。
      int probcut_search_reduction() const {return probcut_search_reduction_;}

      // History Pruning。
      // (ヒストリーが無効の場合は設定にかかわらず無効になる。)
      // 有効かどうか。
      bool enable_history_pruning() const {return enable_history_pruning_;}
      // 残り深さの制限。
      int history_pruning_limit_depth() const {
        return history_pruning_limit_depth_;
      }
      // 全候補手中、枝刈りしない手数の閾値。 1.0から0.0。
      double history_pruning_move_threshold() const {
        return history_pruning_move_threshold_;
      }
      // 指定の候補手数以上で実行する。
      int history_pruning_after() const {
        return history_pruning_after_;
      }
      // 最大値に対する閾値。 1.0から0.0。
      double history_pruning_threshold() const {
        return history_pruning_threshold_;
      }
      // リダクションする深さ。
      int history_pruning_reduction() const {
        return history_pruning_reduction_;
      }

      // Late Move Reduction。
      // 有効かどうか。
      bool enable_lmr() const {return enable_lmr_;}
      // 残り深さの制限。
      int lmr_limit_depth() const {return lmr_limit_depth_;}
      // 全候補手中、リダクションしない手数の閾値。 1.0から0.0。
      double lmr_threshold() const {return lmr_threshold_;}
      // 指定の候補手数以上で実行する。
      int lmr_after() const {return lmr_after_;}
      // リダクションする深さ。
      int lmr_search_reduction() const {return lmr_search_reduction_;}

      // Futility Pruning。
      // 有効かどうか。
      bool enable_futility_pruning() const {return enable_futility_pruning_;}
      // 有効にする残り深さ。
      int futility_pruning_depth() const {return futility_pruning_depth_;}
      // 1プライあたりのマージン。
      int futility_pruning_margin() const {return futility_pruning_margin_;}

      /******************/
      /* ミューテータ。 */
      /******************/
      // マテリアル。
      void material(const int (& table)[NUM_PIECE_TYPES]);

      // YBWC。
      // 深さ制限。
      void ybwc_limit_depth(int depth) {ybwc_limit_depth_ = depth;}
      // 指定の候補手数以上で実行する。
      void ybwc_after(int num_moves) {ybwc_after_ = num_moves;}

      // Aspiration Windows。
      // 有効かどうか。
      void enable_aspiration_windows(bool enable) {
        enable_aspiration_windows_ = enable;
      }
      // 残り深さの制限。
      void aspiration_windows_limit_depth(std::uint32_t depth) {
        aspiration_windows_limit_depth_ = depth;
      }
      // デルタ値。
      void aspiration_windows_delta(int delta) {
        aspiration_windows_delta_ = delta;
      }

      // SEE。
      // 有効かどうか。
      void enable_see(bool enable) {enable_see_ = enable;}

      // ヒストリー。
      // 有効かどうか。
      void enable_history(bool enable) {enable_history_ = enable;}

      // キラームーブ。
      // 有効かどうか。
      void enable_killer(bool enable) {enable_killer_ = enable;}
      // 2プライ先のキラームーブが有効かどうか。
      void enable_killer_2(bool enable) {
        if (enable_killer_) enable_killer_2_ = enable;
        else enable_killer_2_ = false;
      }

      // トランスポジションテーブル。
      // 有効かどうか。
      void enable_ttable(bool enable) {enable_ttable_ = enable;}

      // Internal Iterative Deepening。
      // 有効かどうか。
      void enable_iid(bool enable) {enable_iid_ = enable;}
      // 残り深さの制限。
      void iid_limit_depth(int depth) {iid_limit_depth_ = depth;}
      // IIDで読む深さ。
      void iid_search_depth(int depth) {iid_search_depth_ = depth;}

      // Null Move Reduction。
      // 有効かどうか。
      void enable_nmr(bool enable) {enable_nmr_ = enable;}
      // 残り深さの制限。
      void nmr_limit_depth(int depth) {nmr_limit_depth_ = depth;}
      // 何プライ浅く読むか。
      void nmr_search_reduction(int reduction) {
        nmr_search_reduction_ = reduction;
      }
      // リダクションする深さ。
      void nmr_reduction(int reduction) {nmr_reduction_ = reduction;}

      // ProbCut。
      // 有効かどうか。
      void enable_probcut(bool enable) {enable_probcut_ = enable;}
      // 残り深さの制限。
      void probcut_limit_depth(int depth) {probcut_limit_depth_ = depth;}
      // ベータ値増加分。
      void probcut_margin(int margin) {probcut_margin_ = margin;}
      // 何プライ浅く読むか。
      void probcut_search_reduction(int reduction) {
        probcut_search_reduction_ = reduction;
      }

      // History Pruning。
      // (ヒストリーが無効の場合は設定にかかわらず無効になる。)
      // 有効かどうか。
      void enable_history_pruning(bool enable) {
        if (enable_history_) enable_history_pruning_ = enable;
        else enable_history_pruning_ = false;
      }
      // 残り深さの制限。
      void history_pruning_limit_depth(int depth) {
        history_pruning_limit_depth_ = depth;
      }
      // 全候補手中、枝刈りしない手数の閾値。 1.0から0.0。
      void history_pruning_move_threshold(double threshold) {
        history_pruning_move_threshold_ = threshold;
      }
      // 指定の候補手数以上で実行する。
      void history_pruning_after(int num_moves) {
        history_pruning_after_ = num_moves;
      }
      // 最大値に対する閾値。 1.0から0.0。
      void history_pruning_threshold(double threshold) {
        history_pruning_threshold_ = threshold;
      }
      // リダクションする深さ。
      void history_pruning_reduction(int reduction) {
        history_pruning_reduction_ = reduction;
      }

      // Late Move Reduction。
      // 有効かどうか。
      void enable_lmr(bool enable) {enable_lmr_ = enable;}
      // 残り深さの制限。
      void lmr_limit_depth(int depth) {lmr_limit_depth_ = depth;}
      // 全候補手中、リダクションしない手数の閾値。 1.0から0.0。
      void lmr_threshold(double threshold) {lmr_threshold_ = threshold;}
      // 指定の候補手数以上で実行する。
      void lmr_after(int num_moves) {lmr_after_ = num_moves;}
      // リダクションする深さ。
      void lmr_search_reduction(int reduction) {
        lmr_search_reduction_ = reduction;
      }

      // Futility Pruning。
      // 有効かどうか。
      void enable_futility_pruning(bool enable) {
        enable_futility_pruning_ = enable;
      }
      // 有効にする残り深さ。
      void futility_pruning_depth(int depth) {
        futility_pruning_depth_ = depth;
      }
      // 1プライあたりのマージン。
      void futility_pruning_margin(int margin) {
        futility_pruning_margin_ = margin;
      }

    private:
      /**********************/
      /* プライベート関数。 */
      /**********************/
      // メンバをコピーする。
      void ScanMember(const SearchParams& params);

      /****************/
      /* メンバ変数。 */
      /****************/
      // マテリアル。
      int material_[NUM_PIECE_TYPES];

      // YBWC。
      int ybwc_limit_depth_;  // 深さ制限。
      int ybwc_after_;  // 指定の候補手数以上で実行する。

      // Aspiration Windows。
      bool enable_aspiration_windows_;  // 有効かどうか。
      std::uint32_t aspiration_windows_limit_depth_;  // 残り深さの制限。
      int aspiration_windows_delta_;  // デルタ値。

      // SEE。
      bool enable_see_;  // 有効かどうか。

      // ヒストリー。
      bool enable_history_;  // 有効かどうか。

      // キラームーブ。
      bool enable_killer_;  // 有効かどうか。
      bool enable_killer_2_;  // 2プライ先のキラームーブが有効かどうか。

      // トランスポジションテーブル。
      bool enable_ttable_;  // 有効かどうか。

      // Internal Iterative Deepening。
      bool enable_iid_;  // 有効かどうか。
      int iid_limit_depth_;  // 残り深さの制限。
      int iid_search_depth_;  // IIDで読む深さ。

      // Null Move Reduction。
      bool enable_nmr_;  // 有効かどうか。
      int nmr_limit_depth_;  // 残り深さの制限。
      int nmr_search_reduction_;  // 何プライ浅く読むか。
      int nmr_reduction_;  // リダクションする深さ。

      // ProbCut。
      bool enable_probcut_;  // 有効かどうか。
      int probcut_limit_depth_;  // 残り深さの制限。
      int probcut_margin_;  // ベータ値増加分。
      int probcut_search_reduction_; // 何プライ浅く読むか。

      // History Pruning。
      // (ヒストリーが無効の場合は設定にかかわらず無効になる。)
      bool enable_history_pruning_;  // 有効かどうか。
      int history_pruning_limit_depth_;  // 残り深さの制限。
      // 全候補手中、枝刈りしない手数の閾値。 1.0から0.0。
      double history_pruning_move_threshold_;
      // 指定の候補手数以上で実行する。
      int history_pruning_after_;
      // 最大値に対する閾値。 1.0 から 0.0。
      double history_pruning_threshold_;
      int history_pruning_reduction_;  // リダクションする深さ。

      // Late Move Reduction。
      bool enable_lmr_;  // 有効かどうか。
      int lmr_limit_depth_;  // 残り深さの制限。
      // 全候補手中、リダクションしない手数の閾値。 1.0 から 0.0。
      double lmr_threshold_;
      int lmr_after_;  // 指定の候補手数以上で実行する。
      int lmr_search_reduction_;  // リダクションする深さ。

      // Futility Pruning。
      bool enable_futility_pruning_;  // 有効かどうか。
      int futility_pruning_depth_;  // 有効にする残り深さ。
      int futility_pruning_margin_;  // 1プライあたりのマージン。
  };

  // 評価関数用パラメータのウェイトのクラス。
  class Weight {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // [引数]
      // openint_weight: 駒(キング以外)が30個の時のウェイト。
      // ending_weight: 駒(キング以外)が0個の時のウェイト。
      Weight(double opening_weight, double ending_weight) :
      opening_weight_(opening_weight),
      ending_weight_(ending_weight) {
        SetLinearParams();
      }
      // デフォルトコンストラクタ。
      Weight() :
      opening_weight_(0.0),
      ending_weight_(0.0) {
        SetLinearParams();
      }
      // コピーコンストラクタ。
      Weight(const Weight& weight) :
      opening_weight_(weight.opening_weight_),
      ending_weight_(weight.ending_weight_),
      slope_(weight.slope_),
      y_intercept_(weight.y_intercept_){}
      // ムーブコンストラクタ。
      Weight(Weight&& weight) :
      opening_weight_(weight.opening_weight_),
      ending_weight_(weight.ending_weight_),
      slope_(weight.slope_),
      y_intercept_(weight.y_intercept_){}
      // コピー代入演算子。
      Weight& operator=(const Weight& weight) {
        opening_weight_ = weight.opening_weight_;
        ending_weight_ = weight.ending_weight_;
        slope_ = weight.slope_;
        y_intercept_ = weight.y_intercept_;
        return *this;
      }
      // ムーブ代入演算子。
      Weight& operator=(Weight&& weight) {
        opening_weight_ = weight.opening_weight_;
        ending_weight_ = weight.ending_weight_;
        slope_ = weight.slope_;
        y_intercept_ = weight.y_intercept_;
        return *this;
      }
      // デストラクタ。
      virtual ~Weight() {}

      /************/
      /* 演算子。 */
      /************/
      // ウェイトを返す。
      // [引数]
      // num_pieces: キング以外の全ての駒の数。
      // [戻り値]
      // ウェイト。
      double operator()(double num_pieces) const {
        return (slope_ * num_pieces) + y_intercept_;
      }

      /**************/
      /* アクセサ。 */
      /**************/
      // オープニング時のウェイト。
      double opening_weight() const {return opening_weight_;}
      // エンディング時のウェイト。
      double ending_weight() const {return ending_weight_;}

      /******************/
      /* ミューテータ。 */
      /******************/
      // オープニング時のウェイト。
      void opening_weight(double opening_weight) {
        opening_weight_ = opening_weight;
        SetLinearParams();
      }
      // エンディング時のウェイト。
      void ending_weight(double ending_weight) {
        ending_weight_ = ending_weight;
        SetLinearParams();
      }

    private:
      // 一次関数のパラメータをセットする。
      void SetLinearParams() {
        slope_ = (opening_weight_ - ending_weight_) / 30.0;
        y_intercept_ = ending_weight_;
      }

      // オープニング時のウェイト。
      double opening_weight_;
      // エンディング時のウェイト。
      double ending_weight_;
      // 一次関数の傾き。
      double slope_;
      // 一次関数のy軸上の切片。
      double y_intercept_;
  };

  // 評価関数用パラメータのクラス。
  class EvalParams {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      EvalParams();
      EvalParams(const EvalParams& params);
      EvalParams(EvalParams&& params);
      EvalParams& operator=(const EvalParams& params);
      EvalParams& operator=(EvalParams&& params);
      virtual ~EvalParams() {}

      /**************/
      /* アクセサ。 */
      /**************/
      // オープニング時の駒の配置の価値テーブル。
      const double (& opening_position_value_table() const)
      [NUM_PIECE_TYPES][NUM_SQUARES] {return opening_position_value_table_;}
      // エンディング時の駒の配置の価値テーブル。
      const double (& ending_position_value_table() const)
      [NUM_PIECE_TYPES][NUM_SQUARES] {return ending_position_value_table_;}
      // 駒への攻撃の価値テーブル。
      const double (& attack_value_table() const)
      [NUM_PIECE_TYPES][NUM_PIECE_TYPES] {return attack_value_table_;}
      // ポーンの盾の配置の価値テーブル。
      const double (& pawn_shield_value_table() const)
      [NUM_SQUARES] {return pawn_shield_value_table_;}

      // オープニング時の駒の配置のウェイト。
      const Weight (& weight_opening_position() const)[NUM_PIECE_TYPES] {
        return weight_opening_position_;
      }
      // エンディング時の駒の配置のウェイト。
      const Weight (& weight_ending_position() const)[NUM_PIECE_TYPES] {
        return weight_ending_position_;
      }
      // 機動力のウェイト。
      const Weight& weight_mobility() const {return weight_mobility_;}
      // センターコントロールのウェイト。
      const Weight& weight_center_control() const {
        return weight_center_control_;
      }
      // スウィートセンターのコントロールのウェイト。
      const Weight& weight_sweet_center_control() const {
        return weight_sweet_center_control_;
      }
      // 駒の展開のウェイト。
      const Weight& weight_development() const {return weight_development_;}
      // 駒への攻撃のウェイト。
      const Weight(& weight_attack() const)[NUM_PIECE_TYPES] {
        return weight_attack_;
      }
      // 相手キング周辺への攻撃。
      const Weight& weight_attack_around_king() const {
        return weight_attack_around_king_;
      }
      // パスポーンのウェイト。
      const Weight& weight_pass_pawn() const {return weight_pass_pawn_;}
      // 守られたパスポーンのウェイト。
      const Weight& weight_protected_pass_pawn() const {
        return weight_protected_pass_pawn_;
      }
      // ダブルポーンのウェイト。
      const Weight& weight_double_pawn() const {return weight_double_pawn_;}
      // 孤立ポーン。
      const Weight& weight_iso_pawn() const {return weight_iso_pawn_;}
      // ポーンの盾のウェイト。
      const Weight& weight_pawn_shield() const {return weight_pawn_shield_;}
      // ビショップペアのウェイト。
      const Weight& weight_bishop_pair() const {return weight_bishop_pair_;}
      // バッドビショップのウェイト。
      const Weight& weight_bad_bishop() const {return weight_bad_bishop_;}
      // ビショップで相手のナイトをピンのウェイト。
      const Weight& weight_pin_knight() const {return weight_pin_knight_;}
      // ルークペアのウェイト。
      const Weight& weight_rook_pair() const {return weight_rook_pair_;}
      // セミオープンファイルのルークのウェイト。
      const Weight& weight_rook_semiopen_fyle() const {
        return weight_rook_semiopen_fyle_;
      }
      // オープンファイルのルーク。
      const Weight& weight_rook_open_fyle() const {
        return weight_rook_open_fyle_;
      }
      // 早すぎるクイーンの始動のウェイト。
      const Weight& weight_early_queen_launched() const {
        return weight_early_queen_launched_;
      }
      // キング周りの弱いマスのウェイト。
      const Weight& weight_weak_square() const {return weight_weak_square_;}
      // キャスリングのウェイト。
      const Weight& weight_castling() const {return weight_castling_;}
      // キャスリングの放棄のウェイト。
      const Weight& weight_abandoned_castling() const {
        return weight_abandoned_castling_;
      }

      /******************/
      /* ミューテータ。 */
      /******************/
      // オープニング時の駒の配置の価値テーブル。
      void opening_position_value_table
      (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]);
      // エンディング時の駒の配置の価値テーブル。
      void ending_position_value_table
      (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]);
      // 駒への攻撃の価値テーブル。
      void attack_value_table
      (const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES]);
      // ポーンの盾の配置の価値テーブル。
      void pawn_shield_value_table(const double (& table)[NUM_SQUARES]);

      // オープニング時の駒の配置のウェイト。
      void weight_opening_position(const Weight (& weights)[NUM_PIECE_TYPES]);
      // エンディング時の駒の配置のウェイト。
      void weight_ending_position(const Weight (& weights)[NUM_PIECE_TYPES]);
      // 機動力のウェイト。
      void weight_mobility(const Weight& weight) {weight_mobility_ = weight;}
      // センターコントロールのウェイト。
      void weight_center_control(const Weight& weight) {
        weight_center_control_ = weight;
      }
      // スウィートセンターのコントロールのウェイト。
      void weight_sweet_center_control(const Weight& weight) {
        weight_sweet_center_control_ = weight;
      }
      // 駒の展開のウェイト。
      void weight_development(const Weight& weight) {
        weight_development_ = weight;
      }
      // 駒への攻撃のウェイト。
      void weight_attack(const Weight (& weights)[NUM_PIECE_TYPES]);
      // 相手キング周辺への攻撃のウェイト。
      void weight_attack_around_king(const Weight& weight) {
        weight_attack_around_king_ = weight;
      }
      // パスポーンのウェイト。
      void weight_pass_pawn(const Weight& weight) {
        weight_pass_pawn_ = weight;
      }
      // 守られたパスポーンのウェイト。
      void weight_protected_pass_pawn(const Weight& weight) {
        weight_protected_pass_pawn_ = weight;
      }
      // ダブルポーンのウェイト。
      void weight_double_pawn(const Weight& weight) {
        weight_double_pawn_ = weight;
      }
      // 孤立ポーンのウェイト。
      void weight_iso_pawn(const Weight& weight) {weight_iso_pawn_ = weight;}
      // ポーンの盾のウェイト。
      void weight_pawn_shield(const Weight& weight) {
        weight_pawn_shield_ = weight;
      }
      // ビショップペアのウェイト。
      void weight_bishop_pair(const Weight& weight) {
        weight_bishop_pair_ = weight;
      }
      // バッドビショップのウェイト。
      void weight_bad_bishop(const Weight& weight) {
        weight_bad_bishop_ = weight;
      }
      // ビショップで相手のナイトをピンのウェイト。
      void weight_pin_knight(const Weight& weight) {
        weight_pin_knight_ = weight;
      }
      // ルークペアのウェイト。
      void weight_rook_pair(const Weight& weight) {weight_rook_pair_ = weight;}
      // セミオープンファイルのルークのウェイト。
      void weight_rook_semiopen_fyle(const Weight& weight) {
        weight_rook_semiopen_fyle_ = weight;
      }
      // オープンファイルのルークのウェイト。
      void weight_rook_open_fyle(const Weight& weight) {
        weight_rook_open_fyle_ = weight;
      }
      // 早すぎるクイーンの始動のウェイト。
      void weight_early_queen_launched(const Weight& weight) {
        weight_early_queen_launched_ = weight;
      }
      // キング周りの弱いマスのウェイト。
      void weight_weak_square(const Weight& weight) {
        weight_weak_square_ = weight;
      }
      // キャスリングのウェイト。
      void weight_castling(const Weight& weight) {weight_castling_ = weight;}
      // キャスリングの放棄のウェイト。
      void weight_abandoned_castling(const Weight& weight) {
        weight_abandoned_castling_ = weight;
      }

    private:
      // メンバをコピーする。
      // [引数]
      // params: コピー元。
      void ScanMember(const EvalParams& params);

      /********************/
      /* 価値テーブル類。 */
      /********************/
      // オープニング時の駒の配置の価値テーブル。
      double opening_position_value_table_[NUM_PIECE_TYPES][NUM_SQUARES];
      // エンディング時の駒の配置の価値テーブル。
      double ending_position_value_table_[NUM_PIECE_TYPES][NUM_SQUARES];
      // 駒への攻撃の価値テーブル。
      double attack_value_table_[NUM_PIECE_TYPES][NUM_PIECE_TYPES];
      // ポーンの盾の配置の価値テーブル。
      double pawn_shield_value_table_[NUM_SQUARES];

      /**************/
      /* ウェイト。 */
      /**************/
      // オープニング時の駒の配置。
      Weight weight_opening_position_[NUM_PIECE_TYPES];
      // エンディング時の駒の配置。
      Weight weight_ending_position_[NUM_PIECE_TYPES];
      // 機動力。
      Weight weight_mobility_;
      // センターコントロール。
      Weight weight_center_control_;
      // スウィートセンターのコントロール。
      Weight weight_sweet_center_control_;
      // 駒の展開。
      Weight weight_development_;
      // 駒への攻撃。
      Weight weight_attack_[NUM_PIECE_TYPES];
      // 相手キング周辺への攻撃。
      Weight weight_attack_around_king_;
      // パスポーン。
      Weight weight_pass_pawn_;
      // 守られたパスポーン。
      Weight weight_protected_pass_pawn_;
      // ダブルポーン。
      Weight weight_double_pawn_;
      // 孤立ポーン。
      Weight weight_iso_pawn_;
      // ポーンの盾。
      Weight weight_pawn_shield_;
      // ビショップペア。
      Weight weight_bishop_pair_;
      // バッドビショップ。
      Weight weight_bad_bishop_;
      // ビショップで相手のナイトをピン。
      Weight weight_pin_knight_;
      // ルークペア。
      Weight weight_rook_pair_;
      // セミオープンファイルのルーク。
      Weight weight_rook_semiopen_fyle_;
      // オープンファイルのルーク。
      Weight weight_rook_open_fyle_;
      // 早すぎるクイーンの始動。
      Weight weight_early_queen_launched_;
      // キング周りの弱いマス。
      Weight weight_weak_square_;
      // キャスリング。
      Weight weight_castling_;
      // キャスリングの放棄。
      Weight weight_abandoned_castling_;
  };
}  // namespace Sayuri

#endif
