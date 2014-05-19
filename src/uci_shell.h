/*
   uci_shell.h: UCIのチェスエンジン側インターフェイス。

   The MIT License (MIT)

   Copyright (c) 2014 Hironori Ishibashi

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
#include <map>
#include <cstddef>
#include <thread>
#include <memory>
#include <functional>
#include <utility>
#include "chess_def.h"
#include "chess_engine.h"
#include "pv_line.h"

namespace Sayuri {
  class ChessEngine;

  // UCIコマンドラインのクラス。
  class UCICommand {
    public:
      // コマンド引数の型。
      using CommandArgs =
      std::map<std::string, std::vector<std::string>>;

      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      UCICommand() : func_vec_(0) {}
      UCICommand(const UCICommand& command) :
      func_vec_(command.func_vec_) {}
      UCICommand(UCICommand&& command) :
      func_vec_(std::move(command.func_vec_)) {}
      UCICommand& operator=(const UCICommand& command) {
        func_vec_ = command.func_vec_;
        return *this;
      }
      UCICommand& operator=(UCICommand&& command) {
        func_vec_ = std::move(command.func_vec_);
        return *this;
      }
      virtual ~UCICommand() {}

      /********************/
      /* パブリック関数。 */
      /********************/
      // コマンドに合わせたコールバック関数を登録する。
      // [引数]
      // command_name: コマンド名。 コマンドラインの先頭の単語。
      // subcommand_name_vec: サブコマンド名の配列。
      // func: そのコマンドを実行するための関数。
      void Add(const std::string& command_name,
      const std::vector<std::string>& subcommand_name_vec,
      std::function<void(CommandArgs&)> func) {
        for (auto& command_func : func_vec_) {
          if (command_func.command_name_ == command_name) {
            command_func.subcommand_name_vec_ = subcommand_name_vec;
            command_func.func_ = func;
            return;
          }
        }

        func_vec_.push_back(CommandFunction
        {command_name, subcommand_name_vec, func});
      }

      void operator()(const std::string& command_line) {
        // トークンに分ける。
        std::vector<std::string> tokens = Util::Split(command_line, " ", "");

        // コマンドを探す。
        CommandArgs args;
        for (auto& command_func : func_vec_) {
          if (command_func.command_name_ == tokens[0]) {
            // コマンドラインをパース。
            std::string temp = "";
            for (auto& word : tokens) {
              for (auto& subcommand_name :
              command_func.subcommand_name_vec_) {
                if (word == subcommand_name) {
                  temp = word;
                  break;
                }
              }
              args[temp].push_back(word);
            }

            // コマンドを実行。
            command_func.func_(args);
            return;
          }
        }
      }

    private:
      // 登録されたコマンド関数。
      struct CommandFunction {
        std::string command_name_;
        std::vector<std::string> subcommand_name_vec_;
        std::function<void(CommandArgs&)> func_;
      };
      std::vector<CommandFunction> func_vec_;
  };

  // UCIのインターフェス。
  class UCIShell {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // [引数]
      // engine: UCIで操作したいエンジン。
      UCIShell(ChessEngine& engine);
      UCIShell(const UCIShell& shell);
      UCIShell(UCIShell&& shell);
      UCIShell& operator=(const UCIShell& shell);
      UCIShell& operator=(UCIShell&& shell);
      virtual ~UCIShell();
      UCIShell() = delete;

      /********************/
      /* パブリック関数。 */
      /********************/
      // UCIコマンドを実行する。("quit"以外。)
      // [引数]
      // input: UCIコマンド入力。("quit"以外。)
      void InputCommand(const std::string input);

      // UCI出力を受け取る関数を登録する。
      // [引数]
      // func: UCI出力を受け取る関数。void(std::string)型。
      void AddOutputListener(std::function<void(const std::string&)> func);

      // PV情報を標準出力に表示。
      // [引数]
      // depth: 基本の深さ。
      // seldepth: Quiesceの深さ。
      // score: 評価値。センチポーン。
      // time: 思考時間。
      // num_nodes: 探索したノード数。
      // pv_line: PVライン。
      void PrintPVInfo(int depth, int seldepth, int score,
      Chrono::milliseconds time, std::uint64_t num_nodes, PVLine& pv_line);

      // 深さ情報を標準出力に表示。
      // [引数]
      // depth: 基本の深さ。
      void PrintDepthInfo(int depth);

      // 現在探索している手の情報を標準出力に表示。
      // [引数]
      // move: 現在探索している手。
      // move_num: 手の番号。
      void PrintCurrentMoveInfo(Move move, int move_num);

      // その他の情報を標準出力に表示。
      // [引数]
      // time: 時間。
      // num_nodes: 探索したノード数。
      // hashfull: トランスポジションテーブルの使用量。
      void PrintOtherInfo(Chrono::milliseconds time,
      std::uint64_t num_nodes, int hashfull);

    private:
      /**********************/
      /* プライベート関数。 */
      /**********************/
      // 思考用スレッド。
      void ThreadThinking();

      /*********************/
      /* UCIコマンド関数。 */
      /*********************/
      // uciコマンド。
      // [引数]
      // args: コマンド引数。
      void CommandUCI(UCICommand::CommandArgs& args);
      // isreadyコマンド。
      // [引数]
      // args: コマンド引数。
      void CommandIsReady(UCICommand::CommandArgs& args);
      // setoptionコマンド。
      // [引数]
      // args: コマンド引数。
      void CommandSetOption(UCICommand::CommandArgs& args);
      // ucinewgameコマンド。
      // [引数]
      // args: コマンド引数。
      void CommandUCINewGame(UCICommand::CommandArgs& args);
      // positionコマンド。
      // [引数]
      // args: コマンド引数。
      void CommandPosition(UCICommand::CommandArgs& args);
      // goコマンド。
      // [引数]
      // args: コマンド引数。
      void CommandGo(UCICommand::CommandArgs& args);
      // stopコマンド。
      // [引数]
      // args: コマンド引数。
      void CommandStop(UCICommand::CommandArgs& args);
      // ponderhitコマンド。
      // [引数]
      // args: コマンド引数。
      void CommandPonderHit(UCICommand::CommandArgs& args);

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
      // UCIコマンドパーサ。
      UCICommand uci_command_;

      // チェスエンジン。
      ChessEngine* engine_ptr_;
      // トランスポジションテーブル。
      std::unique_ptr<TranspositionTable> table_ptr_;
      // スレッド。
      std::thread thinking_thread_;
      // 思考すべき候補手のベクトル。
      std::vector<Move> moves_to_search_;

      // オプション。トランスポジションテーブルのサイズ。
      std::size_t table_size_;
      // オプション。ポンダリングするかどうか。
      bool enable_pondering_;
      // オプション。スレッドの数。
      int num_threads_;
      // アナライズモード。
      bool analyse_mode_;

      // UCI出力を受け取る関数のベクトル。
      std::vector<std::function<void(const std::string&)>> output_listeners_;
  };
}  // namespace Sayuri

#endif
