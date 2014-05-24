/*
   job.cpp: マルチスレッド探索用の仕事クラスの実装。

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

#include "job.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include "common.h"
#include "chess_engine.h"
#include "transposition_table.h"
#include "pv_line.h"
#include "move_maker.h"
#include "position_record.h"

namespace Sayuri {
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  Job::Job(std::mutex& mutex, MoveMaker& maker, ChessEngine& client,
  PositionRecord& record, NodeType node_type, Hash pos_hash, int depth,
  int level, int& alpha, int& beta, int& delta, TranspositionTable& table,
  PVLine& pv_line, bool is_null_searching, int null_reduction,
  ScoreType& score_type, int material, bool is_checked, int num_all_moves,
  bool& has_legal_move, const std::vector<Move>& moves_to_search,
  TimePoint& next_print_info_time) :
  mutex_ptr_(&mutex),
  client_ptr_(&client),
  record_ptr_(&record),
  node_type_(node_type),
  pos_hash_(pos_hash),
  depth_(depth),
  level_(level),
  alpha_ptr_(&alpha),
  beta_ptr_(&beta),
  delta_ptr_(&delta),
  table_ptr_(&table),
  pv_line_ptr_(&pv_line),
  is_null_searching_(is_null_searching),
  null_reduction_(null_reduction),
  score_type_ptr_(&score_type),
  material_(material),
  is_checked_(is_checked),
  num_all_moves_(num_all_moves),
  has_legal_move_ptr_(&has_legal_move),
  moves_to_search_ptr_(&moves_to_search),
  next_print_info_time_ptr_(&next_print_info_time),
  maker_ptr_(&maker),
  helper_counter_(0),
  counter_(0) {}

  // コピーコンストラクタ。
  Job::Job(const Job& job) {
    ScanMember(job);
  }

  // ムーブコンストラクタ。
  Job::Job(Job&& job) {
    ScanMember(job);
  }

  // コピー代入。
  Job& Job::operator=(const Job& job) {
    ScanMember(job);

    return *this;
  }

  // ムーブ代入。
  Job& Job::operator=(Job&& job) {
    ScanMember(job);

    return *this;
  }

  /********************/
  /* パブリック関数。 */
  /********************/
  // 手を得る。
  Move Job::PickMove() {
    return maker_ptr_->PickMove();
  }

  // ヘルパーのカウント数を増やす。
  void Job::CountHelper() {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。
    helper_counter_++;
  }

  // 仕事終了の合図を出す。
  void Job::FinishMyJob() {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。
    helper_counter_--;
    cond_.notify_all();
  }

  // ヘルパー全員の仕事終了まで待機する。
  void Job::WaitForHelpers() {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。
    while (helper_counter_ > 0) {
      cond_.wait(lock);
    }
  }

  // 数を数える。探索した手の数を数えるときに使う。
  int Job::Count() {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。
    counter_++;
    return counter_;
  }

  /**********************/
  /* プライベート関数。 */
  /**********************/
  // メンバーをコピーする。
  void Job::ScanMember(const Job& job) {
    mutex_ptr_ = job.mutex_ptr_;
    client_ptr_ = job.client_ptr_;
    record_ptr_ = job.record_ptr_;
    node_type_ = job.node_type_;
    pos_hash_ = job.pos_hash_;
    depth_ = job.depth_;
    level_ = job.level_;
    alpha_ptr_ = job.alpha_ptr_;
    beta_ptr_ = job.beta_ptr_;
    delta_ptr_ = job.delta_ptr_;
    table_ptr_ = job.table_ptr_;
    pv_line_ptr_ = job.pv_line_ptr_;
    is_null_searching_ = job.is_null_searching_;
    null_reduction_ = job.null_reduction_;
    score_type_ptr_ = job.score_type_ptr_;
    material_ = job.material_;
    is_checked_ = job.is_checked_;
    num_all_moves_ = job.num_all_moves_;
    has_legal_move_ptr_ = job.has_legal_move_ptr_;
    moves_to_search_ptr_ = job.moves_to_search_ptr_;
    next_print_info_time_ptr_ = job.next_print_info_time_ptr_;
    maker_ptr_ = job.maker_ptr_;
    helper_counter_ = job.helper_counter_;
    counter_ = job.counter_;
  }
}  // namespace Sayuri
