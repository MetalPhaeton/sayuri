/* pgn_parser.h: PGNファイルをパースする。
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

#ifndef PGN_PARSER_H
#define PGN_PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include "chess_def.h"
#include "chess_util.h"
#include "chess_board.h"
#include "move.h"

namespace Misaki {
  class PGNMove;
  class PGNMoveList;
  class PGNGameInfo;
  class PGNGameInfoList;
  class PGNGame;
  class PGNDocument;

  class ChessBoard;
  class Move;
  class MoveList;

  /*********************
   * PGNの手のクラス。 *
   *********************/
  class PGNMove {
    public:
      /****************************************
       * コンストラクタと代入とデストラクタ。 *
       ****************************************/
      // コンストラクタ。
      PGNMove(std::string move_str) throw (bool) :
      piece_type_(EMPTY),
      piece_mask_(-1ULL),
      goal_square_(A1),
      promotion_(EMPTY) {
        ParseMove(move_str);
      }
      // コピーコンストラクタ。
      PGNMove(const PGNMove& pgn_move) :
      piece_type_(pgn_move.piece_type_),
      piece_mask_(pgn_move.piece_mask_),
      goal_square_(pgn_move.goal_square_),
      promotion_(pgn_move.promotion_) {}
      // 代入。
      PGNMove& operator=(const PGNMove& pgn_move) {
        piece_type_ = pgn_move.piece_type_;
        piece_mask_ = pgn_move.piece_mask_;
        goal_square_ = pgn_move.goal_square_;
        promotion_ = pgn_move.promotion_;
        return *this;
      }
      // デストラクタ。
      virtual ~PGNMove() {}

      /**************
       * アクセサ。 *
       **************/
      // 駒の種類。
      piece_t piece_type() const {return piece_type_;}
      // 駒のマスク。
      bitboard_t piece_mask() const {return piece_mask_;}
      // 移動先の位置。
      square_t goal_square() const {return goal_square_;}
      // 昇格する駒の種類。
      piece_t promotion() const {return promotion_;}

    private:
      /****************
       * メンバ変数。 *
       ****************/
      // 駒の種類。
      piece_t piece_type_;
      // 移動元の位置のマスク。
      bitboard_t piece_mask_;
      // 移動先の位置。
      square_t goal_square_;
      // 昇格する駒の種類。
      piece_t promotion_;

      /**********************
       * プライベート関数。 *
       **********************/
      // 手をパースする。
      // [引数]
      // move_str: 手の文字列。
      // [例外]
      // パースできなかったらfalse。
      void ParseMove(std::string move_str) throw (bool);
      // ファイルをパースする。
      // [引数]
      // fyle_str: ファイルの文字列。
      // [戻り値]
      // ファイル。
      // [例外]
      // パースできなかったらfalse。
      fyle_t ParseFyle(std::string fyle_str) const throw (bool);
      // ランクをパースする。
      // [引数]
      // rank_str: ランクの文字列。
      // [戻り値]
      // ランク。
      // [例外]
      // パースできなかったらfalse。
      rank_t ParseRank(std::string rank_str) const throw (bool);
      // 位置をパースする。
      // [引数]
      // square_str: 位置の文字列。
      // [戻り値]
      // 位置。
      // [例外]
      // パースできなかったらfalse。
      square_t ParseSquare(std::string square_str) const throw (bool);
      // 駒の種類をパースする。
      // [引数]
      // piece_type_str: 駒の種類の文字列。
      // [戻り値]
      // 駒の種類。
      // [例外]
      // パースできなかったらfalse。
      piece_t ParsePieceType(std::string piece_type_str) const throw (bool);
  };

  /*********************
   * PGNの手のリスト。 *
   *********************/
  class PGNMoveList {
    public:
      /**************************************
       * インスタンスの生成とデストラクタ。 *
       **************************************/
      // インスタンスの生成。
      // [戻り値]
      // インスタンス。
      static PGNMoveList* New() {
        return new PGNMoveList();
      }
      // デストラクタ。
      virtual ~PGNMoveList() {}

      /************
       * 演算子。 *
       ************/
      // 追加演算子。
      PGNMoveList& operator+=(const PGNMove& pgn_move) {
        pgn_move_vector_.push_back(PGNMove(pgn_move));
        return *this;
      }
      // インデックス。
      const PGNMove& operator[](int index) const {
        return pgn_move_vector_[index];
      }

      /**********
       * 関数。 *
       **********/
      // サイズを得る。
      // [戻り値]
      // サイズ。
      int GetSize() const {return pgn_move_vector_.size();}

    protected:
      /********************
       * コンストラクタ。 *
       ********************/
      PGNMoveList() {}

    private:
      /******************
       * コピーの禁止。 *
       ******************/
      PGNMoveList(const PGNMoveList&);  // 削除。
      PGNMoveList& operator=(const PGNMoveList&);  // 削除。

      /****************
       * メンバ変数。 *
       ****************/
      // 手のベクトル。
      std::vector<PGNMove> pgn_move_vector_;
  };

  /**************************
   * ゲームの情報のクラス。 *
   **************************/
  class PGNGameInfo {
    public:
      /****************************************
       * コンストラクタと代入とデストラクタ。 *
       ****************************************/
      // コンストラクタ。
      // [引数]
      // tag: タグ。
      // value: 値。
      PGNGameInfo(std::string tag, std::string value) :
      tag_(tag), value_(value) {}
      // コピーコンストラクタ。
      // [引数]
      // info: コピーするオブジェクト。
      PGNGameInfo(const PGNGameInfo& info):
      tag_(info.tag_), value_(info.value_) {}
      // 代入。
      PGNGameInfo& operator=(const PGNGameInfo& info) {
        tag_ = info.tag_;
        value_ = info.value_;
      }
      // デストラクタ。
      virtual ~PGNGameInfo() {}

      /**************
       * アクセサ。 *
       **************/
      // タグ。
      std::string tag() const {return tag_;}
      // 値。
      std::string value() const {return value_;}

    private:
      /****************
       * メンバ変数。 *
       ****************/
      // タグ。
      std::string tag_;
      // 値。
      std::string value_;
  };

  /**************************
   * ゲームの情報のリスト。 *
   **************************/
  class PGNGameInfoList {
    public:
      /**************************************
       * インスタンスの生成とデストラクタ。 *
       **************************************/
      // インスタンスの生成。
      // [戻り値]
      // インスタンス。
      static PGNGameInfoList* New() {
        return new PGNGameInfoList();
      }
      // デストラクタ。
      virtual ~PGNGameInfoList() {}

      /************
       * 演算子。 *
       ************/
      // 追加演算子。
      PGNGameInfoList& operator+=(const PGNGameInfo& info) {
        info_vector_.push_back(PGNGameInfo(info));
        return *this;
      }
      // インデックス。
      const PGNGameInfo& operator[](int index) const {
        return info_vector_[index];
      }

      /**********
       * 関数。 *
       **********/
      // サイズを得る。
      // [戻り値]
      // サイズ。
      int GetSize() const {return info_vector_.size();}

    protected:
      /********************
       * コンストラクタ。 *
       ********************/
      PGNGameInfoList() {}

    private:
      /******************
       * コピーの禁止。 *
       ******************/
      PGNGameInfoList(const PGNGameInfoList&);  // 削除。
      PGNGameInfo& operator=(const PGNGameInfoList&);  // 削除。

      /****************
       * メンバ変数。 *
       ****************/
      // ゲームの情報のベクトル。
      std::vector<PGNGameInfo> info_vector_;
  };

  /*************************
   * PGNのゲームのクラス。 *
   *************************/
  class PGNGame {
    public:
      /******************
       * デストラクタ。 *
       ******************/
      virtual  ~PGNGame() {
        delete info_list_ptr_;
        delete move_list_ptr_;
      }

      /**********
       * 関数。 *
       **********/
      // 手のリストを作る。
      // [戻り値]
      // 手のリスト。
      MoveList* CreateMoveList() const;
      // タグの値を得る。
      // [引数]
      // tag_name: タグの名前。
      // [戻り値]
      // タグの値。
      std::string GetTagValue(std::string tag_name) const;

      /***************
       * アクセサ。　*
       ***************/
      // ゲームの情報のリスト。
      const PGNGameInfoList& info_list() const {return *info_list_ptr_;}
      // ゲームの手のリスト。
      const PGNMoveList& move_list() const {return *move_list_ptr_;}

    private:
      /**************************************************
       * インスタンスの生成、継承の禁止。 フレンドのみ。*
       **************************************************/
      // コンストラクタ。
      PGNGame() {
        info_list_ptr_ = PGNGameInfoList::New();
        move_list_ptr_ = PGNMoveList::New();
      }
      PGNGame(const PGNGame&);  // 削除。
      PGNGame& operator=(const PGNGame&);  // 削除。

      /**************
       * フレンド。 *
       **************/
      friend class PGNDocument;

      /****************
       * メンバ変数。 *
       ****************/
      // ゲームの情報。
      PGNGameInfoList* info_list_ptr_;
      // ゲームの手。
      PGNMoveList* move_list_ptr_;
  };

  /*******************************
   * PGNのドキュメントのクラス。 *
   *******************************/
  class PGNDocument {
    public:
      /**************************************
       * インスタンスの生成とデストラクタ。 *
       **************************************/
      // インスタンスの生成。
      // [引数]
      // PGNのファイル名。
      // [戻り値]
      // インスタンス。
      // [例外]
      // パースできなければfalse。
      static PGNDocument* New(std::string file_name) throw (bool) {
        return new PGNDocument(file_name);
      }
      // デストラクタ。
      virtual ~PGNDocument() {
        for (int i = 0; i < game_ptr_vector_.size(); i++) {
          delete game_ptr_vector_[i];
        }
      }

      /************
       * 演算子。 *
       ************/
      // インデックス。
      const PGNGame& operator[](int index) const {
        return *(game_ptr_vector_[index]);
      }

      /**********
       * 関数。 *
       **********/
      // サイズを得る。
      // [戻り値]
      // サイズ。
      int GetSize() const {return game_ptr_vector_.size();}

    protected:
      /********************
       * コンストラクタ。 *
       ********************/
      // コンストラクタ。
      // [引数]
      // PGNのファイル名。
      // [例外]
      // パースできなければfalse。
      PGNDocument(std::string file_name) throw (bool);

    private:
      /******************
       * コピーの禁止。 *
       ******************/
      PGNDocument(const PGNDocument&);  // 削除。
      PGNDocument& operator=(const PGNDocument&);  // 削除。

      /****************
       * メンバ変数。 *
       ****************/
      std::vector<PGNGame*> game_ptr_vector_;
  };
}  // Misaki

#endif
