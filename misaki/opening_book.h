/* opening_book.h: オープニングのブック。
   copyright (c) 2011 石橋宏之利
 */

#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <iostream>
#include <string>
#include <vector>
#include "chess_def.h"
#include "chess_util.h"
#include "move.h"
#include "game_record.h"

namespace Misaki {
  class OpeningBook;
  class Opening;
  class GameRecord;

  /**************************
   * オープニングのクラス。 *
   **************************/
  class Opening {
    public:
      /**********************************
       * コンストラクタとデストラクタ。 *
       **********************************/
      // コンストラクタ。
      // [引数]
      // position: 駒の配置。
      // castling_rights: キャスリングの権利。
      // en_passant_target: アンパッサンのターゲット。
      // can_en_passant: アンパッサンできるかどうか。
      // to_move: 手番。
      // next_move: 次の手。
      Opening(const bitboard_t (& position)[NUM_SIDES][NUM_PIECE_TYPES],
      castling_t castling_rights, square_t en_passant_target,
      bool can_en_passant, side_t to_move, Move next_move);
      // コンストラクタ。
      // [引数]
      // record: 記録したい局面。
      // next_move: 次の手。
      Opening(const GameRecord& record, Move next_move);
      // コンストラクタ。
      // [引数]
      // csv_record: CSVデータのレコード。
      // [例外]
      // パースできなかったらfalse。
      Opening(std::string csv_record) throw (bool);
      // コピーコンストラクタ。
      // [引数]
      // opening: コピーするオブジェクト。
      Opening(const Opening& opening);
      // 代入。
      Opening& operator=(const Opening& opening);
      // デストラクタ。
      virtual ~Opening() {}

      /****************
       * 比較演算子。 *
       ****************/
      bool operator==(const Opening& opening) const;
      bool operator!=(const Opening& opening) const;

      /**********
       * 関数。 *
       **********/
      // オープニングのCSVレコードを得る。
      // [戻り値]
      // CSVレコード。
      std::string GetCSVRecord() const;

      /**************
       * アクセサ。 *
       **************/
      // 駒の配置。
      const bitboard_t (& position() const) [NUM_SIDES][NUM_PIECE_TYPES] {
        return position_;
      }
      // キャスリングの権利。
      castling_t castling_rights() const {return castling_rights_;}
      // アンパッサンのターゲット。
      square_t en_passant_target() const {return en_passant_target_;}
      // アンパッサンできるかどうか。
      bool can_en_passant() const {return can_en_passant_;}
      // 手番。
      side_t to_move() const {return to_move_;}
      // 次の手。
      Move next_move() const {return next_move_;}

    private:
      /********************
       * フレンドに公開。 *
       ********************/
      friend class OpeningBook;
      bool operator==(const GameRecord& record) const;
      bool operator!=(const GameRecord& record) const;

      /****************
       * メンバ変数。 *
       ****************/
      // 駒の配置。
      bitboard_t position_[NUM_SIDES][NUM_PIECE_TYPES];
      // キャスリングの権利。
      castling_t castling_rights_;
      // アンパッサンのターゲット。
      square_t en_passant_target_;
      // アンパッサンできるかどうか。
      bool can_en_passant_;
      // 手番。
      side_t to_move_;
      // 次の手。
      Move next_move_;

      /**********************
       * プライベート関数。 *
       **********************/
      // 駒の配置をパース。
      // [引数]
      // position_str: 駒の配置の文字列。
      // [例外]
      // パースできなかったらfalse。
      void ParsePosition(std::string position_str) throw (bool);
      // キャスリングの権利をパース。
      // [引数]
      // castling_rights_str: キャスリングの権利の文字列。
      // [例外]
      // パースできなかったらfalse。
      void ParseCastlingRights(std::string castling_rights_str)
      throw (bool);
      // アンパッサンのターゲットをパース。
      // [引数]
      // en_passant_target_str: アンパッサンのターゲットの文字列。
      // [例外]
      // パースできなかったらfalse。
      void ParseEnPassantTarget(std::string en_passant_target_str)
      throw (bool);
      // 手番をパース。
      // [引数]
      // to_move_str: 手番の文字列。
      // [例外]
      // パースできなかったらfalse。
      void ParseToMove(std::string to_move_str) throw (bool);
      // 次の手をパース。
      // [引数]
      // next_move_str: 次の手の文字列。
      // [例外]
      // パースできなかったらfalse。
      void ParseNextMove(std::string next_move_str) throw (bool);
      // 位置をパース。
      // [引数]
      // square_str: 位置の文字列。
      // [戻り値]
      // 位置。
      // [例外]
      // パースできなかったらfalse。
      square_t ParseSquare(std::string square_str) throw (bool);
  };

  /********************************
   * オープニングブックのクラス。 *
   ********************************/
  class OpeningBook {
    public:
      /**************************************
       * インスタンスの生成とデストラクタ。 *
       **************************************/
      // インスタンスの生成。
      static OpeningBook* New() {
        return new OpeningBook();
      }
      // デストラクタ。
      virtual ~OpeningBook() {}

      /************
       * 演算子。 *
       ************/
      // 追加演算子。
      OpeningBook& operator+=(const Opening& opening) {
        opening_vector_.push_back(opening);
        return *this;
      }
      // 削除演算子。
      OpeningBook& operator-=(const Opening& opening);
      // インデックス。
      const Opening& operator[](int index) const {
        return opening_vector_[index];
      }

      /**********
       * 関数。 *
       **********/
      // オープニングの次の手のリストを得る。
      // [引数]
      // record: 現在の局面。
      // [戻り値]
      // 次の手のリスト。
      MoveList* CreateNextMoveList(const GameRecord& record) const;
      // サイズを得る。
      // [戻り値]
      // サイズ。
      int GetSize() const {return opening_vector_.size();}

    protected:
      /********************
       * コンストラクタ。 *
       ********************/
      // コンストラクタ。
      OpeningBook() {}

    private:
      /******************
       * コピーの禁止。 *
       ******************/
      OpeningBook(const OpeningBook&);  // 削除。
      OpeningBook& operator=(const OpeningBook&);  // 削除。

      /****************
       * メンバ変数。 *
       ****************/
      std::vector<Opening> opening_vector_;
  };
}  // Misaki

#endif
