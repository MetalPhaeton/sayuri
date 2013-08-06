/*
   uci_shell.h: UCIのチェスエンジン側インターフェイス。

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

#ifndef UCI_SHELL_H
#define UCI_SHELL_H

#include <iostream>
#include <cstddef>
#include "chess_def.h"
#include "chess_engine.h"
#include "pv_line.h"

namespace Sayuri {
  // UCIのインターフェス。
  class UCIShell {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // コンストラクタ。
      UCIShell(ChessEngine* engine_ptr);
      UCIShell() = delete;
      UCIShell(const UCIShell& shell);
      UCIShell(UCIShell&& shell);
      UCIShell& operator=(const UCIShell& shell);
      UCIShell& operator=(UCIShell&& shell);

      /****************/
      /* static関数。 */
      /****************/
      // PV情報を標準出力に送る。
      // [引数]
      // depth: 基本の深さ。
      // seldepth: Quiesceの深さ。
      // score: 評価値。センチポーン。
      // time: 思考時間。
      // num_nodes: 探索したノード数。
      // pv_line: PVライン。
      static void SendPVInfo(int depth, int seldepth, int score,
      Chrono::milliseconds time, std::size_t num_nodes, PVLine& pv_line);
      // 深さ情報を標準出力に送る。
      // [引数]
      // depth: 基本の深さ。
      static void SendDepthInfo(int depth);
      // その他の情報を標準出力に送る。
      // [引数]
      // time: 時間。
      // hashfull: トランスポジションテーブルの使用量。
      // nps: Node Per Seconds。
      static void SendOtherInfo(Chrono::milliseconds time,
      int hashfull, int nps);

    private:
      /****************/
      /* メンバ変数。 */
      /****************/
      // チェスエンジン。
      ChessEngine* engine_ptr_;
  };
}  // namespace Sayuri

#endif
