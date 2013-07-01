/* transposition_table.h: トランスポジションテーブル。
   Copyright (c) 2011 Ishibashi Hironori

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
#include <list>
#include <boost/thread.hpp>
#include "chess_def.h"
#include "chess_util.h"

namespace Sayuri {
  class TranspositionTable;
  class TranspositionTableSlotList;
  class TranspositionTableSlot;

  /**************************************************
   * トランスポジションテーブルのスロットのクラス。 *
   **************************************************/
  class TranspositionTableSlot {
    public:
      /****************************************
       * コンストラクタとデストラクタと代入。 *
       ****************************************/
      // コンストラクタ。
      // [引数]
      // key: ハッシュキー。
      // level: レベル。
      // depth: 深さ。
      // to_move: 手番。
      // upper_bound: 上限。
      // lower_bound: 下限。
      // best_move: 最善手。
      TranspositionTableSlot(hash_key_t key, int level, int depth,
      side_t to_move, int upper_bound, int lower_bound, move_t best_move);
      // コピーコンストラクタ。
      // [引数]
      // slot: コピーしたいスロット。
      TranspositionTableSlot(const TranspositionTableSlot& slot);
      // デストラクタ。
      virtual ~TranspositionTableSlot() {}
      // 代入。
      TranspositionTableSlot& operator=(const TranspositionTableSlot& slot);

      /************************
       * ソート用比較演算子。 *
       ************************/
      bool operator==(const TranspositionTableSlot& slot) const {
        return level_ == slot.level_;
      }
      bool operator!=(const TranspositionTableSlot& slot) const {
        return level_ != slot.level_;
      }
      bool operator<(const TranspositionTableSlot& slot) const {
        return level_ < slot.level_;
      }
      bool operator>(const TranspositionTableSlot& slot) const {
        return level_ > slot.level_;
      }

      /**********
       * 関数。 *
       **********/
      // ハッシュキーとレベルと深さと手番から
      // 同じポジションかどうか判定する。
      // [引数]
      // key: ハッシュキー。
      // level: レベル。
      // depth: 深さ。
      // to_move: 手番。
      // [戻り値]
      // 同じならtrue。
      bool Equals(hash_key_t key, int level, int depth,
      side_t to_move) const {
        if (level < level_) return false;
        if (depth > depth_) return false;
        if (to_move != to_move_) return false;
        if (key != key_) return false;
        return true;
      }

      /**************
       * アクセサ。 *
       **************/
      // 同期オブジェクト。
      boost::mutex& sync() {return sync_;}
      // ハッシュキー。
      hash_key_t key() const {return key_;}
      // レベル。
      int level() const {return level_;}
      // 深さ。
      int depth() const {return depth_;}
      // 手番。
      side_t to_move() const {return to_move_;}
      // 上限。
      int upper_bound() const {return upper_bound_;}
      // 下限。
      int lower_bound() const {return lower_bound_;}
      // 最善手。
      move_t best_move() const {return best_move_;}

      /******************
       * ミューテータ。 *
       ******************/
      // 上限。
      void upper_bound(int upper_bound) {upper_bound_ = upper_bound;}
      // 下限。
      void lower_bound(int lower_bound) {lower_bound_ = lower_bound;}
      // 最善手。
      void best_move(move_t best_move) {best_move_ = best_move;}

    private:
      /****************
       * メンバ変数。 *
       ****************/
      // 同期オブジェクト。
      boost::mutex sync_;
      // ハッシュキー。
      hash_key_t key_;
      // レベル。
      int level_;
      // 深さ。
      int depth_;
      // 手番。
      side_t to_move_;
      // 上限。
      int upper_bound_;
      // 下限。
      int lower_bound_;
      // 最善手。
      move_t best_move_;
  };

  /********************************************************
   * トランスポジションテーブルのスロットリストのクラス。 *
   ********************************************************/
  class TranspositionTableSlotList {
    public:
      /**********************************
       * コンストラクタとデストラクタ。 *
       **********************************/
      TranspositionTableSlotList() {}
      virtual ~TranspositionTableSlotList() {}

      /**********
       * 関数。 *
       **********/
      // リストのサイズを得る。
      // [戻り値]
      // リストのサイズ。
      int GetSize() const {
        return slot_list_.size();
      }
      // スロットを追加する。
      // [引数]
      // key: ハッシュキー。
      // level: レベル。
      // depth: 深さ。
      // to_move: 手番。
      // upper_bound: 上限。
      // lower_bound: 下限。
      // best_move: 最善手。
      void Add(hash_key_t key, int level, int depth, side_t to_move,
      int upper_bound, int lower_bound, move_t best_move) {
        slot_list_.push_back(TranspositionTableSlot(key, level, depth,
        to_move, upper_bound, lower_bound, best_move));
        slot_list_.sort();
      }
      // スロットを削除する。
      void DeleteOne() {
        slot_list_.pop_back();
      }
      // 最も高いレベルを得る。
      // [戻り値]
      // 最も高いレベル。
      int GetHighestLevel() const {
        if (!slot_list_.size()) return 0;
        return slot_list_.back().level();
      }
      // 同じポジションのスロットを得る。
      // [引数]
      // key: ハッシュキー。
      // level: レベル。
      // depth: 深さ。
      // to_move: 手番。
      // [戻り値]
      // 同じポジションのスロット。
      // [例外]
      // bool: 見つからなければfalse。
      TranspositionTableSlot& GetSameSlot(hash_key_t key, int level, int depth,
      side_t to_move) throw (bool);

    private:
      /******************
       * コピーの禁止。 *
       ******************/
      TranspositionTableSlotList(const TranspositionTableSlotList&);  // 削除。
      TranspositionTableSlotList&
      operator=(const TranspositionTableSlotList&);  // 削除。

      /****************
       * メンバ変数。 *
       ****************/
      // スロットのリスト。
      std::list<TranspositionTableSlot> slot_list_;
  };

  /****************************************
   * トランスポジションテーブルのクラス。 *
   ****************************************/
  class TranspositionTable {
    private:
      /****************
       * 定数の定義。 *
       ****************/
      enum {
        MAX_SIZE = 500000,
        NUM_LISTS = 0x3ffff + 1
      };

    public:
      /**************************************
       * インスタンスの生成とデストラクタ。 *
       **************************************/
      // インスタンスを生成する。
      // [戻り値]
      // トランスポジションテーブルのインスタンス。
      static TranspositionTable* New() {
        return new TranspositionTable();
      }
      // デストラクタ。
      virtual ~TranspositionTable() {}

      /**********
       * 関数。 *
       **********/
      // テーブルに追加する。
      // [引数]
      // key: ハッシュキー。
      // level: レベル。
      // depth: 深さ。
      // to_move: 手番。
      // upper_bound: 上限。
      // lower_bound: 下限。
      // best_move: 最善手。
      void Add(hash_key_t key, int level, int depth, side_t to_move,
      int upper_bound, int lower_bound, move_t best_move);
      // 同じ配置のスロットを得る。
      // [引数]
      // key: ハッシュキー。
      // level: レベル。
      // depth: 深さ。
      // to_move: 手番。
      // [例外]
      // bool: 見つからなければfalse。
      TranspositionTableSlot& GetSameSlot(hash_key_t key, int level, int depth,
      side_t to_move) throw (bool){
        // ロック。
        boost::mutex::scoped_lock lock(sync_);

        int index = GetTableIndex(key);
        return table_[index].GetSameSlot(key, level, depth, to_move);
      }
      // テーブルのサイズを得る。
      // [戻り値]
      // テーブルのサイズ。
      int GetSize() const {return num_slots_;}

    protected:
      /********************
       * コンストラクタ。 *
       ********************/
      TranspositionTable();

      /**********************************
       * アクセサ。派生クラスのみ公開。 *
       **********************************/
      // テーブル。
      const TranspositionTableSlotList (& table() const)[NUM_LISTS] {
        return table_;
      }

    private:
      /******************
       * コピーの禁止。 *
       ******************/
      TranspositionTable(const TranspositionTable&);  // 削除。
      TranspositionTable& operator=(const TranspositionTable&);  // 削除。

      /**********************
       * プライベート関数。 *
       **********************/
      int GetTableIndex(hash_key_t key) const {
        return key & (NUM_LISTS - 1);
      }

      /****************
       * メンバ変数。 *
       ****************/
      // 同期オブジェクト。
      boost::mutex sync_;
      // ハッシュテーブル。
      TranspositionTableSlotList table_[NUM_LISTS];
      // 現在のスロット数。
      int num_slots_;
  };
}  // namespace Sayuri

#endif
