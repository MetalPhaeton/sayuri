/* chess_board_pondering.cpp: Pondering関連。
   copyright (c) 2011 石橋宏之利
 */

#include "chess_board.h"

#include <iostream>
#include <boost/thread.hpp>
#include "chess_def.h"
#include "chess_util.h"
#include "transposition_table.h"

namespace Misaki {
  // Ponderingする。
  void ChessBoard::Ponder(int depth, TranspositionTable& table,
  const EvalWeights& weights) {
    // スレッドをロック。
    boost::mutex::scoped_lock lock(sync_);

    // 現在の時間と探索時間を得る。
    start_time_ = std::time(NULL);
    searching_time_ = static_cast<double>(INFINITY);

    // 現在のハッシュキー。
    hash_key_t key = history_[current_game_]->key_;

    // 候補手を展開する。
    const int level = 0;
    move_t* buffer_ptr = &pondering_buffer_[0];
    GenCheckEscapeMove(level);
    Node* ptr = stack_ptr_[level] - 1;
    // SEEで簡易評価値を付ける。
    for (; ptr >= tree_ptr_[level]; ptr--) {
      ptr->quick_score_ = SEE(ptr->move_);
    }
    // バッファにコピーする。
    // 簡易評価の良いものから順に。
    while (stack_ptr_[level] != tree_ptr_[level]) {
      *(buffer_ptr) = PopBestMove(level);
      buffer_ptr++;
    }
    ClearMoves(level);

    // バッファの最後のポインタ。
    move_t* end_ptr = buffer_ptr;

    // Ponderingする。
    move_t candidate_move;
    int alpha = -INFINITE;
    int beta = INFINITE;
    hash_key_t next_key;
    for (int i_depth = 1; i_depth <= depth; i_depth++) {
      // ポインタを初めにセット。
      buffer_ptr = &pondering_buffer_[0];
      while (buffer_ptr != end_ptr) {
        // 候補手を取り出す。
        candidate_move = *(buffer_ptr);
        buffer_ptr++;

        // 次の局面のハッシュキーを得る。
        next_key = GetNextKey(key, candidate_move);

        // 探索する。
        MakeMove(candidate_move);
        Search(0, i_depth, alpha, beta, false, next_key, table, weights);
        UnmakeMove(candidate_move);
      }
    }
  }
  // Ponderingを開始する。
  void ChessBoard::StartPondering(int depth, TranspositionTable& table,
  const EvalWeights& weights) const {
    // ストップされていなければ何もしない。
    if (!stop_pondering_flag_) return;

    // Ponderingを開始する。
    ChessBoard* self = const_cast<ChessBoard*>(this);
    self->stop_pondering_flag_ = false;
    self->pondering_thread_ptr_ = new boost::thread(&ChessBoard::Ponder,
    boost::ref(*self), depth, boost::ref(table), boost::ref(weights));
  }
  // Ponderingを止める。
  void ChessBoard::StopPondering() const {
    ChessBoard* self = const_cast<ChessBoard*>(this);
    self->stop_pondering_flag_ = true;
    self->searching_time_ = 0;
    if (self->pondering_thread_ptr_) {
      self->pondering_thread_ptr_->join();
      delete self->pondering_thread_ptr_;
      self->pondering_thread_ptr_ = NULL;
    }
  }
}  // Misaki
