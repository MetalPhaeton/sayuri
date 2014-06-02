/*
   helper_queue.cpp: マルチスレッド探索用のスレッドのキューの実装。

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

#include "helper_queue.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include "common.h"
#include "job.h"

namespace Sayuri {
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  HelperQueue::HelperQueue() : job_ptr_(nullptr), no_more_help_(false),
  num_helpers_(0), is_root_client_waiting_(false) {}

  // コピーコンストラクタ。
  HelperQueue::HelperQueue(const HelperQueue& queue) :
  job_ptr_(queue.job_ptr_), no_more_help_(queue.no_more_help_),
  num_helpers_(queue.num_helpers_), is_root_client_waiting_(false) {}

  // ムーブコンストラクタ。
  HelperQueue::HelperQueue(HelperQueue&& queue) :
  job_ptr_(queue.job_ptr_), no_more_help_(queue.no_more_help_),
  num_helpers_(queue.num_helpers_), is_root_client_waiting_(false) {}

  // コピー代入演算子。
  HelperQueue& HelperQueue::operator=(const HelperQueue& queue) {
    job_ptr_ = queue.job_ptr_;
    no_more_help_ = queue.no_more_help_;
    num_helpers_ = queue.num_helpers_;
    is_root_client_waiting_ = queue.is_root_client_waiting_;
    return *this;
  }

  // ムーブ代入演算子。
  HelperQueue& HelperQueue::operator=(HelperQueue&& queue) {
    job_ptr_ = queue.job_ptr_;
    no_more_help_ = queue.no_more_help_;
    num_helpers_ = queue.num_helpers_;
    is_root_client_waiting_ = queue.is_root_client_waiting_;
    return *this;
  }
  /********************/
  /* パブリック関数。 */
  /********************/
  // 空きスレッドが仕事を得る。
  Job* HelperQueue::GetJob() {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    // ヘルパーが必要なければnullptrを返す。
    if (no_more_help_) {
      client_cond_.notify_all();
      return nullptr;
    }

    // 待つ。
    // ルートノードのクライアントが待っていれば待たない。
    if (!is_root_client_waiting_) {
      while (!job_ptr_) {
        num_helpers_++;
        helper_cond_.wait(lock);
        num_helpers_--;
        // 待っている間にno_more_help_が立っているかもしれないので、
        // もう一度確認。
        // ヘルパーが必要なければnullptrを返す。
        if (no_more_help_) {
          client_cond_.notify_all();
          return nullptr;
        }
      }
    }
    is_root_client_waiting_ = false;


    // 仕事の準備。
    job_ptr_->CountHelper();
    Job* temp = job_ptr_;
    job_ptr_ = nullptr;

    // 準備完了を通知。
    client_cond_.notify_all();

    return temp;
  }

  // 空きスレッドに仕事を依頼する。
  void HelperQueue::Help(Job& job) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。
    
    // ヘルパーがいれば仕事を依頼。
    if (num_helpers_ > 0) {
      job_ptr_ = &job;
      helper_cond_.notify_one();

      // ヘルパーの準備が完了するまで待つ。
      client_cond_.wait(lock);
    }
  }

  // 空きスレッドに仕事を依頼する。ルートノード用。
  void HelperQueue::HelpRoot(Job& job) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    job_ptr_ = &job;
    helper_cond_.notify_one();

    // ヘルパーがやってきて、準備が完了するまで待つ。
    is_root_client_waiting_ = true;
    client_cond_.wait(lock);
    is_root_client_waiting_ = false;
  }

  // 空きスレッドをキューから開放する。
  void HelperQueue::ReleaseHelpers() {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    job_ptr_ = nullptr;
    no_more_help_ = true;
    helper_cond_.notify_all();

    // 無事、ヘルパーが抜けるまで待つ。
    while (num_helpers_ > 0) {
      client_cond_.wait(lock);
    }
  }
}  // namespace Sayuri
