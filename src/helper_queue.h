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
 * @file helper_queue.h
 * @author Hironori Ishibashi
 * @brief 並列探索用スレッドのキュー。
 */

#ifndef HELPER_QUEUE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define HELPER_QUEUE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <mutex>
#include <condition_variable>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;
  class Job;

  /** 並列探索のスレッドのキューのクラス。 */
  class HelperQueue {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      HelperQueue();
      /**
       * コピーコンストラクタ。
       * @param queue コピー元。
       */
      HelperQueue(const HelperQueue& queue);
      /**
       * ムーブコンストラクタ。
       * @param queue ムーブ元。
       */
      HelperQueue(HelperQueue&& queue);
      /**
       * コピー代入演算子。
       * @param queue コピー元。
       */
      HelperQueue& operator=(const HelperQueue& queue);
      /**
       * ムーブ代入演算子。
       * @param queue ムーブ元。
       */
      HelperQueue& operator=(HelperQueue&& queue);
      /** デストラクタ。 */
      virtual ~HelperQueue() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * スレッドが仕事を得る。 得るまで待機。
       * @param helper_ptr 得ようとしているヘルパーのポインタ。
       * @return 仕事へのポインタ。 なければnullptr。
       */
      Job* GetJob(ChessEngine* helper_ptr);

      /**
       * 待機中のスレッドの数を得る。
       * @return 待機中のスレッドの数。
       */
      int CountHelpers() const {return num_helpers_;}

      /**
       * スレッドに仕事を依頼する。
       * @param job 依頼する仕事。
       */
      void Help(Job& job);
      /**
       * スレッドに仕事を依頼する。 (ルートノード用)
       * @param job 依頼する仕事。
       */
      void HelpRoot(Job& job);

      /** 待機中のスレッドをキューから開放する。 */
      void ReleaseHelpers();

    private:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** 仕事の一時保管場所。 */
      Job* job_ptr_;
      /** ミューテックス。 */
      std::mutex mutex_;
      /** ヘルパー用コンディション。 */
      std::condition_variable helper_cond_;
      /** クライアント用コンディション。 */
      std::condition_variable client_cond_;
      /** もうヘルパーは必要ない。 */
      volatile bool no_more_help_;
      /** 待っているヘルパーの数。 */
      volatile int num_helpers_;
      /** ルートノードのクライアントが待っているかどうかのフラグ。 */
      volatile bool is_root_client_waiting_;
  };
}  // namespace Sayuri

#endif
