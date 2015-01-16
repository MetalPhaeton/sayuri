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
 * @file transposition_table.cpp
 * @author Hironori Ishibashi
 * @brief トランスポジションテーブルの実装。
 */

#include "transposition_table.h"

#include <iostream>
#include <utility>
#include <mutex>
#include <cstddef>
#include <cstdint>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ========================== //
  // トランスポジションテーブル //
  // ========================== //
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  TranspositionTable::TranspositionTable(std::size_t table_size) :
  null_entry_(),
  num_entries_(1),
  num_used_entries_(0),
  entry_table_(nullptr),
  index_mask_(0),
  age_(0x00010000UL) {
    // エントリーをいくつ作るか決める。
    std::uint64_t temp = table_size / sizeof(TTEntry);
    temp = temp < 1 ? 1 : temp;

    // 以下は最上位ビットだけを残し、他のビットをゼロにするアルゴリズム。
    temp = (temp & 0xffffffff00000000ULL)
    ? (temp & 0xffffffff00000000ULL) : temp;

    temp = (temp & 0xffff0000ffff0000ULL)
    ? (temp & 0xffff0000ffff0000ULL) : temp;

    temp = (temp & 0xff00ff00ff00ff00ULL)
    ? (temp & 0xff00ff00ff00ff00ULL) : temp;

    temp = (temp & 0xf0f0f0f0f0f0f0f0ULL)
    ? (temp & 0xf0f0f0f0f0f0f0f0ULL) : temp;

    temp = (temp & 0xccccccccccccccccULL)
    ? (temp & 0xccccccccccccccccULL) : temp;

    temp = (temp & 0xaaaaaaaaaaaaaaaaULL)
    ? (temp & 0xaaaaaaaaaaaaaaaaULL) : temp;

    num_entries_ = temp;
    index_mask_ = temp - 1;

    // テーブルを作成。
    entry_table_.reset(new TTEntry[num_entries_]);
  }

  // コピーコンストラクタ。
  TranspositionTable::TranspositionTable(const TranspositionTable& table) :
  null_entry_(),
  num_entries_(table.num_entries_),
  num_used_entries_(table.num_used_entries_),
  entry_table_(new TTEntry[table.num_entries_]),
  index_mask_(table.index_mask_),
  age_(table.age_) {}

  // ムーブコンストラクタ。
  TranspositionTable::TranspositionTable( TranspositionTable&& table) :
  null_entry_(),
  num_entries_(table.num_entries_),
  num_used_entries_(table.num_used_entries_),
  entry_table_(std::move(table.entry_table_)),
  index_mask_(table.index_mask_),
  age_(table.age_) {}

  // コピー代入演算子。
  TranspositionTable&
  TranspositionTable::operator=(const TranspositionTable& table) {
    num_entries_ = table.num_entries_;
    num_used_entries_ = table.num_used_entries_;
    entry_table_.reset(new TTEntry[num_entries_]);
    for (std::size_t i = 0; i < num_entries_; ++i) {
      entry_table_[i] = table.entry_table_[i];
    }
    index_mask_ = table.index_mask_;
    age_ = table.age_;

    return *this;
  }

  // ムーブ代入演算子。
  TranspositionTable&
  TranspositionTable::operator=(TranspositionTable&& table) {
    num_entries_ = table.num_entries_;
    num_used_entries_ = table.num_used_entries_;
    entry_table_ = std::move(table.entry_table_);
    index_mask_ = table.index_mask_;
    age_ = table.age_;

    return *this;
  }

  // テーブルにエントリーを追加する。
  void TranspositionTable::Add(Hash pos_hash, int depth, int score,
  ScoreType score_type, Move best_move) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    // テーブルのインデックスを得る。
    std::size_t index = pos_hash & index_mask_;

    // depthをプラスにしておく。
    depth = depth < 0 ? 0 : depth;

    // 空いているエントリーへの登録なら使用済みエントリー数をカウント。
    if (entry_table_[index].depth() == 0) {
      ++num_used_entries_;
    }

    // テーブルが若い時にに登録されているものなら上書き。
    // depthがすでに登録されているエントリー以上なら登録。
    std::uint32_t age_depth = age_ | depth;
    if (entry_table_[index].age_depth_ <= age_depth) {
      entry_table_[index].pos_hash_ = pos_hash;
      entry_table_[index].age_depth_ = age_depth;
      entry_table_[index].score_ = score;
      entry_table_[index].score_type_ = score_type;
      entry_table_[index].best_move_ = best_move;
    }
  }

  // 条件を満たすエントリーを返す。
  const TTEntry& TranspositionTable::GetEntry(Hash pos_hash, int depth) const {
    // depthをunsigned intにしておく。
    std::uint32_t depth_2 = depth < 0 ? 0 : depth;

    // エントリーを得る。
    std::size_t index = pos_hash & index_mask_;

    if (((entry_table_[index].age_depth_ & TTEntry::DEPTH_MASK) >= depth_2)
    && (entry_table_[index].pos_hash_ == pos_hash)) {
      return entry_table_[index];
    }

    // 条件外なので、無効なエントリーを返す。
    return null_entry_;
  }

  // ================== //
  // エントリーのクラス //
  // ================== //
  // --- static定数 --- //
  constexpr std::uint32_t TTEntry::DEPTH_MASK;
  constexpr std::uint32_t TTEntry::AGE_MASK;
  constexpr int TTEntry::DEPTH_SHIFT;
  constexpr int TTEntry::AGE_SHIFT;

  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  TTEntry::TTEntry(Hash pos_hash, int depth, int score,
  ScoreType score_type, Move best_move, std::uint32_t table_age) :
  pos_hash_(pos_hash),
  age_depth_(table_age | depth),
  score_(score),
  score_type_(score_type),
  best_move_(best_move) {}

  // コンストラクタ。
  TTEntry::TTEntry() :
  pos_hash_(0),
  age_depth_(0),
  score_(0),
  score_type_(ScoreType::ALPHA),
  best_move_(0) {}

  // コピーコンストラクタ。
  TTEntry::TTEntry(const TTEntry& entry) :
  pos_hash_(entry.pos_hash_),
  age_depth_(entry.age_depth_),
  score_(entry.score_),
  score_type_(entry.score_type_),
  best_move_(entry.best_move_) {}

  // ムーブコンストラクタ。
  TTEntry::TTEntry(TTEntry&& entry) :
  pos_hash_(entry.pos_hash_),
  age_depth_(entry.age_depth_),
  score_(entry.score_),
  score_type_(entry.score_type_),
  best_move_(entry.best_move_) {}

  // コピー代入演算子。
  TTEntry& TTEntry::operator=(const TTEntry& entry) {
    pos_hash_ = entry.pos_hash_;
    age_depth_ = entry.age_depth_;
    score_ = entry.score_;
    score_type_ = entry.score_type_;
    best_move_ = entry.best_move_;

    return *this;
  }

  // ムーブ代入演算子。
  TTEntry& TTEntry::operator=(TTEntry&& entry) {
    pos_hash_ = entry.pos_hash_;
    age_depth_ = entry.age_depth_;
    score_ = entry.score_;
    score_type_ = entry.score_type_;
    best_move_ = entry.best_move_;

    return *this;
  }
}  // namespace Sayuri
