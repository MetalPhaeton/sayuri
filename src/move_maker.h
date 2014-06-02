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
 * @file move_maker.h
 * @author Hironori Ishibashi
 * @brief 候補手を生成するクラス。
 */

#ifndef MOVE_MAKER_H
#define MOVE_MAKER_H

#include <iostream>
#include <mutex>
#include <cstddef>
#include <cstdint>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;

  /** 候補手を生成するクラス。 */
  class MoveMaker {
    private:
      /** 候補手を保存する構造体。 */
      struct MoveSlot {
        /** 候補手。 */
        Move move_;
        /** 候補手の評価。 */
        int score_;
      };

    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param engine 候補手を生成するためのエンジン。
       */
      MoveMaker(const ChessEngine& engine);
      /** コンストラクタ。 */
      MoveMaker() {}
      /**
       * コピーコンストラクタ。
       * @param maker コピー元。
       */
      MoveMaker(const MoveMaker& maker);
      /**
       * ムーブコンストラクタ。
       * @param maker ムーブ元。
       */
      MoveMaker(MoveMaker&& maker);
      /**
       * コピー演算子。
       * @param maker コピー元。
       */
      MoveMaker& operator=(const MoveMaker& maker);
      /**
       * ムーブ演算子。
       * @param maker ムーブ元。
       */
      MoveMaker& operator=(MoveMaker&& maker);
      /** デストラクタ。 */
      virtual ~MoveMaker() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * スタックに候補手を生成する。
       * 自らチェックされる手も作る。
       * @param <Type> 生成する手の種類。
       * - NON_CAPTURE: 駒を取らない手。
       * - CAPTURE: 駒を取る手。
       * - ALL: 上記の両方。
       * @param prev_best Iterative Deepeningによる前回の繰り返しの最善手。
       * @param iid_move IIDによる最善手。
       * @param killer_1 同一レベルでのキラームーブ。
       * @param killer_2 2プライ前のレベルのキラームーブ。
       * @return 生成した候補手の数。
       */
      template<GenMoveType Type> int GenMoves(Move prev_best,
      Move iid_move, Move killer_1, Move killer_2);

      /**
       * スタックに候補手を再生成する。
       * @return 生成した候補手の数。
       */
      int RegenMoves() {
        last_ = max_;
        return CountMoves();
      }

      /** スタックをリセットする。 */
      void ResetStack() {
        last_ = max_ = 0;
        history_max_ = 1;
      }

      /**
       * 次の候補手を取り出す。
       * @return 次の候補手。 なければ、0。
       */
      Move PickMove();

      /**
       * スタック内の残っている候補手の数を返す。
       * @return 残っている候補手の数。
       */
      int CountMoves() const {return last_;}

    private:
      /** フレンドのデバッグ用関数。 */
      friend int DebugMain(int argc, char* argv[]);

      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * スタックに候補手を生成する。 (内部用)
       * 自らチェックされる手も作る。
       * @param <Type> 生成する手の種類。
       * - NON_CAPTURE: 駒を取らない手。
       * - CAPTURE: 駒を取る手。
       * - ALL: 上記の両方。
       * @param prev_best Iterative Deepeningによる前回の繰り返しの最善手。
       * @param iid_move IIDによる最善手。
       * @param killer_1 同一レベルでのキラームーブ。
       * @param killer_2 2プライ前のレベルのキラームーブ。
       * @return 生成した候補手の数。
       */
      template<GenMoveType Type> void GenMovesCore(Move prev_best,
      Move iid_move, Move killer_1, Move killer_2);

      /**
       * 候補手に点数をつける。
       * @param <Type> 生成する手の種類。
       * - NON_CAPTURE: 駒を取らない手。
       * - CAPTURE: 駒を取る手。
       * - ALL: 上記の両方。
       * @param start 点数をつける候補手の最初のインデックス。
       * @param prev_best Iterative Deepeningによる前回の繰り返しの最善手。
       * @param iid_move IIDによる最善手。
       * @param killer_1 同一レベルでのキラームーブ。
       * @param killer_2 2プライ前のレベルのキラームーブ。
       * @param side 候補手のサイド。
       */
      template<GenMoveType Type>
      void ScoreMoves(std::size_t start, Move prev_best, Move iid_move,
      Move killer_1, Move killer_2, Side side);

      // ========== //
      // メンバ変数 //
      // ========== //
      /** 候補手生成に使うエンジン。 */
      const ChessEngine* engine_ptr_;

      /** 候補手のスタック。 [候補手の番号] */
      MoveSlot move_stack_[MAX_CANDIDATES + 1];

      /** スタックのインデックス。 */
      std::size_t last_;
      std::size_t max_;

      /** ヒストリーの最大値。 */
      std::uint64_t history_max_;

      /** ミューテックス。 */
      std::mutex mutex_;
  };
}  // namespace Sayuri

#endif
