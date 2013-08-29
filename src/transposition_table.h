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
      // コンストラクタ。
      // [引数]
      // pos_key: ポジションのハッシュキー。
      // depth: 探索の深さ。
      // to_move: 手番。
      // value: 評価値。
      // value_flag: 評価値の種類。
      // best_move: 最善手。
      TTEntry(HashKey pos_key, int depth,
      Side to_move, int value, TTValueFlag value_flag, Move best_move);
      // デフォルトコンストラクタ。
      TTEntry();
      // コピーコンストラクタ。
      TTEntry(const TTEntry& entry);
      // ムーブコンストラクタ。
      TTEntry(TTEntry&& entry);
      // デストラクタ。
      virtual ~TTEntry() {}
      // 代入。
      TTEntry& operator=(const TTEntry& entry);
      TTEntry& operator=(TTEntry&& entry);

      /**********/
      /* 関数。 */
      /**********/
      // ハッシュキーとレベルと深さと手番から
      // 同じポジションかどうか判定する。
      // [引数]
      // key: ハッシュキー。
      // depth: 探索の深さ。
      // to_move: 手番。
      // [戻り値]
      // 同じならtrue。
      bool Fulfil(HashKey key, int depth, Side to_move) const;
      // エントリーをアップデートする。
      // [引数]
      // value: 評価値。
      // value_flag: 評価値の種類。
      // best_move: 最善手。
      void Update(int value, TTValueFlag value_flag, Move best_move) {
        value_ = value;
        value_flag_ = value_flag;
        best_move_ = best_move;
      }

      /**************/
      /* アクセサ。 */
      /**************/
      // データがあるかどうか。
      bool exists() const {return exists_;}
      // ハッシュキー。
      HashKey key() const {return key_;}
      // 深さ。
      int depth() const {return depth_;}
      // 手番。
      Side to_move() const {return to_move_;}
      // 評価値。
      int value() const {return value_;}
      // 評価値の種類。
      TTValueFlag value_flag() const {return value_flag_;}
      // 最善手。
      Move best_move() const {return best_move_;}

      /**********************/
      /* ソート用比較関数。 */
      /**********************/
      static bool Compare(const TTEntry& first, const TTEntry& second);

    private:
      /**************/
      /* フレンド。 */
      /**************/
      friend class TranspositionTable;

      /****************/
      /* メンバ変数。 */
      /****************/
      // データがあるかどうか。
      bool exists_;
      // ハッシュキー。
      HashKey key_;
      // 探索のレベル。
      int depth_;
      // 手番。
      Side to_move_;
      // 評価値。
      int value_;
      // 評価値の種類。
      TTValueFlag value_flag_;
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
      // ハッシュキーのテーブル用マスク。
      static constexpr HashKey TABLE_KEY_MASK = 0XffffULL;
      // テーブルの大きさ。
      static constexpr std::size_t TABLE_SIZE = TABLE_KEY_MASK + 1;

    public:
      // コンストラクタ。
      // [引数]
      // max_bytes: トランスポジションテーブルのサイズ指定。
      TranspositionTable(std::size_t max_bytes);
      // コピーコンストラクタ。
      TranspositionTable(const TranspositionTable& table);
      // ムーブコンストラクタ。
      TranspositionTable(TranspositionTable&& table);
      TranspositionTable() = delete;
      // デストラクタ。
      virtual ~TranspositionTable() {}

      /**********/
      /* 関数。 */
      /**********/
      // テーブルに追加する。
      // [引数]
      // key: ハッシュキー。
      // depth: 探索の深さ。
      // to_move: 手番。
      // value: 評価値。
      // value_flag: 評価値の種類。
      // best_move: 最善手。
      void Add(HashKey key, int depth, Side to_move,
      int value, TTValueFlag value_flag, Move best_move);
      // 条件を満たすエントリーを得る。
      // [引数]
      // pos_key: ハッシュキー。
      // depth: 探索の深さ。
      // to_move: 手番。
      // [戻り値]
      // 条件を満たすエントリー。
      // なければnullptr。
      TTEntry* GetFulfiledEntry(HashKey pos_key, int depth,
      Side to_move) const;

      // 大きさが何バイトか返す。
      // [戻り値]
      // サイズをバイト数で返す。
      std::size_t GetSizeBytes() const;

      // 使用されているエントリーのサイズを全体の何パーミルかを返す。
      // [戻り値]
      // エントリーのパーミル。
      int GetUsedPermill() const;

      // エントリーの種類ごとの比率データ。
      // [引数]
      // <Type>: 調べたい種類。
      // [戻り値]
      // 調べたい種類の比率データ。パーミル。
      template<TTValueFlag Type>
      int GetRatioPermill() const;

      /****************/
      /* static関数。 */
      /****************/
      // テーブルの最小サイズを得る。
      // [戻り値]
      // 最小サイズ。
      static std::size_t GetMinSize();

      // テーブルの最大サイズを得る。
      // [戻り値]
      // 最大サイズ。
      static std::size_t GetMaxSize();

    private:
      /**************/
      /* テスト用。 */
      /**************/
      friend int DebugMain(int, char**);

      /**********************/
      /* プライベート関数。 */
      /**********************/
      // テーブルのインデックスを得る。
      // [引数]
      // pos_key: ポジションのハッシュキー。
      int GetTableIndex(HashKey pos_key) const {
        return pos_key & (TABLE_KEY_MASK);
      }

      /****************/
      /* メンバ変数。 */
      /****************/
      // サイズの上限。
      int max_bytes_;
      // エントリーを登録するテーブル。
      std::unique_ptr<std::vector<TTEntry>[]> entry_table_;
  };
}  // namespace Sayuri

#endif
