/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Hironori Ishibashi
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
 * @file params.h
 * @author Hironori Ishibashi
 * @brief 探索アルゴリズム、評価関数を変更するパラメータ。
 */

#ifndef PARAMS_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define PARAMS_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <vector>
#include <tuple>
#include <initializer_list>
#include <algorithm>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /** 探索関数用パラメータのクラス。 */
  class SearchParams {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      SearchParams();
      /**
       * コピーコンストラクタ。
       * @param params コピー元。
       */
      SearchParams(const SearchParams& params);
      /**
       * ムーブコンストラクタ。
       * @param params ムーブ元。
       */
      SearchParams(SearchParams&& params);
      /**
       * コピー代入演算子。
       * @param params コピー元。
       */
      SearchParams& operator=(const SearchParams& params);
      /**
       * ムーブ代入演算子。
       * @param params ムーブ元。
       */
      SearchParams& operator=(SearchParams&& params);
      /** デストラクタ。 */
      virtual ~SearchParams() {}

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - マテリアル。 [駒の種類]
       * @return マテリアル。
       */
      const int (& material() const)[NUM_PIECE_TYPES] {return material_;}

      // --- Quiescence Search --- //
      /**
       * アクセサ - Quiescence Search - 有効無効。
       * @return 有効無効。
       */
      bool enable_quiesce_search() const {return enable_quiesce_search_;}

      // --- 繰り返しチェック --- //
      /**
       * アクセサ - 繰り返しチェック - 有効無効。
       * @return 有効無効。
       */
      bool enable_repetition_check() const {
        return enable_repetition_check_;
      }

      // --- Check Extension --- //
      /**
       * アクセサ - Check Extension - 有効無効。
       * @return 有効無効。
       */
      bool enable_check_extension() const {return enable_check_extension_;}

      // --- YBWC --- //
      /**
       * アクセサ - YBMC - 残り深さ制限。
       * @return 残り深さ制限。
       */
      int ybwc_limit_depth() const {return ybwc_limit_depth_;}
      /**
       * アクセサ - YBWC - 何手目以降の候補手で実行するか。
       * @return 何手目以降の候補手で実行するか。
       */
      int ybwc_invalid_moves() const {return ybwc_invalid_moves_;}

      // --- Aspiration Windows --- //
      /**
       * アクセサ - Aspiration Windows - 有効無効。
       * @return 有効無効。
       */
      bool enable_aspiration_windows() const {
        return enable_aspiration_windows_;
      }
      /**
       * アクセサ - Aspiration Windows - 残り深さ制限。
       * @return 残り深さ制限。
       */
      int aspiration_windows_limit_depth() const {
        return aspiration_windows_limit_depth_;
      }
      /**
       * アクセサ - Aspiration Windows - デルタ値。
       * @return デルタ値。
       */
      int aspiration_windows_delta() const {return aspiration_windows_delta_;}

      // --- SEE --- //
      /**
       * アクセサ - SEE - 有効無効。
       * @return 有効無効。
       */
      bool enable_see() const {return enable_see_;}

      // --- ヒストリー --- //
      /**
       * アクセサ - ヒストリー - 有効無効。
       * @return 有効無効。
       */
      bool enable_history() const {return enable_history_;}

      // --- キラームーブ --- //
      /**
       * アクセサ - キラームーブ - 有効無効。
       * @return 有効無効。
       */
      bool enable_killer() const {return enable_killer_;}

      // --- トランスポジションテーブル --- //
      /**
       * アクセサ - トランスポジションテーブル - 有効無効。
       * @return 有効無効。
       */
      bool enable_ttable() const {return enable_ttable_;}

      // --- Internal Iterative Deepening --- //
      /**
       * アクセサ - Internal Iterative Deepening - 有効無効。
       * @return 有効無効。
       */
      bool enable_iid() const {return enable_iid_;}
      /**
       * アクセサ - Internal Iterative Deepening - 残り深さ制限。
       * @return 残り深さ制限。
       */
      int iid_limit_depth() const {return iid_limit_depth_;}
      /**
       * アクセサ - Internal Iterative Deepening - 探索の深さ。
       * @return 探索の深さ。
       */
      int iid_search_depth() const {return iid_search_depth_;}

      // --- Null Move Reduction --- //
      /**
       * アクセサ - Null Move Reduction - 有効無効。
       * @return 有効無効。
       */
      bool enable_nmr() const {return enable_nmr_;}
      /**
       * アクセサ - Null Move Reduction - 残り深さ制限。
       * @return 残り深さ制限。
       */
      int nmr_limit_depth() const {return nmr_limit_depth_;}
      /**
       * アクセサ - Null Move Reduction - 何プライ浅く探索するか。
       * @return 何プライ浅く探索するか。
       */
      int nmr_search_reduction() const {return nmr_search_reduction_;}
      /**
       * アクセサ - Null Move Reduction - リダクションする深さ。
       * @return リダクションする深さ。
       */
      int nmr_reduction() const {return nmr_reduction_;}

      // --- ProbCut --- //
      /**
       * アクセサ - ProbCut - 有効無効。
       * @return 有効無効。
       */
      bool enable_probcut() const {return enable_probcut_;}
      /**
       * アクセサ - ProbCut - 残り深さ制限。
       * @return 残り深さ制限。
       */
      int probcut_limit_depth() const {return probcut_limit_depth_;}
      /**
       * アクセサ - ProbCut - ベータ値の増分。
       * @return ベータ値の増分。
       */
      int probcut_margin() const {return probcut_margin_;}
      /**
       * アクセサ - ProbCut - 何プライ浅く探索するか。
       * @return 何プライ浅く探索するか。
       */
      int probcut_search_reduction() const {return probcut_search_reduction_;}

      // --- History Pruning --- //
      /**
       * アクセサ - History Pruning - 有効無効。
       * (ヒストリーが無効なら、無効。)
       * @return 有効無効。
       */
      bool enable_history_pruning() const {return enable_history_pruning_;}
      /**
       * アクセサ - History Pruning - 残り深さ制限。
       * @return 残り深さ制限。
       */
      int history_pruning_limit_depth() const {
        return history_pruning_limit_depth_;
      }
      /**
       * アクセサ - History Pruning - 実行する候補手の閾値。
       * (1.0 から 0.0)
       * @return 実行する候補手の閾値。
       */
      double history_pruning_move_threshold() const {
        return history_pruning_move_threshold_;
      }
      /**
       * アクセサ - History Pruning - 何手目以降の候補手で実行するか。
       * @return 何手目以降の候補手で実行するか。
       */
      int history_pruning_invalid_moves() const {
        return history_pruning_invalid_moves_;
      }
      /**
       * アクセサ - History Pruning - 最大ヒストリー値に対する閾値。 
       * (1.0 から 00)
       * @return 最大ヒストリー値に対する閾値。
       */
      double history_pruning_threshold() const {
        return history_pruning_threshold_;
      }
      /**
       * アクセサ - History Pruning - リダクションする深さ。
       * @return リダクションする深さ。
       */
      int history_pruning_reduction() const {
        return history_pruning_reduction_;
      }

      // --- Late Move Reduction --- //
      /**
       * アクセサ - Late Move Reduction - 有効無効。
       * @return 有効無効。
       */
      bool enable_lmr() const {return enable_lmr_;}
      /**
       * アクセサ - Late Move Reduction - 残り深さ制限。
       * @return 残り深さ制限。
       */
      int lmr_limit_depth() const {return lmr_limit_depth_;}
      /**
       * アクセサ - Late Move Reduction - 実行する候補手の閾値。
       * (1.0 から 0.0)
       * @return 実行する候補手の閾値。
       */
      double lmr_move_threshold() const {return lmr_move_threshold_;}
      /**
       * アクセサ - Late Move Reduction - 何手目以降の候補手で実行するか。
       * @return 何手目以降の候補手で実行するか。
       */
      int lmr_invalid_moves() const {return lmr_invalid_moves_;}
      /**
       * アクセサ - Late Move Reduction - リダクションする深さ。
       * @return リダクションする深さ。
       */
      int lmr_search_reduction() const {return lmr_search_reduction_;}

      // --- Futility Pruning --- //
      /**
       * アクセサ - Futility Pruning - 有効無効。
       * @return 有効無効。
       */
      bool enable_futility_pruning() const {return enable_futility_pruning_;}
      /**
       * アクセサ - Futility Pruning - 有効にする残り深さ。
       * @return 有効にする残り深さ。
       */
      int futility_pruning_depth() const {return futility_pruning_depth_;}
      /**
       * アクセサ - Futility Pruning - 残り深さ1プライあたりのマージン。
       * @return 残り深さ1プライあたりのマージン。
       */
      int futility_pruning_margin() const {return futility_pruning_margin_;}

      // ============ //
      // ミューテータ //
      // ============ //
      /**
       * ミューテータ - マテリアル。
       * @param table マテリアル。
       */
      void material(const int (& table)[NUM_PIECE_TYPES]);

      // --- Quiescence Search --- //
      /**
       * ミューテータ - Quiescence Search - 有効無効。
       * @param enable 有効無効。
       */
      void enable_quiesce_search(bool enable) {
        enable_quiesce_search_ = enable;
      }

      // --- 繰り返しチェック --- //
      /**
       * ミューテータ - 繰り返しチェック - 有効無効。
       * @param enable 有効無効。
       */
      void enable_repetition_check(bool enable) {
        enable_repetition_check_= enable;
      }

      // --- Check Extension --- //
      /**
       * ミューテータ - Check Extension - 有効無効。
       * @param enable 有効無効。
       */
      void enable_check_extension(bool enable) {
        enable_check_extension_ = enable;
      }

      // --- YBWC --- //
      /**
       * ミューテータ - YBMC - 残り深さ制限。
       * @param depth 残り深さ制限。
       */
      void ybwc_limit_depth(int depth) {
        ybwc_limit_depth_ = Util::GetMax(depth, 0);
      }
      /**
       * ミューテータ - YBWC - 何手目以降の候補手で実行するか。
       * @param num_moves 何手目以降の候補手で実行するか。
       */
      void ybwc_invalid_moves(int num_moves) {
        ybwc_invalid_moves_ = Util::GetMax(num_moves, 0);
      }

      // --- Aspiration Windows --- //
      /**
       * ミューテータ - Aspiration Windows - 有効無効。
       * @param enable 有効無効。
       */
      void enable_aspiration_windows(bool enable) {
        enable_aspiration_windows_ = enable;
      }
      /**
       * ミューテータ - Aspiration Windows - 残り深さ制限。
       * @param depth 残り深さ制限。
       */
      void aspiration_windows_limit_depth(int depth) {
        aspiration_windows_limit_depth_ = Util::GetMax(depth, 0);
      }
      /**
       * ミューテータ - Aspiration Windows - デルタ値。
       * @param delta デルタ値。
       */
      void aspiration_windows_delta(int delta) {
        aspiration_windows_delta_ = delta;
      }

      // --- SEE --- //
      /**
       * ミューテータ - SEE - 有効無効。
       * @param enable 有効無効。
       */
      void enable_see(bool enable) {enable_see_ = enable;}

      // --- ヒストリー --- //
      /**
       * ミューテータ - ヒストリー - 有効無効。
       * @param enable 有効無効。
       */
      void enable_history(bool enable) {enable_history_ = enable;}

      // --- キラームーブ --- //
      /**
       * ミューテータ - キラームーブ - 有効無効。
       * @param enable 有効無効。
       */
      void enable_killer(bool enable) {enable_killer_ = enable;}

      // --- トランスポジションテーブル --- //
      /**
       * ミューテータ - トランスポジションテーブル - 有効無効。
       * @param enable 有効無効。
       */
      void enable_ttable(bool enable) {enable_ttable_ = enable;}

      // --- Internal Iterative Deepening --- //
      /**
       * ミューテータ - Internal Iterative Deepening - 有効無効。
       * @param enable 有効無効。
       */
      void enable_iid(bool enable) {enable_iid_ = enable;}
      /**
       * ミューテータ - Internal Iterative Deepening - 残り深さ制限。
       * @param depth 残り深さ制限。
       */
      void iid_limit_depth(int depth) {
        iid_limit_depth_ = Util::GetMax(depth, 0);
      }
      /**
       * ミューテータ - Internal Iterative Deepening - 探索の深さ。
       * @param depth 探索の深さ。
       */
      void iid_search_depth(int depth) {
        iid_search_depth_ = Util::GetMax(depth, 0);
      }

      // --- Null Move Reduction --- //
      /**
       * ミューテータ - Null Move Reduction - 有効無効。
       * @param enable 有効無効。
       */
      void enable_nmr(bool enable) {enable_nmr_ = enable;}
      /**
       * ミューテータ - Null Move Reduction - 残り深さ制限。
       * @param depth 残り深さ制限。
       */
      void nmr_limit_depth(int depth) {
        nmr_limit_depth_ = Util::GetMax(depth, 0);
      }
      /**
       * ミューテータ - Null Move Reduction - 何プライ浅く探索するか。
       * @param reduction 何プライ浅く探索するか。
       */
      void nmr_search_reduction(int reduction) {
        nmr_search_reduction_ = Util::GetMax(reduction, 0);
      }
      /**
       * ミューテータ - Null Move Reduction - リダクションする深さ。
       * @param reduction リダクションする深さ。
       */
      void nmr_reduction(int reduction) {
        nmr_reduction_ = Util::GetMax(reduction, 0);
      }

      // --- ProbCut --- //
      /**
       * ミューテータ - ProbCut - 有効無効。
       * @param enable 有効無効。
       */
      void enable_probcut(bool enable) {enable_probcut_ = enable;}
      /**
       * ミューテータ - ProbCut - 残り深さ制限。
       * @param enable 残り深さ制限。
       */
      void probcut_limit_depth(int depth) {
        probcut_limit_depth_ = Util::GetMax(depth, 0);
      }
      /**
       * ミューテータ - ProbCut - ベータ値の増分。
       * @param margin ベータ値の増分。
       */
      void probcut_margin(int margin) {probcut_margin_ = margin;}
      /**
       * ミューテータ - ProbCut - 何プライ浅く探索するか。
       * @param reduction 何プライ浅く探索するか。
       */
      void probcut_search_reduction(int reduction) {
        probcut_search_reduction_ = Util::GetMax(reduction, 0);
      }

      // --- History Pruning --- //
      /**
       * ミューテータ - History Pruning - 有効無効。
       * (ヒストリーが無効なら、無効。)
       * @param enable 有効無効。
       */
      void enable_history_pruning(bool enable) {
        enable_history_pruning_ = enable;
      }
      /**
       * ミューテータ - History Pruning - 残り深さ制限。
       * @param depth 残り深さ制限。
       */
      void history_pruning_limit_depth(int depth) {
        history_pruning_limit_depth_ = Util::GetMax(depth, 0);
      }
      /**
       * ミューテータ - History Pruning - 実行する候補手の閾値。
       * (1.0 から 0.0)
       * @param threshold 実行する候補手の閾値。
       */
      void history_pruning_move_threshold(double threshold) {
        history_pruning_move_threshold_ =
        Util::GetMin(Util::GetMax(threshold, 0.0), 1.0);
      }
      /**
       * ミューテータ - History Pruning - 何手目以降の候補手で実行するか。
       * @param num_moves 何手目以降の候補手で実行するか。
       */
      void history_pruning_invalid_moves(int num_moves) {
        history_pruning_invalid_moves_ = Util::GetMax(num_moves, 0);
      }
      /**
       * ミューテータ - History Pruning - 最大ヒストリー値に対する閾値。 
       * (1.0 から 00)
       * @param threshold 最大ヒストリー値に対する閾値。
       */
      void history_pruning_threshold(double threshold) {
        history_pruning_threshold_ =
        Util::GetMin(Util::GetMax(threshold, 0.0), 1.0);
      }
      /**
       * ミューテータ - History Pruning - リダクションする深さ。
       * @param reduction リダクションする深さ。
       */
      void history_pruning_reduction(int reduction) {
        history_pruning_reduction_ = Util::GetMax(reduction, 0);
      }

      // --- Late Move Reduction --- //
      /**
       * ミューテータ - Late Move Reduction - 有効無効。
       * @param enable 有効無効。
       */
      void enable_lmr(bool enable) {enable_lmr_ = enable;}
      /**
       * ミューテータ - Late Move Reduction - 残り深さ制限。
       * @param depth 残り深さ制限。
       */
      void lmr_limit_depth(int depth) {
        lmr_limit_depth_ = Util::GetMax(depth, 0);
      }
      /**
       * ミューテータ - Late Move Reduction - 実行する候補手の閾値。
       * (1.0 から 0.0)
       * @param threshold 実行する候補手の閾値。
       */
      void lmr_move_threshold(double threshold) {
        lmr_move_threshold_ = Util::GetMin(Util::GetMax(threshold, 0.0), 1.0);
      }
      /**
       * ミューテータ - Late Move Reduction - 何手目以降の候補手で実行するか。
       * @param num_moves 何手目以降の候補手で実行するか。
       */
      void lmr_invalid_moves(int num_moves) {
        lmr_invalid_moves_ = Util::GetMax(num_moves, 0);
      }
      /**
       * ミューテータ - Late Move Reduction - リダクションする深さ。
       * @param reduction リダクションする深さ。
       */
      void lmr_search_reduction(int reduction) {
        lmr_search_reduction_ = Util::GetMax(reduction, 0);
      }

      // --- Futility Pruning --- //
      /**
       * ミューテータ - Futility Pruning - 有効無効。
       * @param enable 有効無効。
       */
      void enable_futility_pruning(bool enable) {
        enable_futility_pruning_ = enable;
      }
      /**
       * ミューテータ - Futility Pruning - 有効にする残り深さ。
       * @param depth 有効にする残り深さ。
       */
      void futility_pruning_depth(int depth) {
        futility_pruning_depth_ = Util::GetMax(depth, 0);
      }
      /**
       * ミューテータ - Futility Pruning - 残り深さ1プライあたりのマージン。
       * @param margin 残り深さ1プライあたりのマージン。
       */
      void futility_pruning_margin(int margin) {
        futility_pruning_margin_ = margin;
      }

    private:
      /** 探索関数のあるChessEngineはフレンド。 */
      friend class ChessEngine;
      /** 探索関数で使うテンプレート部品。 */
      template<NodeType TYPE>
      friend struct DoIID;
      /** 探索関数で使うテンプレート部品。 */
      template<NodeType TYPE>
      friend struct DoNMR;
      /** 探索関数で使うテンプレート部品。 */
      template<NodeType TYPE>
      friend struct DoProbCut;

      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * メンバをコピーする。
       * @param params コピー元。
       */
      void ScanMember(const SearchParams& params);

      // ========== //
      // メンバ変数 //
      // ========== //
      /** マテリアル。 */
      int material_[NUM_PIECE_TYPES];

      // --- Quiescence Search --- //
      /** Quiescence Search - 有効無効。 */
      bool enable_quiesce_search_;

      // --- 繰り返しチェック --- //
      /** 繰り返しチェック - 有効無効。 */
      bool enable_repetition_check_;

      // --- Check Extension --- //
      /** Check Extension - 有効無効。 */
      bool enable_check_extension_;

      // --- YBWC --- //
      /** YBWC - 残り深さ制限。 */
      int ybwc_limit_depth_;
      /** YBWC - 何手目以降の候補手で実行するか。 */
      int ybwc_invalid_moves_;

      // --- Aspiration Windows --- //
      /** Aspiration Windows - 有効無効。 */
      bool enable_aspiration_windows_;
      /** Aspiration Windows - 残り深さ制限。 */
      int aspiration_windows_limit_depth_;
      /** Aspiration Windows - デルタ値。 */
      int aspiration_windows_delta_;

      // --- SEE --- //
      /** SEE - 有効無効。 */
      bool enable_see_;

      // --- ヒストリー --- //
      /** ヒストリー - 有効無効。 */
      bool enable_history_;

      // --- キラームーブ --- //
      /** キラームーブ - 有効無効。 */
      bool enable_killer_;

      // --- トランスポジションテーブル --- //
      /** トランスポジションテーブル - 有効無効。 */
      bool enable_ttable_;

      // --- Internal Iterative Deepening --- //
      /** Internal Iterative Deepening - 有効無効。 */
      bool enable_iid_;
      /** Internal Iterative Deepening - 残り深さ制限。 */
      int iid_limit_depth_;
      /** Internal Iterative Deepening - 探索の深さ。 */
      int iid_search_depth_;

      // --- Null Move Reduction --- //
      /** Null Move Reduction - 有効無効。 */
      bool enable_nmr_;
      /** Null Move Reduction - 残り深さ制限。 */
      int nmr_limit_depth_;
      /** Null Move Reduction - 何プライ浅く探索するか。 */
      int nmr_search_reduction_;
      /** Null Move Reduction - リダクションする深さ。 */
      int nmr_reduction_;

      // --- ProbCut --- //
      /** ProbCut - 有効無効。 */
      bool enable_probcut_;
      /** ProbCut - 残り深さ制限。 */
      int probcut_limit_depth_;
      /** ProbCut - ベータ値の増分。 */
      int probcut_margin_;
      /** ProbCut - 何プライ浅く探索するか。 */
      int probcut_search_reduction_;

      // --- History Pruning --- //
      /** History Pruning - 有効無効。 */
      bool enable_history_pruning_;
      /** History Pruning - 残り深さ制限。 */
      int history_pruning_limit_depth_;
      /** History Pruning - 実行する候補手の閾値。 */
      double history_pruning_move_threshold_;
      /** History Pruning - 何手目以降の候補手で実行するか。 */
      int history_pruning_invalid_moves_;
      /** History Pruning - 最大ヒストリー値に対する閾値。 */
      double history_pruning_threshold_;
      /** History Pruning - リダクションする深さ。 */
      int history_pruning_reduction_;

      // --- Late Move Reduction --- //
      /** Late Move Reduction - 有効無効。 */
      bool enable_lmr_;
      /** Late Move Reduction - 残り深さ制限。 */
      int lmr_limit_depth_;
      /** Late Move Reduction - 実行する候補手の閾値。 */
      double lmr_move_threshold_;
      /** Late Move Reduction - 何手目以降の候補手で実行するか。 */
      int lmr_invalid_moves_;
      /** Late Move Reduction - リダクションする深さ。 */
      int lmr_search_reduction_;

      // --- Futility Pruning --- //
      /** Futility Pruning - 有効無効。 */
      bool enable_futility_pruning_;
      /** Futility Pruning - 有効にする残り深さ。 */
      int futility_pruning_depth_;
      /** Futility Pruning - 残り深さ1プライあたりのマージン。 */
      int futility_pruning_margin_;
  };

  /** 評価関数用パラメータのウェイトのクラス。 */
  class Weight : protected std::vector<std::tuple<int, double>> {
    protected:
      using Super = std::vector<std::tuple<int, double>>;
      using InitList = std::initializer_list<std::initializer_list<double>>;

    public:
      // 駒の数とフェーズの関係。
      /** オープニング開始時。 */
      static constexpr int OPENING_START = 32;
      /** オープニング終了時。 */
      static constexpr int OPENING_END = 24;
      /** エンディング開始時。 */
      static constexpr int ENDING_START = 14;
      /** エンディング終了時。 */
      static constexpr int ENDING_END = 2;

      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param list 初期化リスト。
       */
      Weight(InitList list) {
        for (auto& elm : list) {
          if (elm.size() >= 2) {
            std::initializer_list<double>::iterator itr = elm.begin();
            this->push_back(std::tuple<int, double>(*itr, *(itr + 1)));
          }
        }

        // ソート。
        Sort();
      }
      /** コンストラクタ。 */
      Weight() : Super() {}
      /**
       * コピーコンストラクタ。
       * @param weight コピー元。
       */
      Weight(const Weight& weight) : Super(weight) {}
      /**
       * ムーブコンストラクタ。
       * @param weight ムーブ元。
       */
      Weight(Weight&& weight) : Super(weight) {}
      /**
       * コピー代入演算子。
       * @param bbb コピー元。
       */
      Weight& operator=(const Weight& weight) {
        Super::operator=(weight);
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param bbb ムーブ元。
       */
      Weight& operator=(Weight&& weight) {
        Super::operator=(weight);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~Weight() {}


      /**
       * ウェイトを返す。
       * @param num_pieces 駒の数。
       * @return ウェイト。
       */
      double operator()(int num_pieces) const {
        // 準備。
        std::size_t len = this->size();
        if (len <= 0) return 0.0;

        // 最小値未満。
        if (num_pieces < std::get<0>(this->at(0))) {
          return std::get<1>(this->at(0));
        }

        // 最大値以上。
        if (num_pieces >= std::get<0>(this->at(len - 1))) {
          return std::get<1>(this->at(len - 1));
        }

        // 最小値以上、最大値未満。
        for (std::size_t i = 1; i < len; ++i) {
          // 点を得る。
          const std::tuple<int, double>& low = this->at(i - 1);
          const std::tuple<int, double>& high = this->at(i);

          if ((num_pieces >= std::get<0>(low))
          && (num_pieces < std::get<0>(high))) {
            return (((std::get<1>(high) - std::get<1>(low))
            / (std::get<0>(high) - std::get<0>(low)))
            * (num_pieces - std::get<0>(low)))
            + std::get<1>(low);
          }
        }

        return 0.0;
      }

      /**
       * サイズを返す。
       * @return サイズ。
       */
      std::size_t Size() const {return this->size();}

      /**
       * 要素を返す。
       * @return 要素のタプル。
       */
      const std::tuple<int, double>& At(std::size_t index) const {
        return this->at(index);
      }

      /**
       * 点を追加。
       * @param num_piece 駒の数。
       * @param weight_val その時のウェイト。
       */
      void Add(int num_pieces, double weight_val) {
        this->push_back(std::tuple<int, double>(num_pieces, weight_val));
        Sort();
      }

      /**
       * 点を更新。
       * @param num_piece 駒の数。
       * @param weight_val その時のウェイト。
       */
      void Update(int num_pieces, double weight_val) {
        // 更新する点を探す。
        for (Weight::iterator itr = this->begin(); itr != this->end(); ++itr) {
          if (std::get<0>(*itr) == num_pieces) {
            *itr = std::tuple<int, double>(num_pieces, weight_val);
            return;
          }
        }

        // どこにもなかったので、追加。
        Add(num_pieces, weight_val);
      }

      /**
       * 点を削除。
       * @param num_piece 駒の数。
       */
      void Delete(int num_pieces) {
        for (Weight::iterator itr = this->begin(); itr != this->end(); ++itr) {
          if (std::get<0>(*itr) == num_pieces) {
            this->erase(itr);
            break;
          }
        }
      }

      /**
       * オープニングとエンディングの値からウェイトを作成する。
       * @param opening_weight オープニングのウェイト。
       * @param ending_weight エンディングのウェイト。
       * @return 完成したウェイトオブジェクト。
       */
      /**
       * オープニングとエンディングの値からウェイトを作成する。
       */
      static Weight CreateWeight(double opening_weight, double ending_weight) {
        return {
          {OPENING_START, opening_weight}, {OPENING_END, opening_weight},
          {ENDING_START, ending_weight}, {ENDING_END, ending_weight}
        };
      }

    protected:
      /** ソートする。 */
      void Sort() {
        std::sort(this->begin(), this->end(),
        [](const std::tuple<int, double>& a,
        std::tuple<int, double>& b) -> bool {
          return std::get<0>(a) < std::get<0>(b);
        });
      }
  };

  /** 評価関数用パラメータのクラス。 */
  class EvalParams {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      EvalParams();
      /**
       * コピーコンストラクタ。
       * @param params コピー元。
       */
      EvalParams(const EvalParams& params);
      /**
       * ムーブコンストラクタ。
       * @param params ムーブ元。
       */
      EvalParams(EvalParams&& params);
      /**
       * コピー代入演算子。
       * @param params コピー元。
       */
      EvalParams& operator=(const EvalParams& params);
      /**
       * ムーブ代入演算子。
       * @param params ムーブ元。
       */
      EvalParams& operator=(EvalParams&& params);
      /** デストラクタ。 */
      virtual ~EvalParams() {}

      // ======== //
      // アクセサ //
      // ======== //
      // --- 価値テーブル --- //
      /**
       * アクセサ - オープニング時の配置の価値テーブル。 [駒の種類][マス]
       * @return オープニング時の配置の価値テーブル。
       */
      const double (& opening_position_value_table() const)
      [NUM_PIECE_TYPES][NUM_SQUARES] {return opening_position_value_table_;}
      /**
       * アクセサ - エンディング時の配置の価値テーブル。 [駒の種類][マス]
       * @return エンディング時の配置の価値テーブル。
       */
      const double (& ending_position_value_table() const)
      [NUM_PIECE_TYPES][NUM_SQUARES] {return ending_position_value_table_;}
      /**
       * アクセサ - 相手への攻撃の価値テーブル。
       * [攻め駒の種類][ターゲットの種類]
       * @return 相手への攻撃の価値テーブル。
       */
      const double (& attack_value_table() const)
      [NUM_PIECE_TYPES][NUM_PIECE_TYPES] {return attack_value_table_;}
      /**
       * アクセサ - 味方への攻撃の価値テーブル。
       * [守り駒の種類][ターゲットの種類]
       * @return 味方への攻撃の価値テーブル。
       */
      const double (& defense_value_table() const)
      [NUM_PIECE_TYPES][NUM_PIECE_TYPES] {return defense_value_table_;}
      /**
       * アクセサ - ポーンの盾の配置の価値テーブル。 [マス]
       * @return ポーンの盾の配置の価値テーブル。
       */
      const double (& pawn_shield_value_table() const)
      [NUM_SQUARES] {return pawn_shield_value_table_;}

      // --- ウェイト --- //
      /**
       * アクセサ - オープニング時の配置のウェイト。 [駒の種類]
       * @return オープニング時の配置のウェイト。
       */
      const Weight (& weight_opening_position() const)[NUM_PIECE_TYPES] {
        return weight_opening_position_;
      }
      /**
       * アクセサ - エンディング時の配置のウェイト。 [駒の種類]
       * @return エンディング時の配置のウェイト。
       */
      const Weight (& weight_ending_position() const)[NUM_PIECE_TYPES] {
        return weight_ending_position_;
      }
      /**
       * アクセサ - 機動力のウェイト。 [駒の種類]
       * @return 機動力のウェイト。
       */
      const Weight (& weight_mobility() const)[NUM_PIECE_TYPES] {
        return weight_mobility_;
      }
      /**
       * アクセサ - センターコントロールのウェイト。 [駒の種類]
       * @return センターコントロールのウェイト。
       */
      const Weight (& weight_center_control() const)[NUM_PIECE_TYPES] {
        return weight_center_control_;
      }
      /**
       * アクセサ - スウィートセンターコントロールのウェイト。 [駒の種類]
       * @return スウィートセンターコントロールのウェイト。
       */
      const Weight (& weight_sweet_center_control() const)[NUM_PIECE_TYPES] {
        return weight_sweet_center_control_;
      }
      /**
       * アクセサ - 駒の展開のウェイト。 [駒の種類]
       * @return 駒の展開のウェイト。
       */
      const Weight (& weight_development() const)[NUM_PIECE_TYPES] {
        return weight_development_;
      }
      /**
       * アクセサ - 相手への攻撃のウェイト。 [駒の種類]
       * @return 相手への攻撃のウェイト。
       */
      const Weight (& weight_attack() const)[NUM_PIECE_TYPES] {
        return weight_attack_;
      }
      /**
       * アクセサ - 味方への攻撃のウェイト。 [駒の種類]
       * @return 味方への攻撃のウェイト。
       */
      const Weight (& weight_defense() const)[NUM_PIECE_TYPES] {
        return weight_defense_;
      }
      /**
       * アクセサ - 相手キング周辺への攻撃のウェイト。
       * @return 相手キング周辺への攻撃のウェイト。
       */
      const Weight (& weight_attack_around_king() const)[NUM_PIECE_TYPES] {
        return weight_attack_around_king_;
      }
      /**
       * アクセサ - パスポーンのウェイト。
       * @return パスポーンのウェイト。
       */
      const Weight& weight_pass_pawn() const {return weight_pass_pawn_;}
      /**
       * アクセサ - 守られたパスポーンのウェイト。
       * @return 守られたパスポーンのウェイト。
       */
      const Weight& weight_protected_pass_pawn() const {
        return weight_protected_pass_pawn_;
      }
      /**
       * アクセサ - ダブルポーンのウェイト。
       * @return ダブルポーンのウェイト。
       */
      const Weight& weight_double_pawn() const {return weight_double_pawn_;}
      /**
       * アクセサ - 孤立ポーンのウェイト。
       * @return 孤立ポーンのウェイト。
       */
      const Weight& weight_iso_pawn() const {return weight_iso_pawn_;}
      /**
       * アクセサ - ポーンの盾のウェイト。
       * @return ポーンの盾のウェイト。
       */
      const Weight& weight_pawn_shield() const {return weight_pawn_shield_;}
      /**
       * アクセサ - ビショップペアのウェイト。
       * @return ビショップペアのウェイト。
       */
      const Weight& weight_bishop_pair() const {return weight_bishop_pair_;}
      /**
       * アクセサ - バッドビショップのウェイト。
       * @return バッドビショップのウェイト。
       */
      const Weight& weight_bad_bishop() const {return weight_bad_bishop_;}
      /**
       * アクセサ - ルークペアのウェイト。
       * @return ルークペアのウェイト。
       */
      const Weight& weight_rook_pair() const {return weight_rook_pair_;}
      /**
       * アクセサ - セミオープンファイルのルークのウェイト。
       * @return セミオープンファイルのルークのウェイト。
       */
      const Weight& weight_rook_semiopen_fyle() const {
        return weight_rook_semiopen_fyle_;
      }
      /**
       * アクセサ - オープンファイルのルークのウェイト。
       * @return オープンファイルのルークのウェイト。
       */
      const Weight& weight_rook_open_fyle() const {
        return weight_rook_open_fyle_;
      }
      /**
       * アクセサ - 早すぎるクイーンの始動のウェイト。
       * @return 早すぎるクイーンの始動のウェイト。
       */
      const Weight& weight_early_queen_starting() const {
        return weight_early_queen_starting_;
      }
      /**
       * アクセサ - キング周りの弱いマスのウェイト。
       * @return キング周りの弱いマスのウェイト。
       */
      const Weight& weight_weak_square() const {return weight_weak_square_;}
      /**
       * アクセサ - キャスリングのウェイト。
       * @return キャスリングのウェイト。
       */
      const Weight& weight_castling() const {return weight_castling_;}
      /**
       * アクセサ - キャスリングの放棄のウェイト。
       * @return キャスリングの放棄のウェイト。
       */
      const Weight& weight_abandoned_castling() const {
        return weight_abandoned_castling_;
      }

      // ============ //
      // ミューテータ //
      // ============ //
      // --- 価値テーブル --- //
      /**
       * ミューテータ - オープニング時の配置の価値テーブル。
       * @param table オープニング時の配置の価値テーブル。
       */
      void opening_position_value_table
      (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]);
      /**
       * ミューテータ - エンディング時の配置の価値テーブル。
       * @param table エンディング時の配置の価値テーブル。
       */
      void ending_position_value_table
      (const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES]);
      /**
       * ミューテータ - 相手への攻撃の価値テーブル。
       * @param table 相手への攻撃の価値テーブル。
       */
      void attack_value_table
      (const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES]);
      /**
       * ミューテータ - 味方への攻撃の価値テーブル。
       * @param table 味方への攻撃の価値テーブル。
       */
      void defense_value_table
      (const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES]);
      /**
       * ミューテータ - ポーンの盾の配置の価値テーブル。
       * @param table ポーンの盾の配置の価値テーブル。
       */
      void pawn_shield_value_table(const double (& table)[NUM_SQUARES]);

      // --- ウェイト --- //
      /**
       * ミューテータ - オープニング時の配置のウェイト。
       * @param weights オープニング時の配置のウェイト。
       */
      void weight_opening_position(const Weight (& weights)[NUM_PIECE_TYPES]);
      /**
       * ミューテータ - エンディング時の配置のウェイト。
       * @param weights エンディング時の配置のウェイト。
       */
      void weight_ending_position(const Weight (& weights)[NUM_PIECE_TYPES]);
      /**
       * ミューテータ - 機動力のウェイト。
       * @param weights 機動力のウェイト。
       */
      void weight_mobility(const Weight (& weights)[NUM_PIECE_TYPES]);
      /**
       * ミューテータ - センターコントロールのウェイト。
       * @param weights センターコントロールのウェイト。
       */
      void weight_center_control(const Weight(& weights)[NUM_PIECE_TYPES]);
      /**
       * ミューテータ - スウィートセンターコントロールのウェイト。
       * @param weights スウィートセンターコントロールのウェイト。
       */
      void weight_sweet_center_control
      (const Weight (& weights)[NUM_PIECE_TYPES]);
      /**
       * ミューテータ - 駒の展開のウェイト。
       * @param weights 駒の展開のウェイト。
       */
      void weight_development(const Weight (& weights)[NUM_PIECE_TYPES]);
      /**
       * ミューテータ - 駒への攻撃のウェイト。
       * @param weights 駒への攻撃のウェイト。
       */
      void weight_attack(const Weight (& weights)[NUM_PIECE_TYPES]);
      /**
       * ミューテータ - 味方への防御のウェイト。
       * @param weights 味方への防御のウェイト。
       */
      void weight_defense(const Weight (& weights)[NUM_PIECE_TYPES]);
      /**
       * ミューテータ - 相手キング周辺への攻撃のウェイト。
       * @param weights 相手キング周辺への攻撃のウェイト。
       */
      void weight_attack_around_king(const Weight(& weight)[NUM_PIECE_TYPES]);
      /**
       * ミューテータ - パスポーンのウェイト。
       * @param weight パスポーンのウェイト。
       */
      void weight_pass_pawn(const Weight& weight) {
        weight_pass_pawn_ = weight;
      }
      /**
       * ミューテータ - 守られたパスポーンのウェイト。
       * @param weight 守られたパスポーンのウェイト。
       */
      void weight_protected_pass_pawn(const Weight& weight) {
        weight_protected_pass_pawn_ = weight;
      }
      /**
       * ミューテータ - ダブルポーンのウェイト。
       * @param weight ダブルポーンのウェイト。
       */
      void weight_double_pawn(const Weight& weight) {
        weight_double_pawn_ = weight;
      }
      /**
       * ミューテータ - 孤立ポーンのウェイト。
       * @param weight 孤立ポーンのウェイト。
       */
      void weight_iso_pawn(const Weight& weight) {weight_iso_pawn_ = weight;}
      /**
       * ミューテータ - ポーンの盾のウェイト。
       * @param weight ポーンの盾のウェイト。
       */
      void weight_pawn_shield(const Weight& weight) {
        weight_pawn_shield_ = weight;
      }
      /**
       * ミューテータ - ビショップペアのウェイト。
       * @param weight ビショップペアのウェイト。
       */
      void weight_bishop_pair(const Weight& weight) {
        weight_bishop_pair_ = weight;
      }
      /**
       * ミューテータ - バッドビショップのウェイト。
       * @param weight バッドビショップのウェイト。
       */
      void weight_bad_bishop(const Weight& weight) {
        weight_bad_bishop_ = weight;
      }
      /**
       * ミューテータ - ルークペアのウェイト。
       * @param weight ルークペアのウェイト。
       */
      void weight_rook_pair(const Weight& weight) {weight_rook_pair_ = weight;}
      /**
       * ミューテータ - セミオープンファイルのルークのウェイト。
       * @param weight セミオープンファイルのルークのウェイト。
       */
      void weight_rook_semiopen_fyle(const Weight& weight) {
        weight_rook_semiopen_fyle_ = weight;
      }
      /**
       * ミューテータ - オープンファイルのルークのウェイト。
       * @param weight オープンファイルのルークのウェイト。
       */
      void weight_rook_open_fyle(const Weight& weight) {
        weight_rook_open_fyle_ = weight;
      }
      /**
       * ミューテータ - 早すぎるクイーンの始動のウェイト。
       * @param weight 早すぎるクイーンの始動のウェイト。
       */
      void weight_early_queen_starting(const Weight& weight) {
        weight_early_queen_starting_ = weight;
      }
      /**
       * ミューテータ - キング周りの弱いマスのウェイト。
       * @param weight キング周りの弱いマスのウェイト。
       */
      void weight_weak_square(const Weight& weight) {
        weight_weak_square_ = weight;
      }
      /**
       * ミューテータ - キャスリングのウェイト。
       * @param weight キャスリングのウェイト。
       */
      void weight_castling(const Weight& weight) {weight_castling_ = weight;}
      /**
       * ミューテータ - キャスリングの放棄のウェイト。
       * @param weight キャスリングの放棄のウェイト。
       */
      void weight_abandoned_castling(const Weight& weight) {
        weight_abandoned_castling_ = weight;
      }

      // ============== //
      // ミューテータ 2 //
      // ============== //
      /**
       * ミューテータ 2 - オープニング時の配置の価値テーブル。
       * @param piece_type 駒の種類。
       * @param square 位置。
       * @param value 価値。
       */
      void opening_position_value_table(PieceType piece_type, Square square,
      double value) {
        opening_position_value_table_[piece_type][square] = value;
      }
      /**
       * ミューテータ 2 - エンディング時の配置の価値テーブル。
       * @param piece_type 駒の種類。
       * @param square 位置。
       * @param value 価値。
       */
      void ending_position_value_table(PieceType piece_type, Square square,
      double value) {
        ending_position_value_table_[piece_type][square] = value;
      }
      /**
       * ミューテータ 2 - 相手への攻撃の価値テーブル。
       * @param piece_type 自分の駒の種類。
       * @param enemy_piece_type 相手の駒の種類。
       * @param value 価値。
       */
      void attack_value_table(PieceType piece_type,
      PieceType enemy_piece_type, double value) {
        attack_value_table_[piece_type][enemy_piece_type] = value;
      }
      /**
       * ミューテータ 2 - 味方への防御の価値テーブル。
       * @param piece_type 自分の駒の種類。
       * @param friend_piece_type 味方の駒の種類。
       * @param value 価値。
       */
      void defense_value_table(PieceType piece_type,
      PieceType friend_piece_type, double value) {
        defense_value_table_[piece_type][friend_piece_type] = value;
      }
      /**
       * ミューテータ 2 - ポーンの盾の価値テーブル。
       * @param square ポーンの位置。
       * @param value 価値。
       */
      void pawn_shield_value_table(Square square, double value) {
        pawn_shield_value_table_[square] = value;
      }
      /**
       * ミューテータ 2 - オープニング時の駒の配置のウェイト。
       * @param piece_type 駒の種類。
       * @param opening_weight オープニング時のウェイト。
       * @param ending_weight エンディング時のウェイト。
       */
      void weight_opening_position(PieceType piece_type,
      const Weight& weight) {
        weight_opening_position_[piece_type] = weight;
      }
      /**
       * ミューテータ 2 - エンディング時の駒の配置のウェイト。
       * @param piece_type 駒の種類。
       * @param opening_weight オープニング時のウェイト。
       * @param ending_weight エンディング時のウェイト。
       */
      void weight_ending_position(PieceType piece_type,
      const Weight& weight) {
        weight_ending_position_[piece_type] = weight;
      }
      /**
       * ミューテータ 2 - 機動力のウェイト。
       * @param piece_type 駒の種類。
       * @param opening_weight オープニング時のウェイト。
       * @param ending_weight エンディング時のウェイト。
       */
      void weight_mobility(PieceType piece_type,
      const Weight& weight) {
        weight_mobility_[piece_type] = weight;
      }
      /**
       * ミューテータ 2 - センターコントロールのウェイト。
       * @param piece_type 駒の種類。
       * @param opening_weight オープニング時のウェイト。
       * @param ending_weight エンディング時のウェイト。
       */
      void weight_center_control(PieceType piece_type,
      const Weight& weight) {
        weight_center_control_[piece_type] = weight;
      }
      /**
       * ミューテータ 2 - スウィートセンターコントロールのウェイト。
       * @param piece_type 駒の種類。
       * @param opening_weight オープニング時のウェイト。
       * @param ending_weight エンディング時のウェイト。
       */
      void weight_sweet_center_control(PieceType piece_type,
      const Weight& weight) {
        weight_sweet_center_control_[piece_type] = weight;
      }
      /**
       * ミューテータ 2 - 駒の展開のウェイト。
       * @param piece_type 駒の種類。
       * @param opening_weight オープニング時のウェイト。
       * @param ending_weight エンディング時のウェイト。
       */
      void weight_development(PieceType piece_type,
      const Weight& weight) {
        weight_development_[piece_type] = weight;
      }
      /**
       * ミューテータ 2 - 相手への攻撃のウェイト。
       * @param piece_type 駒の種類。
       * @param opening_weight オープニング時のウェイト。
       * @param ending_weight エンディング時のウェイト。
       */
      void weight_attack(PieceType piece_type,
      const Weight& weight) {
        weight_attack_[piece_type] = weight;
      }
      /**
       * ミューテータ 2 - 味方への防御のウェイト。
       * @param piece_type 駒の種類。
       * @param opening_weight オープニング時のウェイト。
       * @param ending_weight エンディング時のウェイト。
       */
      void weight_defense(PieceType piece_type,
      const Weight& weight) {
        weight_defense_[piece_type] = weight;
      }
      /**
       * ミューテータ 2 - 相手キング周辺への攻撃のウェイト。
       * @param piece_type 駒の種類。
       * @param opening_weight オープニング時のウェイト。
       * @param ending_weight エンディング時のウェイト。
       */
      void weight_attack_around_king(PieceType piece_type,
      const Weight& weight) {
        weight_attack_around_king_[piece_type] = weight;
      }

    private:
      /**
       * メンバをコピーする。
       * @param params コピー元。
       */
      void ScanMember(const EvalParams& params);

      // ============== //
      // 価値テーブル類 //
      // ============== //
      /** オープニング時の配置の価値テーブル。 */
      double opening_position_value_table_[NUM_PIECE_TYPES][NUM_SQUARES];
      /** エンディング時の配置の価値テーブル。 */
      double ending_position_value_table_[NUM_PIECE_TYPES][NUM_SQUARES];
      /** 相手への攻撃の価値テーブル。 */
      double attack_value_table_[NUM_PIECE_TYPES][NUM_PIECE_TYPES];
      /** 味方への防御の価値テーブル。 */
      double defense_value_table_[NUM_PIECE_TYPES][NUM_PIECE_TYPES];
      /** ポーンの盾の配置の価値テーブル。 */
      double pawn_shield_value_table_[NUM_SQUARES];

      // ======== //
      // ウェイト //
      // ======== //
      /** オープニング時の配置のウェイト。 */
      Weight weight_opening_position_[NUM_PIECE_TYPES];
      /** エンディング時の駒の配置のウェイト。 */
      Weight weight_ending_position_[NUM_PIECE_TYPES];
      /** 機動力のウェイト。 */
      Weight weight_mobility_[NUM_PIECE_TYPES];
      /** センターコントロールのウェイト。 */
      Weight weight_center_control_[NUM_PIECE_TYPES];
      /** スウィートセンターのコントロールのウェイト。 */
      Weight weight_sweet_center_control_[NUM_PIECE_TYPES];
      /** 駒の展開のウェイト。 */
      Weight weight_development_[NUM_PIECE_TYPES];
      /** 相手への攻撃のウェイト。 */
      Weight weight_attack_[NUM_PIECE_TYPES];
      /** 味方への防御のウェイト。 */
      Weight weight_defense_[NUM_PIECE_TYPES];
      /** 相手キング周辺への攻撃のウェイト。 */
      Weight weight_attack_around_king_[NUM_PIECE_TYPES];
      /** パスポーンのウェイト。 */
      Weight weight_pass_pawn_;
      /** 守られたパスポーンのウェイト。 */
      Weight weight_protected_pass_pawn_;
      /** ダブルポーンのウェイト。 */
      Weight weight_double_pawn_;
      /** 孤立ポーンのウェイト。 */
      Weight weight_iso_pawn_;
      /** ポーンの盾のウェイト。 */
      Weight weight_pawn_shield_;
      /** ビショップペアのウェイト。 */
      Weight weight_bishop_pair_;
      /** バッドビショップのウェイト。 */
      Weight weight_bad_bishop_;
      /** ルークペアのウェイト。 */
      Weight weight_rook_pair_;
      /** セミオープンファイルのルークのウェイト。 */
      Weight weight_rook_semiopen_fyle_;
      /** オープンファイルのルークのウェイト。 */
      Weight weight_rook_open_fyle_;
      /** 早すぎるクイーンの始動のウェイト。 */
      Weight weight_early_queen_starting_;
      /** キング周りの弱いマスのウェイト。 */
      Weight weight_weak_square_;
      /** キャスリングのウェイト。 */
      Weight weight_castling_;
      /** キャスリングの放棄のウェイト。 */
      Weight weight_abandoned_castling_;
  };
}  // namespace Sayuri

#endif
