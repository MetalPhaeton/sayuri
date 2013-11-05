/*
   search_queue.h: マルチスレッド探索用キュー。

   The MIT License (MIT)

   Copyright (c) 2013 Ishibashi Hironori

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

#ifndef SEARCH_QUEUE_H
#define SEARCH_QUEUE_H

#include <iostream>
#include <mutex>
#include <condition_variable>
#include "chess_def.h"
#include "chess_engine.h"
#include "transposition_table.h"
#include "pv_line.h"

namespace Sayuri {
  class ChessEngine;

  // マルチスレッド探索用キューのクラス。
  class SearchQueue {
    public:
      /********************/
      /* パブリンク関数。 */
      /********************/
      void Enqueue(Move move);
      Move Dequeue();
      void StopChild();

    private:
      /****************/
      /* メンバ変数。 */
      /****************/
      // 親スレッドより情報。
      ChessEngine* engine_ptr_;
      Hash pos_hash_;
      int depth_;
      int level_;
      int* alpha_ptr_;
      int* beta_ptr_;
      int* delta_ptr_;
      TranspositionTable* table_ptr_;
      PVLine* pv_line_ptr_;
      int* searched_moves_ptr_;
      bool is_reduced_by_null_;
      bool do_futility_pruning_;
      std::vector<Move>* moves_to_search_ptr_;
      std::vector<Move>* root_move_vec_ptr_;

      // 指し手。
      Move move;

      // 子エンジン。
      std::vector<ChessEngine*> child_ptr_vec_;
      // ミューテックス。
      std::mutex mutex_;
      // エンキュー用コンディション。
      std::condition_variable enq_cond_;
      // デキュー用コンディション。
      std::condition_variable deq_cond_;
  };
}  // namespace Sayuri

#endif
