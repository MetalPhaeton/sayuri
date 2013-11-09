/*
   transposition_table.cpp: トランスポジションテーブル。

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
  // static定数。
  constexpr Hash TranspositionTable::TABLE_HASH_MASK;
  constexpr std::size_t TranspositionTable::TABLE_SIZE;

  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  TranspositionTable::TranspositionTable(std::size_t max_bytes) :
  num_used_entries_(0),
  entry_table_ptr_(new std::array<std::vector<TTEntry>, TABLE_SIZE>()) {
    // 大きさを整える。
    if (max_bytes > GetMaxSize()) {
      max_bytes = GetMaxSize();
    } else if (max_bytes < GetMinSize()) {
      max_bytes = GetMinSize();
    }
    max_bytes_ = max_bytes;

    // エントリーをいくつ作るか計算する。
    std::size_t num_entries = max_bytes / sizeof(TTEntry);

    // 一つのインデックスにつきいくつのエントリーを用意するか決める。
    num_entries /= TABLE_SIZE;
    if (num_entries <= 0) {
      num_entries = 1;
    }

    // 配列をリサイズ。
    for (std::size_t i = 0; i < TABLE_SIZE; i++) {
      (*entry_table_ptr_)[i].resize(num_entries);
    }

    // 全てのエントリーの個数をセット。
    num_all_entries_ = TABLE_SIZE * num_entries;
  }

  // コピーコンストラクタ。
  TranspositionTable::TranspositionTable(const TranspositionTable& table) :
  max_bytes_(table.max_bytes_),
  num_all_entries_(table.num_all_entries_),
  num_used_entries_(table.num_used_entries_),
  entry_table_ptr_(new std::array<std::vector<TTEntry>, TABLE_SIZE>()) {
    // テーブルをコピー。
    for (std::size_t i = 0; i < TABLE_SIZE; i++) {
      (*entry_table_ptr_)[i] = (*(table.entry_table_ptr_))[i];
    }
  }

  // ムーブコンストラクタ。
  TranspositionTable::TranspositionTable( TranspositionTable&& table) :
  max_bytes_(table.max_bytes_),
  num_all_entries_(table.num_all_entries_),
  num_used_entries_(table.num_used_entries_) {
    entry_table_ptr_ = std::move(table.entry_table_ptr_); 
  }

  // コピー代入。
  TranspositionTable&
  TranspositionTable::operator=(const TranspositionTable& table) {
    num_all_entries_ = table.num_all_entries_;
    num_used_entries_ = table.num_used_entries_;
    max_bytes_ = table.max_bytes_;

    // テーブルをコピー。
    for (std::size_t i = 0; i < TABLE_SIZE; i++) {
      (*entry_table_ptr_)[i] = (*(table.entry_table_ptr_))[i];
    }

    return *this;
  }

  // ムーブ代入。
  TranspositionTable&
  TranspositionTable::operator=(TranspositionTable&& table) {
    num_all_entries_ = table.num_all_entries_;
    num_used_entries_ = table.num_used_entries_;
    max_bytes_ = table.max_bytes_;
    entry_table_ptr_ = std::move(table.entry_table_ptr_);

    return *this;
  }

  // テーブルに追加する。
  void TranspositionTable::Add(Hash pos_hash, int depth,
  int score, ScoreType score_type, Move best_move) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    // テーブルのインデックスを得る。
    int index = GetTableIndex(pos_hash);

    // 最後のエントリーのdepthを比べ、追加する側が大きければ追加。
    if (depth > (*entry_table_ptr_)[index].back().depth()) {
      // 使用済みエントリーの数を増加。
      if ((*entry_table_ptr_)[index].back().depth() <= -MAX_VALUE) {
        num_used_entries_++;
      }
      // エントリーを追加。
      (*entry_table_ptr_)[index].back() = TTEntry(pos_hash, depth, score,
      score_type, best_move);

      // ソート。
      std::sort((*entry_table_ptr_)[index].begin(),
      (*entry_table_ptr_)[index].end(), TTEntry::Compare);
    }
  }

  // 該当するTTEntryを返す。
  TTEntry* TranspositionTable::GetEntry(Hash pos_hash,
  int depth) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    // テーブルのインデックスを得る。
    int index = GetTableIndex(pos_hash);

    TTEntry* entry_ptr = nullptr;
    for (auto& entry : (*entry_table_ptr_)[index]) {
      if (entry.depth() <= -MAX_VALUE) break;  // エントリーがない。

      if (entry.Fulfil(pos_hash, depth)) {
        entry_ptr = &entry;  // エントリーが見つかった。
        break;
      }

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

  // 該当するならtrue。
  bool TTEntry::Fulfil(Hash hash, int depth) const {
    if (depth > depth_) return false;
    if (hash != hash_) return false;
    return true;
  }

  // エントリーをアップデートする。
  void TTEntry::Update(int score, ScoreType score_type, Move best_move) {
    score_ = score;
    score_type_ = score_type;
    best_move_ = best_move;
  }

  // ソート用比較関数。
  bool TTEntry::Compare(const TTEntry& first, const TTEntry& second) {
    return first.depth_ > second.depth_;
  }
}  // namespace Sayuri
