/*
   transposition_table.h: トランスポジションテーブル。

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

#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <iostream>
#include <memory>
#include <mutex>
#include <cstddef>
#include "chess_def.h"
#include "chess_util.h"
#include "error.h"

namespace Sayuri {
  class TranspositionTable;

  // トランスポジションテーブルのエントリー。
  class TTEntry {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // [引数]
      // pos_hash: ポジションのハッシュ。
      // depth: 探索の深さ。
      // to_move: 手番。
      // value: 評価値。
      // score_type: 評価値の種類。
      // best_move: 最善手。
      // ply_mate: メイトまでのプライ。-1ならメイトなし。
      // table_age: トランスポジションテーブルの年齢。
      TTEntry(Hash pos_hash, int depth, int value, ScoreType score_type,
      Move best_move, int ply_mate, int table_age);
      TTEntry();
      TTEntry(const TTEntry& entry);
      TTEntry(TTEntry&& entry);
      TTEntry& operator=(const TTEntry& entry);
      TTEntry& operator=(TTEntry&& entry);
      virtual ~TTEntry() {}

      /**************/
      /* アクセサ。 */
      /**************/
      // ハッシュ。
      Hash pos_hash() const {return pos_hash_;}
      // 深さ。
      int depth() const {return depth_;}
      // 評価値。
      int score() const {return score_;}
      // 評価値の種類。
      ScoreType score_type() const {return score_type_;}
      // 最善手。
      Move best_move() const {return best_move_;}
      // メイトまでのプライ。-1ならメイトなし。
      int ply_mate() const {return ply_mate_;}
      // トランスポジションテーブルの年齢。
      int table_age() const {return table_age_;}

    private:
      /****************/
      /* メンバ変数。 */
      /****************/
      // ハッシュ。
      Hash pos_hash_;
      // 探索のレベル。
      int depth_;
      // 評価値。
      int score_;
      // 評価値の種類。
      ScoreType score_type_;
      // 最善手。
      Move best_move_;
      // メイトまでのプライ。-1ならメイトなし。
      int ply_mate_;
      // 記録時のトランスポジションテーブルの年齢。
      int table_age_;
  };

  // トランスポジションテーブルのクラス。
  class TranspositionTable {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // [引数]
      // table_size: トランスポジションテーブルのサイズ指定。
      TranspositionTable(std::size_t table_size);
      TranspositionTable(const TranspositionTable& table);
      TranspositionTable(TranspositionTable&& table);
      TranspositionTable& operator=(const TranspositionTable& table);
      TranspositionTable& operator=(TranspositionTable&& table);
      virtual ~TranspositionTable() {}
      TranspositionTable() = delete;

      /********************/
      /* パブリック関数。 */
      /********************/
      // テーブルに追加する。
      // [引数]
      // pos_hash: ハッシュ。
      // depth: 探索の深さ。
      // value: 評価値。
      // score_type: 評価値の種類。
      // best_move: 最善手。
      // ply_mate: メイトまでのプライ。-1ならメイトなし。
      void Add(Hash pos_hash, int depth, int value, ScoreType score_type,
      Move best_move, int ply_mate);

      // 条件を満たすエントリーを得る。
      // [引数]
      // pos_hash: ハッシュ。
      // depth: 探索の深さ。
      // [戻り値]
      // 条件を満たすエントリー。
      // なければnullptr。
      TTEntry* GetEntry(Hash pos_hash, int depth);

      // 年を取る。
      void GrowOld() {age_++;}

      // 大きさが何バイトか返す。
      // [戻り値]
      // サイズをバイト数で返す。
      std::size_t GetSizeBytes() const {
        return num_entries_ * sizeof(TTEntry);
      }


      // 使用されているエントリーのサイズを全体の何パーミルかを返す。
      // [戻り値]
      // エントリーのパーミル。
      int GetUsedPermill() const {
        return static_cast<int>
        ((num_used_entries_ * 1000) / num_entries_);
      }

      // トランスポジションテーブルのロックを使う。
      void Lock() {mutex_.lock();}
      void Unlock() {mutex_.unlock();}

      /**************/
      /* アクセサ。 */
      /**************/
      // 年齢。
      int age() const {return age_;}

    private:
      /**********************/
      /* プライベート関数。 */
      /**********************/
      // テーブルのインデックスを得る。
      // [引数]
      // pos_hash: ポジションのハッシュ。
      std::size_t GetTableIndex(Hash pos_hash) const {
        return pos_hash % num_entries_;
      }

      /****************/
      /* メンバ変数。 */
      /****************/
      // エントリーの個数。
      std::size_t num_entries_;
      // 使用済みのエントリーの個数。
      std::size_t num_used_entries_;
      // エントリーを登録するテーブル。
      std::unique_ptr<TTEntry[]> entry_table_;
      // 年齢。
      int age_;
      // ミューテックス。
      std::mutex mutex_;
  };
}  // namespace Sayuri

#endif
