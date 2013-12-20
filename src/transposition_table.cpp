/*
   transposition_table.cpp: トランスポジションテーブル。

   The MIT License (MIT)

   Copyright (c) 2013 Hironori Ishibashi

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

#include "transposition_table.h"

#include <iostream>
#include <utility>
#include <vector>
#include <array>
#include <algorithm>
#include <mutex>
#include <cstddef>
#include "chess_def.h"
#include "chess_util.h"

namespace Sayuri {
  /****************************************/
  /* トランスポジションテーブルのクラス。 */
  /****************************************/
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  TranspositionTable::TranspositionTable(std::size_t table_size) :
  num_used_entries_(0),
  entry_table_ptr_(new std::vector<TTEntry>(1)) {
    // エントリーをいくつ作るか計算する。
    std::size_t num_entries = table_size / sizeof(TTEntry);

    // テーブルを作成。
    if (num_entries > 1) {
      entry_table_ptr_->resize(num_entries);
    }
  }

  // コピーコンストラクタ。
  TranspositionTable::TranspositionTable(const TranspositionTable& table) :
  num_used_entries_(table.num_used_entries_),
  entry_table_ptr_(new std::vector<TTEntry>(*(table.entry_table_ptr_))) {}

  // ムーブコンストラクタ。
  TranspositionTable::TranspositionTable( TranspositionTable&& table) :
  num_used_entries_(table.num_used_entries_),
  entry_table_ptr_(std::move(table.entry_table_ptr_)){}

  // コピー代入。
  TranspositionTable&
  TranspositionTable::operator=(const TranspositionTable& table) {
    num_used_entries_ = table.num_used_entries_;
    *entry_table_ptr_ = *(table.entry_table_ptr_);

    return *this;
  }

  // ムーブ代入。
  TranspositionTable&
  TranspositionTable::operator=(TranspositionTable&& table) {
    num_used_entries_ = table.num_used_entries_;
    entry_table_ptr_ = std::move(table.entry_table_ptr_);

    return *this;
  }

  // テーブルに追加する。
  void TranspositionTable::Add(Hash pos_hash, int depth,
  int score, ScoreType score_type, Move best_move) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    // テーブルのインデックスを得る。
    std::size_t index = GetTableIndex(pos_hash);

    // 空いているエントリーへの登録なら使用済みエントリー数をカウント。
    if ((*entry_table_ptr_)[index].depth() <= -MAX_VALUE) {
      num_used_entries_++;
    }

    // depthがすでに登録されているエントリーより大きければ登録。
    if (depth > (*entry_table_ptr_)[index].depth()) {
      (*entry_table_ptr_)[index] =
      TTEntry(pos_hash, depth, score, score_type, best_move);
    }
  }

  // 該当するTTEntryを返す。
  TTEntry* TranspositionTable::GetEntry(Hash pos_hash,
  int depth) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    // エントリーを得る。
    TTEntry* entry_ptr = nullptr;
    std::size_t index = GetTableIndex(pos_hash);
    if ((depth <= (*entry_table_ptr_)[index].depth())
    && (pos_hash == (*entry_table_ptr_)[index].hash())) {
      entry_ptr = &((*entry_table_ptr_)[index]);
    }

    return entry_ptr;
  }

  /************************/
  /* エントリーのクラス。 */
  /************************/
  // コンストラクタ。
  TTEntry::TTEntry(Hash hash, int depth, int score,
  ScoreType score_type, Move best_move) :
  hash_(hash),
  depth_(depth),
  score_(score),
  score_type_(score_type),
  best_move_(best_move) {
  }

  // デフォルトコンストラクタ。
  TTEntry::TTEntry() :
  hash_(0ULL),
  depth_(-MAX_VALUE),
  score_(0),
  score_type_(ScoreType::ALPHA) {
  }

  // コピーコンストラクタ。
  TTEntry::TTEntry(const TTEntry& entry) :
  hash_(entry.hash_),
  depth_(entry.depth_),
  score_(entry.score_),
  score_type_(entry.score_type_),
  best_move_(entry.best_move_) {
  }

  // ムーブコンストラクタ。
  TTEntry::TTEntry(TTEntry&& entry) :
  hash_(entry.hash_),
  depth_(entry.depth_),
  score_(entry.score_),
  score_type_(entry.score_type_),
  best_move_(entry.best_move_) {
  }

  // コピー代入。
  TTEntry& TTEntry::operator=(const TTEntry& entry) {
    hash_ = entry.hash_;
    depth_ = entry.depth_;
    score_ = entry.score_;
    score_type_ = entry.score_type_;
    best_move_ = entry.best_move_;

    return *this;
  }

  // ムーブ代入。
  TTEntry& TTEntry::operator=(TTEntry&& entry) {
    hash_ = entry.hash_;
    depth_ = entry.depth_;
    score_ = entry.score_;
    score_type_ = entry.score_type_;
    best_move_ = entry.best_move_;

    return *this;
  }

  // エントリーをアップデートする。
  void TTEntry::Update(int score, ScoreType score_type, Move best_move) {
    score_ = score;
    score_type_ = score_type;
    best_move_ = best_move;
  }
}  // namespace Sayuri
