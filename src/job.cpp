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
 * @file job.cpp
 * @author Hironori Ishibashi
 * @brief 並列探索用仕事の実装。
 */

#include "job.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "common.h"
#include "chess_engine.h"
#include "transposition_table.h"
#include "pv_line.h"
#include "move_maker.h"
#include "position_record.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  Job::Job() : maker_ptr_(nullptr), helper_counter_(0),
  betacut_listener_vec_(0), counter_(0) {}

  // コピーコンストラクタ。
  Job::Job(const Job& job) {
    ScanMember(job);
  }

  // ムーブコンストラクタ。
  Job::Job(Job&& job) {
    ScanMember(job);
  }

  // コピー代入演算子。
  Job& Job::operator=(const Job& job) {
    ScanMember(job);

    return *this;
  }

  // ムーブ代入演算子。
  Job& Job::operator=(Job&& job) {
    ScanMember(job);

    return *this;
  }

  // ============== //
  // パブリック関数 //
  // ============== //
  // 候補手を得る。
  Move Job::PickMove() {
    std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
    return maker_ptr_->PickMove();
  }

  // この仕事を請け負っているヘルパーの数を増やす。
  void Job::CountHelper() {
    std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
    ++helper_counter_;
  }

  // 仕事終了を知らせる。
  void Job::FinishMyJob() {
    std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
    --helper_counter_;
    cond_.notify_all();
  }

  // ヘルパーが全員仕事を終えるまで待機する。
  void Job::WaitForHelpers() {
    std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
    while (helper_counter_ > 0) {
      cond_.wait(lock);
    }
  }

  // ヘルパーにベータカットが発生したことを知らせる。
  void Job::NotifyBetaCut() {
    std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。

    for (auto& listener : betacut_listener_vec_) {
      listener(*this);
    }
    betacut_listener_vec_.clear();
  }

  // 数を数える。
  int Job::Count() {
    std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。

    ++counter_;
    return counter_;
  }

  // ================ //
  // プライベート関数 //
  // ================ //
  // メンバーをコピーする。
  void Job::ScanMember(const Job& job) {
    std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。

    client_ptr_ = job.client_ptr_;
    record_ptr_ = job.record_ptr_;
    node_type_ = job.node_type_;
    pos_hash_ = job.pos_hash_;
    depth_ = job.depth_;
    level_ = job.level_;
    alpha_ = job.alpha_;
    beta_ = job.beta_;
    delta_ = job.delta_;
    table_ptr_ = job.table_ptr_;
    pv_line_ptr_ = job.pv_line_ptr_;
    is_null_searching_ = job.is_null_searching_;
    null_reduction_ = job.null_reduction_;
    score_type_ = job.score_type_;
    material_ = job.material_;
    is_checked_ = job.is_checked_;
    num_all_moves_ = job.num_all_moves_;
    has_legal_move_ = job.has_legal_move_;
    moves_to_search_ptr_ = job.moves_to_search_ptr_;
    next_print_info_time_ = job.next_print_info_time_;
    maker_ptr_ = job.maker_ptr_;
    helper_counter_ = job.helper_counter_;
    betacut_listener_vec_ = job.betacut_listener_vec_;
    counter_ = job.counter_;
  }
}  // namespace Sayuri
