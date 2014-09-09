/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
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
 * @file transposition_table.h
 * @author Hironori Ishibashi
 * @brief トランスポジションテーブル。
 */

#ifndef TRANSPOSITION_TABLE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define TRANSPOSITION_TABLE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <memory>
#include <mutex>
#include <cstddef>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class TranspositionTable;

  /** トランスポジションテーブルのエントリー。 */
  class TTEntry {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param pos_hash ポジションのハッシュ。
       * @param depth 残り深さ。
       * @param score 評価値。
       * @param score_type 評価値の種類。
       * @param best_move 最善手。
       * @param mate_in メイトまでの手数。 -1はメイトなし。
       * @param table_age トランスポジションテーブルの年齢。
       */
      TTEntry(Hash pos_hash, int depth, int score, ScoreType score_type,
      Move best_move, int mate_in,  int table_age);
      /** コンストラクタ。 */
      TTEntry();
      /**
       * コピーコンストラクタ。
       * @param entry コピー元。
       */
      TTEntry(const TTEntry& entry);
      /**
       * ムーブコンストラクタ。
       * @param entry ムーブ元。
       */
      TTEntry(TTEntry&& entry);
      /**
       * コピー代入演算子。
       * @param entry コピー元。
       */
      TTEntry& operator=(const TTEntry& entry);
      /**
       * ムーブ代入演算子。
       * @param entry ムーブ元。
       */
      TTEntry& operator=(TTEntry&& entry);
      /** デストラクタ。 */
      virtual ~TTEntry() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 内容が有効かどうか。
       */
      explicit operator bool() const {return depth_ > -MAX_VALUE;}

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - ポジションのハッシュ。
       * @return ポジションのハッシュ。
       */
      Hash pos_hash() const {return pos_hash_;}
      /**
       * アクセサ - 残り深さ。
       * @return 残り深さ。
       */
      int depth() const {return depth_;}
      /**
       * アクセサ - 評価値。
       * @return 評価値。
       */
      int score() const {return score_;}
      /**
       * アクセサ - 評価値の種類。
       * @return 評価値の種類。
       */
      ScoreType score_type() const {return score_type_;}
      /**
       * アクセサ - 最善手。
       * @return 最善手。
       */
      Move best_move() const {return best_move_;}
      /**
       * アクセサ - メイトまでの手数。 -1はメイトなし。
       * @return メイトまでの手数。
       */
      int mate_in() const {return mate_in_;}
      /**
       * アクセサ - トランスポジションテーブルの年齢。
       * @return トランスポジションテーブルの年齢。
       */
      int table_age() const {return table_age_;}

    private:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** ポジションのハッシュ。 */
      Hash pos_hash_;
      /** 残り深さ。 */
      int depth_;
      /** 評価値。 */
      int score_;
      /** 評価値の種類。 */
      ScoreType score_type_;
      /** 最善手。 */
      Move best_move_;
      /** メイトまでの手数。 */
      int mate_in_;
      /** 記録時のトランスポジションテーブルの年齢。 */
      int table_age_;
  };

  /** トランスポジションテーブルのクラス。 */
  class TranspositionTable {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param table_size テーブルのサイズ。 (バイト)
       */
      TranspositionTable(std::size_t table_size);
      /**
       * コピーコンストラクタ。
       * @param table コピー元。
       */
      TranspositionTable(const TranspositionTable& table);
      /**
       * ムーブコンストラクタ。
       * @param table ムーブ元。
       */
      TranspositionTable(TranspositionTable&& table);
      /**
       * コピー代入演算子。
       * @param table コピー元。
       */
      TranspositionTable& operator=(const TranspositionTable& table);
      /**
       * ムーブ代入演算子。
       * @param table ムーブ元。
       */
      TranspositionTable& operator=(TranspositionTable&& table);
      /** デストラクタ。 */
      virtual ~TranspositionTable() {}
      /** コンストラクタ。 (削除) */
      TranspositionTable() = delete;

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * テーブルにエントリーを追加。
       * @param pos_hash ポジションのハッシュ。
       * @param depth 残り深さ。
       * @param score 評価値。
       * @param score_type 評価値の種類。
       * @param best_move 最善手。
       * @param mate_in メイトまでの手数。 -1はメイトなし。
       */
      void Add(Hash pos_hash, int depth, int score, ScoreType score_type,
      Move best_move, int mate_in);

      /**
       * 条件を満たすエントリーを得る。
       * @param pos_hash 条件のハッシュ。
       * @param depth 条件の残り深さ。
       * @return 条件を満たすエントリー。
       */
      const TTEntry& GetEntry(Hash pos_hash, int depth) const;

      /**
       * 前回の最善手を得る。
       * @param pos_hash 現在のハッシュ。
       * @param depth 現在の残り深さ。
       * @return 条件を満たすエントリー。
       */
      Move GetPrevBest(Hash pos_hash, int depth) const;

      /** 年を取る。 */
      void GrowOld() {++age_;}

      /**
       * テーブルのサイズを返す。 (バイト)
       * @return テーブルのサイズ。
       */
      std::size_t GetSizeBytes() const {
        return num_entries_ * sizeof(TTEntry);
      }

      /**
       * 使用済みエントリーの占める割合。 (パーミル)
       * @return 使用済みエントリーの占める割合。
       */
      int GetUsedPermill() const {
        return (num_used_entries_ * 1000) / num_entries_;
      }

      /** トランスポジションテーブルのロックでロックする。 */
      void Lock() {mutex_.lock();}
      /** トランスポジションテーブルのロックのロックを解除する。。 */
      void Unlock() {mutex_.unlock();}

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - 年齢。
       * @return 年齢。
       */
      int age() const {return age_;}

    private:
      /** フレンドのデバッグ用関数。 */
      friend int DebugMain(int argc, char* argv[]);

      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * テーブルのインデックスを得る。
       * @param ポジションのハッシュ。
       * @return テーブルのインデックス。
       */
      std::size_t GetTableIndex(Hash pos_hash) const {
        return pos_hash % num_entries_;
      }

      // ========== //
      // メンバ変数 //
      // ========== //
      /** 無効なエントリー。 */
      const TTEntry null_entry_;
      /** エントリーの個数。 */
      std::size_t num_entries_;
      /** 使用済みのエントリーの個数。 */
      std::size_t num_used_entries_;
      /** エントリーを登録するテーブル。 */
      std::unique_ptr<TTEntry[]> entry_table_;
      /** 年齢。 */
      int age_;
      /** ミューテックス。 */
      std::mutex mutex_;
  };
}  // namespace Sayuri

#endif
