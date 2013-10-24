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
#include <string>
#include <vector>
#include <cstddef>
#include <thread>
#include <memory>
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

      /********************/
      /* パブリック関数。 */
      /********************/
      // エンジンを実行する。
      void Run();

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
      // 現在探索している手の情報を標準出力に送る。
      // [引数]
      // move: 現在探索している手。
      // move_num: 手の番号。
      static void SendCurrentMoveInfo(Move move, int move_num);
      // その他の情報を標準出力に送る。
      // [引数]
      // time: 時間。
      // num_nodes: 探索したノード数。
      // hashfull: トランスポジションテーブルの使用量。
      static void SendOtherInfo(Chrono::milliseconds time,
      std::size_t num_nodes, int hashfull);

      // インプット用スレッド。
      // [引数]
      // shell: UCIシェル。
      static void ThreadUCIInput(UCIShell& shell);
      // 思考用スレッド。
      // [引数]
      // shell: UCIシェル。
      static void ThreadThinking(UCIShell& shell);

    private:
      /*********************/
      /* UCIコマンド関数。 */
      /*********************/
      // uciコマンド。
      void CommandUCI();
      // isreadyコマンド。
      void CommandIsReady();
      // setoptionコマンド。
      // [引数]
      // argv: コマンド引数。argv[0]はコマンド名。
      void CommandSetOption(const std::vector<std::string>& argv);
      // ucinewgameコマンド。
      void CommandUCINewGame();
      // positionコマンド。
      // [引数]
      // argv: コマンド引数。argv[0]はコマンド名。
      void CommandPosition(const std::vector<std::string>& argv);
      // goコマンド。
      // [引数]
      // argv: コマンド引数。argv[0]はコマンド名。
      void CommandGo(const std::vector<std::string>& argv);
      // stopコマンド。
      void CommandStop();
      // ponderhitコマンド。
      void CommandPonderHit();

      /*************************/
      /* UCIコマンドのパーサ。 */
      /*************************/
      // 単語の種類。
      enum class WordType {
        KEYWORD,  // キーワード。
        PARAM,  // 引数。
        DELIM  // 区切り。
      };
      // コマンドの単語。
      struct Word {
        std::string str_;  //単語の文字列。
        WordType type_;  // 単語の種類。

        Word& operator=(const Word& word) {
          str_ = word.str_;
          type_ = word.type_;
          return *this;
        }
      };
      // パーサ。
      class CommandParser {
        public:
          /**************************/
          /* コンストラクタと代入。 */
          /**************************/
          // [引数]
          // keywords: キーワードの配列。
          // argv: コマンドの引数リスト。
          CommandParser(const std::vector<std::string>& keywords,
          const std::vector<std::string>& argv);
          CommandParser() = delete;
          CommandParser(const CommandParser& parser);
          CommandParser(CommandParser&& parser);
          CommandParser& operator=(const CommandParser& parser);
          CommandParser& operator=(CommandParser&& parser);

          /********************/
          /* パブリック関数。 */
          /********************/
          // 単語を得る。
          const Word& Get();
          // 単語があるかどうか。
          bool HasNext() const;
          // 区切りかどうか。
          bool IsDelim() const;
          // 次のキーワードへジャンプ。
          void JumpToNextKeyword();
          // 冒頭にリセット。
          void Reset();

        private:
          // 構文リスト。
          std::vector<Word> syntax_vector_;
          // 構文リストのイテレータ。
          std::vector<Word>::const_iterator itr_;
      };

      /**************/
      /* 便利関数。 */
      /**************/
      // 手を文字に変換する。
      // [引数]
      // move: 変換する手。
      // [戻り値]
      // 手の文字列。
      static std::string TransMoveToString(Move move);
      // 文字列を手に変換する。
      // [引数]
      // move_str: 変換する文字列。
      // [戻り値]
      // 変換された手。
      static Move TransStringToMove(std::string move_str);

      /****************/
      /* メンバ変数。 */
      /****************/
      // チェスエンジン。
      ChessEngine* engine_ptr_;
      // スレッド。
      std::thread thinking_thread_;
      // 思考すべき候補手のベクトル。
      std::unique_ptr<std::vector<Move>> moves_to_search_ptr_;

      // オプション。トランスポジションテーブルのサイズ。
      std::size_t table_size_;
  };
}  // namespace Sayuri

#endif
