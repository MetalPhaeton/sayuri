/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Hironori Ishibashi
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
 * @file uci_shell.cpp
 * @author Hironori Ishibashi
 * @brief エンジン側のUCIインターフェイスの実装。
 */

#include "uci_shell.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <iterator>
#include <thread>
#include <mutex>
#include <system_error>
#include <chrono>
#include <sstream>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <cctype>
#include <functional>
#include <set>
#include <climits>
#include "common.h"
#include "chess_engine.h"
#include "transposition_table.h"
#include "pv_line.h"
#include "fen.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ====== //
  // static //
  // ====== //
  std::mutex UCIShell::print_mutex_;

  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  UCIShell::UCIShell(ChessEngine& engine) :
  uci_command_(),
  engine_ptr_(&engine),
  moves_to_search_(0),
  enable_pondering_(UCI_DEFAULT_PONDER),
  num_threads_(UCI_DEFAULT_THREADS),
  analyse_mode_(UCI_DEFAULT_ANALYSE_MODE),
  output_listeners_(0) {
    // コマンドを登録する。
    // uciコマンド。
    uci_command_.Add("uci", {"uci"},
    [this](UCICommand::CommandArgs& args) {this->CommandUCI(args);});

    // isreadyコマンド。
    uci_command_.Add("isready", {"isready"},
    [this](UCICommand::CommandArgs& args) {this->CommandIsReady(args);});

    // setoptionコマンド。
    uci_command_.Add("setoption", {"setoption", "name", "value"},
    [this](UCICommand::CommandArgs& args) {this->CommandSetOption(args);});

    // ucinewgameコマンド。
    uci_command_.Add("ucinewgame", {"ucinewgame"},
    [this](UCICommand::CommandArgs& args) {this->CommandUCINewGame(args);});

    // positionコマンド。
    uci_command_.Add("position", {"position", "startpos", "fen", "moves"},
    [this](UCICommand::CommandArgs& args) {this->CommandPosition(args);});

    // goコマンド。
    uci_command_.Add("go", {
      "go", "searchmoves", "ponder", "wtime", "btime", "winc", "binc",
      "movestogo", "depth", "nodes", "mate", "movetime", "infinite"
    },
    [this](UCICommand::CommandArgs& args) {this->CommandGo(args);});

    // stopコマンド。
    uci_command_.Add("stop", {"stop"},
    [this](UCICommand::CommandArgs& args) {this->CommandStop(args);});

    // ponderhitコマンド。
    uci_command_.Add("ponderhit", {"ponderhit"},
    [this](UCICommand::CommandArgs& args) {this->CommandPonderHit(args);});
  }

  // コピーコンストラクタ。
  UCIShell::UCIShell(const UCIShell& shell) :
  uci_command_(shell.uci_command_),
  engine_ptr_(shell.engine_ptr_),
  moves_to_search_(shell.moves_to_search_),
  enable_pondering_(shell.enable_pondering_),
  num_threads_(shell.num_threads_),
  analyse_mode_(shell.analyse_mode_),
  output_listeners_(shell.output_listeners_) {
  }

  // ムーブコンストラクタ。
  UCIShell::UCIShell(UCIShell&& shell) :
  uci_command_(std::move(shell.uci_command_)),
  engine_ptr_(shell.engine_ptr_),
  moves_to_search_(std::move(shell.moves_to_search_)),
  enable_pondering_(shell.enable_pondering_),
  num_threads_(shell.num_threads_),
  analyse_mode_(shell.analyse_mode_),
  output_listeners_(std::move(shell.output_listeners_)) {
  }

  // コピー代入演算子。
  UCIShell& UCIShell::operator=(const UCIShell& shell) {
    uci_command_ = shell.uci_command_;
    engine_ptr_ = shell.engine_ptr_;
    moves_to_search_ = shell.moves_to_search_;
    enable_pondering_ = shell.enable_pondering_;
    num_threads_ = shell.num_threads_;
    analyse_mode_ = shell.analyse_mode_;
    output_listeners_ = shell.output_listeners_;
    return *this;
  }

  // ムーブ代入演算子。
  UCIShell& UCIShell::operator=(UCIShell&& shell) {
    uci_command_ = std::move(shell.uci_command_);
    engine_ptr_ = shell.engine_ptr_;
    moves_to_search_ = std::move(shell.moves_to_search_);
    enable_pondering_ = shell.enable_pondering_;
    num_threads_ = shell.num_threads_;
    analyse_mode_ = shell.analyse_mode_;
    output_listeners_ = std::move(shell.output_listeners_);
    return *this;
  }

  // デストラクタ。
  UCIShell::~UCIShell() {
    try {
      thinking_thread_.join();
    } catch (std::system_error err) {
      // 無視。
    }
  }

  // ============== //
  // パブリック関数 //
  // ============== //
  // UCIコマンドを実行する。
  bool UCIShell::InputCommand(const std::string input) {
    // コマンド実行。
    return uci_command_(input);
  }

  // UCIShell空の出力を受け取るコールバック関数を登録する。
  void UCIShell::AddOutputListener
  (std::function<void(const std::string&)> func) {
    output_listeners_.push_back(func);
  }

  // PVライン情報を出力する。
  void UCIShell::PrintPVInfo(int depth, int seldepth, int score,
  Chrono::milliseconds time, std::uint64_t num_nodes, int hashfull,
  PVLine& pv_line) {
    std::unique_lock<std::mutex> lock(print_mutex_);  // ロック。

    int time_2 = time.count();
    if (time_2 <= 0) time_2 = 1;

    std::ostringstream sout;
    sout << "info";
    sout << " depth " << depth;
    sout << " seldepth " << seldepth;
    sout << " time " << time_2;
    sout << " nodes " << num_nodes;
    sout << " hashfull " << hashfull;
    sout << " nps " << (num_nodes * 1000) / time_2;

    sout << " score ";
    if (pv_line.mate_in() >= 0) {
      int winner = pv_line.mate_in() % 2;
      if (winner == 1) {
        // エンジンがメイトした。
        sout << "mate " << (pv_line.mate_in() / 2) + 1;
      } else {
        // エンジンがメイトされた。
        sout << "mate -" << pv_line.mate_in() / 2;
      }
    } else {
      sout << "cp " << score;
    }

    // PVラインを送る。
    sout << " pv";
    for (std::uint32_t i = 0; i < pv_line.length(); ++i) {
      if (!(pv_line[i])) break;

      sout << " " << Util::MoveToString(pv_line[i]);
    }

    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }
  }

  // 深さ情報を出力する。
  void UCIShell::PrintDepthInfo(int depth) {
    std::unique_lock<std::mutex> lock(print_mutex_);  // ロック。

    std::ostringstream sout;
    sout << "info depth " << depth;
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }
  }

  // 現在探索している候補手の情報を出力する。
  void UCIShell::PrintCurrentMoveInfo(Move move, int move_num) {
    std::unique_lock<std::mutex> lock(print_mutex_);  // ロック。

    std::ostringstream sout;
    // 手の情報を送る。
    sout << "info currmove " << Util::MoveToString(move);

    // 手の番号を送る。
    sout << " currmovenumber " << move_num;

    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }
  }

  // その他の情報を出力する。
  void UCIShell::PrintOtherInfo(Chrono::milliseconds time,
  std::uint64_t num_nodes, int hashfull) {
    std::unique_lock<std::mutex> lock(print_mutex_);  // ロック。

    std::ostringstream sout;

    int time_2 = time.count();
    if (time_2 <= 0) time_2 = 1;
    sout << "info time " << time_2;
    sout << " nodes " << num_nodes;
    sout << " hashfull " << hashfull;
    sout << " nps " << (num_nodes * 1000) / time_2;

    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }
  }

  // 探索後の最終出力を出力する。
  void UCIShell::PrintFinalInfo(int depth, Chrono::milliseconds time,
  std::uint64_t num_nodes, int hashfull, int score, PVLine& pv_line) {
    std::unique_lock<std::mutex> lock(print_mutex_);  // ロック。

    std::ostringstream sout;

    sout << "info depth " << depth;

    int time_2 = time.count();
    if (time_2 <= 0) time_2 = 1;
    sout << " time " << time_2;
    sout << " nodes " << num_nodes;
    sout << " hashfull " << hashfull;
    sout << " nps " << (num_nodes * 1000) / time_2;
    sout << " score ";
    if (pv_line.mate_in() >= 0) {
      int winner = pv_line.mate_in() % 2;
      if (winner == 1) {
        // エンジンがメイトした。
        sout << "mate " << (pv_line.mate_in() / 2) + 1;
      } else {
        // エンジンがメイトされた。
        sout << "mate -" << pv_line.mate_in() / 2;
      }
    } else {
      sout << "cp " << score;
    }

    // pvラインを送る。
    sout << " pv";
    for (std::uint32_t i = 0; i < pv_line.length(); ++i) {
      if (!(pv_line[i])) break;

      sout << " " << Util::MoveToString(pv_line[i]);
    }

    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }
  }

  // 探索スレッド。
  void UCIShell::ThreadThinking() {
    // アナライズモードならトランスポジションテーブルを初期化。
    if (analyse_mode_) {
      engine_ptr_->table().Clear();
    }

    // テーブルの年齢の増加。
    engine_ptr_->table().GrowOld();

    // 思考開始。
    PVLine pv_line =
    engine_ptr_->Calculate(num_threads_, moves_to_search_, *this);

    // 最善手を表示。
    std::ostringstream sout;

    sout << "bestmove ";
    if (pv_line.length() >= 1) {
      std::string move_str = Util::MoveToString(pv_line[0]);
      sout << move_str;

      // 2手目があるならponderで表示。
      if (pv_line.length() >= 2) {
        move_str = Util::MoveToString(pv_line[1]);
        sout << " ponder " << move_str;
      }
    }
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }
  }

  // =============== //
  // uciコマンド関数 //
  // =============== //
  // 「uci」コマンドのコールバック関数。
  void UCIShell::CommandUCI(UCICommand::CommandArgs& args) {
    std::ostringstream sout;

    // idを表示。
    sout << "id name " << ID_NAME;
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }

    sout.str("");
    sout << "id author " << ID_AUTHOR;
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }

    // 変更可能オプションの表示。
    // トランスポジションテーブルのサイズの変更。
    sout.str("");
    sout << "option name Hash type spin default "
    << (UCI_DEFAULT_TABLE_SIZE / (1024 * 1024)) << " min "
    << UCI_MIN_TABLE_SIZE / (1024 * 1024) << " max "
    << UCI_MAX_TABLE_SIZE / (1024 * 1024);
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }

    // トランスポジションテーブルの初期化。
    sout.str("");
    sout << "option name Clear Hash type button";
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }

    // ポンダリングできるかどうか。
    sout.str("");
    sout << "option name Ponder type check default ";
    if (UCI_DEFAULT_PONDER) {
      sout << "true";
    } else {
      sout << "false";
    }
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }

    // スレッドの数。
    sout.str("");
    sout << "option name Threads type spin default "
    << UCI_DEFAULT_THREADS << " min " << 1 << " max " << UCI_MAX_THREADS;
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }

    // アナライズモード。
    sout.str("");
    sout << "option name UCI_AnalyseMode type check default ";
    if (UCI_DEFAULT_ANALYSE_MODE) sout << "true";
    else sout << "false";
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func(sout.str());
    }

    // オーケー。
    // 出力関数に送る。
    for (auto& func : output_listeners_) {
      func("uciok");
    }

    // オプションの初期設定。
    engine_ptr_->table().SetSize(UCI_DEFAULT_TABLE_SIZE);
    enable_pondering_ = UCI_DEFAULT_PONDER;
    num_threads_ = UCI_DEFAULT_THREADS;
  }

  // 「isready」コマンドのコールバック関数。
  void UCIShell::CommandIsReady(UCICommand::CommandArgs& args) {
    for (auto& func : output_listeners_) {
      func("readyok");
    }
  }

  // 「setoption」コマンドのコールバック関数。
  void UCIShell::CommandSetOption(UCICommand::CommandArgs& args) {
    // nameとvalueがあるかどうか。
    // なければ設定できない。
    if ((args.find("name") == args.end())
    || (args.find("value") == args.end())) {
      return;
    }

    // nameの文字列をくっつける。
    std::string name_str = "";
    for (unsigned int i = 1; i < args["name"].size(); ++i) {
      name_str += args["name"][i] + " ";
    }
    name_str.pop_back();

    // nameの文字を全部小文字にする。
    for (auto& c : name_str) c = std::tolower(c);

    // nameごとの処理。
    if (name_str == "hash") {
      // トランスポジションテーブルのサイズ変更。
      try {
        std::size_t table_size = Util::GetMax
        (std::stoull(args["value"][1]) * 1024ull * 1024ull,
        UCI_MIN_TABLE_SIZE);

        Util::UpdateMin(table_size, UCI_MAX_TABLE_SIZE);

        engine_ptr_->table().SetSize(table_size);
      } catch (...) {
        // 無視。
      }
    } else if (name_str == "clear hash") {
      // トランスポジションテーブルの初期化。
      engine_ptr_->table().Clear();
    } else if (name_str == "ponder") {
      // ponderの有効化、無効化。
      if (args["value"][1] == "true") enable_pondering_ = true;
      else if (args["value"][1] == "false") enable_pondering_ = false;
    } else if (name_str == "threads") {
      // スレッドの数の変更。
      try {
        num_threads_ = Util::GetMax(std::stol(args["value"][1]), 1);
        Util::UpdateMin(num_threads_, UCI_MAX_THREADS);
      } catch (...) {
        // 無視。
      }
    } else if (name_str == "uci_analysemode") {
      // アナライズモードの有効化、無効化。
      if (args["value"][1] == "true") analyse_mode_ = true;
      else if (args["value"][1] == "false") analyse_mode_ = false;
    }
  }

  // 「ucinewgame」コマンドのコールバック関数。
  void UCIShell::CommandUCINewGame(UCICommand::CommandArgs& args) {
    engine_ptr_->SetNewGame();
    engine_ptr_->table().Clear();
  }

  // 「position」コマンドのコールバック関数。
  void UCIShell::CommandPosition(UCICommand::CommandArgs& args) {
    // startposコマンド。
    if (args.find("startpos") != args.end()) {
      engine_ptr_->SetStartPosition();
    }

    // fenコマンド。
    if (args.find("fen") != args.end()) {
      // fenコマンドをくっつける。
      std::string fen_str = "";
      for (auto& token : args["fen"]) {
        if (token == "fen") continue;
        fen_str += token + " ";
      }
      fen_str.pop_back();

      // エンジンに読み込む。
      engine_ptr_->LoadFEN(FEN(fen_str));
    }

    // movesコマンド。
    if (args.find("moves") != args.end()) {
      // 手を指していく。
      for (auto& token : args["moves"]) {
        if (token == "moves") continue;
        Move move = Util::StringToMove(token);
        if (move) engine_ptr_->PlayMove(move);
      }
    }
  }

  // 「go」コマンドのコールバック関数。
  void UCIShell::CommandGo(UCICommand::CommandArgs& args) {
    // 準備。
    std::uint32_t max_depth = MAX_PLYS;
    std::uint64_t max_nodes = MAX_NODES;
    Chrono::milliseconds thinking_time(INT_MAX);
    bool infinite_thinking = false;
    moves_to_search_.clear();

    // 思考スレッドを終了させる。
    engine_ptr_->StopCalculation();
    try {
      thinking_thread_.join();
    } catch (std::system_error err) {
      // 無視。
    }

    // searchmovesコマンド。
    if (args.find("searchmoves") != args.end()) {
      for (auto& token : args["searchmoves"]) {
        if (token == "searchmoves") continue;
        Move move = Util::StringToMove(token);
        if (move) moves_to_search_.push_back(move);
      }
    }

    // ponderコマンド。
    if (args.find("ponder") != args.end()) {
      infinite_thinking = true;
    }

    // wtimeコマンド。
    if (args.find("wtime") != args.end()) {
      // 10分以上あるなら1分考える。5分未満なら持ち時間の10分の1。
      if (engine_ptr_->to_move() == WHITE) {
        try {
          thinking_time = Chrono::milliseconds
          (TimeLimitToMoveTime(std::stol(args["wtime"][1])));
        } catch (...) {
          // 無視。
        }
      }
    }

    // btimeコマンド。
    if (args.find("btime") != args.end()) {
      // 10分以上あるなら1分考える。5分未満なら持ち時間の10分の1。
      if (engine_ptr_->to_move() == BLACK) {
        try {
          thinking_time = Chrono::milliseconds
          (TimeLimitToMoveTime(std::stol(args["btime"][1])));
        } catch (...) {
          // 無視。
        }
      }
    }

    // wincコマンド。
    if (args.find("winc") != args.end()) {
      // 何もしない。
    }

    // bincコマンド。
    if (args.find("binc") != args.end()) {
      // 何もしない。
    }

    // movestogoコマンド。
    if (args.find("movestogo") != args.end()) {
      // 何もしない。
    }

    // depthコマンド。
    if (args.find("depth") != args.end()) {
      try {
        max_depth = Util::GetMin(std::stol(args["depth"][1]), MAX_PLYS);
      } catch (...) {
        // 無視。
      }
    }

    // nodesコマンド。
    if (args.find("nodes") != args.end()) {
      try {
        max_nodes = Util::GetMin(std::stoull(args["nodes"][1]), MAX_NODES);
      } catch (...) {
        // 無視。
      }
    }

    // mateコマンド。
    if (args.find("mate") != args.end()) {
      try {
        max_depth = Util::GetMin(((std::stol(args["mate"][1]) * 2) - 1),
        MAX_PLYS);
      } catch (...) {
        // 無視。
      }
    }

    // movetimeコマンド。
    if (args.find("movetime") != args.end()) {
      try {
        thinking_time = Chrono::milliseconds(std::stol(args["movetime"][1]));
      } catch (...) {
        // 無視。
      }
    }

    // infiniteコマンド。
    if (args.find("infinite") != args.end()) {
      infinite_thinking = true;
    }

    // 別スレッドで思考開始。
    engine_ptr_->SetStopper(max_depth, max_nodes, thinking_time,
    infinite_thinking);
    thinking_thread_ = std::thread([this]() {this->ThreadThinking();});
  }

  // 「stop」コマンドのコールバック関数。
  void UCIShell::CommandStop(UCICommand::CommandArgs& args) {
    // 思考スレッドを終了させる。
    engine_ptr_->StopCalculation();
    try {
      thinking_thread_.join();
    } catch (std::system_error err) {
      // 無視。
    }
  }

  // 「ponderhit」コマンドのコールバック関数。
  void UCIShell::CommandPonderHit(UCICommand::CommandArgs& args) {
    engine_ptr_->EnableInfiniteThinking(false);
  }

  // ============== //
  // パブリック関数 //
  // ============== //
  // コマンドに対するコールバック関数を登録する。
  void UCICommand::Add(const std::string& command_name,
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

  // コマンドを実行する。
  bool UCICommand::operator()(const std::string& command_line) {
    std::vector<std::string> tokens =
    Util::Split<char>(command_line, {' '}, std::set<char> {});

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
        return true;
      }
    }

    return false;
  }
}  // namespace Sayuri
