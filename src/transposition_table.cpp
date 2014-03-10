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
  num_entries_(0),
  num_used_entries_(0),
  entry_table_(nullptr),
  age_(0) {
    // エントリーをいくつ作るか計算する。
    num_entries_ = table_size / sizeof(TTEntry);
    num_entries_ = num_entries_ >= 1 ? num_entries_ : 1;

    // テーブルを作成。
    entry_table_.reset(new TTEntry[num_entries_]);
  }

  // コピーコンストラクタ。
  TranspositionTable::TranspositionTable(const TranspositionTable& table) :
  num_entries_(table.num_entries_),
  num_used_entries_(table.num_used_entries_),
  entry_table_(new TTEntry[table.num_entries_]),
  age_(table.age_) {}

  // ムーブコンストラクタ。
  TranspositionTable::TranspositionTable( TranspositionTable&& table) :
  num_entries_(table.num_entries_),
  num_used_entries_(table.num_used_entries_),
  entry_table_(std::move(table.entry_table_)),
  age_(table.age_) {}

  // コピー代入。
  TranspositionTable&
  TranspositionTable::operator=(const TranspositionTable& table) {
    num_entries_ = table.num_entries_;
    num_used_entries_ = table.num_used_entries_;
    entry_table_.reset(new TTEntry[num_entries_]);
    for (int i = 0; i < num_entries_; i++) {
      entry_table_[i] = table.entry_table_[i];
    }
    age_ = table.age_;

    return *this;
  }

  // ムーブ代入。
  TranspositionTable&
  TranspositionTable::operator=(TranspositionTable&& table) {
    num_entries_ = table.num_entries_;
    num_used_entries_ = table.num_used_entries_;
    entry_table_ = std::move(table.entry_table_);
    age_ = table.age_;

    return *this;
  }

  // テーブルに追加する。
  void TranspositionTable::Add(Hash pos_hash, int depth, int score,
  ScoreType score_type, Move best_move, int ply_mate) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    // テーブルのインデックスを得る。
    std::size_t index = GetTableIndex(pos_hash);

    // 空いているエントリーへの登録なら使用済みエントリー数をカウント。
    if (entry_table_[index].depth() <= -MAX_VALUE) {
      num_used_entries_++;
    }

    // テーブルが若い時にに登録されているものなら上書き。
    // depthがすでに登録されているエントリー以上なら登録。
    if ((entry_table_[index].table_age() < age_)
    || (depth >= entry_table_[index].depth())) {
      entry_table_[index] =
      TTEntry(pos_hash, depth, score, score_type, best_move, ply_mate, age_);
    }
  }

  // 該当するTTEntryを返す。
  TTEntry* TranspositionTable::GetEntry(Hash pos_hash,
  int depth) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    // エントリーを得る。
    TTEntry* entry_ptr = nullptr;
    std::size_t index = GetTableIndex(pos_hash);
    if ((entry_table_[index].depth() >= depth)
    && (entry_table_[index].pos_hash() == pos_hash)) {
      entry_ptr = &(entry_table_[index]);
    }

    return entry_ptr;
  }

  /************************/
  /* エントリーのクラス。 */
  /************************/
  // コンストラクタ。
  TTEntry::TTEntry(Hash pos_hash, int depth, int score, ScoreType score_type,
  Move best_move, int ply_mate, int table_age) :
  pos_hash_(pos_hash),
  depth_(depth),
  score_(score),
  score_type_(score_type),
  best_move_(best_move),
  ply_mate_(ply_mate),
  table_age_(table_age) {}

  // デフォルトコンストラクタ。
  TTEntry::TTEntry() :
  pos_hash_(0ULL),
  depth_(-MAX_VALUE),
  score_(0),
  score_type_(ScoreType::ALPHA),
  best_move_(0U),
  ply_mate_(-1),
  table_age_(-1) {}

  // コピーコンストラクタ。
  TTEntry::TTEntry(const TTEntry& entry) :
  pos_hash_(entry.pos_hash_),
  depth_(entry.depth_),
  score_(entry.score_),
  score_type_(entry.score_type_),
  best_move_(entry.best_move_),
  ply_mate_(entry.ply_mate_),
  table_age_(entry.table_age_) {}

  // ムーブコンストラクタ。
  TTEntry::TTEntry(TTEntry&& entry) :
  pos_hash_(entry.pos_hash_),
  depth_(entry.depth_),
  score_(entry.score_),
  score_type_(entry.score_type_),
  best_move_(entry.best_move_),
  ply_mate_(entry.ply_mate_),
  table_age_(entry.table_age_) {}

  // コピー代入。
  TTEntry& TTEntry::operator=(const TTEntry& entry) {
    pos_hash_ = entry.pos_hash_;
    depth_ = entry.depth_;
    score_ = entry.score_;
    score_type_ = entry.score_type_;
    best_move_ = entry.best_move_;
    ply_mate_ = entry.ply_mate_;
    table_age_ = entry.table_age_;

    return *this;
  }

  // ムーブ代入。
  TTEntry& TTEntry::operator=(TTEntry&& entry) {
    pos_hash_ = entry.pos_hash_;
    depth_ = entry.depth_;
    score_ = entry.score_;
    score_type_ = entry.score_type_;
    best_move_ = entry.best_move_;
    ply_mate_ = entry.ply_mate_;
    table_age_ = entry.table_age_;

    return *this;
  }
}  // namespace Sayuri
