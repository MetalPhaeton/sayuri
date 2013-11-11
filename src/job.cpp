/*
   job.cpp: マルチスレッド探索用の仕事クラスの実装。

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

#include "job.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include "chess_def.h"
#include "chess_engine.h"
#include "transposition_table.h"
#include "pv_line.h"
#include "move_maker.h"

namespace Sayuri {
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  Job::Job(MoveMaker& maker, ChessEngine& client, NodeType node_type,
  Hash pos_hash, int depth, int level, int& alpha, int& beta, int& delta,
  TranspositionTable& table, PVLine& pv_line, bool is_reduced_by_null,
  int& num_searched_moves, bool& is_searching_pv,
  ScoreType& score_type, int material, bool is_checked, bool& has_legal_move,
  std::vector<Move>* moves_to_search_ptr,
  std::vector<Move>* root_move_vec_ptr) :
  client_ptr_(&client),
  node_type_(node_type),
  pos_hash_(pos_hash),
  depth_(depth),
  level_(level),
  alpha_ptr_(&alpha),
  beta_ptr_(&beta),
  delta_ptr_(&delta),
  table_ptr_(&table),
  pv_line_ptr_(&pv_line),
  is_reduced_by_null_(is_reduced_by_null),
  num_searched_moves_ptr_(&num_searched_moves),
  is_searching_pv_ptr_(&is_searching_pv),
  score_type_ptr_(&score_type),
  material_(material),
  is_checked_(is_checked),
  has_legal_move_ptr_(&has_legal_move),
  moves_to_search_ptr_(moves_to_search_ptr),
  root_move_vec_ptr_(root_move_vec_ptr),
  maker_ptr_(&maker),
  helper_counter_(0) {}

  // コピーコンストラクタ。
  Job::Job(const Job& job) :
  client_ptr_(job.client_ptr_),
  node_type_(job.node_type_)
  pos_hash_(job.pos_hash_),
  depth_(job.depth_),
  level_(job.level_),
  alpha_ptr_(job.alpha_ptr_),
  beta_ptr_(job.beta_ptr_),
  delta_ptr_(job.delta_ptr_),
  table_ptr_(job.table_ptr_),
  pv_line_ptr_(job.pv_line_ptr_),
  is_reduced_by_null_(job.is_reduced_by_null_),
  num_searched_moves_ptr_(job.num_searched_moves_ptr_),
  is_searching_pv_ptr_(job.is_searching_pv_ptr_),
  score_type_ptr_(job.score_type_ptr_),
  material_(job.material_),
  is_checked_(job.is_checked_),
  has_legal_move_ptr_(job.has_legal_move_ptr_),
  moves_to_search_ptr_(job.moves_to_search_ptr_),
  root_move_vec_ptr_(job.root_move_vec_ptr_),
  maker_ptr_(job.maker_ptr_),
  helper_counter_(job.helper_counter_) {}

  // ムーブコンストラクタ。
  Job::Job(Job&& job) :
  client_ptr_(job.client_ptr_),
  node_type_(job.node_type_),
  pos_hash_(job.pos_hash_),
  depth_(job.depth_),
  level_(job.level_),
  alpha_ptr_(job.alpha_ptr_),
  beta_ptr_(job.beta_ptr_),
  delta_ptr_(job.delta_ptr_),
  table_ptr_(job.table_ptr_),
  pv_line_ptr_(job.pv_line_ptr_),
  is_reduced_by_null_(job.is_reduced_by_null_),
  num_searched_moves_ptr_(job.num_searched_moves_ptr_),
  is_searching_pv_ptr_(job.is_searching_pv_ptr_),
  score_type_ptr_(job.score_type_ptr_),
  material_(job.material_),
  is_checked_(job.is_checked_),
  has_legal_move_ptr_(job.has_legal_move_ptr_),
  moves_to_search_ptr_(job.moves_to_search_ptr_),
  root_move_vec_ptr_(job.root_move_vec_ptr_),
  maker_ptr_(job.maker_ptr_),
  helper_counter_(job.helper_counter_) {}

  // コピー代入。
  Job& Job::operator=(const Job& job) {
    client_ptr_ = job.client_ptr_;
    node_type_ = job.node_type_;
    pos_hash_ = job.pos_hash_;
    depth_ = job.depth_;
    level_ = job.level_;
    alpha_ptr_ = job.alpha_ptr_;
    beta_ptr_ = job.beta_ptr_;
    delta_ptr_ = job.delta_ptr_;
    table_ptr_ = job.table_ptr_;
    pv_line_ptr_ = job.pv_line_ptr_;
    is_reduced_by_null_ = job.is_reduced_by_null_;
    num_searched_moves_ptr_ = job.num_searched_moves_ptr_;
    is_searching_pv_ptr_ = job.is_searching_pv_ptr_;
    score_type_ptr_ = job.score_type_ptr_;
    material_ = job.material_;
    is_checked_ = job.is_checked_;
    has_legal_move_ptr_ = job.has_legal_move_ptr_;
    moves_to_search_ptr_ = job.moves_to_search_ptr_;
    root_move_vec_ptr_ = job.root_move_vec_ptr_;
    maker_ptr_ = job.maker_ptr_;
    helper_counter_ = job.helper_counter_;

    return *this;
  }

  // ムーブ代入。
  Job& Job::operator=(Job&& job) {
    client_ptr_ = job.client_ptr_;
    node_type_ = job.node_type_;
    pos_hash_ = job.pos_hash_;
    depth_ = job.depth_;
    level_ = job.level_;
    alpha_ptr_ = job.alpha_ptr_;
    beta_ptr_ = job.beta_ptr_;
    delta_ptr_ = job.delta_ptr_;
    table_ptr_ = job.table_ptr_;
    pv_line_ptr_ = job.pv_line_ptr_;
    is_reduced_by_null_ = job.is_reduced_by_null_;
    num_searched_moves_ptr_ = job.num_searched_moves_ptr_;
    is_searching_pv_ptr_ = job.is_searching_pv_ptr_;
    score_type_ptr_ = job.score_type_ptr_;
    material_ = job.material_;
    is_checked_ = job.is_checked_;
    has_legal_move_ptr_ = job.has_legal_move_ptr_;
    moves_to_search_ptr_ = job.moves_to_search_ptr_;
    root_move_vec_ptr_ = job.root_move_vec_ptr_;
    maker_ptr_ = job.maker_ptr_;
    helper_counter_ = job.helper_counter_;

    return *this;
  }

  /********************/
  /* パブリック関数。 */
  /********************/
  // 手を得る。
  Move Job::PickMove() {
    return maker_ptr_->PickMove();
  }

  // ヘルパーの数を増やす。
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
    cond_.wait(lock, [this] {return helper_counter_ <= 0;});
  }
}  // namespace Sayuri
