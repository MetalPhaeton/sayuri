/*
   helper_queue.h: マルチスレッド探索用のスレッドのキュー。

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

#ifndef HELPER_QUEUE_H
#define HELPER_QUEUE_H

#include <iostream>
#include <mutex>
#include <condition_variable>
#include "chess_engine.h"
#include "job.h"

namespace Sayuri {
  class ChessEngine;
  class Job;

  // マルチスレッド探索のスレッドのキューのクラス。
  class HelperQueue {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      HelperQueue();
      HelperQueue(const HelperQueue& queue);
      HelperQueue(HelperQueue&& queue);
      HelperQueue& operator=(const HelperQueue& queue);
      HelperQueue& operator=(HelperQueue&& queue);
      virtual ~HelperQueue() {}

      /********************/
      /* パブリック関数。 */
      /********************/
      // 空きスレッドが仕事を得る。
      // [戻り値]
      // 仕事へのポインタ。なければnullptr。
      Job* GetJob();
      // 待っているヘルパーの数を得る。
      // [戻り値]
      // 待っているヘルパーの数。
      int CountHelpers() const {return num_helpers_;}
      // 空きスレッドに仕事を依頼する。
      // [引数]
      // job: 仕事。
      void Help(Job& job);
      // 空きスレッドに仕事を依頼する。ルートノード用。
      // [引数]
      // job: 仕事。
      void HelpRoot(Job& job);
      // 待機中の空きスレッドをキューから開放する。
      void ReleaseHelpers();

    private:
      /****************/
      /* メンバ変数。 */
      /****************/
      // 仕事の一時保管場所。
      Job* job_ptr_;
      // ミューテックス。
      std::mutex mutex_;
      // ヘルパー用コンディション。
      std::condition_variable helper_cond_;
      // クライアント用コンディション。
      std::condition_variable client_cond_;
      // もうヘルパーは必要ない。
      volatile bool no_more_help_;
      // 待っているヘルパーの数。
      volatile int num_helpers_;
      // ルートノードのクライアントが待っているかどうか。
      volatile bool is_root_client_waiting_;
  };
}  // namespace Sayuri

#endif
