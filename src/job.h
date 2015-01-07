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
#include <functional>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;
  class TranspositionTable;
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
        maker_ptr_ = &maker;
        helper_counter_ = 0;
        counter_ = 0;
        betacut_listener_vec_.clear();
      }

      /**
       * 候補手を得る。
       * @return 候補手。
       */
      Move PickMove();

      /** この仕事を請け負っているヘルパーの数を増やす。 */
      void CountHelper();

      /** 
       * ベータカットのお知らせを受け取る関数を登録する。
       * @param listener ベータカットのお知らせを受け取る関数オブジェクト。
       */
      void AddBetaCutListener(const std::function<void(Job&)>& listener) {
        std::unique_lock<std::mutex> lock(my_mutex_);  // ロック。
        betacut_listener_vec_.push_back(listener);
      }

      /** 仕事終了を知らせる。 */
      void FinishMyJob();

      /** ヘルパーが全員仕事を終えるまで待機する。 */
      void WaitForHelpers();

      /** ヘルパーにベータカットが発生したことを知らせる。 */
      void NotifyBetaCut();

      /**
       *  数を数える。 探索した手の数を数えるときに使う。
       * @return 次の数字。
       */
      int Count();

      /** ロックする。 */
      void Lock() {mutex_.lock();}
      /** ロックを解除する。 */
      void Unlock() {mutex_.unlock();}

      // ==================== //
      // 仕事用パブリック変数 //
      // ==================== //
      /** クライアントのポインタ。 */
      ChessEngine* client_ptr_;
      /** ポジションの記録のポインタ。 */
      const PositionRecord* record_ptr_;
      /** 共有ノードのノードタイプ。 */
      NodeType node_type_;
      /** 共有ノードのハッシュ。 */
      Hash pos_hash_;
      /** 共有ノードの深さ。 */
      int depth_;
      /** 共有ノードのレベル。 */
      int level_;
      /** 共有ノードのアルファ値。 */
      int alpha_;
      /** 共有ノードのベータ値。 */
      int beta_;
      /** 共有ノードのデルタ値。 */
      int delta_;
      /** トランスポジションテーブルのポインタ。 */
      TranspositionTable* table_ptr_;
      /** 共有ノードのPVラインのポインタ。 */
      PVLine* pv_line_ptr_;
      /** クライアントがNull Search中かどうかのフラグ。 */
      bool is_null_searching_;
      /** 共有ノードでのNull Move Reductionの結果。 */
      int null_reduction_;
      /** 共有ノードの評価値の種類。 */
      ScoreType score_type_;
      /** 共有ノードでのマテリアル。 */
      int material_;
      /** 共有ノードでチェックされているかどうかのフラグ。 */
      bool is_checked_;
      /** 共有ノードでの候補手の数。 */
      int num_all_moves_;
      /** 共有ノードで合法手が見つかったかどうかのフラグ。 */
      bool has_legal_move_;
      /** 探索する候補手のベクトルのポインタ。 */
      const std::vector<Move>* moves_to_search_ptr_;
      /** 次にinfoコマンドを送る時間のポインタ。 */
      TimePoint next_print_info_time_;

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
      /** 仕事用MoveMakerのポインタ。 */
      MoveMaker* maker_ptr_;
      /** 請け負っているヘルパーの数。 */
      volatile int helper_counter_;
      /** ベータカットのお知らせを受け取る関数のベクトル。 */
      std::vector<std::function<void(Job&)>> betacut_listener_vec_;
      /** ミューテックス。 */
      std::mutex mutex_;
      /** 自分用コンディション。 */
      std::condition_variable cond_;
      /** 自分用ミューテックス。 */
      std::mutex my_mutex_;
      /** 数を数えるためのカウンター。 */
      volatile int counter_;
  };
}  // namespace Sayuri

#endif
