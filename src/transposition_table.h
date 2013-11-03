/*
   transposition_table.h: トランスポジションテーブル。

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

#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <iostream>
#include <vector>
#include <memory>
#include <cstddef>
#include "chess_def.h"
#include "chess_util.h"
#include "error.h"

namespace Sayuri {
  class TranspositionTable;

  /********************************************/
  /* トランスポジションテーブルのエントリー。 */
  /********************************************/
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
      TTEntry(Hash pos_hash, int depth,
      int value, ScoreType score_type, Move best_move);
      TTEntry();
      TTEntry(const TTEntry& entry);
      TTEntry(TTEntry&& entry);
      TTEntry& operator=(const TTEntry& entry);
      TTEntry& operator=(TTEntry&& entry);
      virtual ~TTEntry() {}

      /**********/
      /* 関数。 */
      /**********/
      // ハッシュとレベルと深さと手番から
      // 同じポジションかどうか判定する。
      // [引数]
      // hash: ハッシュ。
      // depth: 探索の深さ。
      // [戻り値]
      // 同じならtrue。
      bool Fulfil(Hash hash, int depth) const;
      // エントリーをアップデートする。
      // [引数]
      // score: 評価値。
      // score_type: 評価値の種類。
      // best_move: 最善手。
      void Update(int score, ScoreType score_type, Move best_move) {
        score_ = score;
        score_type_ = score_type;
        best_move_ = best_move;
      }

      /**************/
      /* アクセサ。 */
      /**************/
      // ハッシュ。
      Hash hash() const {return hash_;}
      // 深さ。
      int depth() const {return depth_;}
      // 評価値。
      int score() const {return score_;}
      // 評価値の種類。
      ScoreType score_type() const {return score_type_;}
      // 最善手。
      Move best_move() const {return best_move_;}

      /**********************/
      /* ソート用比較関数。 */
      /**********************/
      static bool Compare(const TTEntry& first, const TTEntry& second);

    private:
      /****************/
      /* メンバ変数。 */
      /****************/
      // ハッシュ。
      Hash hash_;
      // 探索のレベル。
      int depth_;
      // 評価値。
      int score_;
      // 評価値の種類。
      ScoreType score_type_;
      // 最善手。
      Move best_move_;
  };

  /****************************************/
  /* トランスポジションテーブルのクラス。 */
  /****************************************/
  class TranspositionTable {
    private:
      /**********/
      /* 定数。 */
      /**********/
      // ハッシュのテーブル用マスク。
      static constexpr Hash TABLE_HASH_MASK = 0XffffULL;
      // テーブルの大きさ。
      static constexpr std::size_t TABLE_SIZE = TABLE_HASH_MASK + 1;

    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // [引数]
      // max_bytes: トランスポジションテーブルのサイズ指定。
      TranspositionTable(std::size_t max_bytes);
      TranspositionTable(const TranspositionTable& table);
      TranspositionTable(TranspositionTable&& table);
      TranspositionTable& operator=(const TranspositionTable& table);
      TranspositionTable& operator=(TranspositionTable&& table);
      virtual ~TranspositionTable() {}
      TranspositionTable() = delete;

      /**********/
      /* 関数。 */
      /**********/
      // テーブルに追加する。
      // [引数]
      // pos_hash: ハッシュ。
      // depth: 探索の深さ。
      // value: 評価値。
      // score_type: 評価値の種類。
      // best_move: 最善手。
      void Add(Hash pos_hash, int depth,
      int value, ScoreType score_type, Move best_move);
      // 条件を満たすエントリーを得る。
      // [引数]
      // pos_hash: ハッシュ。
      // depth: 探索の深さ。
      // [戻り値]
      // 条件を満たすエントリー。
      // なければnullptr。
      TTEntry* GetEntry(Hash pos_hash, int depth) const;

      // 大きさが何バイトか返す。
      // [戻り値]
      // サイズをバイト数で返す。
      std::size_t GetSizeBytes() const {
        return num_all_entries_ * sizeof(TTEntry);
      }


      // 使用されているエントリーのサイズを全体の何パーミルかを返す。
      // [戻り値]
      // エントリーのパーミル。
      int GetUsedPermill() const {
        return static_cast<int>
        ((num_used_entries_ * 1000) / num_all_entries_);
      }


      /****************/
      /* static関数。 */
      /****************/
      // テーブルの最小サイズを得る。
      // [戻り値]
      // 最小サイズ。
      static std::size_t GetMinSize() {
        return sizeof(TTEntry) * TABLE_SIZE;
      }

      // テーブルの最大サイズを得る。
      // [戻り値]
      // 最大サイズ。
      static std::size_t GetMaxSize() {
        // 最大1ギガ。
        return 1024 * 1024 * 1024;
      }

    private:
      /**********************/
      /* プライベート関数。 */
      /**********************/
      // テーブルのインデックスを得る。
      // [引数]
      // pos_hash: ポジションのハッシュ。
      int GetTableIndex(Hash pos_hash) const {
        return pos_hash & (TABLE_HASH_MASK);
      }

      /****************/
      /* メンバ変数。 */
      /****************/
      // サイズの上限。
      std::size_t max_bytes_;
      // 全てのエントリーの個数。
      std::size_t num_all_entries_;
      // 使用済みのエントリーの個数。
      std::size_t num_used_entries_;
      // エントリーを登録するテーブル。
      std::unique_ptr<std::vector<TTEntry>[]> entry_table_;
  };
}  // namespace Sayuri

#endif
