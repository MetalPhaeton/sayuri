/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Hironori Ishibashi
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
 * @file job.h
 * @author Hironori Ishibashi
 * @brief 並列探索用仕事。
 */

#ifndef JOB_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define JOB_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <list>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;
  class MoveMaker;
  class PositionRecord;
  class PVLine;

  /** 並列探索用の仕事クラス。 */
  class Job {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      Job();
      /**
       * コピーコンストラクタ。
       * @param job コピー元。
       */
      Job(const Job& job);
      /**
       * ムーブコンストラクタ。
       * @param job ムーブ元。
       */
      Job(Job&& job);
      /**
       * コピー代入演算子。
       * @param job コピー元。
       */
      Job& operator=(const Job& job);
      /**
       * ムーブ代入演算子。
       * @param job ムーブ元。
       */
      Job& operator=(Job&& job);
      /** デストラクタ。 */
      virtual ~Job() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 再初期化。
       * @param maker 使用するMoveMaker。
       */
      void Init(MoveMaker& maker) {
        std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
        end_ = helpers_table_;
        num_helpers_ = 0;
        maker_ptr_ = &maker;
        counter_ = 0;
      }

      /**
       * 候補手を得る。
       * @return 候補手。
       */
      Move PickMove();

      /**
       * 数を数える。 (候補手を数えるときに使う。)
       * @return 数字。
       */
      int Count() {
        std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
        return ++counter_;
      }

      /**
       * ヘルパー登録する。
       * @param helper 登録するヘルパー。
       */
      void RegisterHelper(ChessEngine& helper) {
        std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
        *end_ = &helper;
        ++end_;
        ++num_helpers_;
      }

      /**
       * ヘルパー登録解除する。
       * @param helper 登録解除するヘルパー。
       */
      void ReleaseHelper(ChessEngine& helper) {
        std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
        for (ChessEngine** ptrptr = helpers_table_; ptrptr < end_;
        ++ptrptr) {
          if (*ptrptr == &helper) {
            *ptrptr = nullptr;
            --num_helpers_;
          }
        }
        cond_.notify_one();
      }

      /**
       * ベータカットを通知する。
       * @param notifier 通知するヘルパー。
       */
      void NotifyBetaCut(ChessEngine& notifier);

      /**
       * ヘルパーが全て抜けるのを待つ。
       */
      void WaitForHelpers() {
        std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
        while (num_helpers_ >= 1) {
          cond_.wait(lock);
        }
      }

      /** ロックする。 */
      void Lock() {mutex_.lock();}
      /** ロックを解除する。 */
      void Unlock() {mutex_.unlock();}

      // ==================== //
      // 仕事用パブリック変数 //
      // ==================== //
      /** クライアントのポインタ。 */
      ChessEngine* client_ptr_;
      /** 共有ノードのノードタイプ。 */
      NodeType node_type_;
      /** 共有ノードのハッシュ。 */
      Hash pos_hash_;
      /** 共有ノードの深さ。 */
      int depth_;
      /** 共有ノードのレベル。 */
      std::uint32_t level_;
      /** 共有ノードのアルファ値。 */
      volatile int alpha_;
      /** 共有ノードのベータ値。 */
      volatile int beta_;
      /** 共有ノードのデルタ値。 */
      volatile int delta_;
      /** 共有ノードのPVラインのポインタ。 */
      PVLine* pv_line_ptr_;
      /** クライアントがNull Search中かどうかのフラグ。 */
      bool is_null_searching_;
      /** 共有ノードでのNull Move Reductionの結果。 */
      int null_reduction_;
      /** 共有ノードの評価値の種類。 */
      volatile ScoreType score_type_;
      /** 共有ノードでのマテリアル。 */
      int material_;
      /** 共有ノードでチェックされているかどうかのフラグ。 */
      bool is_checked_;
      /** 共有ノードでの候補手の数。 */
      int num_all_moves_;
      /** 共有ノードで合法手が見つかったかどうかのフラグ。 */
      volatile bool has_legal_move_;
      /** 探索する候補手のベクトルのポインタ。 */
      const std::vector<Move>* moves_to_search_ptr_;

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * メンバをコピーする。
       * @param job コピー元。
       */
      void ScanMember(const Job& job);

      // ========== //
      // メンバ変数 //
      // ========== //
      /** 登録されているヘルパーの配列。 */
      ChessEngine* helpers_table_[UCI_MAX_THREADS + 1];
      /** ヘルパーのテーブルの終端。 */
      ChessEngine** end_;
      /** 登録されているヘルパーの数。 */
      volatile int num_helpers_;
      /** ヘルパーを待つためのコンディション。 */
      std::condition_variable cond_;

      /** 仕事用MoveMakerのポインタ。 */
      MoveMaker* maker_ptr_;
      /** ミューテックス。 */
      std::mutex mutex_;
      /** 自分用ミューテックス。 */
      std::mutex my_mutex_;
      /** 候補手を数えるためのカウンター。 */
      volatile int counter_;
  };
}  // namespace Sayuri

#endif
