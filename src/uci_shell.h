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
 * @file uci_shell.h
 * @author Hironori Ishibashi
 * @brief エンジン側のUCIインターフェイス。
 */

#ifndef UCI_SHELL_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define UCI_SHELL_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstddef>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>
#include <utility>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;
  class PVLine;

  /** UCIコマンドラインのパーサのクラス。 */
  class UCICommand {
    public:
      /** コマンド引数の型。 */
      using CommandArgs = std::map<std::string, std::vector<std::string>>;

      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      UCICommand() : func_vec_(0) {}
      /**
       * コピーコンストラクタ。
       * @param command コピー元。
       */
      UCICommand(const UCICommand& command) :
      func_vec_(command.func_vec_) {}
      /**
       * ムーブコンストラクタ。
       * @param command ムーブ元。
       */
      UCICommand(UCICommand&& command) :
      func_vec_(std::move(command.func_vec_)) {}
      /**
       * コピー代入演算子。
       * @param command コピー元。
       */
      UCICommand& operator=(const UCICommand& command) {
        func_vec_ = command.func_vec_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param command ムーブ元。
       */
      UCICommand& operator=(UCICommand&& command) {
        func_vec_ = std::move(command.func_vec_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~UCICommand() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * コマンドに対するコールバック関数を登録する。
       * @param command_name コマンド名。
       * @param subcommand_name_vec サブコマンド名の配列。
       * @param func 「command_name」に対するコールバック関数。
       */
      void Add(const std::string& command_name,
      const std::vector<std::string>& subcommand_name_vec,
      std::function<void(CommandArgs&)> func);

      /**
       * コマンドを実行する。
       * @param command_line 実行するコマンド。
       * @return コマンドが実行されればtrue。
       */
      bool operator()(const std::string& command_line);

    private:
      /** 登録されたコマンドの構造体。 */
      struct CommandFunction {
        std::string command_name_;
        std::vector<std::string> subcommand_name_vec_;
        std::function<void(CommandArgs&)> func_;
      };
      /** 登録されたコマンドの配列。 */
      std::vector<CommandFunction> func_vec_;
  };

  /** UCIのインターフェス。 */
  class UCIShell {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param engine 関連付けるChessEngine。
       */
      UCIShell(ChessEngine& engine);
      /**
       * コピーコンストラクタ。
       * @param shell コピー元。
       */
      UCIShell(const UCIShell& shell);
      /**
       * ムーブコンストラクタ。
       * @param shell ムーブ元。
       */
      UCIShell(UCIShell&& shell);
      /**
       * コピー代入演算子。
       * @param shell コピー元。
       */
      UCIShell& operator=(const UCIShell& shell);
      /**
       * ムーブ代入演算子。
       * @param shell ムーブ元。
       */
      UCIShell& operator=(UCIShell&& shell);
      /** デストラクタ。 */
      virtual ~UCIShell();
      /** コンストラクタ。 (削除) */
      UCIShell() = delete;

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * UCIコマンドを実行する。 (「quit」コマンド以外。)
       * @param input UCIコマンド。
       * @return コマンドが実行されればtrue。
       */
      bool InputCommand(const std::string input);

      /**
       * UCIShellからの出力を受け取るコールバック関数を登録する。
       * @param func 出力を受け取る。コールバック関数。
       */
      void AddOutputListener(std::function<void(const std::string&)> func);

      /**
       * PVライン情報を出力する。
       * @param depth 繰り返しの深さ。
       * @param seldepth Quiesce探索の深さ。
       * @param score 評価値。
       * @param time 思考時間。
       * @param num_nodes 探索したノード数。
       * @param hashfull トランスポジションテーブルの使用率。
       * @param pv_line PVライン。
       */
      void PrintPVInfo(int depth, int seldepth, int score,
      Chrono::milliseconds time, std::uint64_t num_nodes, int hashfull,
      PVLine& pv_line);

      /**
       * 深さ情報を出力する。
       * @param depth 繰り返しの深さ。
       */
      void PrintDepthInfo(int depth);

      /**
       * 現在探索している候補手の情報を出力する。
       * @param move 現在探索している候補手。
       * @param move_num 候補手の番号。
       */
      void PrintCurrentMoveInfo(Move move, int move_num);

      /**
       * その他の情報を出力する。
       * @param time 探索時間。
       * @param num_nodes 探索したノード数。
       * @param hashfull トランスポジションテーブルの使用率。
       */
      void PrintOtherInfo(Chrono::milliseconds time,
      std::uint64_t num_nodes, int hashfull);

      /**
       * 探索終了時の最終出力を出力する。
       * @param depth 繰り返しの深さ。
       * @param time 探索時間。
       * @param num_nodes 探索したノード数。
       * @param hashfull トランスポジションテーブルの使用率。
       * @param score 評価値。
       * @param pv_line PVライン。
       */
      void PrintFinalInfo(int depth, Chrono::milliseconds time,
      std::uint64_t num_nodes, int hashfull, int score,  PVLine& pv_line);

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - 登録されているエンジン。
       * @return 登録されているエンジン。
       */
      const ChessEngine& engine() const {return *engine_ptr_;}
      /**
       * アクセサ - スレッドの数。
       * @return スレッドの数。
       */
      int num_threads() const {return num_threads_;}

      // ============ //
      // ミューテータ //
      // ============ //
      /**
       * ミューテータ - スレッドの数。
       * @param num_threads スレッドの数。
       */
      void num_threads(int num_threads) {
        num_threads_ = Util::GetMax(num_threads, 1);
      }

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
      /** 探索スレッド。 */
      void ThreadThinking();

      // =============== //
      // UCIコマンド関数 //
      // =============== //
      /**
       * 「uci」コマンドのコールバック関数。
       * @param args コマンドライン。
       */
      void CommandUCI(UCICommand::CommandArgs& args);
      /**
       * 「isready」コマンドのコールバック関数。
       * @param args コマンドライン。
       */
      void CommandIsReady(UCICommand::CommandArgs& args);
      /**
       * 「setoption」コマンドのコールバック関数。
       * @param args コマンドライン。
       */
      void CommandSetOption(UCICommand::CommandArgs& args);
      /**
       * 「ucinewgame」コマンドのコールバック関数。
       * @param args コマンドライン。
       */
      void CommandUCINewGame(UCICommand::CommandArgs& args);
      /**
       * 「position」コマンドのコールバック関数。
       * @param args コマンドライン。
       */
      void CommandPosition(UCICommand::CommandArgs& args);
      /**
       * 「go」コマンドのコールバック関数。
       * @param args コマンドライン。
       */
      void CommandGo(UCICommand::CommandArgs& args);
      /**
       * 「stop」コマンドのコールバック関数。
       * @param args コマンドライン。
       */
      void CommandStop(UCICommand::CommandArgs& args);
      /**
       * 「ponderhit」コマンドのコールバック関数。
       * @param args コマンドライン。
       */
      void CommandPonderHit(UCICommand::CommandArgs& args);

      // ======== //
      // 便利関数 //
      // ======== //
      /**
       * Moveを文字列に変換する。
       * @param move 文字列に変換するMove。
       * @return 変換後の文字列。
       */
      static std::string MoveToString(Move move);
      /**
       * 文字列をMoveに変換する。
       * @param move_str Moveに変換する文字列。
       * @return 変換後のMove。
       */
      static Move StringToMove(std::string move_str);

      // ========== //
      // メンバ変数 //
      // ========== //
      /** UCIコマンドのパーサ。 */
      UCICommand uci_command_;

      /** 関連付けられたチェスエンジン。 */
      ChessEngine* engine_ptr_;

      /** 探索スレッド。 */
      std::thread thinking_thread_;
      /** 探索する候補手のベクトル。 */
      std::vector<Move> moves_to_search_;

      /** UCIオプション。 ポンダリングするかどうかのフラグ。 */
      bool enable_pondering_;
      /** UCIオプション。 スレッドの数。 */
      int num_threads_;
      /** UCIオプション。 アナライズモード。 */
      bool analyse_mode_;

      /** 出力用ミューテックス。 */
      static std::mutex print_mutex_;
      /** 出力を受け取るコールバック関数のベクトル。 */
      std::vector<std::function<void(const std::string&)>> output_listeners_;
  };
}  // namespace Sayuri

#endif
