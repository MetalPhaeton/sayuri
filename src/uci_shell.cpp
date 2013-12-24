/*
   uci_shell.cpp: UCIのチェスエンジン側インターフェイスの実装。

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

#include "uci_shell.h"

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <iterator>
#include <thread>
#include <chrono>
#include <sstream>
#include <utility>
#include "chess_def.h"
#include "chess_util.h"
#include "pv_line.h"
#include "error.h"

namespace Sayuri {
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  UCIShell::UCIShell(ChessEngine& engine) :
  engine_ptr_(&engine),
  table_ptr_(new TranspositionTable(UCI_MIN_TABLE_SIZE)),
  table_size_(UCI_DEFAULT_TABLE_SIZE),
  enable_pondering_(UCI_DEFAULT_PONDER),
  num_threads_(UCI_DEFAULT_THREADS),
  analyse_mode_(UCI_DEFAULT_ANALYSE_MODE) {}

  // コピーコンストラクタ。
  UCIShell::UCIShell(const UCIShell& shell) :
  engine_ptr_(shell.engine_ptr_),
  table_ptr_(new TranspositionTable(*(shell.table_ptr_))),
  table_size_(shell.table_size_),
  enable_pondering_(shell.enable_pondering_),
  num_threads_(shell.num_threads_),
  analyse_mode_(shell.analyse_mode_) {
    *(moves_to_search_ptr_) = *(shell.moves_to_search_ptr_);
  }

  // ムーブコンストラクタ。
  UCIShell::UCIShell(UCIShell&& shell) :
  engine_ptr_(shell.engine_ptr_),
  table_ptr_(std::move(shell.table_ptr_)),
  table_size_(shell.table_size_),
  enable_pondering_(shell.enable_pondering_),
  num_threads_(shell.num_threads_),
  analyse_mode_(shell.analyse_mode_) {
    moves_to_search_ptr_ = std::move(shell.moves_to_search_ptr_);
  }

  // コピー代入。
  UCIShell& UCIShell::operator=(const UCIShell& shell) {
    engine_ptr_ = shell.engine_ptr_;
    *table_ptr_ = *(shell.table_ptr_);
    table_size_ = shell.table_size_;
    enable_pondering_ = shell.enable_pondering_;
    num_threads_ = shell.num_threads_;
    analyse_mode_ = shell.analyse_mode_;
    *(moves_to_search_ptr_) = *(shell.moves_to_search_ptr_);
    return *this;
  }

  // ムーブ代入。
  UCIShell& UCIShell::operator=(UCIShell&& shell) {
    engine_ptr_ = shell.engine_ptr_;
    table_ptr_ = std::move(shell.table_ptr_);
    table_size_ = shell.table_size_;
    enable_pondering_ = shell.enable_pondering_;
    num_threads_ = shell.num_threads_;
    analyse_mode_ = shell.analyse_mode_;
    moves_to_search_ptr_ = std::move(shell.moves_to_search_ptr_);
    return *this;
  }

  /********************/
  /* パブリック関数。 */
  /********************/
  // エンジンを実行する。
  void UCIShell::Run() {
    // コマンド。
    std::vector<std::string> commands;
    commands.push_back("uci");
    commands.push_back("isready");
    commands.push_back("setoption");
    commands.push_back("ucinewgame");
    commands.push_back("position");
    commands.push_back("go");
    commands.push_back("stop");
    commands.push_back("ponderhit");
    commands.push_back("quit");

    // メインループ。
    bool loop = true;
    while (loop) {
      std::string input;
      std::getline(std::cin, input);
      std::vector<std::string> argv = Util::Split(input, " ", "");
      CommandParser parser(commands, argv);
      parser.JumpToNextKeyword();
      while (parser.HasNext()) {
        Word word = parser.Get();
        if (word.str_ == "uci") {
          CommandUCI();
        } else if (word.str_ == "isready") {
          CommandIsReady();
        } else if (word.str_ == "setoption") {
          CommandSetOption(argv);
        } else if (word.str_ == "ucinewgame") {
          CommandUCINewGame();
        } else if (word.str_ == "position") {
          CommandPosition(argv);
        } else if (word.str_ == "go") {
          CommandGo(argv);
        } else if (word.str_ == "stop") {
          CommandStop();
        } else if (word.str_ == "ponderhit") {
          CommandPonderHit();
        } else if (word.str_ == "quit") {
          CommandStop();
          loop = false;
          break;
        }
        parser.JumpToNextKeyword();
      }
    }
  }

  /****************/
  /* static関数。 */
  /****************/
  // PV情報を標準出力に送る。
  void UCIShell::PrintPVInfo(int depth, int seldepth, int score,
  Chrono::milliseconds time, std::size_t num_nodes, PVLine& pv_line) {
    std::cout << "info";
    std::cout << " depth " << depth;
    std::cout << " seldepth " << seldepth;
    std::cout << " score ";
    if (pv_line.line()[pv_line.length() - 1].has_checkmated()) {
      int winner = (pv_line.length() - 1) % 2;
      if (winner == 1) {
        // エンジンがメイトした。
        std::cout << "mate " << ((pv_line.length() - 1) / 2) + 1;
      } else {
        // エンジンがメイトされた。
        std::cout << "mate -" << (pv_line.length() - 1) / 2;
      }
    } else {
      std::cout << "cp " << score;
    }
    std::cout << " time " << time.count();
    std::cout << " nodes " << num_nodes;
    // PVラインを送る。
    std::cout << " pv";
    for (std::size_t i = 0; i < pv_line.length(); i++) {
      if (pv_line.line()[i].has_checkmated()) break;

      if (!(pv_line.line()[i].move().all_)) break;

      std::cout << " " << TransMoveToString(pv_line.line()[i].move());
    }
    std::cout << std::endl;
  }

  // 深さ情報を標準出力に送る。
  void UCIShell::PrintDepthInfo(int depth) {
    std::cout << "info depth " << depth << std::endl;
  }

  // 現在探索している手の情報を標準出力に送る。
  void UCIShell::PrintCurrentMoveInfo(Move move, int move_num) {
    // 手の情報を送る。
    std::cout << "info currmove " << TransMoveToString(move);

    // 手の番号を送る。
    std::cout << " currmovenumber " << move_num << std::endl;
  }

  // その他の情報を標準出力に送る。
  void UCIShell::PrintOtherInfo(Chrono::milliseconds time,
  std::size_t num_nodes, int hashfull) {
    int time_2 = time.count();
    if (time_2 <= 0) time_2 = 1;
    std::cout << "info time " << time_2;
    std::cout << " nodes " << num_nodes;
    std::cout << " hashfull " << hashfull;
    std::cout << " nps " << (num_nodes * 1000) / time_2 << std::endl;
  }

  // 思考スレッド。
  void UCIShell::ThreadThinking(UCIShell& shell) {
    // アナライズモードならトランスポジションテーブルを初期化。
    if (shell.analyse_mode_) {
      shell.table_ptr_.reset(new TranspositionTable(shell.table_size_));
    }

    // テーブルの年齢の増加。
    shell.table_ptr_->GrowOld();

    // 思考開始。
    PVLine pv_line = shell.engine_ptr_->Calculate (shell.num_threads_,
    *(shell.table_ptr_.get()), shell.moves_to_search_ptr_.get());

    // 最善手を表示。
    std::cout << "bestmove ";
    if ((pv_line.length() > 0)
    && (!(pv_line.line()[0].has_checkmated()))) {
      std::string move_str = TransMoveToString(pv_line.line()[0].move());
      std::cout << move_str;

      // 2手目があるならponderで表示。
      if ((pv_line.length() >= 2)
      && (!(pv_line.line()[1].has_checkmated()))) {
        move_str = TransMoveToString(pv_line.line()[1].move());
        std::cout << " ponder " << move_str;
      }
    }
    std::cout << std::endl;
  }

  /*********************/
  /* UCIコマンド関数。 */
  /*********************/
  // uciコマンド。
  void UCIShell::CommandUCI() {
    // IDを表示。
    std::cout << "id name " << ID_NAME << std::endl;
    std::cout << "id author " << ID_AUTHOR << std::endl;

    // 変更可能オプションの表示。
    // トランスポジションテーブルのサイズの変更。
    std::cout << "option name Hash type spin default "
    << (UCI_DEFAULT_TABLE_SIZE / (1024 * 1024)) << " min "
    << UCI_MIN_TABLE_SIZE / (1024 * 1024) << " max "
    << UCI_MAX_TABLE_SIZE / (1024 * 1024) << std::endl;

    // トランスポジションテーブルの初期化。
    std::cout << "option name Clear Hash type button" << std::endl;

    // ポンダリングできるかどうか。
    std::cout << "option name Ponder type check default ";
    if (UCI_DEFAULT_PONDER) {
      std::cout << "true";
    } else {
      std::cout << "false";
    }
    std::cout << std::endl;

    // スレッドの数。
    std::cout << "option name Threads type spin defalut "
    << UCI_DEFAULT_THREADS << " min " << 1 << " max "
    << UCI_MAX_THREADS << std::endl;

    // アナライズモード。
    std::cout << "option name UCI_AnalyseMode type check default ";
    if (UCI_DEFAULT_ANALYSE_MODE) std::cout << "true";
    else std::cout << "false";
    std::cout << std::endl;

    // オーケー。
    std::cout << "uciok" << std::endl;

    // オプションの初期設定。
    table_size_ = UCI_DEFAULT_TABLE_SIZE;
    table_ptr_.reset(new TranspositionTable(table_size_));
    enable_pondering_ = UCI_DEFAULT_PONDER;
    num_threads_ = UCI_DEFAULT_THREADS;

  }

  // isreadyコマンド。
  void UCIShell::CommandIsReady() {
    std::cout << "readyok" << std::endl;
  }

  // setoptionコマンド。
  void UCIShell::CommandSetOption(const std::vector<std::string>& argv) {
    // 全部小文字にしておく。
    std::vector<std::string> argv2 = argv;
    constexpr char DELTA = 'A' - 'a';
    for (auto& a : argv2) {
      for (auto& b : a) {
        if ((b >= 'A') && (b <= 'Z')) {
          b -= DELTA;
        }
      }
    }

    // サブコマンド。
    std::vector<std::string> sub_commands;
    sub_commands.push_back("name");
    sub_commands.push_back("value");

    // パーサ。
    CommandParser parser(sub_commands, argv2);

    while (parser.HasNext()) {
      Word word = parser.Get();

      if (word.str_ == "name") {
        // nameコマンド。
        while (!(parser.IsDelim())) {
          word = parser.Get();
          if (word.str_ == "hash") {
            // ハッシュ値の変更の場合。
            parser.JumpToNextKeyword();
            while (parser.HasNext()) {
              word = parser.Get();
              if (word.str_ == "value") {
                try {
                  table_size_ =
                  std::stoull(parser.Get().str_) * 1024ULL * 1024ULL;

                  table_size_ = table_size_ >= UCI_MIN_TABLE_SIZE
                  ? table_size_ : UCI_MIN_TABLE_SIZE;

                  table_size_ = table_size_ <= UCI_MAX_TABLE_SIZE
                  ? table_size_ : UCI_MAX_TABLE_SIZE;

                  table_ptr_.reset(new TranspositionTable(table_size_));
                } catch (...) {
                  // 無視。
                }
                break;
              }

              parser.JumpToNextKeyword();
            }
            break;
          } else if (word.str_ == "clear") {
            // トランスポジションテーブルの初期化の場合。
            word = parser.Get();
            if (word.str_ == "hash") {
              table_ptr_.reset(new TranspositionTable(table_size_));
            }
            break;
          } else if (word.str_ == "ponder") {
            // ポンダリングの設定の場合。
            parser.JumpToNextKeyword();
            while (parser.HasNext()) {
              word = parser.Get();
              if (word.str_ == "value") {
                word = parser.Get();
                if (word.str_ == "true") {
                  enable_pondering_ = true;
                } else if (word.str_ == "false") {
                  enable_pondering_ = false;
                }
                break;
              }

              parser.JumpToNextKeyword();
            }
            break;
          } else if (word.str_ == "threads") {
            // スレッドの数の変更の場合。
            parser.JumpToNextKeyword();
            while (parser.HasNext()) {
              word = parser.Get();
              if (word.str_ == "value") {
                try {
                  num_threads_ = std::stoi(parser.Get().str_);

                  num_threads_ = num_threads_ >= 1 ? num_threads_ : 1;
                  num_threads_ = num_threads_ <= UCI_MAX_THREADS
                  ? num_threads_ : UCI_MAX_THREADS;
                } catch (...) {
                  // 無視。
                }
                break;
              }

              parser.JumpToNextKeyword();
            }
            break;
          } else if (word.str_ == "uci_analysemode") {
            // アナライズモードの場合。
            parser.JumpToNextKeyword();
            while (parser.HasNext()) {
              word = parser.Get();
              if (word.str_ == "value") {
                word = parser.Get();
                if (word.str_ == "true") {
                  analyse_mode_ = true;
                } else if (word.str_ == "false") {
                  analyse_mode_ = false;
                }
                break;
              }

              parser.JumpToNextKeyword();
            }
            break;
          }
        }
      }

      parser.JumpToNextKeyword();
    }
  }

  // ucinewgameコマンド。
  void UCIShell::CommandUCINewGame() {
    engine_ptr_->SetNewGame();
    table_ptr_.reset(new TranspositionTable(table_size_));
  }

  // positionコマンド。
  void UCIShell::CommandPosition(const std::vector<std::string>& argv) {
    // サブコマンド。
    std::vector<std::string> sub_commands;
    sub_commands.push_back("startpos");
    sub_commands.push_back("fen");
    sub_commands.push_back("moves");

    // パーサ。
    CommandParser parser(sub_commands, argv);

    while (parser.HasNext()) {
      Word word = parser.Get();

      if (word.str_ == "startpos") {
        // startposコマンド。
        engine_ptr_->SetNewGame();
      } else if (word.str_ == "fen") {
        // fenコマンド。
        // FENを合体。
        std::string fen_str = "";
        while (!(parser.IsDelim())) {
          fen_str += parser.Get().str_ + " ";
        }
        // エンジンにロード。
        engine_ptr_->LoadFen(Fen(fen_str));
      } else if (word.str_ == "moves") {
        // moveコマンド。
        while (!(parser.IsDelim())) {
          Move move = TransStringToMove(parser.Get().str_);
          // 手を指す。
          if (move.all_) {
            engine_ptr_->PlayMove(move);
          }
        }
      }

      parser.JumpToNextKeyword();
    }
  }

  // goコマンド。
  void UCIShell::CommandGo(const std::vector<std::string>& argv) {
    // 思考スレッドを終了させる。
    engine_ptr_->StopCalculation();
    if (thinking_thread_.joinable()) {
      thinking_thread_.join();
    }

    // サブコマンドの配列。
    std::vector<std::string> sub_commands;
    sub_commands.push_back("searchmoves");
    sub_commands.push_back("ponder");
    sub_commands.push_back("wtime");
    sub_commands.push_back("btime");
    sub_commands.push_back("winc");
    sub_commands.push_back("binc");
    sub_commands.push_back("movestogo");
    sub_commands.push_back("depth");
    sub_commands.push_back("nodes");
    sub_commands.push_back("mate");
    sub_commands.push_back("movetime");
    sub_commands.push_back("infinite");

    // パース。
    CommandParser parser(sub_commands, argv);

    // 準備。
    int max_depth = MAX_PLYS;
    std::size_t max_nodes = MAX_NODES;
    Chrono::milliseconds thinking_time(-1U >> 1);
    bool infinite_thinking = false;
    moves_to_search_ptr_.reset(nullptr);

    // サブコマンドを解析。
    while (parser.HasNext()) {
      Word word = parser.Get();

      if (word.str_ == "searchmoves") {
        if (parser.IsDelim()) continue;

        // searchmovesコマンド。
        while (!(parser.IsDelim())) {
          Move move = TransStringToMove(parser.Get().str_);
          if (move.all_) {
            if (moves_to_search_ptr_ == nullptr) {
              moves_to_search_ptr_.reset(new std::vector<Move>());
            }
            moves_to_search_ptr_->push_back(move);
          } else {
            break;
          }
        }
      } else if (word.str_ == "ponder") {
        // ponderコマンド。
        infinite_thinking = true;
      } else if (word.str_ == "wtime") {
        // wtimeコマンド。
        // 5分以上あるなら1分考える。5分未満なら持ち時間の5分の1。
        if (!(parser.IsDelim())) {
          if (engine_ptr_->to_move() == WHITE) {
            try {
              Chrono::milliseconds time_control =
              Chrono::milliseconds(std::stoull(parser.Get().str_));
              if (time_control.count() >= 300000) {
                thinking_time = Chrono::milliseconds(60000);
              } else {
                thinking_time = time_control / 5;
              }
            } catch (...) {
              // 無視。
            }
          }
        }
      } else if (word.str_ == "btime") {
        // btimeコマンド。
        // 5分以上あるなら1分考える。5分未満なら持ち時間の5分の1。
        if (!(parser.IsDelim())) {
          if (engine_ptr_->to_move() == BLACK) {
            try {
              Chrono::milliseconds time_control =
              Chrono::milliseconds(std::stoull(parser.Get().str_));
              if (time_control.count() >= 300000) {
                thinking_time = Chrono::milliseconds(60000);
              } else {
                thinking_time = time_control / 5;
              }
            } catch (...) {
              // 無視。
            }
          }
        }
      } else if (word.str_ == "depth") {
        // depthコマンド。
        if (!(parser.IsDelim())) {
          try {
            max_depth = std::stoi(parser.Get().str_);
            if (max_depth > MAX_PLYS) max_depth = MAX_PLYS;
          } catch (...) {
            // 無視。
          }
        }
      } else if (word.str_ == "nodes") {
        // nodesコマンド。
        if (!(parser.IsDelim())) {
          try {
            max_nodes = std::stoull(parser.Get().str_);
            if (max_nodes > MAX_NODES) max_nodes = MAX_NODES;
          } catch (...) {
            // 無視。
          }
        }
      } else if (word.str_ == "mate") {
        // mateコマンド。
        if (!(parser.IsDelim())) {
          try {
            max_depth = (std::stoi(parser.Get().str_) * 2) - 1;
            if (max_depth > MAX_PLYS) max_depth = MAX_PLYS;
          } catch (...) {
            // 無視。
          }
        }
      } else if (word.str_ == "movetime") {
        // movetimeコマンド。
        if (!(parser.IsDelim())) {
          try {
            thinking_time =
            Chrono::milliseconds(std::stoull(parser.Get().str_));
          } catch (...) {
            // 無視。
          }
        }
      } else if (word.str_ == "infinite") {
        // infiniteコマンド。
        infinite_thinking = true;
      }

      parser.JumpToNextKeyword();
    }

    // 別スレッドで思考開始。
    engine_ptr_->SetStopper(max_depth, max_nodes, thinking_time,
    infinite_thinking);
    thinking_thread_ = std::thread(ThreadThinking, std::ref(*this));
  }

  // stopコマンド。
  void UCIShell::CommandStop() {
    // 思考スレッドを終了させる。
    if (thinking_thread_.joinable()) {
      engine_ptr_->StopCalculation();
      thinking_thread_.join();
    }
  }

  // ponderhitコマンド。
  void UCIShell::CommandPonderHit() {
    engine_ptr_->EnableInfiniteThinking(false);
  }

  /**************/
  /* 便利関数。 */
  /**************/
  // 手を文字列に変換。
  std::string UCIShell::TransMoveToString(Move move) {
    // 文字列ストリーム。
    std::ostringstream oss;

    // ストリームに流しこむ。
    oss << static_cast<char>(Util::GetFyle(move.from_) + 'a');
    oss << static_cast<char>(Util::GetRank(move.from_) + '1');
    oss << static_cast<char>(Util::GetFyle(move.to_) + 'a');
    oss << static_cast<char>(Util::GetRank(move.to_) + '1');
    switch (move.promotion_) {
      case KNIGHT:
        oss << 'n';
        break;
      case BISHOP:
        oss << 'b';
        break;
      case ROOK:
        oss << 'r';
        break;
      case QUEEN:
        oss << 'q';
        break;
      default:
        break;
    }

    return oss.str();
  }

  // 文字列を手に変換。
  Move UCIShell::TransStringToMove(std::string move_str) {
    Move move, null_move;

    if (move_str.size() < 4) return null_move;

    // fromをパース。
    if ((move_str[0] < 'a') || (move_str[0] > 'h')) {
      return null_move;
    }
    int fyle = move_str[0] - 'a';
    move.from_ |= fyle;
    if ((move_str[1] < '1') || (move_str[1] > '8')) {
      return null_move;
    }
    int rank = move_str[1] - '1';
    move.from_ |= (rank << 3);

    // toをパース。
    if ((move_str[2] < 'a') || (move_str[2] > 'h')) {
      return null_move;
    }
    fyle = move_str[2] - 'a';
    move.to_ |= fyle;
    if ((move_str[3] < '1') || (move_str[3] > '8')) {
      return null_move;
    }
    rank = move_str[3] - '1';
    move.to_ |= (rank << 3);

    // 昇格をパース。
    if (move_str.size() >= 5) {
      switch (move_str[4]) {
        case 'n':
          move.promotion_ = KNIGHT;
          break;
        case 'b':
          move.promotion_ = BISHOP;
          break;
        case 'r':
          move.promotion_ = ROOK;
          break;
        case 'q':
          move.promotion_ = QUEEN;
          break;
        default:
          break;
      }
    }

    return move;
  }

  /*************************/
  /* UCIコマンドのパーサ。 */
  /*************************/
  // コンストラクタ。
  UCIShell::CommandParser::CommandParser
  (const std::vector<std::string>& keywords,
  const std::vector<std::string>& argv) {
    // 構文解析。
    for (auto& a : argv) {
      bool is_keyword = false;

      // キーワードかどうか調べる。
      for (auto& b : keywords) {
        if (a == b) {
          is_keyword = true;
          break;
        }
      }

      // 構文リストに入れる。
      if (is_keyword) {
        syntax_vector_.push_back(Word("", WordType::DELIM));
        syntax_vector_.push_back(Word(a, WordType::KEYWORD));
      } else {
        syntax_vector_.push_back(Word(a, WordType::PARAM));
      }
    }

    // 最後に区切りを入れる。
    syntax_vector_.push_back(Word("", WordType::DELIM));

    // イテレータをセット。
    itr_ = syntax_vector_.begin();
  }

  // コピーコンストラクタ。
  UCIShell::CommandParser::CommandParser(const CommandParser& parser) {
    syntax_vector_ = parser.syntax_vector_;
    itr_ = syntax_vector_.begin()
    + std::distance(parser.syntax_vector_.begin(), parser.itr_);
  }

  // ムーブコンストラクタ。
  UCIShell::CommandParser::CommandParser(CommandParser&& parser) {
    syntax_vector_ = std::move(parser.syntax_vector_);
    itr_ = std::move(parser.itr_);
  }

  // コピー代入。
  UCIShell::CommandParser&
  UCIShell::CommandParser::operator=(const CommandParser& parser) {
    syntax_vector_ = parser.syntax_vector_;
    itr_ = syntax_vector_.begin()
    + std::distance(parser.syntax_vector_.begin(), parser.itr_);
    return *this;
  }

  // ムーブ代入。
  UCIShell::CommandParser&
  UCIShell::CommandParser::operator=(CommandParser&& parser) {
    syntax_vector_ = std::move(parser.syntax_vector_);
    itr_ = std::move(parser.itr_);
    return *this;
  }

  // 単語を得る。
  const UCIShell::Word& UCIShell::CommandParser::Get() {
    itr_++;
    return *(itr_ - 1);
  }

  // 単語があるかどうか。
  bool UCIShell::CommandParser::HasNext() const {
    return itr_ < syntax_vector_.end();
  }

  // 区切りかどうか。
  bool UCIShell::CommandParser::IsDelim() const {
    return itr_->type_ == WordType::DELIM;
  }

  // 次のキーワードへジャンプ。
  void UCIShell::CommandParser::JumpToNextKeyword() {
    itr_++;
    while (itr_ < syntax_vector_.end()) {
      if (itr_->type_ == WordType::KEYWORD) {
        return;
      }
      itr_++;
    }
  }

  // 冒頭にリセット。
  void UCIShell::CommandParser::Reset() {
    itr_ = syntax_vector_.begin();
  }
}  // namespace Sayuri
