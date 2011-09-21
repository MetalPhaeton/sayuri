/* game_record.h: ゲームの記録のデータ。
   copyright (c) 2011 石橋宏之利
 */

#ifndef GAME_RECORD_H
#define GAME_RECORD_H

#include <iostream>
#include "chess_def.h"
#include "chess_board.h"

namespace Misaki {
  class GameRecord;
  class ChessBoard;  // 相互依存。

  /**********************************
   * ゲームの記録のデータのクラス。 *
   **********************************/
  std::ostream& operator<<(std::ostream& stream, const GameRecord& record);
  class GameRecord {
    public:
      /**********************************************
       * コピーコンストラクタとデストラクタと代入。 *
       **********************************************/
      GameRecord(const GameRecord& record);
      virtual ~GameRecord() {}
      GameRecord& operator=(const GameRecord& record);

      /**********
       * 関数。 *
       **********/
      // その位置の駒の種類を得る。
      // [引数]
      // piece_square: 駒の位置。
      // [戻り値]
      // 駒の種類。
      piece_t GetPieceType(square_t piece_square) const;
      // その位置の駒のサイドを得る。
      // [引数]
      // piece_square: 駒の位置。
      // [戻り値]
      // 駒のサイド。
      side_t GetSide(square_t piece_square) const;

      /**************
       * アクセサ。 *
       **************/
      // 駒の配置。
      const bitboard_t (& position() const)[NUM_SIDES][NUM_PIECE_TYPES] {
        return position_;
      }
      // 手番。
      side_t to_move() const {return to_move_;}
      // プライ。
      int ply() const {return ply_;}
      // キャスリングの権利。
      castling_t castling_rights() const {return castling_rights_;}
      // アンパッサンのターゲット。
      square_t en_passant_target() const {return en_passant_target_;}
      // アンパッサンできるかどうか。
      bool can_en_passant() const {return can_en_passant_;}
      // 50手ルールの手数。
      int ply_100() const {return ply_100_;}
      // 繰り返しの回数。
      int repetition() const {return repetition_;}
      // 最後の手。
      Move last_move() const {
        return Move(last_move_.piece_square_, last_move_.goal_square_,
        last_move_.promotion_);
      }
      // 局面のハッシュキー。
      hash_key_t key() const {return key_;}

    protected:
      /********************
       * コンストラクタ。 *
       ********************/
      // コンストラクタ。
      // [引数]
      // board: ボード。
      // ply: プライ。
      // ply_100: 50手ルールの手数。
      // repetition: 同一局面の回数。
      // last_move: 最後の手。
      // key: この局面のハッシュキー。
      GameRecord(const ChessBoard& board,
      int ply, int ply_100, int repetition, move_t last_move,
      hash_key_t key);

    private:
      /********************
       * フレンドクラス。 *
       ********************/
      friend class ChessBoard;

      /****************
       * 出力演算子。 *
       ****************/
      friend std::ostream& operator<<(std::ostream& stream,
      const GameRecord& record);

      /**************************************
       * プライベート関数。フレンドに公開。 *
       **************************************/
      // ボードと同じ駒の配置かどうかチェックする。
      // [引数]
      // board: 調べたいボード。
      // [戻り値]
      // 同じ配置ならtrue。
      bool EqualsPosition(const ChessBoard& board) const;

      /****************
       * メンバ変数。 *
       ****************/
      // 駒の配置。
      bitboard_t position_[NUM_SIDES][NUM_PIECE_TYPES];
      // 手番。
      side_t to_move_;
      // プライ。
      int ply_;
      // キャスリングの権利。
      castling_t castling_rights_;
      // アンパッサンのターゲット。
      square_t en_passant_target_;
      // アンパッサンできるかどうか。
      bool can_en_passant_;
      // 50手ルールの手数。
      int ply_100_;
      // 繰り返しの回数。
      int repetition_;
      // 最後の手。
      move_t last_move_;
      // 局面のハッシュキー。
      hash_key_t key_;
  };
}  // Misaki

#endif
