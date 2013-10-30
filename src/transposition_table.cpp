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
  entry_table_(new std::vector<TTEntry>[TABLE_SIZE]) {
    // 大きさを整える。
    if (max_bytes > GetMaxSize()) {
      max_bytes = GetMaxSize();
    } else if (max_bytes < GetMinSize()) {
      max_bytes = GetMinSize();
    }
    max_bytes_ = max_bytes;

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
  void TranspositionTable::Add(HashKey pos_key, int depth,
  int score, TTScoreType score_type, Move best_move) {
    // テーブルのインデックスを得る。
    int index = GetTableIndex(pos_key);

    // 最後のエントリーのdepthを比べ、追加する側が大きければ追加。
    if (depth > entry_table_[index].back().depth()) {
      entry_table_[index].back() = TTEntry(pos_key, depth, score,
      score_type, best_move);

      // ソート。
      std::sort(entry_table_[index].begin(), entry_table_[index].end(),
      &TTEntry::Compare);
    }
  }

  // 該当するTTEntryを返す。
  TTEntry* TranspositionTable::GetFulfiledEntry(HashKey pos_key,
  int depth) const {
    // テーブルのインデックスを得る。
    int index = GetTableIndex(pos_key);

    TTEntry* entry_ptr = nullptr;
    for (auto& entry : entry_table_[index]) {
      if (!(entry.exists())) return nullptr;  // エントリーがない。

      if (entry.Fulfil(pos_key, depth)) {
        entry_ptr = &entry;  // エントリーが見つかった。
        break;
      }

    }
    return entry_ptr;
  }

  //　テーブルのサイズのバイト数を返す。
  std::size_t TranspositionTable::GetSizeBytes() const {
    // TTEntryのサイズ。
    std::size_t bytes = sizeof(TTEntry) * entry_table_[0].size() * TABLE_SIZE;

    return bytes;
  }

  // エントリーのパーミル。
  int TranspositionTable::GetUsedPermill() const {
    // 全エントリーの数。
    int num_all = TABLE_SIZE * entry_table_[0].size();

    // 記録されているエントリーの数。
    int num_entries = 0;
    for (std::size_t i = 0; i < TABLE_SIZE; i++) {
      for (auto& entry : entry_table_[i]) {
        if (entry.exists()) {
          num_entries++;
        } else {
          break;
        }
      }
    }

    return (num_entries * 1000) / num_all;
  }

  // エントリーの種類ごとの比率データ。
  template<TTScoreType Type>
  int TranspositionTable::GetRatioPermill() const {
    int num_all = 0;
    int num_entry = 0;
    for (std::size_t i = 0; i < TABLE_SIZE; i++) {
      for (auto& entry : entry_table_[i]) {
        if (entry.exists()) {
          num_all += 1;
          if (entry.score_type() == Type) {
            num_entry += 1;
          }
        } else {
          break;
        }
      }
    }

    return (num_entry * 1000) / num_all;
  }
  // 実体化。
  template
  int TranspositionTable::GetRatioPermill<TTScoreType::EXACT>() const;
  template
  int TranspositionTable::GetRatioPermill<TTScoreType::ALPHA>() const;
  template
  int TranspositionTable::GetRatioPermill<TTScoreType::BETA>() const;

  /****************/
  /* static関数。 */
  /****************/
  // テーブルの最小サイズを得る。
  std::size_t TranspositionTable::GetMinSize() {
    return sizeof(TTEntry) * TABLE_SIZE;
  }

  // テーブルの最大サイズを得る。
  std::size_t TranspositionTable::GetMaxSize() {
    return sizeof(TTEntry) * TABLE_SIZE * 100;
  }

  /************************/
  /* エントリーのクラス。 */
  /************************/
  // コンストラクタ。
  TTEntry::TTEntry(HashKey key, int depth, int score,
  TTScoreType score_type, Move best_move) :
  exists_(true),
  key_(key),
  depth_(depth),
  score_(score),
  score_type_(score_type),
  best_move_(best_move) {
  }

  // デフォルトコンストラクタ。
  TTEntry::TTEntry() :
  exists_(false),
  key_(0ULL),
  depth_(-MAX_VALUE),
  score_(0),
  score_type_(TTScoreType::ALPHA) {
  }

  // コピーコンストラクタ。
  TTEntry::TTEntry(const TTEntry& entry) :
  exists_(entry.exists_),
  key_(entry.key_),
  depth_(entry.depth_),
  score_(entry.score_),
  score_type_(entry.score_type_),
  best_move_(entry.best_move_) {
  }

  // ムーブコンストラクタ。
  TTEntry::TTEntry(TTEntry&& entry) :
  exists_(entry.exists_),
  key_(entry.key_),
  depth_(entry.depth_),
  score_(entry.score_),
  score_type_(entry.score_type_),
  best_move_(entry.best_move_) {
  }

  // コピー代入。
  TTEntry& TTEntry::operator=(const TTEntry& entry) {
    exists_ = entry.exists_;
    key_ = entry.key_;
    depth_ = entry.depth_;
    score_ = entry.score_;
    score_type_ = entry.score_type_;
    best_move_ = entry.best_move_;

    return *this;
  }

  // ムーブ代入。
  TTEntry& TTEntry::operator=(TTEntry&& entry) {
    exists_ = entry.exists_;
    key_ = entry.key_;
    depth_ = entry.depth_;
    score_ = entry.score_;
    score_type_ = entry.score_type_;
    best_move_ = entry.best_move_;

    return *this;
  }

  // 該当するならtrue。
  bool TTEntry::Fulfil(HashKey key, int depth) const {
    if (!exists_) return false;
    if (depth > depth_) return false;
    if (key != key_) return false;
    return true;
  }

  // ソート用比較関数。
  bool TTEntry::Compare(const TTEntry& first, const TTEntry& second) {
    return first.depth_ > second.depth_;
  }
}  // namespace Sayuri
