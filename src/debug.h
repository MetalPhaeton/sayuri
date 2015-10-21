/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Hironori Ishibashi
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
 * @file debug.h
 * @author Hironori Ishibashi
 * @brief Sayuriのデバッグツール。
 */

#ifndef SAYURI_DEBUG_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define SAYURI_DEBUG_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class PositionRecord;
  class Evaluator;

  /**
   * デバッグ用メイン関数。
   * @param argc コマンド引数の数。
   * @param argv コマンド引数。
   * @return 終了コード。
   */
  int DebugMain(int argc, char* argv[]);

  /**
   * ビットボードの状態を以下のように出力する。
   * @code
   *  +-----------------+
   * 8| . . . . @ . . @ |
   * 7| . . . . . . . . |
   * 6| . . . . @ . @ . |
   * 5| . . . . . . . . |
   * 4| . @ . . . . . . |
   * 3| . . . . . . . . |
   * 2| . . . . . . @ . |
   * 1| @ . . . . . . . |
   *  +-----------------+
   *    a b c d e f g h 
   * @endcode
   * @param bitboard 対象のビットボード。
   */
  void PrintBitboard(Bitboard bitboard);

  /**
   * Moveの状態を出力する。
   * @param move 対象のMove。
   */
  void PrintMove(Move move);

  /**
   * 駒の配置の状態を以下のように出力する。
   * @code
   *  +-----------------+
   * 8| r . b q k b n r |
   * 7| p p p p . p p p |
   * 6| . . n . . . . . |
   * 5| . . . . p . . . |
   * 4| . . . . P . . . |
   * 3| . . . . . . N . |
   * 2| P P P P . P P P |
   * 1| R N B Q K B . R |
   *  +-----------------+
   *    a b c d e f g h 
   * @endcode
   * @param position 対象の駒の配置。
   */
  void PrintPosition(const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]);

  /**
   * PositionRecordの状態を出力する。
   * @param record 対象のPositionRecord。
   */
  void PrintPositionRecord(const PositionRecord& record);

  /**
   * Evaluatorのvalue_table_の中身を出力する。
   * @param evaluator 調べたいEvaluator。
   */
  // void PrintValueTable(const Evaluator& evaluator);

  // ================ //
  // ストップウォッチ //
  // ================ //
  /** 時間を測るストップウォッチのクラス。 */
  class StopWatch {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      StopWatch();
      /** 
       * コピーコンストラクタ。
       * @param watch コピー元。
       */
      StopWatch(const StopWatch& watch);
      /** 
       * ムーブコンストラクタ。
       * @param watch ムーブ元。
       */
      StopWatch(StopWatch&& watch);
      /** 
       * コピー代入演算子。
       * @param watch コピー元。
       */
      StopWatch& operator=(const StopWatch& watch);
      /** 
       * ムーブ代入演算子。
       * @param watch ムーブ元。
       */
      StopWatch& operator=(StopWatch&& watch);
      /** デストラクタ。 */
      virtual ~StopWatch() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /** ストップウォッチをスタート。 */
      void Start();

      /** ストップウォッチをストップ。 */
      void Stop();

      /**
       * 計測時間を得る。 (ミリ秒)
       * @return 計測時間。 (ミリ秒)
       */
      int GetTime();

    private:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** スタート時間。 */
      TimePoint start_point_;
      /** ストップ時間。 */
      TimePoint stop_point_;
  };
}  // namespace Sayuri

#endif
