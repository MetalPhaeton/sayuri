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
#include "chess_def.h"

namespace Sayuri {
  // 探索アルゴリズムのパラメータのクラス。
  class SearchParams {
  };

  // 評価関数のパラメータのウェイトのクラス。
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
      // コピー代入。
      Weight& operator=(const Weight& weight) {
        opening_weight_ = weight.opening_weight_;
        ending_weight_ = weight.ending_weight_;
        slope_ = weight.slope_;
        y_intercept_ = weight.y_intercept_;
        return *this;
      }
      // ムーブ代入。
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

  // 評価関数のパラメータのクラス。
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
