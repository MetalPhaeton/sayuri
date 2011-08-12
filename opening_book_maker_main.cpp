/* opening_book_maker.cpp: オープニングブックを作る。
   copyright (c) 2011 石橋宏之利
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "misaki.h"

class OpeningFile {
  public:
    OpeningFile(std::string file_name) {
      file_name_ = file_name;
    }
    OpeningFile(const OpeningFile& file) {
      file_name_ = file.file_name_;
      for (int i = 0; file.csv_record_vector_.size(); i++) {
        csv_record_vector_.push_back(file.csv_record_vector_[i]);
      }
    }
    OpeningFile& operator=(const OpeningFile& file) {
      file_name_ = file.file_name_;
      for (int i = 0; file.csv_record_vector_.size(); i++) {
        csv_record_vector_.push_back(file.csv_record_vector_[i]);
      }
      return *this;
    }
    std::string file_name() const {
      return file_name_;
    }
    void Add(std::string csv_record_str) {
      csv_record_vector_.push_back(csv_record_str);
    }
    void Write() const {
      std::string full_name = file_name_ + ".csv";
      std::ofstream fout(full_name.c_str());

      for (int i = 0; i < csv_record_vector_.size(); i++) {
        fout << csv_record_vector_[i] << std::endl;
      }

      fout.close();
    }

  private:
    std::string file_name_;
    std::vector<std::string> csv_record_vector_;
};

int main(int argc, char* argv[]) {
  // Misakiを初期化。
  Misaki::Init();

  // 引数を確認。
  if (argc != 3) {
    std::cout << "Usage: openingbookmaker <pgn name> <tag name>" << std::endl;
    return 1;
  }

  // PGNファイルを読み込む。
  Misaki::PGNDocument* doc = NULL;
  try {
    doc = Misaki::PGNDocument::New(argv[1]);
  } catch (...) {
    std::cout << "Fail to parse..." << std::endl;
    return 1;
  }
  if (doc->GetSize() <= 0) {
    std::cout << "I couldn't read PGN..." << std::endl;
    return 1;
  }

  // タグが全てのゲームに存在するかチェックする。
  for (int index = 0; index < doc->GetSize(); index++) {
    if ((*doc)[index].GetTagValue(argv[2]) == "") {
      std::cout << "There is no such tag..." << std::endl;
      return 1;
    }
  }

  // オープニングファイルのオブジェクトを作る。
  std::vector<OpeningFile> o_file_vector;
  bool has_name;
  for (int index = 0; index < doc->GetSize(); index++) {
    has_name = false;
    for (int index2 = 0; index2 < o_file_vector.size(); index2++) {
      if (o_file_vector[index2].file_name()
      == (*doc)[index].GetTagValue(argv[2])) {
        has_name = true;
        break;
      }
    }
    if (!has_name) {
      o_file_vector.push_back(OpeningFile((*doc)[index].GetTagValue(argv[2])));
    }
  }

  // データを作る。
  Misaki::ChessBoard* board = NULL;
  Misaki::MoveList* move_list = NULL;
  int o_file_vec_index;
  for (int index = 0; index < doc->GetSize(); index++) {
    // ファイルのインデックスを検索する。
    for (int index2 = 0; index2 < o_file_vector.size(); index2++) {
      if (o_file_vector[index2].file_name()
      == (*doc)[index].GetTagValue(argv[2])) {
        o_file_vec_index = index2;
        break;
      }
    }

    // ボードを作る。
    board = Misaki::ChessBoard::New();

    // 手のリストを作る。
    move_list = (*doc)[index].CreateMoveList();

    // 一通り手を指す。
    for (int index2 = 0; index2 < move_list->GetSize(); index2++) {
      board->TakeMove((*move_list)[index2]);
    }

    // ゲームの履歴を元にファイルオブジェクトにCSVを追加する。
    for (int index2 = 0; index2 < move_list->GetSize(); index2++) {
      o_file_vector[o_file_vec_index].Add(
      Misaki::Opening(board->GetGameRecord(index2), (*move_list)[index2])
      .GetCSVRecord());
    }

    // 解放。
    delete move_list;
    delete board;
  }

  // ファイルに書き出す。
  for (int index = 0; index < o_file_vector.size(); index++) {
    o_file_vector[index].Write();
  }

  return 0;
}
