/* main.cpp: チェスアプリのメイン。
   copyright (c) 2011 石橋宏之利
 */

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <boost/thread.hpp>
#include <boost/ref.hpp>
#include <boost/random.hpp>

#include "misaki.h"

class App {
  public:
    /**********************************
     * コンストラクタとデストラクタ。 *
     **********************************/
    App() : player_side_(Misaki::WHITE),
    now_thinking_thr_(NULL),
    is_now_thinking_(false) {
      board_ = Misaki::ChessBoard::New();
    }
    virtual ~App() {
      delete board_;
    }

    /**********
     * 関数。 *
     **********/
    // アプリの実行。
    void Run();

  private:
    /******************
     * コピーの禁止。 *
     ******************/
    App(const App&);  // 削除。
    App& operator=(const App&);  // 削除。

    /****************
     * メンバ変数。 *
     ****************/
    // チェスエンジン。
    Misaki::ChessBoard* board_;
    // プレイヤーのサイド。
    Misaki::side_t player_side_;
    // 考え中スレッド。
    boost::thread* now_thinking_thr_;
    // 考え中フラグ。
    bool is_now_thinking_;

    /**********************
     * プライベート関数。 *
     **********************/
    // タイトルを表示する。
    void PrintTitle() const;
    // どちらのサイドで遊ぶかを入力させる。
    void InputPlayerSide();
    // 考え中を表示する。
    void PrintNowThinking();
    // 考え中を開始。
    void StartNowThinking() {
      if (is_now_thinking_) return;

      is_now_thinking_ = true;
      now_thinking_thr_ =
      new boost::thread(&App::PrintNowThinking, boost::ref(*this));
    }
    // 考え中を中止。
    void StopNowThinking() {
      is_now_thinking_ = false;

      if (now_thinking_thr_) {
        now_thinking_thr_->join();
        delete now_thinking_thr_;
      }
    }
    // ボードを表示する。
    // [引数]
    // first_board: 最初の局面かどうか。
    void PrintBoard(bool first_board) const;
    // 位置の文字列をパースする。
    // [引数]
    // square_str: 位置を表す文字列。
    // [戻り値]
    // 位置。
    // [例外]
    // パースできなかったらfale。
    Misaki::square_t ParseSquare(std::string square_str) const throw (bool);
    // 手をパースする。
    // [引数]
    // move_str: 手を表す文字列。
    // [戻り値]
    // 手。
    // [例外]
    // パースできなかったらfale。
    Misaki::Move ParseMove(std::string move_str) const throw (bool);
    // ゲーム終了かどうかチェックする。
    // [戻り値]
    // ゲーム終了ならtrue。
    bool IsGameOver() const;
};

void App::Run() {
  // 乱数を初期化。
  typedef boost::mt19937 mt;
  typedef boost::uniform_int<> uniform;
  typedef boost::variate_generator<mt&, uniform> generator;
  mt gen(static_cast<unsigned long>(time(NULL)));

  // オープニングブックを読み込む。
  Misaki::OpeningBook* book = Misaki::OpeningBook::New();
  std::ifstream fin("book.csv");
  if (fin) {
    std::string line;
    while (std::getline(fin, line)) {
      try {
        *book += Misaki::Opening(line);
      } catch (...) {
        continue;
      }
    }
    fin.close();
  }
  Misaki::MoveList* opening_list = NULL;
  bool opening_result;

  // タイトルを表示。
  PrintTitle();

  // どちらのサイドで遊ぶか入力を求める。
  InputPlayerSide();

  // 手。
  Misaki::Move player_move(Misaki::A1, Misaki::A1);
  Misaki::Move cpu_move(Misaki::A1, Misaki::A1);

  // 思考の設定。
  Misaki::TranspositionTable* table = NULL;  // トランスポジションテーブル。
  Misaki::EvalWeights weights;  // 評価の重み。
  double searching_time = 10.0;  // 探索時間。

  // 最初の局面を表示。
  if (player_side_ == Misaki::WHITE) {
    PrintBoard(true);
  } else {
    opening_list = book->CreateNextMoveList(board_->GetCurrentGameRecord());
    opening_result = false;
    if (opening_list->GetSize()) {
      uniform dist(0, opening_list->GetSize() - 1);
      generator rand(gen, dist);
      opening_result = board_->TakeMove((*opening_list)[rand()]);
    }
    delete opening_list;
    if (!opening_result) {
      StartNowThinking();
      table = Misaki::TranspositionTable::New();
      cpu_move = board_->GetBestMove(searching_time, *table, weights);
      delete table;
      StopNowThinking();
      board_->TakeMove(cpu_move);
    }
    PrintBoard(false);
  }

  // メインループ。
  std::string input;
  while (true) {
    // Ponderingを開始する。
    table = Misaki::TranspositionTable::New();
    board_->StartPondering(5, *table, weights);

    // 入力を求める。
    std::cout << "Input Command. (\"q\" to quit.)" << std::endl;
    std::cout << "-->";
    std::getline(std::cin, input);

    // Ponderingを終了する。
    board_->StopPondering();

    // 終了ならループを抜ける。
    if (input == "q") {
      delete table;
      break;
    }

    // 手をパース。
    try {
      player_move = ParseMove(input);
    } catch (...) {
      std::cout << "I couldn't parse your move..." << std::endl;
      continue;
    }

    // 手をさす。
    if (!(board_->TakeMove(player_move))) {
      std::cout << "Your move is not legal move..." << std::endl;
      continue;
    }

    // ゲーム終了かどうかチェックする。
    if (IsGameOver()) {
      PrintBoard(false);
      delete table;
      break;
    }

    // 思考する。
    opening_list = book->CreateNextMoveList(board_->GetCurrentGameRecord());
    opening_result = false;
    if (opening_list->GetSize()) {
      uniform dist(0, opening_list->GetSize() - 1);
      generator rand(gen, dist);
      opening_result = board_->TakeMove((*opening_list)[rand()]);
    }
    if (!opening_result) {
      StartNowThinking();
      cpu_move = board_->GetBestMove(searching_time, *table, weights);
      StopNowThinking();
      board_->TakeMove(cpu_move);
    }
    delete opening_list;
    delete table;

    // ボードを表示する。
    PrintBoard(false);

    // ゲーム終了かどうかチェックする。
    if (IsGameOver()) {
      break;
    }
  }

  // ゲーム終了。
  std::cout << "Goobye!" << std::endl;
}

void App::PrintTitle() const {
  std::cout << "***********************" << std::endl;
  std::cout << "* Welcome to Misaki!! *" << std::endl;
  std::cout << "***********************" << std::endl;
  std::cout << std::endl;
}
void App::InputPlayerSide() {
  std::cout << "Which side do you want to play with?" << std::endl;
  std::cout << "\"b\" is Black, else White." << std::endl;
  std::cout << "-->";
  std::string input;
  std::getline(std::cin, input);
  if (input == "b") player_side_ = Misaki::BLACK;
  else player_side_ = Misaki::WHITE;
}
void App::PrintBoard(bool first_move) const {
  // 現在のゲームを得る。
  const Misaki::GameRecord& record = board_->GetCurrentGameRecord();

  // 文字のボード。
  char string_board[Misaki::NUM_SQUARES][4];

  // 文字のボードに駒の種類をセットする。
  for (int square = 0; square < Misaki::NUM_SQUARES; square++) {
    string_board[square][3] = '\0';
    switch (record.GetPieceType(static_cast<Misaki::square_t>(square))) {
      case Misaki::PAWN:
        string_board[square][1] = 'P';
        break;
      case Misaki::KNIGHT:
        string_board[square][1] = 'N';
        break;
      case Misaki::BISHOP:
        string_board[square][1] = 'B';
        break;
      case Misaki::ROOK:
        string_board[square][1] = 'R';
        break;
      case Misaki::QUEEN:
        string_board[square][1] = 'Q';
        break;
      case Misaki::KING:
        string_board[square][1] = 'K';
        break;
      default:
        string_board[square][1] = ' ';
        break;
    }
    switch (record.GetSide(static_cast<Misaki::square_t>(square))) {
      case Misaki::WHITE:
        string_board[square][0] = '-';
        string_board[square][2] = '-';
        break;
      case Misaki::BLACK:
        string_board[square][0] = '<';
        string_board[square][2] = '>';
        break;
      default:
        string_board[square][0] = ' ';
        string_board[square][2] = ' ';
        break;
    }
  }

  // ファイルとランクの文字。
  static const char fyle_array[Misaki::NUM_FYLES] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
  };
  static const char rank_array[Misaki::NUM_RANKS] = {
    '1', '2', '3', '4', '5', '6', '7', '8'
  };

  // 駒の文字。
  static const char piece_array[Misaki::NUM_PIECE_TYPES] = {
    ' ', 'P', 'N', 'B', 'R', 'Q', 'K'
  };

  // ボードの境界線。
  const char* border = " +---+---+---+---+---+---+---+---+";

  // 1行目を出力。
  std::cout << border;
  std::cout << "  Last Move" << std::endl;

  // 2行目を出力。
  if (player_side_ == Misaki::WHITE) {
    std::cout << "8|";
    for (int i = 56; i <= 63; i++) {
      std::cout << string_board[i] << "|";
    }
  } else {
    std::cout << "1|";
    for (int i = 7; i >= 0; i--) {
      std::cout << string_board[i] << "|";
    }
  }
  Misaki::Move last_move = record.last_move();
  if (!first_move) {
    std::cout << "  ";
    Misaki::fyle_t fyle =
    Misaki::ChessUtil::GetFyle(last_move.piece_square());
    Misaki::rank_t rank =
    Misaki::ChessUtil::GetRank(last_move.piece_square());
    std::cout << fyle_array[fyle];
    std::cout << rank_array[rank];
    fyle = Misaki::ChessUtil::GetFyle(last_move.goal_square());
    rank = Misaki::ChessUtil::GetRank(last_move.goal_square());
    std::cout << fyle_array[fyle];
    std::cout << rank_array[rank];
    std::cout << piece_array[last_move.promotion()];
  }
  std::cout << std::endl;

  // 3行目を出力。
  std::cout << border << std::endl;

  // 4行目を出力。
  if (player_side_ == Misaki::WHITE) {
    std::cout << "7|";
    for (int i = 48; i <= 55; i++) {
      std::cout << string_board[i] << "|";
    }
  } else {
    std::cout << "2|";
    for (int i = 15; i >= 8; i--) {
      std::cout << string_board[i] << "|";
    }
  }
  std::cout << std::endl;

  // 5行目を出力。
  std::cout << border << std::endl;

  // 6行目を出力。
  if (player_side_ == Misaki::WHITE) {
    std::cout << "6|";
    for (int i = 40; i <= 47; i++) {
      std::cout << string_board[i] << "|";
    }
  } else {
    std::cout << "3|";
    for (int i = 23; i >= 16; i--) {
      std::cout << string_board[i] << "|";
    }
  }
  std::cout << std::endl;

  // 7行目を出力。
  std::cout << border << std::endl;

  // 8行目を出力。
  if (player_side_ == Misaki::WHITE) {
    std::cout << "5|";
    for (int i = 32; i <= 39; i++) {
      std::cout << string_board[i] << "|";
    }
  } else {
    std::cout << "4|";
    for (int i = 31; i >= 24; i--) {
      std::cout << string_board[i] << "|";
    }
  }
  std::cout << std::endl;

  // 9行目を出力。
  std::cout << border << std::endl;

  // 10行目を出力。
  if (player_side_ == Misaki::WHITE) {
    std::cout << "4|";
    for (int i = 24; i <= 31; i++) {
      std::cout << string_board[i] << "|";
    }
  } else {
    std::cout << "5|";
    for (int i = 39; i >= 32; i--) {
      std::cout << string_board[i] << "|";
    }
  }
  std::cout << std::endl;

  // 11行目を出力。
  std::cout << border << std::endl;

  // 12行目を出力。
  if (player_side_ == Misaki::WHITE) {
    std::cout << "3|";
    for (int i = 16; i <= 23; i++) {
      std::cout << string_board[i] << "|";
    }
  } else {
    std::cout << "6|";
    for (int i = 47; i >= 40; i--) {
      std::cout << string_board[i] << "|";
    }
  }
  std::cout << std::endl;

  // 13行目を出力。
  std::cout << border << std::endl;

  // 14行目を出力。
  if (player_side_ == Misaki::WHITE) {
    std::cout << "2|";
    for (int i = 8; i <= 15; i++) {
      std::cout << string_board[i] << "|";
    }
  } else {
    std::cout << "7|";
    for (int i = 55; i >= 48; i--) {
      std::cout << string_board[i] << "|";
    }
  }
  std::cout << std::endl;

  // 15行目を出力。
  std::cout << border << std::endl;

  // 16行目を出力。
  if (player_side_ == Misaki::WHITE) {
    std::cout << "1|";
    for (int i = 0; i <= 7; i++) {
      std::cout << string_board[i] << "|";
    }
  } else {
    std::cout << "8|";
    for (int i = 63; i >= 56; i--) {
      std::cout << string_board[i] << "|";
    }
  }
  std::cout << std::endl;

  // 17行目を出力。
  std::cout << border << std::endl;

  // 18行目を出力。
  if (player_side_ == Misaki::WHITE) {
    std::cout << "   a   b   c   d   e   f   g   h" << std::endl;
  } else {
    std::cout << "   h   g   f   e   d   c   b   a" << std::endl;
  }
}
void App::PrintNowThinking() {
  std::cout << "Now Thinking..." << std::flush;
  int frame = 0;
  while (true) {
    if (!is_now_thinking_) break;

    switch (frame) {
      case 0:
        std::cout << "|" << std::flush;
        frame = 1;
        break;
      case 1:
        std::cout << "/" << std::flush;
        frame = 2;
        break;
      case 2:
        std::cout << "-" << std::flush;
        frame = 3;
        break;
      case 3:
        std::cout << "\\" << std::flush;
        frame = 0;
        break;
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    std::cout << "\b" << std::flush;
  }

  std::cout << std::endl;
}
Misaki::square_t App::ParseSquare(std::string square_str) const throw (bool) {
  // ファイルをパース。
  Misaki::fyle_t fyle;
  if (square_str[0] == 'a') {
    fyle = Misaki::FYLE_A;
  } else if (square_str[0] == 'b') {
    fyle = Misaki::FYLE_B;
  } else if (square_str[0] == 'c') {
    fyle = Misaki::FYLE_C;
  } else if (square_str[0] == 'd') {
    fyle = Misaki::FYLE_D;
  } else if (square_str[0] == 'e') {
    fyle = Misaki::FYLE_E;
  } else if (square_str[0] == 'f') {
    fyle = Misaki::FYLE_F;
  } else if (square_str[0] == 'g') {
    fyle = Misaki::FYLE_G;
  } else if (square_str[0] == 'h') {
    fyle = Misaki::FYLE_H;
  } else {  // パースできなかったら例外。
    throw false;
  }

  // ランクをパース。
  Misaki::rank_t rank;
  if (square_str[1] == '1') {
    rank = Misaki::RANK_1;
  } else if (square_str[1] == '2') {
    rank = Misaki::RANK_2;
  } else if (square_str[1] == '3') {
    rank = Misaki::RANK_3;
  } else if (square_str[1] == '4') {
    rank = Misaki::RANK_4;
  } else if (square_str[1] == '5') {
    rank = Misaki::RANK_5;
  } else if (square_str[1] == '6') {
    rank = Misaki::RANK_6;
  } else if (square_str[1] == '7') {
    rank = Misaki::RANK_7;
  } else if (square_str[1] == '8') {  // パースできなかったら例外。
    rank = Misaki::RANK_8;
  } else {
    throw false;
  }

  // 位置の配列。
  static const Misaki::square_t square_array
  [Misaki::NUM_FYLES][Misaki::NUM_RANKS] = {
    {Misaki::A1, Misaki::A2, Misaki::A3, Misaki::A4,
      Misaki::A5, Misaki::A6, Misaki::A7, Misaki::A8},
    {Misaki::B1, Misaki::B2, Misaki::B3, Misaki::B4,
      Misaki::B5, Misaki::B6, Misaki::B7, Misaki::B8},
    {Misaki::C1, Misaki::C2, Misaki::C3, Misaki::C4,
      Misaki::C5, Misaki::C6, Misaki::C7, Misaki::C8},
    {Misaki::D1, Misaki::D2, Misaki::D3, Misaki::D4,
      Misaki::D5, Misaki::D6, Misaki::D7, Misaki::D8},
    {Misaki::E1, Misaki::E2, Misaki::E3, Misaki::E4,
      Misaki::E5, Misaki::E6, Misaki::E7, Misaki::E8},
    {Misaki::F1, Misaki::F2, Misaki::F3, Misaki::F4,
      Misaki::F5, Misaki::F6, Misaki::F7, Misaki::F8},
    {Misaki::G1, Misaki::G2, Misaki::G3, Misaki::G4,
      Misaki::G5, Misaki::G6, Misaki::G7, Misaki::G8},
    {Misaki::H1, Misaki::H2, Misaki::H3, Misaki::H4,
      Misaki::H5, Misaki::H6, Misaki::H7, Misaki::H8}
  };

  // 位置を返す。
  return square_array[fyle][rank];
}
Misaki::Move App::ParseMove(std::string move_str) const throw (bool) {
  // 文字の長さをチェック。
  if ((move_str.length() != 4) && (move_str.length() != 5))
    throw false;

  // 駒の位置をパース。
  Misaki::square_t piece_square = ParseSquare(move_str.substr(0, 2));

  // 移動先の位置をパース。
  Misaki::square_t goal_square = ParseSquare(move_str.substr(2, 2));

  // 昇格する駒の種類をパース。
  Misaki::piece_t promotion = Misaki::EMPTY;
  if (move_str.length() == 5) {
    if (move_str[4] == 'N') {
      promotion = Misaki::KNIGHT;
    } else if (move_str[4] == 'B') {
      promotion = Misaki::BISHOP;
    } else if (move_str[4] == 'R') {
      promotion = Misaki::ROOK;
    } else if (move_str[4] == 'Q') {
      promotion = Misaki::QUEEN;
    } else {  // パースできないので例外。
      throw false;
    }
  }

  return Misaki::Move(piece_square, goal_square, promotion);
}
bool App::IsGameOver() const {
  // ゲームレコードを得る。
  const Misaki::GameRecord& record = board_->GetCurrentGameRecord();

  // 千日手。
  if (record.repetition() >= 3) {
    std::cout << "Draw by 3 times repetition!" << std::endl;
    return true;
  }

  // 50手ルール。
  if (record.ply_100() >= 100) {
    std::cout << "Draw by 50 moves!" << std::endl;
    return true;
  }

  // ステールメイト。
  if (board_->IsStalemated()) {
    std::cout << "Draw by stalemate!" << std::endl;
    return true;
  }

  // チェックメイト。
  if (board_->IsCheckmated()) {
    if (player_side_ == record.to_move()) {
      std::cout << "You lose..." << std::endl;
      return true;
    } else {
      std::cout << "You win!!" << std::endl;
      return true;
    }
  }

  return false;
}

int main(int argc, char* argv[]) {
  // Misakiを初期化。
  Misaki::Init();

  App app;
  app.Run();

  return 0;
}
