/*
   job.h: マルチスレッド探索用仕事クラス。

   The MIT License (MIT)

   Copyright (c) 2013 Hironori Ishibashi

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

#ifndef JOB_H
#define JOB_H

#include <iostream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <memory>
#include "chess_def.h"
#include "chess_engine.h"
#include "transposition_table.h"
#include "pv_line.h"
#include "move_maker.h"
#include "position_record.h"

namespace Sayuri {
  class ChessEngine;
  class MoveMaker;
  class PositionRecord;

  // マルチスレッド探索用の仕事クラス。
  class Job {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // [引数]
      // mutex: そのノードのミューテックス。
      // maker: 仕事用の手が入っているムーブメーカー。
      // client: クライアントのチェスエンジン。
      // record: 仕事作成時のクライアントの駒の配置など。
      // node_type: ノードのタイプ。
      // pos_hash: 仕事作成時のノードの局面のハッシュ。
      // depth: 仕事作成時のノードの深さ。
      // level: 仕事作成時のノードのレベル。
      // alpha: 現在のアルファ値の変数。(更新される。)
      // beta: 現在のベータ値の変数。(更新される。)
      // delta: ルート探索時、ベータ値の増分の変数。(更新される。)
      // table: トランスポジションテーブル。(更新される。)
      // pv_line: 現在のノードのPVライン。(更新される。)
      // is_null_searching: Null Moveで探索中かどうか。
      // null_reduction: Null Move Reductionのリダクションの数。 
      // num_serached_moves: いくつ手を探索したかの変数。(更新される。)
      // score_type: 評価値のタイプ。(更新される。)
      // material: 仕事作成時のノードのマテリアル。
      // is_checked: チェックされているかどうか。
      // has_legal_move: 合法手が見つかったかどうか。(更新される。)
      // moves_to_search_ptr: ルートで探索すべき手のベクトル。ないならnullptr。
      // next_print_info_time: 情報を出力する時間。(更新される。)
      Job(std::mutex& mutex, MoveMaker& maker, ChessEngine& client,
      PositionRecord& record, NodeType node_type, Hash pos_hash, int depth,
      int level, int& alpha, int& beta, int& delta, TranspositionTable& table,
      PVLine& pv_line, bool is_null_searching, int null_reduction,
      int& num_searched_moves, ScoreType& score_type, int material,
      bool is_checked, bool& has_legal_move,
      std::vector<Move>* moves_to_search_ptr,
      TimePoint& next_print_info_time);

      Job(const Job& job);
      Job(Job&& job);
      Job& operator=(const Job& job);
      Job& operator=(Job&& job);
      virtual ~Job() {}
      Job() = delete;

      /********************/
      /* パブリック関数。 */
      /********************/
      // 手を得る。
      // [戻り値]
      // 手。
      Move PickMove();
      // ヘルパーの数を一つ増やす。
      void CountHelper();
      // 仕事終了の合図を出す。
      void FinishMyJob();
      // ヘルパーが全員仕事を終えるまで待機する。
      void WaitForHelpers();
      // 数を数える。UCIのcurrmovenumberの表示に使用する。
      int Count();

      /**************/
      /* アクセサ。 */
      /**************/
      // そのノードのミューテックス。
      std::mutex& mutex() {return *mutex_ptr_;}
      // クライアントのチェスエンジン。
      ChessEngine& client() {return *client_ptr_;}
      // 仕事作成時のクライアントの駒の配置など。
      PositionRecord& record() {return *record_ptr_;}
      // ノードのタイプ。
      NodeType node_type() const {return node_type_;}
      // 仕事作成時のノードの局面のハッシュ。
      Hash pos_hash() const {return pos_hash_;}
      // 仕事作成時のノードの深さ。
      int depth() const {return depth_;}
      // 仕事作成時のノードのレベル。
      int level() const {return level_;}
      // 現在のアルファ値の変数。(更新される。)
      int& alpha() {return *alpha_ptr_;}
      // 現在のベータ値の変数。(更新される。)
      int& beta() {return *beta_ptr_;}
      // ルート探索時、ベータ値の増分の変数。(更新される。)
      int& delta() {return *delta_ptr_;}
      // トランスポジションテーブル。(更新される。)
      TranspositionTable& table() {return *table_ptr_;}
      // 現在のノードのPVライン。(更新される。)
      PVLine& pv_line() {return *pv_line_ptr_;}
      // Null Move探索中かどうか。
      bool is_null_searching() const {return is_null_searching_;}
      // Null Move Reductionのリダクションの数。
      int null_reduction() const {return null_reduction_;}
      // いくつ手を探索したかの変数。(更新される。)
      int& num_searched_moves() {return *num_searched_moves_ptr_;}
      // 評価値のタイプ。(更新される。)
      ScoreType& score_type() {return *score_type_ptr_;}
      // 仕事作成時のノードのマテリアル。
      int material() const {return material_;}
      // チェックされているかどうか。
      bool is_checked() const {return is_checked_;}
      // 合法手が見つかったかどうか。(更新される。)
      bool& has_legal_move() {return *has_legal_move_ptr_;}
      // ルートで探索すべき手のベクトル。ないならnullptr。
      std::vector<Move>* moves_to_search_ptr() {return moves_to_search_ptr_;}
      // 情報を出力する時間。(更新される。)
      TimePoint& next_print_info_time() {return *next_print_info_time_ptr_;}

    private:
      /**********************/
      /* プライベート関数。 */
      /**********************/
      // メンバーをコピーする。
      // [引数]
      // job: コピー元。
      void ScanMember(const Job& job);

      /****************/
      /* メンバ変数。 */
      /****************/
      // 親スレッドより情報。
      std::mutex* mutex_ptr_;
      ChessEngine* client_ptr_;
      PositionRecord* record_ptr_;
      NodeType node_type_;
      Hash pos_hash_;
      int depth_;
      int level_;
      int* alpha_ptr_;
      int* beta_ptr_;
      int* delta_ptr_;
      TranspositionTable* table_ptr_;
      PVLine* pv_line_ptr_;
      bool is_null_searching_;
      int null_reduction_;
      int* num_searched_moves_ptr_;
      ScoreType* score_type_ptr_;
      int material_;
      bool is_checked_;
      bool* has_legal_move_ptr_;
      std::vector<Move>* moves_to_search_ptr_;
      TimePoint* next_print_info_time_ptr_;

      // 仕事用ムーブメーカー。
      MoveMaker* maker_ptr_;
      // ヘルパーカウンター。
      volatile int helper_counter_;
      // ミューテックス。
      std::mutex mutex_;
      // コンディション。
      std::condition_variable cond_;
      // 数を数えるためのカウンター。UCIのcurrmovenumberの表示に使用する。
      volatile int counter_;
  };
}  // namespace Sayuri

#endif
