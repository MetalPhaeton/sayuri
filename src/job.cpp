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
  Job::Job() : num_helpers_(0), maker_ptr_(nullptr), counter_(0) {}

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

  // ベータカットを通知する。
  void Job::NotifyBetaCut(ChessEngine& notifier) {
    std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
    // クライアントに通知。
    if ((client_ptr_ != &notifier)
    && (client_ptr_->notice_cut_level_ > level_)) {
      client_ptr_->notice_cut_level_ = level_;
    }

    // ヘルパーに通知。
    for (ChessEngine** ptrptr = helpers_table_; ptrptr < end_;
    ++ptrptr) {
      if (*ptrptr && (*ptrptr != &notifier)
      && ((*ptrptr)->notice_cut_level_ > level_)) {
        (*ptrptr)->notice_cut_level_ = level_;
      }
    }
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

    COPY_ARRAY(helpers_table_, job.helpers_table_);
    end_ = (job.end_ - job.helpers_table_) + helpers_table_;
    num_helpers_ = job.num_helpers_;
    maker_ptr_ = job.maker_ptr_;
    counter_ = job.counter_;
  }
}  // namespace Sayuri
