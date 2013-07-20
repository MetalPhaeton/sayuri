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
#include <algorithm>
#include <cstddef>
#include "chess_def.h"
#include "chess_util.h"

namespace Sayuri {
  /****************************************/
  /* トランスポジションテーブルのクラス。 */
  /****************************************/
  // static定数。
  constexpr HashKey TranspositionTable::TABLE_KEY_MASK;
  constexpr std::size_t TranspositionTable::TABLE_SIZE;

  // コンストラクタ。
  TranspositionTable::TranspositionTable(std::size_t max_bytes) :
  max_bytes_(max_bytes),
  entry_table_(new std::vector<TTEntry>[TABLE_SIZE]) {
    // 大きさを整える。
    if (max_bytes > TT_MAX_SIZE_BYTES) {
      max_bytes = TT_MAX_SIZE_BYTES;
    } else if (max_bytes < TT_MIN_SIZE_BYTES) {
      max_bytes = TT_MIN_SIZE_BYTES;
    }

    // テーブル自体のサイズを引く。
    max_bytes -= sizeof(TranspositionTable)
    + (sizeof(std::vector<TTEntry>) * TABLE_SIZE);

    // エントリーをいくつ作るか計算する。
    std::size_t num_entries = max_bytes / sizeof(TTEntry);

    // 一つのテーブルにつきいくつのエントリーを用意するか決める。
    num_entries /= TABLE_SIZE;
    if (num_entries <= 0) {
      num_entries = 1;
    }

    // 配列をリサイズ。
    for (std::size_t i = 0; i < TABLE_SIZE; i++) {
      entry_table_[i].resize(num_entries);
    }
  }
  // コピーコンストラクタ。
  TranspositionTable::TranspositionTable(const TranspositionTable& table) :
  max_bytes_(table.max_bytes_),
  entry_table_(new std::vector<TTEntry>[TABLE_SIZE]) {
    // テーブルをコピー。
    for (std::size_t i = 0; i < TABLE_SIZE; i++) {
      entry_table_[i].resize(table.entry_table_[i].size());
      for (unsigned int j = 0; j < table.entry_table_[i].size(); j++) {
        entry_table_[i][j] = table.entry_table_[i][j];
      }
    }
  }
  // ムーブコンストラクタ。
  TranspositionTable::TranspositionTable( TranspositionTable&& table) :
  max_bytes_(table.max_bytes_){
    entry_table_ = std::move(table.entry_table_); 
  }

  // テーブルに追加する。
  void TranspositionTable::Add(HashKey pos_key, int depth, int level,
  Side to_move, int value, TTValueFlag value_flag, Move best_move) {
    // テーブルに追加。
    int index = GetTableIndex(pos_key);
    for (auto& entry : entry_table_[index]) {
      if (!(entry.exists())) {
        entry = TTEntry(pos_key, depth, level, to_move,
        value, value_flag, best_move);
        break;
      }
    }

    // ソート。
    std::sort(entry_table_[index].begin(), entry_table_[index].end(),
    &TTEntry::Compare);
  }
  // 該当するTTEntryを返す。
  const TTEntry* TranspositionTable::GetFulfiledEntry(HashKey pos_key,
  int depth, int level, Side to_move) const {
    // テーブルのインデックスを得る。
    int index = GetTableIndex(pos_key);

    TTEntry* entry_ptr = nullptr;
    for (auto& entry : entry_table_[index]) {
      if (!(entry.exists())) return nullptr;  // エントリーがない。

      if (entry.Fulfil(pos_key, depth, level, to_move)) {
        entry_ptr = &entry;  // エントリーが見つかった。
        break;
      }

    }
    return entry_ptr;
  }
  //　テーブルのサイズのバイト数を返す。
  std::size_t TranspositionTable::GetSizeBytes() const {
    // テーブル自身の大きさ。
    std::size_t bytes = sizeof(TranspositionTable);
    bytes += sizeof(std::vector<TTEntry>) * TABLE_SIZE;

    // TTEntryのサイズ。
    for (std::size_t i = 0; i < TABLE_SIZE; i++) {
      bytes += sizeof(TTEntry) * entry_table_[i].size();
    }

    return bytes;
  }
  double TranspositionTable::GetSizePermill() const {
    double max = static_cast<double>(max_bytes_);
    double size = static_cast<double>(GetSizeBytes());
    return (size / max) * 1000;
  }

  /************************/
  /* エントリーのクラス。 */
  /************************/
  // コンストラクタ。
  TTEntry::TTEntry(HashKey key, int depth, int level, Side to_move,
  int value, TTValueFlag value_flag, Move best_move) :
  exists_(true),
  key_(key),
  depth_(depth),
  level_(level),
  to_move_(to_move),
  value_(value),
  value_flag_(value_flag),
  best_move_(best_move) {
  }
  // デフォルトコンストラクタ。
  TTEntry::TTEntry() :
  exists_(false),
  key_(0ULL),
  depth_(-MAX_VALUE),
  level_(MAX_VALUE),
  to_move_(NO_SIDE),
  value_(0),
  value_flag_(TTValueFlag::ALPHA) {
  }
  // コピーコンストラクタ。
  TTEntry::TTEntry(const TTEntry& entry) :
  exists_(entry.exists_),
  key_(entry.key_),
  depth_(entry.depth_),
  level_(entry.level_),
  to_move_(entry.to_move_),
  value_(entry.value_),
  value_flag_(entry.value_flag_),
  best_move_(entry.best_move_) {
  }
  // ムーブコンストラクタ。
  TTEntry::TTEntry(TTEntry&& entry) :
  exists_(entry.exists_),
  key_(entry.key_),
  depth_(entry.depth_),
  level_(entry.level_),
  to_move_(entry.to_move_),
  value_(entry.value_),
  value_flag_(entry.value_flag_),
  best_move_(entry.best_move_) {
  }
  // コピー代入。
  TTEntry& TTEntry::operator=(const TTEntry& entry) {
    exists_ = entry.exists_;
    key_ = entry.key_;
    depth_ = entry.depth_;
    level_ = entry.level_;
    to_move_ = entry.to_move_;
    value_ = entry.value_;
    value_flag_ = entry.value_flag_;
    best_move_ = entry.best_move_;

    return *this;
  }
  // ムーブ代入。
  TTEntry& TTEntry::operator=(TTEntry&& entry) {
    exists_ = entry.exists_;
    key_ = entry.key_;
    depth_ = entry.depth_;
    level_ = entry.level_;
    to_move_ = entry.to_move_;
    value_ = entry.value_;
    value_flag_ = entry.value_flag_;
    best_move_ = entry.best_move_;

    return *this;
  }
  // ソート用比較関数。
  bool TTEntry::Compare(const TTEntry& first, const TTEntry& second) {
    return first.level_ < second.level_;
  }
  // 該当するならtrue。
  bool TTEntry::Fulfil(HashKey key, int depth, int level, Side to_move) const {
    if (!exists_) return false;
    if (depth > depth_) return false;
    if (level < level_) return false;
    if (to_move != to_move_) return false;
    if (key != key_) return false;
    return true;
  }
}  // namespace Sayuri
