/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
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

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <iostream>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;

  /** 評価した結果を格納する構造体。 */
  struct EvalResult {
    /** 総合評価値。 */
    double score_;

    /** マテリアル。 */
    double material_;
    /** オープニング時の駒の配置の評価値。 [駒の種類] */
    double score_opening_position_[NUM_PIECE_TYPES];
    /** エンディング時の駒の配置の評価値。 [駒の種類] */
    double score_ending_position_[NUM_PIECE_TYPES];
    /** 機動力の評価値。 */
    double score_mobility_;
    /** センターコントロールの評価値。 */
    double score_center_control_;
    /** スウィートセンターのコントロールの評価値。 */
    double score_sweet_center_control_;
    /** 駒の展開の評価値。 */
    double score_development_;
    /** 攻撃の評価値。 [駒の種類] */
    double score_attack_[NUM_PIECE_TYPES];
    /** 相手キング周辺への攻撃の評価値。 */
    double score_attack_around_king_;
    /** パスポーンの評価値。 */
    double score_pass_pawn_;
    /** 守られたパスポーンの評価値。 */
    double score_protected_pass_pawn_;
    /** ダブルポーンの評価値。 */
    double score_double_pawn_;
    /** 孤立ポーンの評価値。 */
    double score_iso_pawn_;
    /** ポーンの盾の評価値。 */
    double score_pawn_shield_;
    /** ビショップペアの評価値。 */
    double score_bishop_pair_;
    /** バッドビショップの評価値。 */
    double score_bad_bishop_;
    /** 相手のナイトをビショップでピンの評価値。 */
    double score_pin_knight_;
    /** ルークペアの評価値。 */
    double score_rook_pair_;
    /** セミオープンファイルのルークの評価値。 */
    double score_rook_semiopen_fyle_;
    /** オープンファイルのルークの評価値。 */
    double score_rook_open_fyle_;
    /** 早すぎるクイーンの始動の評価値。 */
    double score_early_queen_launched_;
    /** キング周りの弱いマスの評価値。 */
    double score_weak_square_;
    /** キャスリングの評価値。 */
    double score_castling_;
    /** キャスリングの放棄の評価値。 */
    double score_abandoned_castling_;
  };

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
       * 現在の局面を評価し、構造体にして結果を返す。
       * @return 評価結果の構造体。
       */
      EvalResult GetEvalResult();

    private:
      /** フレンドのデバッグ用関数。 */
      friend int DebugMain(int argc, char* argv[]);

      // ================== //
      // 価値を計算する関数 //
      // ================== //
      /**
       * 各駒の価値を計算する。
       * @param <Type> 駒の種類。
       * @param piece_square 駒のいるマス。
       * @param piece_side 駒のサイド。
       */
      template<Piece Type>
      void CalValue(Square piece_square, Side piece_side);

      // ====================== //
      // 局面評価に使用する関数 //
      // ====================== //
      /**
       * メイトに必要な駒があるかどうか調べる。
       * @param side 調べるサイド。
       * @return 必要な駒があればtrue。
       */
      bool HasEnoughPieces(Side side) const;

      // ========== //
      // 局面分析用 //
      // ========== //
      /** 駒の初期位置のビットボード。 [サイド][駒の種類] */
      static Bitboard start_position_[NUM_SIDES][NUM_PIECE_TYPES];
      /** start_position_[][]を初期化。 */
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
      // --- 価値の変数 --- //
      /** オープニング時の駒の配置の価値。 [駒の種類] */
      double opening_position_value_[NUM_PIECE_TYPES];
      /** エンディング時の駒の配置の価値。 [駒の種類] */
      double ending_position_value_[NUM_PIECE_TYPES];
      /** 機動力の価値。 */
      double mobility_value_;
      /** センターコントロールの価値。 */
      double center_control_value_;
      /** スウィートセンターコントロールの価値。 */
      double sweet_center_control_value_;
      /** 駒の展開の価値。 */
      double development_value_;
      /** 攻撃の価値。 [駒の種類] */
      double attack_value_[NUM_PIECE_TYPES];
      /** 相手キング周辺への攻撃の価値。 */
      double attack_around_king_value_;
      /** パスポーンの価値。 */
      double pass_pawn_value_;
      /** 守られたパスポーンの価値。 */
      double protected_pass_pawn_value_;
      /** ダブルポーンの価値。 */
      double double_pawn_value_;
      /** 孤立ポーンの価値。 */
      double iso_pawn_value_;
      /** ポーンの盾の価値。 */
      double pawn_shield_value_;
      /** ビショップペアの価値。 */
      double bishop_pair_value_;
      /** バッドビショップの価値。 */
      double bad_bishop_value_;
      /** ビショップで相手のナイトをピンの価値。 */
      double pin_knight_value_;
      /** ルークペアの価値。 */
      double rook_pair_value_;
      /** セミオープンファイルのルークの価値。 */
      double rook_semiopen_fyle_value_;
      /** オープンファイルのルークの価値。 */
      double rook_open_fyle_value_;
      /** 早すぎるクイーンの始動の価値。 */
      double early_queen_launched_value_;
      /** キング周りの弱いマスの価値。 */
      double weak_square_value_;
      /** キャスリングの価値。 */
      double castling_value_;
      /** キャスリングの放棄の価値。 */
      double abandoned_castling_value_;
  };
}  // namespace Sayuri

#endif
