/*
   job.h: マルチスレッド探索用仕事クラス。

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

#ifndef JOB_H
#define JOB_H

#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <memory>
#include "common.h"

namespace Sayuri {
  class ChessEngine;
  class TranspositionTable;
  class MoveMaker;
  class PositionRecord;
  class PVLine;

  // マルチスレッド探索用の仕事クラス。
  class Job {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      Job();
      Job(const Job& job);
      Job(Job&& job);
      Job& operator=(const Job& job);
      Job& operator=(Job&& job);
      virtual ~Job() {}

      /********************/
      /* パブリック関数。 */
      /********************/
      // Jobの初期化。
      void Init(MoveMaker& maker) {
        maker_ptr_ = &maker;
        helper_counter_ = 0;
        counter_ = 0;
      }
      // 手を得る。
      // [戻り値]
      // 手。
      Move PickMove();
      // ヘルパーの数を一つ増やす。
      void CountHelper();
      // 仕事終了の合図を出す。
      void FinishMyJob();
      // ヘルパーが全員仕事を終えるまで待機する。
      void WaitForHelpers();
      // 数を数える。探索した手の数を数えるときに使う。
      int Count();

      /***********************************/
      /* 仕事変数。 自由にアクセス可能。 */
      /***********************************/
      ChessEngine* client_ptr_;
      std::mutex* mutex_ptr_;
      PositionRecord* record_ptr_;
      NodeType node_type_;
      Hash pos_hash_;
      int depth_;
      int level_;
      int* alpha_ptr_;
      int* beta_ptr_;
      int* delta_ptr_;
      TranspositionTable* table_ptr_;
      PVLine* pv_line_ptr_;
      bool is_null_searching_;
      int null_reduction_;
      ScoreType* score_type_ptr_;
      int material_;
      bool is_checked_;
      int num_all_moves_;
      bool* has_legal_move_ptr_;
      const std::vector<Move>* moves_to_search_ptr_;
      TimePoint* next_print_info_time_ptr_;

    private:
      /**********************/
      /* プライベート関数。 */
      /**********************/
      // メンバーをコピーする。
      // [引数]
      // job: コピー元。
      void ScanMember(const Job& job);

      /****************/
      /* メンバ変数。 */
      /****************/
      // 親スレッドより情報。

      // 仕事用ムーブメーカー。
      MoveMaker* maker_ptr_;
      // ヘルパーカウンター。
      volatile int helper_counter_;
      // ミューテックス。
      std::mutex mutex_;
      // コンディション。
      std::condition_variable cond_;
      // 数を数えるためのカウンター。UCIのcurrmovenumberの表示に使用する。
      volatile int counter_;
  };
}  // namespace Sayuri

#endif
