/* pgn_parser.cpp: PGNファイルをパースする。
   Copyright (c) 2011 Ishibashi Hironori

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

#include "pgn_parser.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include "chess_def.h"
#include "chess_util.h"
#include "chess_board.h"
#include "move.h"

#include "misaki_debug.h"

namespace Sayuri {
  namespace {
    // トークンに分ける。
    // [引数]
    // file_name: PGNのファイル名。
    // [戻り値]
    // トークンのベクトル。
    // トークンに分ける。
    std::vector<std::string> TokenizePGN(std::string file_name) {
      // ベクトル。
      std::vector<std::string> vec;

      // トークナイザーの定義。
      typedef boost::char_separator<char> separator;
      typedef boost::tokenizer<separator> tokenizer;

      // セパレータを作る。
      separator sep(" \t", "[]{}\".");

      // ファイルを開く。
      std::ifstream fin(file_name.c_str());
      if (!fin) return vec;

      // トークンのベクトルを作る。
      std::string line;
      while (getline(fin, line)) {
        // 空行は無視。
        if (line == "") continue;

        // トークンをベクトルに追加していく。
        tokenizer tok(line, sep);
        tokenizer::iterator itr = tok.begin();
        while (itr != tok.end()) {
          vec.push_back(*itr);
          ++itr;
        }
      }

      // ファイルを閉じる。
      fin.close();

      // ベクトルを返す。
      return vec;
    }
  }

  /*********************
   * PGNの手のクラス。 *
   *********************/
  // 手をパースする。
  void PGNMove::ParseMove(std::string move_str) throw (bool) {
    // 文字が空なら例外。
    if (move_str.length() == 0) throw false;

    // 文字列のインデックスと最終の位置。
    int index = 0;
    const int end_index = move_str.length();
    if (index >= end_index) throw false;

    // 駒の種類をパース。
    piece_type_ = ParsePieceType(move_str.substr(index, 1));
    if (piece_type_ != PAWN) {
      index++;
    } else if (move_str[index + 1] == 'x') {
      piece_mask_ = ChessUtil::FYLE[ParseFyle(move_str.substr(index, 1))];
      index++;
    }
    if (index >= end_index) throw false;

    // xなら無視。
    if (move_str[index] == 'x') index++;
    if (index >= end_index) throw false;

    // 次の文字がファイルかxなら今の文字はpiece_mask。
    if (((move_str[index + 1] >= 'a') && (move_str[index + 1] <= 'h'))
    || (move_str[index + 1] == 'x')) {
      if ((move_str[index] >= 'a') && (move_str[index] <= 'h')) {
        piece_mask_ = ChessUtil::FYLE[ParseFyle(move_str.substr(index, 1))];
      } else {
        piece_mask_ = ChessUtil::RANK[ParseRank(move_str.substr(index, 1))];
      }

      index++;
    }
    if (index >= end_index) throw false;

    // xなら無視。
    if (move_str[index] == 'x') index++;
    if (index >= end_index) throw false;

    // 移動先の位置をパース。
    goal_square_ = ParseSquare(move_str.substr(index, 2));
    index += 2;
    if (index >= end_index) return;

    // =なら昇格。+ならチェック。#ならチェックメイト。
    if (move_str[index] == '=') {
      index++;
      if (index >= end_index) throw false;
      // 昇格する駒が不正なら例外。
      piece_t piece = ParsePieceType(move_str.substr(index, 1));
      if ((piece != KNIGHT) && (piece != BISHOP)
      && (piece != ROOK) && (piece != QUEEN))
        throw false;
      promotion_ = piece;
      index++;
    } else if (move_str[index] == '+') {
      return;
    } else if (move_str[index] == '#') {
      return;
    } else {
      throw false;
    }
  }
  // ファイルをパースする。
  fyle_t PGNMove::ParseFyle(std::string fyle_str) const throw (bool) {
    // 文字数が1文字でなければ例外。
    if (fyle_str.length() != 1) throw false;

    // ファイルの文字を取得する。
    char c = fyle_str[0];
    if ((c < 'a') || (c > 'h')) throw false;

    // ファイルを返す。
    return static_cast<fyle_t>(c - 'a');
  }
  // ランクをパースする。
  rank_t PGNMove::ParseRank(std::string rank_str) const throw (bool) {
    // 文字が1文字でなければ例外。
    if (rank_str.length() != 1) throw false;

    // ランクの文字を取得する。
    char c = rank_str[0];
    if ((c < '1') || (c > '8')) throw false;

    // ランクを返す。
    return static_cast<rank_t>(c - '1');
  }
  // 位置をパースする。
  square_t PGNMove::ParseSquare(std::string square_str) const throw (bool) {
    // 文字が2文字でなければ例外。
    if (square_str.length() != 2) throw false;

    // ファイルとランクを取得する。
    fyle_t fyle = ParseFyle(square_str.substr(0, 1));
    rank_t rank = ParseRank(square_str.substr(1, 1));

    // 手を返す。
    return static_cast<square_t>((rank << 3) | fyle);
  }
  // 駒の種類をパース。
  piece_t PGNMove::ParsePieceType(std::string piece_type_str)
  const throw (bool) {
    // 文字が1文字でなければ例外。
    if (piece_type_str.length() != 1) throw false;

    // 文字を得る。
    char c = piece_type_str[0];

    // 駒の種類を返す。
    if ((c >= 'a') && (c <= 'h')) return PAWN;
    else if (c == 'N') return KNIGHT;
    else if (c == 'B') return BISHOP;
    else if (c == 'R') return ROOK;
    else if (c == 'Q') return QUEEN;
    else if (c == 'K') return KING;
    else throw false;
  }

  /*************************
   * PGNのゲームのクラス。 *
   *************************/
  // 手のリストを作る。
  MoveList* PGNGame::CreateMoveList() const {
    // チェスボードを作る。
    ChessBoard* board = ChessBoard::New();

    // 返すための手のリスト。
    MoveList* ret = MoveList::New();

    // 1手1手調べて合法手ならリストに追加する。
    Move move(A1, A1);
    MoveList* next_list = NULL;
    int move_index = WHITE;
    bool found_move;
    for (int index = 0; index < move_list_ptr_->GetSize(); index++) {
      move_index = -1;

      // 候補手を得る。
      next_list = board->CreateNextMoveList();

      // 該当する手を見つける。
      found_move = false;
      for (int index2 = 0; index2 < next_list->GetSize(); index2++) {
        // 移動先の位置が同じかどうかチェックする。
        if ((*move_list_ptr_)[index].goal_square()
        == (*next_list)[index2].goal_square()) {
          // 駒の種類が同じかどうかチェックする。
          if ((*move_list_ptr_)[index].piece_type()
          == board->GetCurrentGameRecord().
          GetPieceType((*next_list)[index2].piece_square())) {
            // 駒の位置がマスクの範囲にあるかどうかチェックする。
            if ((*move_list_ptr_)[index].piece_mask()
            & ChessUtil::BIT[(*next_list)[index2].piece_square()]) {
              // 昇格する駒の種類をチェックする。
              if ((*move_list_ptr_)[index].promotion()
              == (*next_list)[index2].promotion()) {
                // 手が一致した。
                *ret += (*next_list)[index2];
                found_move = board->TakeMove((*next_list)[index2]);
                break;
              }
            }
          }
        }
      }

      // 候補手を解放する。
      delete next_list;

      // 該当する手が見つからなかったらループを抜ける。
      if (!found_move) break;
    }

    // 解放。
    delete board;

    // 手のリストを返す。
    return ret;
  }
  // タグの値を得る。
  std::string PGNGame::GetTagValue(std::string tag_name) const {
    // 返す文字列。
    std::string str = "";

    // タグを探す。
    for (int index = 0; index < info_list_ptr_->GetSize(); index++) {
      if (tag_name == (*info_list_ptr_)[index].tag()) {
        str += (*info_list_ptr_)[index].value();
        break;
      }
    }

    return str;
  }

  /*******************************
   * PGNのドキュメントのクラス。 *
   *******************************/
  // コンストラクタ。
  PGNDocument::PGNDocument(std::string file_name) throw (bool){
    // ファイルのトークンを得る。
    std::vector<std::string> token_vector = TokenizePGN(file_name);

    // 最初のインデックスと終わりのインデックス。
    int index = 0;
    int end_index = token_vector.size();

    // 最初のコメントを飛ばす。
    while (token_vector[index] != "[") {
      index++;
      if (index >= end_index) return;
    }

    // パース。
    std::string tag_str;
    std::string value_str;
    std::string move_str;
    int game_index = 0;
    while (index < end_index) {
      // 最初は[。
      if (token_vector[index] == "[") {
        // ゲームを作る。
        game_ptr_vector_.push_back(new PGNGame());

        // 情報リストをパース。
        while (token_vector[index] == "[") {
          index++;
          if (index >= end_index) throw false;
          // "までタグ。
          tag_str = "";
          while (token_vector[index] != "\"") {
            tag_str += token_vector[index];
            if (token_vector[index + 1] == ".") {
              tag_str += ".";
              index++;
              if (index >= end_index) throw false;
            } else if (token_vector[index + 1] != "\"") {
              tag_str += " ";
            }
            index++;
            if (index >= end_index) throw false;
          }
          index++;
          if (index >= end_index) throw false;

          // " まで値。
          value_str = "";
          while (token_vector[index] != "\"") {
            value_str += token_vector[index];
            if (token_vector[index + 1] == ".") {
              value_str += ".";
              index++;
              if (index >= end_index) throw false;
            } else if (token_vector[index + 1] != "\"") {
              value_str += " ";
            }
            index++;
            if (index >= end_index) throw false;
          }
          index++;
          if (index >= end_index) throw false;

          // ]まで読み飛ばす。
          while (token_vector[index] != "]") {
            index++;
            if (index >= end_index) throw false;
          }
          index++;
          if (index >= end_index) throw false;

          // 情報を入れる。
          *(game_ptr_vector_[game_index]->info_list_ptr_) +=
          PGNGameInfo(tag_str, value_str);
        }

        // 手のリストを作る。
        while (token_vector[index] != "[") {
          // コメントは無視する。
          if (token_vector[index] == "{") {
            while (token_vector[index] == "}") {
              index++;
              if (index >= end_index) throw false;
            }
            index++;
            if (index >= end_index) return;
          }
          // キャスリングならキャスリングを入れる。
          if ((token_vector[index] == "o-o")
          || (token_vector[index] == "O-O")
          || (token_vector[index] == "0-0")) {  // ショートキャスリング。
            // 奇数で黒の手番。
            // 偶数で白の手番。
            if (game_ptr_vector_[game_index]->move_list_ptr_->GetSize() & 1) {
              *(game_ptr_vector_[game_index]->move_list_ptr_) +=
              PGNMove("Kg8");
            } else {
              *(game_ptr_vector_[game_index]->move_list_ptr_) +=
              PGNMove("Kg1");
            }
            index++;
            if (index >= end_index) return;
            continue;
          } else if ((token_vector[index] == "o-o-o")
          || (token_vector[index] == "O-O-O")
          || (token_vector[index] == "0-0-0")) {  // ロングキャスリング。
            // 奇数で黒の手番。
            // 偶数で白の手番。
            if (game_ptr_vector_[game_index]->move_list_ptr_->GetSize() & 1) {
              *(game_ptr_vector_[game_index]->move_list_ptr_) +=
              PGNMove("Kc8");
            } else {
              *(game_ptr_vector_[game_index]->move_list_ptr_) +=
              PGNMove("Kc1");
            }
            index++;
            if (index >= end_index) return;
            continue;
          }
          // 最初の文字が手なら手を入れる。
          if (((token_vector[index][0] >= 'a')
          && (token_vector[index][0] <= 'h'))
          || (token_vector[index][0] == 'N')
          || (token_vector[index][0] == 'B')
          || (token_vector[index][0] == 'R')
          || (token_vector[index][0] == 'Q')
          || (token_vector[index][0] == 'K')) {
            *(game_ptr_vector_[game_index]->move_list_ptr_) +=
            PGNMove(token_vector[index]);
            index++;
            if (index >= end_index) return;
            continue;
          }
          index++;
          if (index >= end_index) return;
        }

        // ゲームのインデックスを増加。
        game_index++;
      } else {
        throw false;
      }
    }
  }
}  // namespace Sayuri
