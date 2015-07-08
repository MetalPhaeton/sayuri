/* The MIT License (MIT)
 *
 * Copyright (c) 2015 Hironori Ishibashi
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
 * @file helper_queue.cpp
 * @author Hironori Ishibashi
 * @brief 並列探索用スレッドのキューの実装。
 */

#include "helper_queue.h"

#include <iostream>
#include <mutex>
#include <memory>
#include <condition_variable>
#include "common.h"
#include "chess_engine.h"
#include "job.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
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
  // ============== //
  // パブリック関数 //
  // ============== //
  // スレッドが仕事を得る。
  Job* HelperQueue::GetJob(ChessEngine* helper_ptr) {
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
        ++num_helpers_;
        helper_cond_.wait(lock);
        --num_helpers_;
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


    // --- 仕事の準備 --- //
    // ヘルパー登録。
    job_ptr_->RegisterHelper(*helper_ptr);

    job_ptr_->Lock();  // ロック。

    // 局面のコピー。
    helper_ptr->LoadRecord(*(job_ptr_->record_ptr_));

    // 探索状態のコピー。
    helper_ptr->is_null_searching_ = job_ptr_->is_null_searching_;

    job_ptr_->Unlock();  // ロック解除。

    // job_ptr_を削除。
    Job* temp = job_ptr_;
    job_ptr_ = nullptr;

    // 準備完了を通知。
    client_cond_.notify_all();

    return temp;
  }

  // スレッドに仕事を依頼する。
  void HelperQueue::Help(Job& job) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。
    
    // ヘルパーがいれば仕事を依頼。
    if (num_helpers_ > 0) {
      job.Lock();
      if (!(job.record_ptr_)) {
        job.record_ptr_ =
        &(job.client_ptr_->GetRecord(job.level_, job.pos_hash_));
      }

      job_ptr_ = &job;
      job.Unlock();
      helper_cond_.notify_one();

      // ヘルパーの準備が完了するまで待つ。
      client_cond_.wait(lock);
    }
  }

  // スレッドに仕事を依頼する。 (ルートノード用)
  void HelperQueue::HelpRoot(Job& job) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    job.Lock();
    if (!(job.record_ptr_)) {
      job.record_ptr_ =
      &(job.client_ptr_->GetRecord(job.level_, job.pos_hash_));
    }

    job_ptr_ = &job;
    job.Unlock();
    helper_cond_.notify_one();

    // ヘルパーがやってきて、準備が完了するまで待つ。
    is_root_client_waiting_ = true;
    client_cond_.wait(lock);
    is_root_client_waiting_ = false;
  }

  // 待機中のスレッドをキューから開放する。
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
