/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Hironori Ishibashi
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
 * @file sayulisp.cpp
 * @author Hironori Ishibashi
 * @brief Sayuri用Lispライブラリの実装。
 */

#include "sayulisp.h"

#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <climits>
#include <sstream>
#include <map>
#include <tuple>
#include "common.h"
#include "params.h"
#include "chess_engine.h"
#include "board.h"
#include "transposition_table.h"
#include "uci_shell.h"
#include "lisp_core.h"
#include "fen.h"
#include "position_record.h"
#include "pv_line.h"
#include "pgn.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ======== //
  // Sayulisp //
  // ======== //
  // ========== //
  // static定数 //
  // ========== //
  const std::map<std::string, Square> Sayulisp::SQUARE_MAP {
    {"A1", A1}, {"B1", B1}, {"C1", C1}, {"D1", D1},
    {"E1", E1}, {"F1", F1}, {"G1", G1}, {"H1", H1},
    {"A2", A2}, {"B2", B2}, {"C2", C2}, {"D2", D2},
    {"E2", E2}, {"F2", F2}, {"G2", G2}, {"H2", H2},
    {"A3", A3}, {"B3", B3}, {"C3", C3}, {"D3", D3},
    {"E3", E3}, {"F3", F3}, {"G3", G3}, {"H3", H3},
    {"A4", A4}, {"B4", B4}, {"C4", C4}, {"D4", D4},
    {"E4", E4}, {"F4", F4}, {"G4", G4}, {"H4", H4},
    {"A5", A5}, {"B5", B5}, {"C5", C5}, {"D5", D5},
    {"E5", E5}, {"F5", F5}, {"G5", G5}, {"H5", H5},
    {"A6", A6}, {"B6", B6}, {"C6", C6}, {"D6", D6},
    {"E6", E6}, {"F6", F6}, {"G6", G6}, {"H6", H6},
    {"A7", A7}, {"B7", B7}, {"C7", C7}, {"D7", D7},
    {"E7", E7}, {"F7", F7}, {"G7", G7}, {"H7", H7},
    {"A8", A8}, {"B8", B8}, {"C8", C8}, {"D8", D8},
    {"E8", E8}, {"F8", F8}, {"G8", G8}, {"H8", H8}
  };
  const std::map<std::string, Fyle> Sayulisp::FYLE_MAP {
    {"FYLE_A", FYLE_A}, {"FYLE_B", FYLE_B},
    {"FYLE_C", FYLE_C}, {"FYLE_D", FYLE_D},
    {"FYLE_E", FYLE_E}, {"FYLE_F", FYLE_F},
    {"FYLE_G", FYLE_G}, {"FYLE_H", FYLE_H},
  };
  const std::map<std::string, Rank> Sayulisp::RANK_MAP {
    {"RANK_1", RANK_1}, {"RANK_2", RANK_2},
    {"RANK_3", RANK_3}, {"RANK_4", RANK_4},
    {"RANK_5", RANK_5}, {"RANK_6", RANK_6},
    {"RANK_7", RANK_7}, {"RANK_8", RANK_8},
  };
  const std::map<std::string, Side> Sayulisp::SIDE_MAP {
    {"NO_SIDE", NO_SIDE}, {"WHITE", WHITE}, {"BLACK", BLACK}
  };
  const std::map<std::string, PieceType> Sayulisp::PIECE_MAP {
    {"EMPTY", EMPTY},
    {"PAWN", PAWN}, {"KNIGHT", KNIGHT}, {"BISHOP", BISHOP},
    {"ROOK", ROOK}, {"QUEEN", QUEEN}, {"KING", KING}
  };
  const std::map<std::string, int> Sayulisp::CASTLING_MAP {
    {"NO_CASTLING", 0},
    {"WHITE_SHORT_CASTLING", 1}, {"WHITE_LONG_CASTLING", 2},
    {"BLACK_SHORT_CASTLING", 3}, {"BLACK_LONG_CASTLING", 4}
  };
  const std::string Sayulisp::SQUARE_MAP_INV[NUM_SQUARES] {
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8"
  };
  const std::string Sayulisp::FYLE_MAP_INV[NUM_FYLES] {
    "FYLE_A", "FYLE_B", "FYLE_C", "FYLE_D",
    "FYLE_E", "FYLE_F", "FYLE_G", "FYLE_H"
  };
  const std::string Sayulisp::RANK_MAP_INV[NUM_RANKS] {
    "RANK_1", "RANK_2", "RANK_3", "RANK_4",
    "RANK_5", "RANK_6", "RANK_7", "RANK_8"
  };
  const std::string Sayulisp::SIDE_MAP_INV[NUM_SIDES] {
    "NO_SIDE", "WHITE", "BLACK"
  };
  const std::string Sayulisp::PIECE_MAP_INV[NUM_PIECE_TYPES] {
    "EMPTY", "PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"
  };
  const std::string Sayulisp::CASTLING_MAP_INV[5] {
    "NO_CASTLING", "WHITE_SHORT_CASTLING", "WHITE_LONG_CASTLING",
    "BLACK_SHORT_CASTLING", "BLACK_LONG_CASTLING"
  };

  // Sayulispの関数を設定する。
  void Sayulisp::SetSayulispFunction() {
    // 定数を登録。
    for (auto& pair : SQUARE_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : FYLE_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : RANK_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : SIDE_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : PIECE_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : CASTLING_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }

    // 関数を登録。
    LC_Function func;
    std::string help;

    func = LC_FUNCTION_OBJ(SayuriLicense);
    INSERT_LC_FUNCTION(func, "sayuri-license", "Sayulisp:sayuri-license");
    help =
R"...(### sayuri-license ###

<h6> Usage </h6>

* `(sayuri-license)`

<h6> Description </h6>

* Returns String of license terms of Sayuri.

<h6> Example </h6>

    (display (sayuri-license))
    
    ;; Output
    ;; > Copyright (c) 2013-2016 Hironori Ishibashi
    ;; > 
    ;; > Permission is hereby granted, free of charge, to any person obtaining
    :: > a copy
    ;; > of this software and associated documentation files (the "Software"),
    ;; > to
    ;; > deal in the Software without restriction, including without limitation
    ;; > the
    ;; > rights to use, copy, modify, merge, publish, distribute, sublicense,
    ;; > and/or
    ;; > sell copies of the Software, and to permit persons to whom the
    ;; > Software is
    ;; > furnished to do so, subject to the following conditions:
    ;; > 
    ;; > The above copyright notice and this permission notice shall be
    ;; > included in
    ;; > all copies or substantial portions of the Software.
    ;; > 
    ;; > THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    ;; > EXPRESS OR
    ;; > IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    ;; > MERCHANTABILITY,
    ;; > FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
    ;; > SHALL THE
    ;; > AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    ;; > LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ;; > ARISING
    ;; > FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    ;; > DEALINGS IN THE SOFTWARE.)...";
    help_dict_.emplace("sayuri-license", help);

    func = LC_FUNCTION_OBJ(SquareToNumber);
    INSERT_LC_FUNCTION(func, "square->number", "Sayulisp:square->number");
    help =
R"...(### square->number ###

<h6> Usage </h6>

* `(square->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Square Symbol, it returns Number indicating to Square.
* If `<Object>` is List, it returns List changed Square Symbol into Number. 

<h6> Example </h6>

    (define symbol-list
      '(A1 B1 C1 (WHITE D3 E4 (F5 PAWN G6) H7 BLACK_LONG_CASTLING)))
    
    (display (square->number symbol-list))
    ;; Output
    ;; > (0 1 2 (WHITE 19 28 (37 PAWN 46) 55 BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("square->number", help);

    func = LC_FUNCTION_OBJ(FyleToNumber);
    INSERT_LC_FUNCTION(func, "fyle->number", "Sayulisp:fyle->number");
    help =
R"...(### fyle->number ###

<h6> Usage </h6>

* `(fyle->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Fyle Symbol, it returns Number indicating to Fyle.
* If `<Object>` is List, it returns List changed Fyle Symbol into Number. 

<h6> Example </h6>

    (define symbol-list
      '(FYLE_A FYLE_B (WHITE FYLE_D E4 (PAWN G6) FYLE_H BLACK_LONG_CASTLING)))
    
    (display (fyle->number symbol-list))
    ;; Output
    ;; > (0 1 (WHITE 3 E4 (PAWN G6) 7 BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("fyle->number", help);

    func = LC_FUNCTION_OBJ(RankToNumber);
    INSERT_LC_FUNCTION(func, "rank->number", "Sayulisp:rank->number");
    help =
R"...(### rank->number ###

<h6> Usage </h6>

* `(rank->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Rank Symbol, it returns Number indicating to Rank.
* If `<Object>` is List, it returns List changed Rank Symbol into Number. 

<h6> Example </h6>

    (define symbol-list
      '(RANK_1 RANK_2 (WHITE RANK_4 E4 (PAWN G6) RANK_8 BLACK_LONG_CASTLING)))
    
    (display (rank->number symbol-list))
    ;; Output
    ;; > (0 1 (WHITE 3 E4 (PAWN G6) 7 BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("rank->number", help);

    func = LC_FUNCTION_OBJ(SideToNumber);
    INSERT_LC_FUNCTION(func, "side->number", "Sayulisp:side->number");
    help =
R"...(### side->number ### {#side-to-number}

<h6> Usage </h6>

* `(side->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Side Symbol, it returns Number indicating to Side.
* If `<Object>` is List, it returns List changed Side Symbol into Number. 

<h6> Example </h6>

    (define symbol-list
      '(NO_SIDE WHITE (FYLE_A BLACK E4 (PAWN G6) BLACK_LONG_CASTLING)))
    
    (display (side->number symbol-list))
    ;; Output
    ;; > (0 1 (FYLE_A 2 E4 (PAWN G6) BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("side->number", help);

    func = LC_FUNCTION_OBJ(PieceToNumber);
    INSERT_LC_FUNCTION(func, "piece->number", "Sayulisp:piece->number");
    help =
R"...(### piece->number ###

<h6> Usage </h6>

* `(piece->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Piece Type Symbol, it returns Number indicating
  to Piece Type.
* If `<Object>` is List, it returns List changed Piece Type Symbol into Number.

<h6> Example </h6>

    (define symbol-list
      '(EMPTY PAWN (FYLE_A QUEEN E4 (RANK_4 G6) KING BLACK_LONG_CASTLING)))
    
    (display (piece->number symbol-list))
    ;; Output
    ;; > (0 1 (FYLE_A 5 E4 (RANK_4 G6) 6 BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("piece->number", help);

    func = LC_FUNCTION_OBJ(CastlingToNumber);
    INSERT_LC_FUNCTION(func, "castling->number", "Sayulisp:castling->number");
    help =
R"...(### castling->number ###

<h6> Usage </h6>

* `(castling->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Castling Right Symbol, it returns Number indicating
  to Piece Type.
* If `<Object>` is List, it returns List changed Castling Right Symbol
  into Number. 

<h6> Example </h6>

    (define symbol-list
      '(NO_CASTLING WHITE_SHORT_CASTLING (FYLE_A E4 (RANK_4 G6) KING)))
    
    (display (castling->number symbol-list))
    ;; Output
    ;; > (0 1 (FYLE_A E4 (RANK_4 G6) KING)))...";
    help_dict_.emplace("castling->number", help);

    func = LC_FUNCTION_OBJ(NumberToSquare);
    INSERT_LC_FUNCTION(func, "number->square", "Sayulisp:number->square");
    help =
R"...(### number->square ### {#number-to-square}

<h6> Usage </h6>

* `(number->square <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Square Symbol.
* If `<Object>` is List, it returns List changed Number into Square Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->square number-list))
    ;; Output
    ;; > (A1 B1 (C1 (D1 E1 "Hello") F1) 100))...";
    help_dict_.emplace("number->square", help);

    func = LC_FUNCTION_OBJ(NumberToFyle);
    INSERT_LC_FUNCTION(func, "number->fyle", "Sayulisp:number->fyle");
    help =
R"...(### number->fyle ### {#number-to-fyle}

<h6> Usage </h6>

* `(number->fyle <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Fyle Symbol.
* If `<Object>` is List, it returns List changed Number into Fyle Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->fyle number-list))
    ;; Output
    ;; > (FYLE_A FYLE_B (FYLE_C (FYLE_D FYLE_E "Hello") FYLE_F) 100))...";
    help_dict_.emplace("number->fyle", help);

    func = LC_FUNCTION_OBJ(NumberToRank);
    INSERT_LC_FUNCTION(func, "number->rank", "Sayulisp:number->rank");
    help =
R"...(### number->rank ### {#number-to-rank}

<h6> Usage </h6>

* `(number->rank <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Rank Symbol.
* If `<Object>` is List, it returns List changed Number into Rank Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->rank number-list))
    ;; Output
    ;; > (RANK_1 RANK_2 (RANK_3 (RANK_4 RANK_5 "Hello") RANK_6) 100))...";
    help_dict_.emplace("number->rank", help);

    func = LC_FUNCTION_OBJ(NumberToSide);
    INSERT_LC_FUNCTION(func, "number->side", "Sayulisp:number->side");
    help =
R"...(### number->side ### {#number-to-side}

<h6> Usage </h6>

* `(number->side <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Side Symbol.
* If `<Object>` is List, it returns List changed Number into Side Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->side number-list))
    ;; Output
    ;; > (NO_SIDE WHITE (BLACK (3 4 "Hello") 5) 100))...";
    help_dict_.emplace("number->side", help);

    func = LC_FUNCTION_OBJ(NumberToPiece);
    INSERT_LC_FUNCTION(func, "number->piece", "Sayulisp:number->piece");
    help =
R"...(### number->piece ### {#number-to-piece}

<h6> Usage </h6>

* `(number->piece <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Piece Type Symbol.
* If `<Object>` is List, it returns List changed Number into Piece Type Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->piece number-list))
    ;; Output
    ;; > (EMPTY PAWN (KNIGHT (BISHOP ROOK "Hello") QUEEN) 100))...";
    help_dict_.emplace("number->piece", help);

    func = LC_FUNCTION_OBJ(NumberToCastling);
    INSERT_LC_FUNCTION(func, "number->castling", "Sayulisp:number->castling");
    help =
R"...(### number->castling ### {#number-to-castling}

<h6> Usage </h6>

* `(number->castling <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Castling Rights Symbol.
* If `<Object>` is List, it returns List changed Number
  into CAstling Rights Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->castling number-list))
    ;; Output
    ;; > (NO_CASTLING WHITE_SHORT_CASTLING (WHITE_LONG_CASTLING
    ;; > (BLACK_SHORT_CASTLING BLACK_LONG_CASTLING "Hello") 5) 100))...";
    help_dict_.emplace("number->castling", help);

    func = LC_FUNCTION_OBJ(GenEngine);
    INSERT_LC_FUNCTION(func, "gen-engine", "Sayulisp:gen-engine");
    help =
R"...(### gen-engine ###

<h6> Usage </h6>

1. `(gen-engine)`
2. `((gen-engine) <Message Symbol> [<Arguments>...])`

<h6> Description </h6>

* 1: Generates chess engine.
* 2: The engine executes something according to `<Message Symbol>`.
* 2: Some `<Message Symbol>` require `<Argument>...`.
* `(help "engine <MessageSymbol>")`
    + Returns description for each message symbol.

<h6> Example </h6>

    (define my-engine (gen-engine))
    (display (my-engine '@get-white-pawn-position))
    ;; Output
    ;; > (A2 B2 C2 D2 E2 F2 G2 H2)
    
    (display (help "engine @get-white-pawn-position"))
    ;; Output
    ;; > ### Getting squares ###
    ;; > Returns List of Symbols of squares where specific pieces are on.
    ;; > 
    ;; > * `@get-white-pawn-position`
    ;; > * `@get-white-knight-position`
    ;; > * `@get-white-bishop-position`
    ;; > * `@get-white-rook-position`
    ;; > * `@get-white-queen-position`
    ;; > * `@get-white-king-position`
    ;; > * `@get-black-pawn-position`
    ;; > * `@get-black-knight-position`
    ;; > * `@get-black-bishop-position`
    ;; > * `@get-black-rook-position`
    ;; > * `@get-black-queen-position`
    ;; > * `@get-black-king-position`
    ;; > * `@get-empty-square-position`
    ;; > 
    ;; > <h6> Example </h6>
    ;; > 
    ;; >     (define my-engine (gen-engine))
    ;; >     (display (my-engine '@get-white-pawn-position))
    ;; >     
    ;; >     ;; Output
    ;; >     ;; > (A2 B2 C2 D2 E2 F2 G2 H2))...";
    help_dict_.emplace("gen-engine", help);

    func = LC_FUNCTION_OBJ(GenPGN);
    INSERT_LC_FUNCTION(func, "gen-pgn", "Sayulisp:gen-pgn");
    help =
R"...(### gen-pgn ###

<h6> Usage </h6>

* `(gen-pgn <PGN string : String>)`

<h6> Description </h6>

* Generates and returns PGN object from `<PGN string>`.
* PGN object is operated by Message Symbol.
* PGN object has 2 states.
    + Current game.
        - This can be changed by `@set-current-game`.
    + Current move.
        - This can be changed by `@next-move`, `@prev-move`, `@alt-move`,
          `@orig-move`, `@rewind-move`.

<h6> Description of Message Symbols </h6>

* `@get-pgn-comments`
    + Returns Lists of comments about PGN.

* `@get-current-game-comments.`
    + Returns List of comments about the current game.

* `@get-current-move-comments`
    + Returns List of comments about the current move.

* `@length`
    + Returns the number of games that PGN has.

* `@set-current-game <Index : Number>`
    + Sets a current game into the `<Index>`th game.

* `@get-current-game-headers`
    + Returns List of Lists composed with headers of the current game.
        - The format is "`((<Name 1> <value 1>) (<Name 2> <Value 2>)...)`".

* `@current-move`
    + Returns the current move text.

* `@next-move`
    + Change the current move into the next move
      and returns the move text.

* `@prev-move`
    + Change the current move into the previous move
      and returns the move text.

* `@alt-move`
    + Change the current move into the alternative move
      and returns the move text.

* `@orig-move`
    + If the current move is an alternative move,
      then change a current move into the original move
      and returns the move text.

* `@rewind-move`
    + Change a current move into the first move
      and returns the move text.

<h6> Example </h6>

    ;; Open PGN File.
    (define pgn-file (input-stream "/path/to/pgnfile.pgn"))
    
    ;; Reads the file and generates PGN object.
    (define my-pgn (gen-pgn (pgn-file '@read)))
    
    ;; Displays the current game headers.
    (display (my-pgn '@get-current-game-headers))
    
    ;; Output
    ;; > (("Black" "Hanako Yamada") ("Site" "Japan")
    ;; > ("White" "Hironori Ishibashi")))...";
    help_dict_.emplace("gen-pgn", help);

    func = LC_FUNCTION_OBJ(ParseFENEPD);
    INSERT_LC_FUNCTION(func, "parse-fen/epd", "Sayulisp:parse-fen/epd");
    help =
R"...(### parse-fen/epd ###

<h6> Usage </h6>

* `(parse-fen/epd <FEN or EPD : String>)`

<h6> Description </h6>

* Parses `<FEN or EPD>` and returns result value.
    +  A result value is `((<Tag 1 : String> <Object 1>)...)`.

<h6> Example </h6>

    (display (parse-fen/epd
        "rnbqkbnr/pp2pppp/3p4/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq d3 0 3"))
    ;; Output
    ;; > (("fen castling" (WHITE_SHORT_CASTLING
    ;; > WHITE_LONG_CASTLING BLACK_SHORT_CASTLING BLACK_LONG_CASTLING))
    ;; > ("fen clock" 0)
    ;; > ("fen en_passant" D3)
    ;; > ("fen ply" 5)
    ;; > ("fen position" ((WHITE ROOK) (WHITE KNIGHT) (WHITE BISHOP)
    ;; > (WHITE QUEEN) (WHITE KING) (WHITE BISHOP) (NO_SIDE EMPTY)
    ;; > (WHITE ROOK) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (WHITE KNIGHT) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (WHITE PAWN)
    ;; > (WHITE PAWN) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (BLACK PAWN) (BLACK PAWN) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK ROOK)
    ;; > (BLACK KNIGHT) (BLACK BISHOP) (BLACK QUEEN) (BLACK KING)
    ;; > (BLACK BISHOP) (BLACK KNIGHT) (BLACK ROOK)))
    ;; > ("fen to_move" BLACK)))...";
    help_dict_.emplace("parse-fen/epd", help);

    func = LC_FUNCTION_OBJ(ToFENPosition);
    INSERT_LC_FUNCTION(func, "to-fen-position", "Sayulisp:to-fen-position");
    help =
R"...(### to-fen-position ###

<h6> Usage </h6>

* `(to-fen-position <Pieces list : List>)`

<h6> Description </h6>

* Analyses `<Pieces list>` and returns FEN position.

<h6> Example </h6>

    (display (to-fen-position
        '((WHITE KING) (WHITE KING)(WHITE KING) (WHITE KING)
        (WHITE QUEEN) (WHITE QUEEN)(WHITE QUEEN) (WHITE QUEEN)
        (WHITE ROOK) (WHITE ROOK)(WHITE ROOK) (WHITE ROOK)
        (WHITE BISHOP) (WHITE BISHOP)(WHITE BISHOP) (WHITE BISHOP)
        (WHITE KNIGHT) (WHITE KNIGHT)(WHITE KNIGHT) (WHITE KNIGHT)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (BLACK KNIGHT) (BLACK KNIGHT)(BLACK KNIGHT) (BLACK KNIGHT)
        (BLACK BISHOP) (BLACK BISHOP)(BLACK BISHOP) (BLACK BISHOP)
        (BLACK ROOK) (BLACK ROOK)(BLACK ROOK) (BLACK ROOK)
        (BLACK QUEEN) (BLACK QUEEN)(BLACK QUEEN) (BLACK QUEEN)
        (BLACK KING) (BLACK KING)(BLACK KING) (BLACK KING))))
    ;; Output
    ;; > qqqqkkkk/bbbbrrrr/4nnnn/8/8/NNNN4/RRRRBBBB/KKKKQQQQ)...";
    help_dict_.emplace("to-fen-position", help);
  }

  // Sayulispを開始する。
  int Sayulisp::Run(std::istream* stream_ptr) {
    // 終了ステータス。
    int status = 0;

    // (exit)関数を作成。
    bool loop = true;
    auto func = [&status, &loop](LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = args.cdr().get();

      // ループをセット。
      loop = false;

      // 引数があった場合は終了ステータスあり。
      if (args_ptr->IsPair()) {
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        status = result->number();
      }

      return Lisp::NewNumber(status);
    };
    scope_chain_.InsertSymbol("exit",
    NewN_Function(func, "Sayulisp:exit", scope_chain_));

    try {
      std::string input;
      while (std::getline(*(stream_ptr), input)) {
        input += "\n";
        Tokenize(input);
        LPointerVec s_tree = Parse();

        for (auto& s : s_tree) {
          Evaluate(*s);
        }

        if (!loop) break;
      }
    } catch (LPointer error) {
      PrintError(error);
    }

    return status;
  }

  // メッセージシンボル関数の準備をする。
  void Sayulisp::GetReadyForMessageFunction(const std::string& symbol,
  const LObject& args, int required_args, LObject** args_ptr_ptr) {
    const LPointer& message_args_ptr = args.cdr()->cdr();

    int ret = Lisp::CountList(*message_args_ptr);
    if (ret < required_args) {
      throw Lisp::GenError("@engine-error",
      "'" + symbol + "' requires "
      + std::to_string(required_args) + " arguments and more. Not "
      + std::to_string(ret) + ".");
    }

    *args_ptr_ptr = message_args_ptr.get();
  }

  // エンジンを生成する。
  DEF_LC_FUNCTION(Sayulisp::GenEngine) {
    // スイートを作成。
    std::shared_ptr<EngineSuite> suite_ptr(new EngineSuite());

    // ネイティブ関数オブジェクトを作成。
    auto func = [suite_ptr](LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return (*suite_ptr)(self, caller, args);
    };

    return NewN_Function(func, "Sayulisp:gen-engine:"
    + std::to_string(reinterpret_cast<std::size_t>(suite_ptr.get())),
    caller->scope_chain());
  }

  // マスのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::SquareToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (SQUARE_MAP.find(ptr->symbol()) != SQUARE_MAP.end()) {
        return NewNumber(SQUARE_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // ファイルのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::FyleToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (FYLE_MAP.find(ptr->symbol()) != FYLE_MAP.end()) {
        return NewNumber(FYLE_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // ランクのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::RankToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (RANK_MAP.find(ptr->symbol()) != RANK_MAP.end()) {
        return NewNumber(RANK_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // サイドのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::SideToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (SIDE_MAP.find(ptr->symbol()) != SIDE_MAP.end()) {
        return NewNumber(SIDE_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 駒の種類のシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::PieceToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (PIECE_MAP.find(ptr->symbol()) != PIECE_MAP.end()) {
        return NewNumber(PIECE_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // キャスリングのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::CastlingToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (CASTLING_MAP.find(ptr->symbol()) != CASTLING_MAP.end()) {
        return NewNumber(CASTLING_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をマスのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToSquare) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_SQUARES))) {
          return NewSymbol(SQUARE_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をファイルのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToFyle) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_FYLES))) {
          return NewSymbol(FYLE_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をランクのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToRank) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_RANKS))) {
          return NewSymbol(RANK_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をサイドのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToSide) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_SIDES))) {
          return NewSymbol(SIDE_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値を駒の種類のシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToPiece) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_PIECE_TYPES))) {
          return NewSymbol(PIECE_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をキャスリングのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToCastling) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < 5)) {
          return NewSymbol(CASTLING_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }


  DEF_LC_FUNCTION(Sayulisp::GenPGN) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // PGN文字列を得る。
    LPointer pgn_str_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*pgn_str_ptr, LType::STRING);
    std::shared_ptr<PGN> pgn_ptr(new PGN());
    pgn_ptr->Parse(pgn_str_ptr->string());

    // 現在のゲームのインデックス。
    std::shared_ptr<int> current_index_ptr(new int(0));

    // メッセージシンボル用オブジェクトを作る。
    std::map<std::string, MessageFunction> message_func_map;

    message_func_map["@get-pgn-comments"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 準備。
      const std::vector<std::string>& comment_vec = pgn_ptr->comment_vec();
      std::size_t len = comment_vec.size();

      LPointerVec ret_vec(len);

      // コメントをコピーする。
      for (unsigned int i = 0; i < len; ++i) {
        ret_vec[i] = NewString(comment_vec[i]);
      }

      return LPointerVecToList(ret_vec);
    };

    message_func_map["@get-current-game-comments"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 現在のゲームのコメントを得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();
      const std::vector<std::string>& comment_vec =
      pgn_ptr->game_vec()[*current_index_ptr]->comment_vec();
      int len = comment_vec.size();

      LPointerVec ret_vec(len);
      for (int i = 0; i < len; ++i) {
        ret_vec[i] = NewString(comment_vec[i]);
      }

      return LPointerVecToList(ret_vec);
    };

    message_func_map["@get-current-move-comments"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 現在の指し手のコメントを得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();
      const std::vector<std::string>& comment_vec =
      pgn_ptr->game_vec()[*current_index_ptr]->current_node_ptr()->
      comment_vec_;

      int len = comment_vec.size();

      LPointerVec ret_vec(len);
      for (int i = 0; i < len; ++i) {
        ret_vec[i] = NewString(comment_vec[i]);
      }

      return LPointerVecToList(ret_vec);
    };

    message_func_map["@length"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      return NewNumber(pgn_ptr->game_vec().size());
    };

    message_func_map["@set-current-game"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

      // インデックス番号を得る。
      LPointer index_ptr = caller->Evaluate(*(args_ptr->car()));
      CheckType(*index_ptr, LType::NUMBER);
      int len = pgn_ptr->game_vec().size();
      int index = index_ptr->number();
      index = index < 0 ? len + index : index;
      if ((index < 0) || (index >= len)) {
        throw GenError("@function-error", "Index '" + index_ptr->ToString()
        + "'is out of range.");
      }

      int old_index = *current_index_ptr;
      *current_index_ptr = index;

      return NewNumber(old_index);
    };

    message_func_map["@get-current-game-headers"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 現在のゲームのヘッダを得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();
      const PGNHeader& header =
      pgn_ptr->game_vec()[*current_index_ptr]->header();
      int len = header.size();

      LPointerVec ret_vec(len);
      LPointer temp;
      int i = 0;
      for (auto& pair : header) {
        ret_vec[i] = NewPair(NewString(pair.first),
        NewPair(NewString(pair.second), NewNil()));
        ++i;
      }

      return LPointerVecToList(ret_vec);
    };

    message_func_map["@current-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 現在の指し手を得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();
      const MoveNode* node_ptr =
      pgn_ptr->game_vec()[*current_index_ptr]->current_node_ptr();

      return NewString(node_ptr->text_);
    };

    message_func_map["@next-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // 次の手へ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Next()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    message_func_map["@prev-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // 前の手へ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Back()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    message_func_map["@alt-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // 代替手へ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Alt()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    message_func_map["@orig-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // オリジナルへ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Orig()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    message_func_map["@rewind-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // オリジナルへ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Rewind()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    // PGNオブジェクトを作成。
    auto pgn_func =
    [message_func_map](LPointer self, LObject* caller, const LObject& args)
    -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      GetReadyForFunction(args, 1, &args_ptr);

      // メッセージシンボルを得る。
      LPointer symbol_ptr = caller->Evaluate(*(args_ptr->car()));
      CheckType(*symbol_ptr, LType::SYMBOL);
      std::string symbol = symbol_ptr->symbol();

      // メッセージシンボル関数を呼び出す。
      if (message_func_map.find(symbol) != message_func_map.end()) {
        return message_func_map.at(symbol)(symbol, self, caller, args);
      }

      throw Lisp::GenError("@engine-error",
      "'" + symbol + "' is not message symbol.");
    };

    return NewN_Function(pgn_func, "Sayulisp:gen-pgn:"
    + std::to_string(reinterpret_cast<std::size_t>(pgn_ptr.get())),
    caller->scope_chain());
  }

  // %%% parse-fen/epd
  DEF_LC_FUNCTION(Sayulisp::ParseFENEPD) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 文字列を得る。
    LPointer fen_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*fen_ptr, LType::STRING);

    // パースする。
    std::map<std::string, std::string> fen_map =
    Util::ParseFEN(fen_ptr->string());

    // パース結果を格納していく。
    LPointerVec ret_vec(fen_map.size());
    LPointerVec::iterator ret_itr = ret_vec.begin();
    LPointer tuple;
    for (auto& pair : fen_map) {
      tuple = NewList(2);
      tuple->car(NewString(pair.first));

      // ポジションをパース。
      if (pair.first == "fen position") {
        LPointerVec position_vec(NUM_SQUARES);
        LPointerVec::iterator position_itr = position_vec.begin();
        for (auto c : pair.second) {
          switch (c) {
            case 'P':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("PAWN"), NewNil()));
              break;
            case 'N':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("KNIGHT"), NewNil()));
              break;
            case 'B':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("BISHOP"), NewNil()));
              break;
            case 'R':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("ROOK"), NewNil()));
              break;
            case 'Q':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("QUEEN"), NewNil()));
              break;
            case 'K':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("KING"), NewNil()));
              break;
            case 'p':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("PAWN"), NewNil()));
              break;
            case 'n':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("KNIGHT"), NewNil()));
              break;
            case 'b':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("BISHOP"), NewNil()));
              break;
            case 'r':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("ROOK"), NewNil()));
              break;
            case 'q':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("QUEEN"), NewNil()));
              break;
            case 'k':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("KING"), NewNil()));
              break;
            default:
              *position_itr = NewPair(NewSymbol("NO_SIDE"),
              NewPair(NewSymbol("EMPTY"), NewNil()));
              break;
          }
          ++position_itr;
        }

        tuple->cdr()->car(LPointerVecToList(position_vec));
        *ret_itr = tuple;

        ++ret_itr;
        continue;
      }

      if (pair.first == "fen to_move") {
        if (pair.second == "w") tuple->cdr()->car(NewSymbol("WHITE"));
        else tuple->cdr()->car(NewSymbol("BLACK"));

        *ret_itr = tuple;

        ++ret_itr;
        continue;
      }

      if (pair.first == "fen castling") {
        LPointerVec castling_vec;
        if (pair.second[0] != '-') {
          castling_vec.push_back(NewSymbol("WHITE_SHORT_CASTLING"));
        }
        if (pair.second[1] != '-') {
          castling_vec.push_back(NewSymbol("WHITE_LONG_CASTLING"));
        }
        if (pair.second[2] != '-') {
          castling_vec.push_back(NewSymbol("BLACK_SHORT_CASTLING"));
        }
        if (pair.second[3] != '-') {
          castling_vec.push_back(NewSymbol("BLACK_LONG_CASTLING"));
        }

        tuple->cdr()->car(LPointerVecToList(castling_vec));
        *ret_itr = tuple;

        ++ret_itr;
        continue;
      }

      if (pair.first == "fen en_passant") {
        if (pair.second != "-") {
          char temp[3] {
            static_cast<char>(pair.second[0] - 'a' + 'A'),
            pair.second[1],
            '\0'
          };
          tuple->cdr()->car(NewSymbol(temp));
        } else {
          tuple->cdr()->car(NewNil());
        }

        *ret_itr = tuple;
        ++ret_itr;
        continue;
      }

      if ((pair.first == "fen ply") || (pair.first == "fen clock")) {
        tuple->cdr()->car(NewNumber(std::stod(pair.second)));
        *ret_itr = tuple;
        ++ret_itr;
        continue;
      }

      // EPD拡張部分。
      tuple->cdr()->car(NewString(pair.second));
      *ret_itr = tuple;
      ++ret_itr;
    }

    return LPointerVecToList(ret_vec);
  }

  // to-fen-position
  DEF_LC_FUNCTION(Sayulisp::ToFENPosition) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 文字列を得る。
    LPointer position_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*position_ptr);

    // ビットボードを作る。
    Bitboard position[NUM_SIDES][NUM_PIECE_TYPES];
    INIT_ARRAY(position);

    // ビットボードにビットをセット。
    LObject* ptr = position_ptr.get();
    FOR_SQUARES(square) {
      // 要素をチェック。
      if (!ptr) {
        throw GenError("@sayulisp-error",
        "Not enough elements. Required 64 elemsents.");
      }
      CheckPiece(*(ptr->car()));

      // セット。
      Side side = ptr->car()->car()->number();
      PieceType piece_type = ptr->car()->cdr()->car()->number();
      position[side][piece_type] |= Util::SQUARE[square][R0];

      Next(&ptr);
    }

    return NewString(Util::ToFENPosition(position));
  }
  // ヘルプを作成する。
  void Sayulisp::SetHelp() {
    std::string help = "";

    // --- 定数 --- //
    help =
R"...(### Constants of Squares ###

<h6> Description </h6>

* Symbols of squares are from 'A1' to 'H8'.
* Each symbol binds Number.

      +---+---+---+---+---+---+---+---+
    8 | 56| 57| 58| 59| 60| 61| 62| 63|
      +---+---+---+---+---+---+---+---+
    7 | 48| 49| 50| 51| 52| 53| 54| 55|
      +---+---+---+---+---+---+---+---+
    6 | 40| 41| 42| 43| 44| 45| 46| 47|
      +---+---+---+---+---+---+---+---+
    5 | 32| 33| 34| 35| 36| 37| 38| 39|
      +---+---+---+---+---+---+---+---+
    4 | 24| 25| 26| 27| 28| 29| 30| 31|
      +---+---+---+---+---+---+---+---+
    3 | 16| 17| 18| 19| 20| 21| 22| 23|
      +---+---+---+---+---+---+---+---+
    2 |  8|  9| 10| 11| 12| 13| 14| 15|
      +---+---+---+---+---+---+---+---+
    1 |  0|  1|  2|  3|  4|  5|  6|  7|
      +---+---+---+---+---+---+---+---+
        a   b   c   d   e   f   g   h)..." ;
    AddHelp("A1", help);
    AddHelp("A2", help);
    AddHelp("A3", help);
    AddHelp("A4", help);
    AddHelp("A5", help);
    AddHelp("A6", help);
    AddHelp("A7", help);
    AddHelp("A8", help);
    AddHelp("B1", help);
    AddHelp("B2", help);
    AddHelp("B3", help);
    AddHelp("B4", help);
    AddHelp("B5", help);
    AddHelp("B6", help);
    AddHelp("B7", help);
    AddHelp("B8", help);
    AddHelp("C1", help);
    AddHelp("C2", help);
    AddHelp("C3", help);
    AddHelp("C4", help);
    AddHelp("C5", help);
    AddHelp("C6", help);
    AddHelp("C7", help);
    AddHelp("C8", help);
    AddHelp("D1", help);
    AddHelp("D2", help);
    AddHelp("D3", help);
    AddHelp("D4", help);
    AddHelp("D5", help);
    AddHelp("D6", help);
    AddHelp("D7", help);
    AddHelp("D8", help);
    AddHelp("E1", help);
    AddHelp("E2", help);
    AddHelp("E3", help);
    AddHelp("E4", help);
    AddHelp("E5", help);
    AddHelp("E6", help);
    AddHelp("E7", help);
    AddHelp("E8", help);
    AddHelp("F1", help);
    AddHelp("F2", help);
    AddHelp("F3", help);
    AddHelp("F4", help);
    AddHelp("F5", help);
    AddHelp("F6", help);
    AddHelp("F7", help);
    AddHelp("F8", help);
    AddHelp("G1", help);
    AddHelp("G2", help);
    AddHelp("G3", help);
    AddHelp("G4", help);
    AddHelp("G5", help);
    AddHelp("G6", help);
    AddHelp("G7", help);
    AddHelp("G8", help);
    AddHelp("H1", help);
    AddHelp("H2", help);
    AddHelp("H3", help);
    AddHelp("H4", help);
    AddHelp("H5", help);
    AddHelp("H6", help);
    AddHelp("H7", help);
    AddHelp("H8", help);

    help =
R"...(### Constants of Fyles ###

<h6> Description </h6>

* Symbols of fyles are form 'FYLE_A' to 'FYLE_H'.
* Each symbol binds Number.
    + 'FYLE_A' is '0'.
    + 'FYLE_B' is '1'.
    + 'FYLE_C' is '2'.
    + 'FYLE_D' is '3'.
    + 'FYLE_E' is '4'.
    + 'FYLE_F' is '5'.
    + 'FYLE_G' is '6'.
    + 'FYLE_H' is '7'.)...";
    AddHelp("FYLE_A", help);
    AddHelp("FYLE_B", help);
    AddHelp("FYLE_C", help);
    AddHelp("FYLE_D", help);
    AddHelp("FYLE_E", help);
    AddHelp("FYLE_F", help);
    AddHelp("FYLE_G", help);
    AddHelp("FYLE_H", help);

    help =
R"...(### Constants of Ranks ###

<h6> Description </h6>

* Symbols of ranks are form 'RANK_1' to 'RANK_8'.
* Each symbol binds Number.
    + 'RANK_1' is '0'.
    + 'RANK_2' is '1'.
    + 'RANK_3' is '2'.
    + 'RANK_4' is '3'.
    + 'RANK_5' is '4'.
    + 'RANK_6' is '5'.
    + 'RANK_7' is '6'.
    + 'RANK_8' is '7'.)...";
    AddHelp("RANK_1", help);
    AddHelp("RANK_2", help);
    AddHelp("RANK_3", help);
    AddHelp("RANK_4", help);
    AddHelp("RANK_5", help);
    AddHelp("RANK_6", help);
    AddHelp("RANK_7", help);
    AddHelp("RANK_8", help);

    help =
R"...(### Constants of Sides ###

<h6> Description </h6>

* Each symbol binds Number.
    + 'NO_SIDE' is '0'.
    + 'WHITE' is '1'.
    + 'BLACK' is '2'.)...";
    AddHelp("NO_SIDE", help);
    AddHelp("WHITE", help);
    AddHelp("BLACK", help);

    help =
R"...(### Constants of Pieces ###

<h6> Description </h6>

* Each symbol binds Number.
    + 'EMPTY' is '0'.
    + 'PAWN' is '1'.
    + 'KNIGHT' is '2'.
    + 'BISHOP' is '3'.
    + 'ROOK' is '4'.
    + 'QUEEN' is '5'.
    + 'KING' is '6'.)...";
    AddHelp("EMPTY", help);
    AddHelp("PAWN", help);
    AddHelp("KNIGHT", help);
    AddHelp("BISHOP", help);
    AddHelp("ROOK", help);
    AddHelp("QUEEN", help);
    AddHelp("KING", help);

    help =
R"...(### Constants of Castling Rights ###

<h6> Description </h6>

* Each symbol binds Number.
    + 'NO_CASTLING' is '0'.
    + 'WHITE_SHORT_CASTLING' is '1'.
    + 'WHITE_LONG_CASTLING' is '2'.
    + 'BLACK_SHORT_CASTLING' is '3'.
    + 'BLACK_LONG_CASTLING' is '4'.)...";
    AddHelp("NO_CASTLING", help);
    AddHelp("WHITE_SHORT_CASTLING", help);
    AddHelp("WHITE_LONG_CASTLING", help);
    AddHelp("BLACK_SHORT_CASTLING", help);
    AddHelp("BLACK_LONG_CASTLING", help);

    help =
R"...(### Getting squares ###

* `@get-white-pawn-position`
    + Returns List of Symbols of squares where White Pawns are on.
* `@get-white-knight-position`
    + Returns List of Symbols of squares where White Knights are on.
* `@get-white-bishop-position`
    + Returns List of Symbols of squares where White Bishops are on.
* `@get-white-rook-position`
    + Returns List of Symbols of squares where White Rooks are on.
* `@get-white-queen-position`
    + Returns List of Symbols of squares where White Queens are on.
* `@get-white-king-position`
    + Returns List of Symbols of squares where White King is on.
* `@get-black-pawn-position`
    + Returns List of Symbols of squares where Black Pawns are on.
* `@get-black-knight-position`
    + Returns List of Symbols of squares where Black Knights are on.
* `@get-black-bishop-position`
    + Returns List of Symbols of squares where Black Bishops are on.
* `@get-black-rook-position`
    + Returns List of Symbols of squares where Black Rooks are on.
* `@get-black-queen-position`
    + Returns List of Symbols of squares where Black Queens are on.
* `@get-black-king-position`
    + Returns List of Symbols of squares where Black King is on.
* `@get-empty-square-position`
    + Returns List of Symbols of empty squares.

<h6> Example </h6>

    (define my-engine (gen-engine))
    (display (my-engine '@get-white-pawn-position))
    
    ;; Output
    ;; > (A2 B2 C2 D2 E2 F2 G2 H2))...";
    AddHelp("engine @get-white-pawn-position", help);
    AddHelp("engine @get-white-knight-position", help);
    AddHelp("engine @get-white-bishop-position", help);
    AddHelp("engine @get-white-rook-position", help);
    AddHelp("engine @get-white-queen-position", help);
    AddHelp("engine @get-white-king-position", help);
    AddHelp("engine @get-black-pawn-position", help);
    AddHelp("engine @get-black-knight-position", help);
    AddHelp("engine @get-black-bishop-position", help);
    AddHelp("engine @get-black-rook-position", help);
    AddHelp("engine @get-black-queen-position", help);
    AddHelp("engine @get-black-king-position", help);

    help =
R"...(### Getting pieces ###

* `@get-piece <Square : Number or Symbol>`
    + Returns a side and type of the piece on `<Square>` as List.

* `@get-all-pieces`
    + Returns  pieces of each square on the board as List.

<h6> Example </h6>

    (define my-engine (gen-engine))
    (display (my-engine '@get-piece D1))
    
    ;; Output
    ;; > (WHITE QUEEN)
    
    (display (my-engine '@get-all-pieces))
    
    ;; Output
    ;; > ((WHITE ROOK) (WHITE KNIGHT) (WHITE BISHOP) (WHITE QUEEN)
    ;; > (WHITE KING) (WHITE BISHOP) (WHITE KNIGHT) (WHITE ROOK)
    ;; > (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN)
    ;; > (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN)
    ;; > (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK PAWN)
    ;; > (BLACK PAWN) (BLACK PAWN) (BLACK ROOK) (BLACK KNIGHT) (BLACK BISHOP)
    ;; > (BLACK QUEEN) (BLACK KING) (BLACK BISHOP) (BLACK KNIGHT)
    ;; > (BLACK ROOK)))...";
    AddHelp("engine @get-piece", help);
    AddHelp("engine @get-all-pieces", help);

    help =
R"...(### Getting states of game ###

* `@get-to-move`
    + Returns turn to move as Symbol.
* `@get-castling-rights`
    + Returns castling rights as Symbol.
* `@get-en-passant-square`
    + Returns en passant square as Symbol if it exists now.
* `@get-ply`
    + Returns plies of moves from starting of the game.
    + 1 move = 2 plies.
* `@get-clock`
    + Returns clock(plies for 50 Moves Rule).
* `@get-white-has-castled`
    + Returns Boolean whether White King has castled or not.
* `@get-black-has-castled`
    + Returns Boolean whether Black King has castled or not.
* `@get-fen`
    + Returns FEN of current position.
* `@to-string`
    + Returns visualized board.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    ;; Move pieces by UCI command.
    ;; 1.e4 e5 2.Nf3 Nc6 3.Bc4 Bc5 4.O-O d5
    ;; +---------------+
    ;; |r . b q k . n r|
    ;; |p p p . . p p p|
    ;; |. . n . . . . .|
    ;; |. . b p p . . .|
    ;; |. . B . P . . .|
    ;; |. . . . . N . .|
    ;; |P P P P . P P P|
    ;; |R N B Q . R K .|
    ;; +---------------+
    (my-engine '@input-uci-command
        "position startpos moves e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 e1g1 d7d5")
    
    (display (my-engine '@get-to-move))
    ;; Output
    ;; > Symbol: WHITE
    
    (display (my-engine '@get-castling-rights))
    ;; Output
    ;; > (BLACK_SHORT_CASTLING BLACK_LONG_CASTLING)
    
    (display (my-engine '@get-en-passant-square))
    ;; Output
    ;; > Symbol: D6
    
    (display (my-engine '@get-ply))
    ;; Output
    ;; > 9
    
    (display (my-engine '@get-clock))
    ;; Output
    ;; > 0
    
    (display (my-engine '@get-white-has-castled))
    ;; Output
    ;; > #t
    
    (display (my-engine '@get-black-has-castled))
    ;; Output
    ;; > #f
    
    (display (my-engine '@get-fen))
    ;; Output
    ;; > r1bqk1nr/ppp2ppp/2n5/2bpp3/2B1P3/5N2/PPPP1PPP/RNBQ1RK1 w kq d6 0 5
    
    (display (my-engine '@to-string))
    ;; Output
    ;; >  +-----------------+
    ;; > 8| r . b q k . n r |
    ;; > 7| p p p . . p p p |
    ;; > 6| . . n . . . . . |
    ;; > 5| . . b p p . . . |
    ;; > 4| . . B . P . . . |
    ;; > 3| . . . . . N . . |
    ;; > 2| P P P P . P P P |
    ;; > 1| R N B Q . R K . |
    ;; >  +-----------------+
    ;; >    a b c d e f g h
    ;; > To Move: w | Clock: 0 | Ply: 8
    ;; > En Passant Square: d6
    ;; > Castling Rights : kq)...";
    AddHelp("engine @get-to-move", help);
    AddHelp("engine @get-castling-rights", help);
    AddHelp("engine @get-en-passant-square", help);
    AddHelp("engine @get-ply", help);
    AddHelp("engine @get-clock", help);
    AddHelp("engine @get-white-has-castled", help);
    AddHelp("engine @get-black-has-castled", help);

    help =
R"...(### Setting states of game ###

* `@set-to-move <Side : Number or Symbol>`
    + Sets turn to move.
    + Returns previous setting.
* `@set-castling_rights <Castling rights : List>`
    + Sets castling rights.
    + Returns previous setting.
* `@set-en-passant-square <<Square : Number or Symbol> or <Nil>>`
    + Sets en passant square.
    + Returns previous setting.
* `@set-ply <Ply : Number>`
    + Sets plies(a half of one move).
    + Returns previous setting.
* `@set-clock <Ply : Number>`
    + Sets clock(plies for 50 moves rule).
    + Returns previous setting.

<h6> Example </h6>

    (define my-engine (gen-engine))
    (my-engine '@place-piece E4 PAWN WHITE)
    
    (display (my-engine '@set-to-move BLACK))
    ;; Output
    ;; > Symbol: WHITE
    
    (display (my-engine '@set-castling-rights
        (list WHITE_LONG_CASTLING BLACK_LONG_CASTLING)))
    ;; Output
    ;; > (WHITE_SHORT_CASTLING WHITE_LONG_CASTLING
    ;; > BLACK_SHORT_CASTLING BLACK_LONG_CASTLING)
    
    (display (my-engine '@set-en-passant-square E3))
    ;; Output
    ;; > ()
    
    (display (my-engine '@set-ply 111))
    ;; Output
    ;; > 1
    
    (display (my-engine '@set-clock 22))
    ;; Output
    ;; > 0
    
    ;; ---------------- ;;
    ;; Current Settings ;;
    ;; ---------------- ;;
    
    (display (my-engine '@get-to-move))
    ;; Output
    ;; > Symbol: BLACK
    
    (display (my-engine '@get-castling-rights))
    ;; Output
    ;; > (WHITE_LONG_CASTLING BLACK_LONG_CASTLING)
    
    (display (my-engine '@get-en-passant-square))
    ;; Output
    ;; > Symbol: E3
    
    (display (my-engine '@get-ply))
    ;; Output
    ;; > 111
    
    (display (my-engine '@get-clock))
    ;; Output
    ;; > 22)...";
    AddHelp("engine @set-to-move", help);
    AddHelp("engine @set-castling-rights", help);
    AddHelp("engine @set-en-passant-square", help);
    AddHelp("engine @set-ply", help);
    AddHelp("engine @set-clock", help);

    help =
R"...(### Placing pieces ###

* `@set-new-game`
    + Sets starting position.
    + Returns #t.
* `@set-fen <FEN : String>`
    + Sets position with FEN.
    + Returns #t.
* `@place-piece <Square : Number or Symbol> <Piece : List>`
    + Sets a `<Piece>` on `<Square>`
      and returns the previous piece placed on `<Square>`.
    + `<Piece>` is `(<Side : Number or Symbol> <Type : Number or Symbol>).
        - For example, White Pawn is `(list WHITE PAWN)`.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@set-new-game))
    ;; Output
    ;; > #t
    
    (display (my-engine '@set-fen
        "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1"))
    ;; Output
    ;; > #t
    
    ;; Sets Black Rook on D1 where White Queen is placed.
    (display (my-engine '@place-piece D1 (list BLACK ROOK)))
    ;; Output
    ;; > (WHITE QUEEN))...";
    AddHelp("engine @set-new-game", help);
    AddHelp("engine @set-fen", help);
    AddHelp("engine @place-piece", help);

    help =
R"...(### Getting candidate moves ###

* `@get-candidate-moves`
    + Generates and returns List of candidate moves.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@get-candidate-moves))
    ;; Output
    ;; >  ((H2 H4 EMPTY) (H2 H3 EMPTY) (G2 G4 EMPTY) (G2 G3 EMPTY)
    ;; > (F2 F4 EMPTY) (F2 F3 EMPTY) (E2 E4 EMPTY) (E2 E3 EMPTY) (D2 D4 EMPTY)
    ;; > (D2 D3 EMPTY) (C2 C4 EMPTY) (C2 C3 EMPTY) (B2 B4 EMPTY) (B2 B3 EMPTY)
    ;; > (A2 A4 EMPTY) (A2 A3 EMPTY) (G1 H3 EMPTY) (G1 F3 EMPTY) (B1 C3 EMPTY)
    ;; > (B1 A3 EMPTY)))...";
    AddHelp("engine @get-candidate-moves", help);

    help =
R"...(### Predicate functions ###

Judges each state of the current position.

* `@correct-position?`
    + If Pawn is on 1st or 8th rank, it returns #f.
    + When turn to move is White, if Black King is checked,
      then it returns #f.
    + When turn to move is Black, if White King is checked,
      then it returns #f.
    + Otherwise, returns #t.
* `@white-checked?`
* `@black-checked?`
* `@checkmated?`
* `@stalemated?`

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    ;; Put Pawn on 1st rank.
    (my-engine '@place-piece D1 PAWN WHITE)
    
    (display (my-engine '@correct-position?))
    ;; Output
    ;; > #f
    
    (define my-engine (gen-engine))
    
    ;; Move pieces by UCI command.
    ;; 1.d4 e6 2.Nf3 Bb4+
    ;; +---------------+
    ;; |r n b q k . n r|
    ;; |p p p p . p p p|
    ;; |. . . . p . . .|
    ;; |. . . . . . . .|
    ;; |. b . P . . . .|
    ;; |. . . . . N . .|
    ;; |P P P . P P P P|
    ;; |R N B Q K B . R|
    ;; +---------------+
    (my-engine '@input-uci-command
        "position startpos moves d2d4 e7e6 g1f3 f8b4")
    
    (display (my-engine '@white-checked?))
    ;; Output
    ;; > #t
    
    (display (my-engine '@black-checked?))
    ;; Output
    ;; > #f
    
    (define my-engine (gen-engine))
    
    ;; Move pieces by UCI command.
    ;; 1.f3 e5 2.g4 Qh4#
    ;; +---------------+
    ;; |r n b . k b n r|
    ;; |p p p p . p p p|
    ;; |. . . . . . . .|
    ;; |. . . . p . . .|
    ;; |. . . . . . P q|
    ;; |. . . . . P . .|
    ;; |P P P P P . . P|
    ;; |R N B Q K B N R|
    ;; +---------------+
    (my-engine '@input-uci-command
        "position startpos moves f2f3 e7e5 g2g4 d8h4")
    
    (display (my-engine '@checkmated?))
    ;; Output
    ;; > #t
    
    (display (my-engine '@stalemated?))
    ;; Output
    ;; > #f)...";
    AddHelp("engine @correct-position?", help);
    AddHelp("engine @white-checked?", help);
    AddHelp("engine @black-checked?", help);
    AddHelp("engine @checkmated?", help);
    AddHelp("engine @stalemated?", help);

    help =
R"...(### Taking a move ###

A move is represented by List.  The List is  
`(<From : Number or Symbol>
  <To : Number or Symbol>
  <Promotion : Number or Symbol>)`.

* `@play-move <Move : List>`
    + Moves one piece legally.
    + Returns #t if it has succeeded, otherwise returns #f.

* `@undo-move`
    + Undoes previous move.
    + Returns previous move.

* `@play-note <PGN move text : String>`
    + Moves one piece legally with `<PGN move text>`.
    + Returns #t if it has succeeded, otherwise returns #f.

* `@move->note <Move : List>`
    + Translates Move into PGN move text according to the current position.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@play-move (list E2 E4 EMPTY)))
    ;; Output
    ;; > #t
    
    (display (my-engine '@get-white-pawn-position))
    ;; Output
    ;; > (A2 B2 C2 D2 F2 G2 H2 E4)
    
    (display (my-engine '@undo-move))
    ;; Output
    ;; > (E2 E4 EMPTY)
    
    (display (my-engine '@get-white-pawn-position))
    ;; Output
    ;; > (A2 B2 C2 D2 E2 F2 G2 H2)
    
    (display (my-engine '@play-note "Nf3"))
    ;; Output
    ;; > #t
    
    (display (my-engine '@get-white-knight-position))
    ;; Output
    ;; > (B1 F3)
    
    (display (my-engine '@move->note (list B8 C6 EMPTY)))
    ;; Output
    ;; > Nc6)...";
    AddHelp("engine @play-move", help);
    AddHelp("engine @undo-move", help);
    AddHelp("engine @play-note", help);
    AddHelp("engine @move->note", help);

    help =
R"...(### UCI Command ###

* `@input-uci-command <UCI command : String>`
    + Executes `<UCI command>`.
    + If success, returns #t. Otherwise, returns #f.
    + If you have input "go" command,
      the engine starts thinking in background.
      So control will come back soon.
* `@add-uci-output-listener <Listener : Function>`
    + Registers Function to receive UCI output from the engine.
    + `<Listener>` is Function that has one argument(UCI output).
* `@run`
    + Runs as UCI Chess Engine until the engine gets "quit" command.
    + The control doesn't come back while the engine is running.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    ;; Create a listener.
    (define (listener message)
        (display "I'm Listener : " message))
    
    ;; Register the listener.
    (my-engine '@add-uci-output-listener listener)
    
    (display (my-engine '@input-uci-command "uci"))
    ;; Output
    ;; > I'm Listener : id name Sayuri 2015.03.27 devel
    ;; > I'm Listener : id author Hironori Ishibashi
    ;; > I'm Listener : option name Hash type spin default 32 min 8 max 8192
    ;; > I'm Listener : option name Clear Hash type button
    ;; > I'm Listener : option name Ponder type check default true
    ;; > I'm Listener : option name Threads type spin default 1 min 1 max 64
    ;; > I'm Listener : option name UCI_AnalyseMode type check default false
    ;; > I'm Listener : uciok
    ;; > #t)...";
    AddHelp("engine @input-uci-command", help);
    AddHelp("engine @add-uci-output-listener", help);
    AddHelp("engine @run", help);

    help =
R"...(### Searching the best move ###

Thinks and returns the information of PV Line in each condition.  
Different from "go" command,
the control won't come back until the engine have found the best move.

* A return value is `(<Score> <Mate in N> <PV Move>...)`
    + `<Score>` is advantage for the engine.
    + About `<Mate in N>`.
        - If that is a positive number, checkmate in N by engine.
        - If that is a negative number, the engine is checkmated in N.
        - If that is Nil, the engine couldn't find checkmate.

* `@go-movetime <Milliseconds : Number> [<Candidate move list : List>]`
    + Thinks for `<Milliseconds>`.
* `@go-timelimit <Milliseconds : Number> [<Candidate move list : List>]`
    + Thinks on the basis of `<Milliseconds>`.
        - If `<Milliseconds>` is more than 600000,
          the engine thinks for 60000 milliseconds.
        - If `<Milliseconds>` is less than 600000,
          the engine thinks for "`<Milliseconds>` / 10" milliseconds.
* `@go-depth <Ply : Number> [<Candidate move list : List>]`
    + Thinks until to reach `<Ply>`th depth.
* `@go-nodes <Nodes : Number> [<Candidate move list : List>]`
    + Thinks until to search `<Nodes>` nodes.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    ;; Starting position.
    (display (my-engine '@go-movetime 10000))
    ;; Output
    ;; > (31 () (E2 E4 EMPTY) (E7 E5 EMPTY) (G1 F3 EMPTY) (G8 F6 EMPTY)
    :: > (B1 C3 EMPTY) (B8 C6 EMPTY) (F1 B5 EMPTY) (F8 B4 EMPTY) (E1 G1 EMPTY)
    :: > (B4 C3 EMPTY) (D2 C3 EMPTY))

    ;; Mate in 2.
    (define fen
      "2bqkbn1/2pppp2/np2N3/r3P1p1/p2N2B1/5Q2/PPPPKPP1/RNB2r2 w - - 0 1")
    (my-engine '@set-fen fen)

    (display (my-engine '@go-movetime 10000))
    ;; > (1000000 2 (F3 F7 EMPTY) (E8 F7 EMPTY) (G4 H5 EMPTY)))...";
    AddHelp("engine @go-movetime", help);
    AddHelp("engine @go-timelimit", help);
    AddHelp("engine @go-depth", help);
    AddHelp("engine @go-nodes", help);

    help =
R"...(### Hash and threads ###

* `@set-hash-size <Size : Number>`
    + Sets size of Hash Table(Transposition Table)
      and returns the previous size.
    + The unit of size is "byte".
* `@set-threads <Number of threads : Number>`
    + Sets `<Number of threads>` and returns the previous number.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    ;; Set size of Hash Table to 128 MB.
    (my-engine '@input-uci-command "setoption name hash value 128")
    
    (display (my-engine '@set-hash-size (* 256 1024 1024)))
    ;; Set size of Hash Table to 256 MB and return 128 * 1024 * 1024 bytes.
    ;; Output
    ;; > 1.34218e+08
    
    ;; Set the number of threads to 3.
    (my-engine '@input-uci-command "setoption name threads value 3")
    
    (display (my-engine '@set-threads 4))
    ;; Set the number of threads to 4 and return 3.
    ;; Output
    ;; > 3)...";
    AddHelp("engine @set-hash-size", help);
    AddHelp("engine @set-threads", help);

    help =
R"...(### Customizing Search Algorithm - Material ###

* `@material [<New materal : List>]`
    + Returns List of material.
        - 1st : Empty (It is always 0)
        - 2nd : Pawn
        - 3rd : Knight
        - 4th : Bishop
        - 5th : Rook
        - 6th : Queen
        - 7th : King
    + If you specify `<New materal>`, the material is updated.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@material (list 111 222 333 444 555 666 777)))
    ;; Output
    ;; > (0 100 400 400 600 1200 1e+06)
    
    (display (my-engine '@material))
    ;; Output
    ;; > (0 222 333 444 555 666 777))...";
    AddHelp("engine @material", help);

    help =
R"...(### Customizing Search Algorithm - Quiescence Search ###

* `@enable-quiesce-search [<New setting : Boolean>]`
    + Returns whether Quiescence Search is enabled or not.
    + If you specify #t to `<New setting>`,
      Quiescence Search is set to be enabled.
      Otherwise, it is set to be disabled.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-quiesce-search #f))
    ;; Output
    :: > #t
    
    (display (my-engine '@enable-quiesce-search))
    ;; Output
    :: > #f)...";
    AddHelp("engine @enable-quiesce-search", help);

    help =
R"...(### Customizing Search Algorithm - Repetition Check ###

* `@enable-repetition-check [<New setting : Boolean>]`
    + Returns whether Repetition Check is enabled or not.
    + If you specify #t to `<New setting>`,
      Repetition Check is set to be enabled.
      Otherwise, it is set to be disabled.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-repetition-check #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-repetition-check))
    ;; Output
    ;; > #f)...";
    AddHelp("engine @enable-repetition-check", help);

    help =
R"...(### Customizing Search Algorithm - Check Extension ###

* `@enable-check-extension [<New setting : Boolean>]`
    + Returns whether Check Extension is enabled or not.
    + If you specify #t to `<New setting>`,
      Check Extension is set to be enabled.
      Otherwise, it is set to be disabled.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-check-extension #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-check-extension))
    ;; Output
    ;; > #f)...";
    AddHelp("engine @enable-check-extension", help);

    help =
R"...(### Customizing Search Algorithm - YBWC ###

* `@ybwc-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter, YBWC is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.

* `@ybwc-invalid-moves [<New number of moves : Number>]`
    + YBWC searches with one thread during this parameter of candidate moves.
    + Return this parameter.
    + If you specify `<New number of moves>`, this parameter is updated.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@ybwc-limit-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@ybwc-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@ybwc-invalid-moves 10))
    ;; Output
    ;; > 3
    
    (display (my-engine '@ybwc-invalid-moves))
    ;; Output
    ;; > 10)...";
    AddHelp("engine @ybwc-limit-depth", help);
    AddHelp("engine @ybwc-invalid-moves", help);

    help =
R"...(### Customizing Search Algorithm - Aspiration Windows ###

* `@enable-aspiration-windows [<New setting : Boolean>]`
    + Returns whether Aspiration Windows is enabled or not.
    + If you specify #t to `<New setting>`,
      Aspiration Windows is set to be enabled.
      Otherwise, it is set to be disabled.
* `@aspiration-windows-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter at the root node,
      Aspiration Windows is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@aspiration-windows-delta [<New delta : Number>]`
    + Return Delta.
    + If you specify `<New delta>`, Delta is updated.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-aspiration-windows #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-aspiration-windows))
    ;; Output
    ;; > #f
    
    (display (my-engine '@aspiration-windows-limit-depth 10))
    ;; Output
    ;; > 5
    
    (display (my-engine '@aspiration-windows-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@aspiration-windows-delta 20))
    ;; Output
    ;; > 15
    
    (display (my-engine '@aspiration-windows-delta))
    ;; Output
    ;; > 20)...";
    AddHelp("engine @enable-aspiration-windows", help);
    AddHelp("engine @aspiration-windows-limit-depth", help);
    AddHelp("engine @aspiration-windows-delta", help);

    help =
R"...(### Customizing Search Algorithm - Aspiration Windows ###

* `@enable-see [<New setting : Boolean>]`
    + Returns whether SEE is enabled or not.
    + If you specify #t to `<New setting>`,
      SEE is set to be enabled. Otherwise, it is set to be disabled.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-see #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-see))
    ;; Output
    ;; > #f)...";
    AddHelp("engine @enable-see", help);

    help =
R"...(### Customizing Search Algorithm - History Heuristics ###

* `@enable-history [<New setting : Boolean>]`
    + Returns whether History Heuristics is enabled or not.
    + If you specify #t to `<New setting>`,
      History Heuristics is set to be enabled.
      Otherwise, it is set to be disabled.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-history #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-history))
    ;; Output
    ;; > #f)...";
    AddHelp("engine @enable-history", help);

    help =
R"...(### Customizing Search Algorithm - Killer Move Heuristics ###

* `@enable-killer [<New setting : Boolean>]`
    + Returns whether Killer Move Heuristics is enabled or not.
    + If you specify #t to `<New setting>`,
      Killer Move Hiuristics is set to be enabled.
      Otherwise, it is set to be disabled.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-killer #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-killer))
    ;; Output
    ;; > #f)...";
    AddHelp("engine @enable-killer", help);

    help =
R"...(### Customizing Search Algorithm - Hash Table ###

* `@enable-hash-table [<New setting : Boolean>]`
    + Returns whether Transposition Table is enabled or not.
    + If you specify #t to `<New setting>`,
      Transposition Table is set to be enabled.
      Otherwise, it is set to be disabled.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-hash-table #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-hash-table))
    ;; Output
    ;; > #t)...";
    AddHelp("engine @enable-hash-table", help);

    help =
R"...(### Customizing Search Algorithm - Internal Iterative Deepening ###

* `@enable-iid [<New setting : Boolean>]`
    + Returns whether Internal Iterative Deepening is enabled or not.
    + If you specify #t to `<New setting>`,
      Internal Iterative Deepening is set to be enabled.
      Otherwise, it is set to be disabled.
* `@iid-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter,
      Internal Iterative Deepening is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@iid-search-depth [<New depth : Number>]`
    + Internal Iterative Deepening searches until depth of this parameter.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-iid #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-iid))
    ;; Output
    ;; > #f
    
    (display (my-engine '@iid-limit-depth 10))
    ;; Output
    ;; > 5
    
    (display (my-engine '@iid-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@iid-search-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@iid-search-depth))
    ;; Output
    ;; > 10)...";
    AddHelp("engine @enable-iid", help);
    AddHelp("engine @iid-limit-depth", help);
    AddHelp("engine @iid-search-depth", help);

    help =
R"...(### Customizing Search Algorithm - Null Move Reduction ###

* `@enable-nmr [<New setting : Boolean>]`
    + Returns whether Null Move Reduction is enabled or not.
    + If you specify #t to `<New setting>`,
      Null Move Reduction is set to be enabled.
      Otherwise, it is set to be disabled.
* `@nmr-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter,
      Null Move Reduction is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@nmr-search-reduction [<New reduction : Number>]`
    + When searching shallowly, the depth is the actual depth
      minus this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.
* `@nmr-reduction [<New reduction : Number>]`
    + If the score is greater than or equals to Beta,
      the remaining depth is reduced by this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-nmr #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-nmr))
    ;; Output
    ;; > #f
    
    (display (my-engine '@nmr-limit-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@nmr-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@nmr-search-reduction 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@nmr-search-reduction))
    ;; Output
    ;; > 10
    
    (display (my-engine '@nmr-reduction 10))
    ;; Output
    ;; > 3
    
    (display (my-engine '@nmr-reduction))
    ;; Output
    ;; > 10)...";
    AddHelp("engine @enable-nmr", help);
    AddHelp("engine @nmr-limit-depth", help);
    AddHelp("engine @nmr-search-reduction", help);
    AddHelp("engine @nmr-reduction", help);

    help =
R"...(### Customizing Search Algorithm - ProbCut ###

* `@enable-probcut [<New setting : Boolean>]`
    + Returns whether ProbCut is enabled or not.
    + If you specify #t to `<New setting>`, ProbCut is set to be enabled.
      Otherwise, it is set to be disabled.
* `@probcut-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter, ProbCut is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@probcut-margin [<New margin : Number>]`
    + When Zero Window Search,
      ProbCut uses the current Beta plus this parameter as temporary Beta.
* `@probcut-search-reduction [<New reduction : Number>]`
    + When Zero Window Search, the depth is the actual depth
      minus this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-probcut #t))
    ;; Output
    ;; > #f
    
    (display (my-engine '@enable-probcut))
    ;; Output
    ;; > #t
    
    (display (my-engine '@probcut-limit-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@probcut-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@probcut-margin 1200))
    ;; Output
    ;; > 400
    
    (display (my-engine '@probcut-margin))
    ;; Output
    ;; > 1200
    
    (display (my-engine '@probcut-search-reduction 10))
    ;; Output
    ;; > 3
    
    (display (my-engine '@probcut-search-reduction))
    ;; Output
    ;; > 10)...";
    AddHelp("engine @enable-probcut", help);
    AddHelp("engine @probcut-limit-depth", help);
    AddHelp("engine @probcut-margin", help);
    AddHelp("engine @probcut-search-reduction", help);

    help =
R"...(### Customizing Search Algorithm - History Pruning ###

* `@enable-history-pruning [<New setting : Boolean>]`
    + Returns whether History Pruning is enabled or not.
    + If you specify #t to `<New setting>`,
      History Pruning is set to be enabled.
      Otherwise, it is set to be disabled.
* `@history-pruning-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter,
      History Pruning is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@history-pruning-move-threshold [<New threshold : Number>]`
    + If the number of the candidate move is less
      than the number of all moves times this parameter,
      History Pruning is invalidated.
    + This parameter is between 0.0 and 1.0.
    + Return this parameter.
    + If you specify `<New threshold>`, this parameter is updated.
* `@history-pruning-invalid-moves [<New number of moves : Number>]`
    + If the number of the candidate moves is less than this parameter,
      History Pruning is invalidated.
    + This parameter is given priority to `@history-pruning-move-threshold`.
    + Return this parameter.
    + If you specify `<New number of moves>`, this parameter is updated.
* `@history-pruning-threshold [<New threshold : Number>]`
    + If the history value of the current candidate move is lower
      than the max history value times this parameter,
      History Pruning temporarily reduces the remaining depth.
    + Return this parameter.
    + If you specify `<New threshold>`, this parameter is updated.
* `@history-pruning-reduction [<New reduction : Number>]`
    + When History Pruning reduces the remaining depth,
      a new depth is the current depth minus this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-history-pruning #t))
    ;; Output
    ;; > #f
    
    (display (my-engine '@enable-history-pruning))
    ;; Output
    ;; > #t
    
    (display (my-engine '@history-pruning-limit-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@history-pruning-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@history-pruning-move-threshold 0.8))
    ;; Output
    ;; > 0.6
    
    (display (my-engine '@history-pruning-move-threshold))
    ;; Output
    ;; > 0.8
    
    (display (my-engine '@history-pruning-invalid-moves 20))
    ;; Output
    ;; > 10
    
    (display (my-engine '@history-pruning-invalid-moves))
    ;; Output
    ;; > 20
    
    (display (my-engine '@history-pruning-threshold 0.8))
    ;; Output
    ;; > 0.5
    
    (display (my-engine '@history-pruning-threshold))
    ;; Output
    ;; > 0.8
    
    (display (my-engine '@history-pruning-reduction 10))
    ;; Output
    ;; > 1
    
    (display (my-engine '@history-pruning-reduction))
    ;; Output
    ;; > 10)...";
    AddHelp("engine @enable-history-pruning", help);
    AddHelp("engine @history-pruning-limit-depth", help);
    AddHelp("engine @history-pruning-move-threshold", help);
    AddHelp("engine @history-pruning-invalid-moves", help);
    AddHelp("engine @history-pruning-threshold", help);
    AddHelp("engine @history-pruning-reduction", help);

    help =
R"...(### Customizing Search Algorithm - Late Move Reduction ###

* `@enable-lmr [<New setting : Boolean>]`
    + Returns whether Late Move Reduction is enabled or not.
    + If you specify #t to `<New setting>`,
      Late Move Reduction is set to be enabled.
      Otherwise, it is set to be disabled.
* `@lmr-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter,
      Late Move Reduction is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@lmr-move-threshold [<New threshold : Number>]`
    + If the number of the candidate move is less
      than the number of all moves times this parameter,
      Late Move Reduction is invalidated.
    + This parameter is between 0.0 and 1.0.
    + Return this parameter.
    + If you specify `<New threshold>`, this parameter is updated.
* `@lmr-invalid-moves [<New number of moves : Number>]`
    + If the number of the candidate moves is less than this parameter,
      Late Move Reduction is invalidated.
    + This parameter is given priority to `@lmr-move-threshold`.
    + Return this parameter.
    + If you specify `<New number of moves>`, this parameter is updated.
* `@lmr-search-reduction [<New reduction : Number>]`
    + When searching shallowly, the depth is the actual depth
      minus this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-lmr #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-lmr))
    ;; Output
    ;; > #f
    
    (display (my-engine '@lmr-limit-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@lmr-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@lmr-move-threshold 0.8))
    ;; Output
    ;; > 0.3
    
    (display (my-engine '@lmr-move-threshold))
    ;; Output
    ;; > 0.8
    
    (display (my-engine '@lmr-invalid-moves 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@lmr-invalid-moves))
    ;; Output
    ;; > 10
    
    (display (my-engine '@lmr-search-reduction 5))
    ;; Output
    ;; > 1
    
    (display (my-engine '@lmr-search-reduction))
    ;; Output
    ;; > 5)...";
    AddHelp("engine @enable-lmr", help);
    AddHelp("engine @lmr-move-threshold", help);
    AddHelp("engine @lmr-invalid-moves", help);
    AddHelp("engine @lmr-search-reduction", help);

    help =
R"...(### Customizing Search Algorithm - Futility Pruning ###

* `@enable-futility-pruning [<New setting : Boolean>]`
    + Returns whether Futility Pruning is enabled or not.
    + If you specify #t to `<New setting>`,
      Futility Pruning is set to be enabled.
      Otherwise, it is set to be disabled.
* `@futility-pruning-depth [<New depth : Number>]`
    + If the remaining depth is less than or equals to this parameter,
      Futility Pruning is executed.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.
* `@futility-pruning-margin [<New margin : Number>]`
    + If the material after the move is lower than Alpha minus this parameter,
      the move is not evaluated.

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-futility-pruning #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-futility-pruning))
    ;; Output
    ;; > #f
    
    (display (my-engine '@futility-pruning-depth 10))
    ;; Output
    ;; > 3
    
    (display (my-engine '@futility-pruning-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@futility-pruning-margin 1200))
    ;; Output
    ;; > 400
    
    (display (my-engine '@futility-pruning-margin))
    ;; Output
    ;; > 1200)...";
    AddHelp("engine @enable-futility-pruning", help);
    AddHelp("engine @futility-pruning-depth", help);
    AddHelp("engine @futility-pruning-margin", help);

    help = R"...( Sayuri Weight System
--------------------

### Weight by Polygonal Line Graph ###

<h6> Definition </h6>

`Point := a List that (number_of_pieces, weight)`

`Weight Graph := a List of Points.`

<h6> Example </h6>

    (define my-engine (gen-engine))
    (define weight-list '((1 111) (2 222) (3 333) (4 444) (5 555)))
    
    (display (my-engine '@weight-pass-pawn weight-list))
    (display (my-engine '@weight-pass-pawn))
    ;; > ((2 30) (14 30) (32 20))
    ;; > ((1 111) (2 222) (3 333) (4 444) (5 555))


### Weight by Opening weight and Ending weight ###

<h6> Definition </h6>

`Opening := From 32 to 15 pieces are on the board.`

`Ending := From 14 to 2 pieces are on the board.`

<h6> Calculation Formulas </h6>

On Opening :

    slope = (opening_weight - ending_weight) / (32 - 14)
    weight = slope * (x - 14) + ending_weight

On Ending :

    weight = ending_weight

<h6> Example </h6>

    (define my-engine (gen-engine))
    (define opening-weight 99.9)
    (define ending-weight 88.8)
    
    (display (my-engine '@weight-pass-pawn opening-weight ending-weight))
    (display (my-engine '@weight-pass-pawn))
    ;; Output
    ;; > ((2 30) (14 30) (32 20))
    ;; > ((2 88.8) (14 88.8) (32 99.9)))...";
    AddHelp("sayuri weight system", help);

    help =
R"...(### Customizing Evaluation Function - Piece Square Table ###

Returns Piece Square Table for each piece type.  
If you specify `<New table>`, this parameter is updated.

`score = weight * value_table[square]`

* `@pawn-square-table-opening [<New table : List>]`
* `@knight-square-table-opening [<New table : List>]`
* `@bishop-square-table-opening [<New table : List>]`
* `@rook-square-table-opening [<New table : List>]`
* `@queen-square-table-opening [<New table : List>]`
* `@king-square-table-opening [<New table : List>]`
* `@pawn-square-table-ending [<New table : List>]`
* `@knight-square-table-ending [<New table : List>]`
* `@bishop-square-table-ending [<New table : List>]`
* `@rook-square-table-ending [<New table : List>]`
* `@queen-square-table-ending [<New table : List>]`
* `@king-square-table-ending [<New table : List>]`

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@pawn-square-table-opening
    (list
      1 2 3 4 5 6 7 8
      11 22 33 44 55 66 77 88
      111 222 333 444 555 666 777 888
      1111 2222 3333 4444 5555 6666 7777 8888
      11111 22222 33333 44444 55555 66666 77777 88888
      111111 222222 333333 444444 555555 666666 777777 888888
      1111111 2222222 3333333 4444444 5555555 6666666 7777777 8888888
      11111111 22222222 33333333 44444444 55555555 66666666 77777777 88888888
    )))
    ;; Output
    ;; > (0 0 0 0 0 0 0 0
    ;; > 0 0 0 0 0 0 0 0
    ;; > 5 10 15 20 20 15 10 5
    ;; > 10 20 30 40 40 30 20 10
    ;; > 15 30 45 60 60 45 30 15
    ;; > 20 40 60 80 80 60 40 20
    ;; > 25 50 75 100 100 75 50 25
    ;; > 30 60 90 120 120 90 60 30)
    
    (display (my-engine '@pawn-square-table-opening))
    ;; Output
    ;; > (1 2 3 4 5 6 7 8
    ;; > 11 22 33 44 55 66 77 88
    ;; > 111 222 333 444 555 666 777 888
    ;; > 1111 2222 3333 4444 5555 6666 7777 8888
    ;; > 11111 22222 33333 44444 55555 66666 77777 88888
    ;; > 111111 222222 333333 444444 555555 666666 777777 888888
    ;; > 1111111 2222222 3333333 4444444 5555555 6666666 7777777 8888888
    ;; > 11111111 22222222 33333333 44444444 55555555 66666666 77777777
    ;; > 88888888)

<h6> Weights </h6>

Usage : Run `(help "sayuri weight system")`

* Opening Position
    + `@weight-pawn-opening-position`
    + `@weight-knight-opening-position`
    + `@weight-bishop-opening-position`
    + `@weight-rook-opening-position`
    + `@weight-queen-opening-position`
    + `@weight-king-opening-position`
* Ending Position
    + `@weight-pawn-ending-position`
    + `@weight-knight-ending-position`
    + `@weight-bishop-ending-position`
    + `@weight-rook-ending-position`
    + `@weight-queen-ending-position`
    + `@weight-king-ending-position`)...";
    AddHelp("engine @pawn-square-table-opening", help);
    AddHelp("engine @knight-square-table-opening", help);
    AddHelp("engine @bishop-square-table-opening", help);
    AddHelp("engine @rook-square-table-opening", help);
    AddHelp("engine @queen-square-table-opening", help);
    AddHelp("engine @king-square-table-opening", help);
    AddHelp("engine @pawn-square-table-ending", help);
    AddHelp("engine @knight-square-table-ending", help);
    AddHelp("engine @bishop-square-table-ending", help);
    AddHelp("engine @rook-square-table-ending", help);
    AddHelp("engine @queen-square-table-ending", help);
    AddHelp("engine @king-square-table-ending", help);
    AddHelp("engine @weight-pawn-opening-position", help);
    AddHelp("engine @weight-knight-opening-position", help);
    AddHelp("engine @weight-bishop-opening-position", help);
    AddHelp("engine @weight-rook-opening-position", help);
    AddHelp("engine @weight-queen-opening-position", help);
    AddHelp("engine @weight-king-opening-position", help);
    AddHelp("engine @weight-pawn-ending-position", help);
    AddHelp("engine @weight-knight-ending-position", help);
    AddHelp("engine @weight-bishop-ending-position", help);
    AddHelp("engine @weight-rook-ending-position", help);
    AddHelp("engine @weight-queen-ending-position", help);
    AddHelp("engine @weight-king-ending-position", help);

    help =
R"...(### Customizing Evaluation Function - Attack ###

Returns List composed of 7 values of attacking score.  
1st: Not used. This is always 0. (for EMPTY)  
2nd: Attacking Pawn.  
3rd: Attacking Knight.  
4th: Attacking Bishop.  
5th: Attacking Rook.  
6th: Attacking Queen.  
7th: Attacking King.

If you specify `<New table>`, this parameter is updated.

`score = weight * value_table[attacking_piece][attacked_piece]`

* `@pawn-attack-table [<New table : List>]`
* `@knight-attack-table [<New table : List>]`
* `@bishop-attack-table [<New table : List>]`
* `@rook-attack-table [<New table : List>]`
* `@queen-attack-table [<New table : List>]`
* `@king-attack-table [<New table : List>]`

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@pawn-attack-table (list 1 2 3 4 5 6 7)))
    ;; Output
    ;; > (0 10 12 14 16 18 20)
    
    (display (my-engine '@pawn-attack-table))
    ;; Output
    ;; > (0 2 3 4 5 6 7)
    
<h6> Weights </h6>

Usage : Run `(help "sayuri weight system")`

* `@weight-pawn-attack`
* `@weight-knight-attack`
* `@weight-bishop-attack`
* `@weight-rook-attack`
* `@weight-queen-attack`
* `@weight-king-attack`)...";
    AddHelp("engine @pawn-attack-table", help);
    AddHelp("engine @knight-attack-table", help);
    AddHelp("engine @bishop-attack-table", help);
    AddHelp("engine @rook-attack-table", help);
    AddHelp("engine @queen-attack-table", help);
    AddHelp("engine @king-attack-table", help);
    AddHelp("engine @weight-pawn-attack", help);
    AddHelp("engine @weight-knight-attack", help);
    AddHelp("engine @weight-bishop-attack", help);
    AddHelp("engine @weight-rook-attack", help);
    AddHelp("engine @weight-queen-attack", help);
    AddHelp("engine @weight-king-attack", help);

    help =
R"...(### Customizing Evaluation Function - Defense ###

Returns List composed of 7 values of defense score.  
1st: Not used. This is always 0. (for EMPTY)  
2nd: Protecting Pawn.  
3rd: Protecting Knight.  
4th: Protecting Bishop.  
5th: Protecting Rook.  
6th: Protecting Queen.  
7th: Protecting King.

If you specify `<New table>`, this parameter is updated.

`score = weight * value_table[defensing_piece][defensed_piece]`

* `@pawn-defense-table [<New table : List>]`
* `@knight-defense-table [<New table : List>]`
* `@bishop-defense-table [<New table : List>]`
* `@rook-defense-table [<New table : List>]`
* `@queen-defense-table [<New table : List>]`
* `@king-defense-table [<New table : List>]`

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@pawn-defense-table (list 1 2 3 4 5 6 7)))
    ;; Output
    ;; > (0 10 0 0 0 0 0)
    
    (display (my-engine '@pawn-defense-table))
    ;; Output
    ;; > (0 2 3 4 5 6 7)
    
<h6> Weights </h6>

Usage : Run `(help "sayuri weight system")`

* `@weight-pawn-defense`
* `@weight-knight-defense`
* `@weight-bishop-defense`
* `@weight-rook-defense`
* `@weight-queen-defense`
* `@weight-king-defense`)...";
    AddHelp("engine @pawn-defense-table", help);
    AddHelp("engine @knight-defense-table", help);
    AddHelp("engine @bishop-defense-table", help);
    AddHelp("engine @rook-defense-table", help);
    AddHelp("engine @queen-defense-table", help);
    AddHelp("engine @king-defense-table", help);
    AddHelp("engine @weight-pawn-defense", help);
    AddHelp("engine @weight-knight-defense", help);
    AddHelp("engine @weight-bishop-defense", help);
    AddHelp("engine @weight-rook-defense", help);
    AddHelp("engine @weight-queen-defense", help);
    AddHelp("engine @weight-king-defense", help);

    help =
R"...(### Customizing Evaluation Function - Pin ###

Returns List of 7 Lists of 7 values.

1st list : Futile list. (For EMPTY)  
2nd list : A target piece is Pawn.  
3rd list : A target piece is Knight.  
4th list : A target piece is Bishop.  
5th list : A target piece is Rook.  
6th list : A target piece is Queen.  
7th list : A target piece is King.

1st value of each list : Futile value.
2nd value of each list : A piece over the target is Pawn.
3rd value of each list : A piece over the target is Knight.
4th value of each list : A piece over the target is Bishop.
5th value of each list : A piece over the target is Rook.
6th value of each list : A piece over the target is Queen.
7th value of each list : A piece over the target is King.

`score = weight * value_table[pinning_piece][target][over_the_target]`

* `@bishop-pin-table [<New value table : List>]`
* `@rook-pin-table [<New value table : List>]`
* `@queen-pin-table [<New value table : List>]`

<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (display (my-engine '@bishop-pin-table
      (list (list 1 2 3 4 5 6 7)
            (list 8 9 10 11 12 13 14)
            (list 15 16 17 18 19 20 21)
            (list 22 23 24 25 26 27 28)
            (list 29 30 31 32 33 34 35)
            (list 36 37 38 39 40 41 42)
            (list 43 44 45 46 47 48 49))))
    ;; Output
    ;; > ((0 0 0 0 0 0 0)
    ;; > (0 0 0 0 5 5 5)
    ;; > (0 0 0 0 10 10 10)
    ;; > (0 0 0 0 0 0 0)
    ;; > (0 0 0 0 20 30 40)
    ;; > (0 0 0 0 30 40 50)
    ;; > (0 0 0 0 40 50 0))
    
    (display (my-engine '@bishop-pin-table))
    ;; Output
    ;; >  ((0 0 0 0 0 0 0)
    ;; > (0 9 10 11 12 13 14)
    ;; > (0 16 17 18 19 20 21)
    ;; > (0 23 24 25 26 27 28)
    ;; > (0 30 31 32 33 34 35)
    ;; > (0 37 38 39 40 41 42)
    ;; > (0 44 45 46 47 48 49))
    
<h6> Weights </h6>

Usage : Run `(help "sayuri weight system")`

* `@weight-bishop-pin`
* `@weight-rook-pin`
* `@weight-queen-pin`)...";
    AddHelp("engine @bishop-pin-table", help);
    AddHelp("engine @rook-pin-table", help);
    AddHelp("engine @queen-pin-table", help);
    AddHelp("engine @weight-bishop-pin", help);
    AddHelp("engine @weight-rook-pin", help);
    AddHelp("engine @weight-queen-pin", help);

    help =
R"...(### Customizing Evaluation Function - Pawn Shield ###

If King on f1(f8) f2(f7) g1(g8) g2(g7) h1(h8) h2(h7),
then Pawn Shield is pawns on f g h files.  
If King on a1(a8) a2(a7) b1(b8) b2(b7) c1(c8) c2(c7),
then Pawn Shield is pawns on a b c files.

`score = weight * value_table[square]`

* `@pawn-shield-table [<New table : List>]`
    + Returns Piece Square Table for Pawn Shield
      as List composed of 64 numbers.
    + If you specify `<New table>`, this parameter is updated.


<h6> Example </h6>

    (define my-engine (gen-engine))
    
    (define table
      (list 1 2 3 4 5 6 7 8
            9 10 11 12 13 14 15 16
            17 18 19 20 21 22 23 24
            25 26 27 28 29 30 31 32
            33 34 35 36 37 38 39 40
            41 42 43 44 45 46 47 48
            49 50 51 52 53 54 55 56
            57 58 59 60 61 62 63 64))
    
    (display (my-engine '@pawn-shield-table table))
    ;; Output
    ;; > (0 0 0 0 0 0 0 0
    ;; > 30 30 30 30 30 30 30 30
    ;; > 0 0 0 0 0 0 0 0
    ;; > -30 -30 -30 -30 -30 -30 -30 -30
    ;; > -60 -60 -60 -60 -60 -60 -60 -60
    ;; > -90 -90 -90 -90 -90 -90 -90 -90
    ;; > -60 -60 -60 -60 -60 -60 -60 -60
    ;; > -30 -30 -30 -30 -30 -30 -30 -30)
    
    (display (my-engine '@pawn-shield-table))
    ;; Output
    ;; > (1 2 3 4 5 6 7 8
    ;; > 9 10 11 12 13 14 15 16
    ;; > 17 18 19 20 21 22 23 24
    ;; > 25 26 27 28 29 30 31 32
    ;; > 33 34 35 36 37 38 39 40
    ;; > 41 42 43 44 45 46 47 48
    ;; > 49 50 51 52 53 54 55 56
    ;; > 57 58 59 60 61 62 63 64)
    
<h6> Weights </h6>

Usage : Run `(help "sayuri weight system")`

* `@weight-pawn-shield`)...";
    AddHelp("engine @pawn-shield-table", help);
    AddHelp("engine @weight-pawn-shield", help);

    help =
R"...(### Customizing Evaluation Function - Mobility ###

About Sayuri's weight system : Run `(help "sayuri weight system")`

`score = weight * num_of_squares_that_piece_can_move_to`

* `@weight-pawn-mobility [<New weight : List>]`
* `@weight-knight-mobility [<New weight : List>]`
* `@weight-bishop-mobility [<New weight : List>]`
* `@weight-rook-mobility [<New weight : List>]`
* `@weight-queen-mobility [<New weight : List>]`
* `@weight-king-mobility [<New weight : List>]`)...";
    AddHelp("engine @weight-pawn-mobility", help);
    AddHelp("engine @weight-knight-mobility", help);
    AddHelp("engine @weight-bishop-mobility", help);
    AddHelp("engine @weight-rook-mobility", help);
    AddHelp("engine @weight-queen-mobility", help);
    AddHelp("engine @weight-king-mobility", help);

    help =
R"...(### Customizing Evaluation Function - Controlling Center ###

About Sayuri's weight system : Run `(help "sayuri weight system")`

Center square are c3 c4 c5 c6 d3 d4 d5 d6 e3 e4 e5 e6 f3 f4 f5 f6.

`score = weight * num_of_center_square_that_piece_attacks`

* `@weight-pawn-center-control [<New weight : List>]`
* `@weight-knight-center-control [<New weight : List>]`
* `@weight-bishop-center-control [<New weight : List>]`
* `@weight-rook-center-control [<New weight : List>]`
* `@weight-queen-center-control [<New weight : List>]`
* `@weight-king-center-control [<New weight : List>]`)...";
    AddHelp("engine @weight-pawn-center-control", help);
    AddHelp("engine @weight-knight-center-control", help);
    AddHelp("engine @weight-bishop-center-control", help);
    AddHelp("engine @weight-rook-center-control", help);
    AddHelp("engine @weight-queen-center-control", help);
    AddHelp("engine @weight-king-center-control", help);

    help =
R"...(### Customizing Evaluation Function - Controlling Sweet Center ###

About Sayuri's weight system : Run `(help "sayuri weight system")`

Sweet Center square are d3 d4 d5 d6 e3 e4 e5 e6.

`score = weight * num_of_sweet_center_square_that_piece_attacks`

* `@weight-pawn-sweet-center-control [<New weight : List>]`
* `@weight-knight-sweet-center-control [<New weight : List>]`
* `@weight-bishop-sweet-center-control [<New weight : List>]`
* `@weight-rook-sweet-center-control [<New weight : List>]`
* `@weight-queen-sweet-center-control [<New weight : List>]`
* `@weight-king-sweet-center-control [<New weight : List>]`)...";
    AddHelp("engine @weight-pawn-sweet-center-control", help);
    AddHelp("engine @weight-knight-sweet-center-control", help);
    AddHelp("engine @weight-bishop-sweet-center-control", help);
    AddHelp("engine @weight-rook-sweet-center-control", help);
    AddHelp("engine @weight-queen-sweet-center-control", help);
    AddHelp("engine @weight-king-sweet-center-control", help);

    help =
R"...(### Customizing Evaluation Function - Development ###

About Sayuri's weight system : Run `(help "sayuri weight system")`

`score = weight * num_of_minor_pieces_on_starting_position`

* `@weight-pawn-development [<New weight : List>]`
* `@weight-knight-development [<New weight : List>]`
* `@weight-bishop-development [<New weight : List>]`
* `@weight-rook-development [<New weight : List>]`
* `@weight-queen-development [<New weight : List>]`
* `@weight-king-development [<New weight : List>]`)...";
    AddHelp("engine @weight-pawn-development", help);
    AddHelp("engine @weight-knight-development", help);
    AddHelp("engine @weight-bishop-development", help);
    AddHelp("engine @weight-rook-development", help);
    AddHelp("engine @weight-queen-development", help);
    AddHelp("engine @weight-king-development", help);

    help =
R"...(### Customizing Evaluation Function - Attack around Enemy King ###

About Sayuri's weight system : Run `(help "sayuri weight system")`

`score = weight * num_of_squares_around_enemy_king_attacked_by_pieces.`

* `@weight-pawn-attack-around-king [<New weight : List>]`
* `@weight-knight-attack-around-king [<New weight : List>]`
* `@weight-bishop-attack-around-king [<New weight : List>]`
* `@weight-rook-attack-around-king [<New weight : List>]`
* `@weight-queen-attack-around-king [<New weight : List>]`
* `@weight-king-attack-around-king [<New weight : List>]`)...";
    AddHelp("engine @weight-pawn-attack-around-king", help);
    AddHelp("engine @weight-knight-attack-around-king", help);
    AddHelp("engine @weight-bishop-attack-around-king", help);
    AddHelp("engine @weight-rook-attack-around-king", help);
    AddHelp("engine @weight-queen-attack-around-king", help);
    AddHelp("engine @weight-king-attack-around-king", help);

    help =
R"...(### Customizing Evaluation Function - Pawn Structure ###

About Sayuri's weight system : Run `(help "sayuri weight system")`

* `@weight-pass-pawn [<New weight : List>]`
    + `score = weight * num_of_pass_pawns`
* `@weight-protected-pass-pawn [<New weight : List>]`
    + `score = weight * num_of_protected_pass_pawns`
* `@weight-double-pawn [<New weight : List>]`
    + `score = weight * num_of_doubled_pawns`
* `@weight-iso-pawn [<New weight : List>]`
    + `score = weight * num_of_isorated_pawns`)...";
    AddHelp("engine @weight-pass-pawn", help);
    AddHelp("engine @weight-protected-pass-pawn", help);
    AddHelp("engine @weight-double-pawn", help);
    AddHelp("engine @weight-iso-pawn", help);

    help =
R"...(### Customizing Evaluation Function - Piece ###

About Sayuri's weight system : Run `(help "sayuri weight system")`

* `@weight-bishop-pair [<New weight : List>]`
    + `score = weight * if_bishop_pair_exists_then_1_else_0`
* `@weight-bad-bishop [<New weight : List>]`
    + `score = weight * num_of_pawns_on_same_colored_square_as_bishop_on`

* `@weight-rook-pair [<New weight : List>]`
* `@weight-rook-semiopen-fyle [<New weight : List>]`
* `@weight-rook-open-fyle [<New weight : List>]`
* `@weight-early-queen-starting [<New weight : List>]`
* `@weight-weak-square [<New weight : List>]`
* `@weight-castling [<New weight : List>]`
* `@weight-abandoned-castling [<New weight : List>]`)...";
    AddHelp("engine @weight-bishop-pair", help);
    AddHelp("engine @weight-bad-bishop", help);
    AddHelp("engine @weight-rook-pair", help);
    AddHelp("engine @weight-rook-semiopen-fyle", help);
    AddHelp("engine @weight-rook-open-fyle", help);
    AddHelp("engine @weight-early-queen-starting", help);
    AddHelp("engine @weight-weak-square", help);
    AddHelp("engine @weight-castling", help);
    AddHelp("engine @weight-abandoned-castling", help);

    help =
R"...(### gen-pgn ###

<h6> Usage </h6>

* `(gen-pgn <PGN string : String>)`

<h6> Description </h6>

* Generates and returns PGN object from `<PGN string>`.
* PGN object is operated by Message Symbol.
* PGN object has 2 states.
    + Current game.
        - This can be changed by `@set-current-game`.
    + Current move.
        - This can be changed by `@next-move`, `@prev-move`, `@alt-move`,
          `@orig-move`, `@rewind-move`.

<h6> Description of Message Symbols </h6>

* `@get-pgn-comments`
    + Returns Lists of comments about PGN.

* `@get-current-game-comments.`
    + Returns List of comments about the current game.

* `@get-current-move-comments`
    + Returns List of comments about the current move.

* `@length`
    + Returns the number of games that PGN has.

* `@set-current-game <Index : Number>`
    + Sets a current game into the `<Index>`th game.

* `@get-current-game-headers`
    + Returns List of Lists composed with headers of the current game.
        - The format is "`((<Name 1> <value 1>) (<Name 2> <Value 2>)...)`".

* `@current-move`
    + Returns the current move text.

* `@next-move`
    + Change the current move into the next move
      and returns the move text.

* `@prev-move`
    + Change the current move into the previous move
      and returns the move text.

* `@alt-move`
    + Change the current move into the alternative move
      and returns the move text.

* `@orig-move`
    + If the current move is an alternative move,
      then change a current move into the original move
      and returns the move text.

* `@rewind-move`
    + Change a current move into the first move
      and returns the move text.

<h6> Example </h6>

    ;; Open PGN File.
    (define pgn-file (input-stream "/path/to/pgnfile.pgn"))
    
    ;; Reads the file and generates PGN object.
    (define my-pgn (gen-pgn (pgn-file '@read)))
    
    ;; Displays the current game headers.
    (display (my-pgn '@get-current-game-headers))
    
    ;; Output
    ;; > (("Black" "Hanako Yamada") ("Site" "Japan")
    ;; > ("White" "Hironori Ishibashi")))...";
    AddHelp("gen-pgn", help);

    help =
R"...(### parse-fen/epd ###

<h6> Usage </h6>

* `(parse-fen/epd <FEN or EPD : String>)`

<h6> Description </h6>

* Parses `<FEN or EPD>` and returns result value.
    +  A result value is `((<Tag 1 : String> <Object 1>)...)`.

<h6> Example </h6>

    (display (parse-fen/epd
        "rnbqkbnr/pp2pppp/3p4/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq d3 0 3"))
    ;; Output
    ;; > (("fen castling" (WHITE_SHORT_CASTLING
    ;; > WHITE_LONG_CASTLING BLACK_SHORT_CASTLING BLACK_LONG_CASTLING))
    ;; > ("fen clock" 0)
    ;; > ("fen en_passant" D3)
    ;; > ("fen ply" 5)
    ;; > ("fen position" ((WHITE ROOK) (WHITE KNIGHT) (WHITE BISHOP)
    ;; > (WHITE QUEEN) (WHITE KING) (WHITE BISHOP) (NO_SIDE EMPTY)
    ;; > (WHITE ROOK) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (WHITE KNIGHT) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (WHITE PAWN)
    ;; > (WHITE PAWN) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (BLACK PAWN) (BLACK PAWN) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK ROOK)
    ;; > (BLACK KNIGHT) (BLACK BISHOP) (BLACK QUEEN) (BLACK KING)
    ;; > (BLACK BISHOP) (BLACK KNIGHT) (BLACK ROOK)))
    ;; > ("fen to_move" BLACK)))...";
    AddHelp("parse-fen/epd", help);

    help =
R"...(### to-fen-position ###

<h6> Usage </h6>

* `(to-fen-position <Pieces list : List>)`

<h6> Description </h6>

* Analyses `<Pieces list>` and returns FEN position.

<h6> Example </h6>

    (display (to-fen-position
        '((WHITE KING) (WHITE KING)(WHITE KING) (WHITE KING)
        (WHITE QUEEN) (WHITE QUEEN)(WHITE QUEEN) (WHITE QUEEN)
        (WHITE ROOK) (WHITE ROOK)(WHITE ROOK) (WHITE ROOK)
        (WHITE BISHOP) (WHITE BISHOP)(WHITE BISHOP) (WHITE BISHOP)
        (WHITE KNIGHT) (WHITE KNIGHT)(WHITE KNIGHT) (WHITE KNIGHT)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (BLACK KNIGHT) (BLACK KNIGHT)(BLACK KNIGHT) (BLACK KNIGHT)
        (BLACK BISHOP) (BLACK BISHOP)(BLACK BISHOP) (BLACK BISHOP)
        (BLACK ROOK) (BLACK ROOK)(BLACK ROOK) (BLACK ROOK)
        (BLACK QUEEN) (BLACK QUEEN)(BLACK QUEEN) (BLACK QUEEN)
        (BLACK KING) (BLACK KING)(BLACK KING) (BLACK KING))))
    ;; Output
    ;; > qqqqkkkk/bbbbrrrr/4nnnn/8/8/NNNN4/RRRRBBBB/KKKKQQQQ)...";
    AddHelp("to-fen-position", help);
  }

  // =========== //
  // EngineSuite //
  // =========== //
  // コンストラクタ。
  EngineSuite::EngineSuite() :
  search_params_ptr_(new SearchParams()),
  eval_params_ptr_(new EvalParams()),
  table_ptr_(new TranspositionTable(UCI_DEFAULT_TABLE_SIZE)),
  engine_ptr_(new ChessEngine(*search_params_ptr_, *eval_params_ptr_,
  *table_ptr_)),
  board_ptr_(&(engine_ptr_->board())),
  shell_ptr_(new UCIShell(*engine_ptr_)) {
    // 出力リスナー。
    shell_ptr_->AddOutputListener
    ([this](const std::string& message) {this->ListenUCIOutput(message);});

    // メッセージシンボル関数の登録。
    SetMessageFunctions();
  }

  // コピーコンストラクタ。
  EngineSuite::EngineSuite(const EngineSuite& suite) :
  search_params_ptr_(new SearchParams(*(suite.search_params_ptr_))),
  eval_params_ptr_(new EvalParams(*(suite.eval_params_ptr_))),
  table_ptr_(new TranspositionTable(suite.table_ptr_->GetSizeBytes())),
  engine_ptr_(new ChessEngine(*search_params_ptr_, *eval_params_ptr_,
  *table_ptr_)),
  board_ptr_(&(engine_ptr_->board())),
  shell_ptr_(new UCIShell(*engine_ptr_)) {
    PositionRecord record(*(suite.engine_ptr_));
    engine_ptr_->LoadRecord(record);

    // 出力リスナー。
    shell_ptr_->AddOutputListener
    ([this](const std::string& message) {this->ListenUCIOutput(message);});

    // メッセージシンボル関数の登録。
    SetMessageFunctions();
  }

  // ムーブコンストラクタ。
  EngineSuite::EngineSuite(EngineSuite&& suite) :
  search_params_ptr_(std::move(suite.search_params_ptr_)),
  eval_params_ptr_(std::move(suite.eval_params_ptr_)),
  table_ptr_(std::move(suite.table_ptr_)),
  engine_ptr_(std::move(suite.engine_ptr_)),
  board_ptr_(&(engine_ptr_->board())),
  shell_ptr_(std::move(suite.shell_ptr_)),
  message_func_map_(std::move(suite.message_func_map_)) {
  }

  // コピー代入演算子。
  EngineSuite& EngineSuite::operator=(const EngineSuite& suite) {
    search_params_ptr_.reset(new SearchParams(*(suite.search_params_ptr_)));
    eval_params_ptr_.reset(new EvalParams(*(suite.eval_params_ptr_)));
    table_ptr_.reset(new TranspositionTable(suite.table_ptr_->GetSizeBytes()));
    engine_ptr_.reset(new ChessEngine(*search_params_ptr_, *eval_params_ptr_,
    *table_ptr_));
    board_ptr_ = &(engine_ptr_->board());
    shell_ptr_.reset(new UCIShell(*engine_ptr_));

    PositionRecord record(*(suite.engine_ptr_));
    engine_ptr_->LoadRecord(record);

    // 出力リスナー。
    shell_ptr_->AddOutputListener
    ([this](const std::string& message) {this->ListenUCIOutput(message);});

    return *this;
  }

  // ムーブ代入演算子。
  EngineSuite& EngineSuite::operator=(EngineSuite&& suite) {
    search_params_ptr_ = std::move(suite.search_params_ptr_);
    eval_params_ptr_ = std::move(suite.eval_params_ptr_);
    table_ptr_ = std::move(suite.table_ptr_);
    engine_ptr_ = std::move(suite.engine_ptr_);
    board_ptr_ = &(engine_ptr_->board());
    shell_ptr_ = std::move(suite.shell_ptr_);

    return *this;
  }

  // メッセージシンボル関数を登録する。
  void EngineSuite::SetMessageFunctions() {
    message_func_map_["@get-white-pawn-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, PAWN>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-knight-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, KNIGHT>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-bishop-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, BISHOP>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-rook-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, ROOK>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-queen-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, QUEEN>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-king-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, KING>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-pawn-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, PAWN>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-knight-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, KNIGHT>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-bishop-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, BISHOP>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-rook-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, ROOK>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-queen-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, QUEEN>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-king-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, KING>(symbol, self, caller, args);
    };

    message_func_map_["@get-piece"] =
    INSERT_MESSAGE_FUNCTION(GetPiece);

    message_func_map_["@get-all-pieces"] =
    INSERT_MESSAGE_FUNCTION(GetAllPieces);

    message_func_map_["@get-to-move"] =
    INSERT_MESSAGE_FUNCTION(GetToMove);

    message_func_map_["@get-castling-rights"] =
    INSERT_MESSAGE_FUNCTION(GetCastlingRights);

    message_func_map_["@get-en-passant-square"] =
    INSERT_MESSAGE_FUNCTION(GetEnPassantSquare);

    message_func_map_["@get-ply"] =
    INSERT_MESSAGE_FUNCTION(GetPly);

    message_func_map_["@get-clock"] =
    INSERT_MESSAGE_FUNCTION(GetClock);

    message_func_map_["@get-white-has-castled"] =
    INSERT_MESSAGE_FUNCTION(GetHasCastled<WHITE>);

    message_func_map_["@get-black-has-castled"] =
    INSERT_MESSAGE_FUNCTION(GetHasCastled<BLACK>);

    message_func_map_["@get-fen"] =
    INSERT_MESSAGE_FUNCTION(GetFEN);

    message_func_map_["@to-string"] =
    INSERT_MESSAGE_FUNCTION(BoardToString);

    message_func_map_["@set-new-game"] =
    INSERT_MESSAGE_FUNCTION(SetNewGame);

    message_func_map_["@set-fen"] =
    INSERT_MESSAGE_FUNCTION(SetFEN);

    message_func_map_["@place-piece"] =
    INSERT_MESSAGE_FUNCTION(PlacePiece);

    message_func_map_["@get-candidate-moves"] =
    INSERT_MESSAGE_FUNCTION(GetCandidateMoves);

    message_func_map_["@set-to-move"] =
    INSERT_MESSAGE_FUNCTION(SetToMove);

    message_func_map_["@set-castling-rights"] =
    INSERT_MESSAGE_FUNCTION(SetCastlingRights);

    message_func_map_["@set-en-passant-square"] =
    INSERT_MESSAGE_FUNCTION(SetEnPassantSquare);

    message_func_map_["@set-ply"] =
    INSERT_MESSAGE_FUNCTION(SetPly);

    message_func_map_["@set-clock"] =
    INSERT_MESSAGE_FUNCTION(SetClock);

    message_func_map_["@correct-position?"] =
    INSERT_MESSAGE_FUNCTION(IsCorrectPosition);

    message_func_map_["@white-checked?"] =
    INSERT_MESSAGE_FUNCTION(IsChecked<WHITE>);

    message_func_map_["@black-checked?"] =
    INSERT_MESSAGE_FUNCTION(IsChecked<BLACK>);

    message_func_map_["@checkmated?"] =
    INSERT_MESSAGE_FUNCTION(IsCheckmated);

    message_func_map_["@stalemated?"] =
    INSERT_MESSAGE_FUNCTION(IsStalemated);

    message_func_map_["@play-move"] =
    INSERT_MESSAGE_FUNCTION(PlayMoveOrNote);

    message_func_map_["@play-note"] =
    INSERT_MESSAGE_FUNCTION(PlayMoveOrNote);

    message_func_map_["@undo-move"] =
    INSERT_MESSAGE_FUNCTION(UndoMove);

    message_func_map_["@move->note"] =
    INSERT_MESSAGE_FUNCTION(MoveToNote);

    message_func_map_["@input-uci-command"] =
    INSERT_MESSAGE_FUNCTION(InputUCICommand);

    message_func_map_["@add-uci-output-listener"] =
    INSERT_MESSAGE_FUNCTION(AddUCIOutputListener);

    message_func_map_["@run"] =
    INSERT_MESSAGE_FUNCTION(RunEngine);

    message_func_map_["@go-movetime"] =
    INSERT_MESSAGE_FUNCTION(GoMoveTime);

    message_func_map_["@go-timelimit"] =
    INSERT_MESSAGE_FUNCTION(GoTimeLimit);

    message_func_map_["@go-depth"] =
    INSERT_MESSAGE_FUNCTION(GoDepth);

    message_func_map_["@go-nodes"] =
    INSERT_MESSAGE_FUNCTION(GoNodes);

    message_func_map_["@set-hash-size"] =
    INSERT_MESSAGE_FUNCTION(SetHashSize);

    message_func_map_["@set-threads"] =
    INSERT_MESSAGE_FUNCTION(SetThreads);

    message_func_map_["@material"] =
    INSERT_MESSAGE_FUNCTION(SetMaterial);

    message_func_map_["@enable-quiesce-search"] =
    INSERT_MESSAGE_FUNCTION(SetEnabelQuiesceSearch);

    message_func_map_["@enable-repetition-check"] =
    INSERT_MESSAGE_FUNCTION(SetEnabelRepetitionCheck);

    message_func_map_["@enable-check-extension"] =
    INSERT_MESSAGE_FUNCTION(SetEnableCheckExtension);

    message_func_map_["@ybwc-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetYBWCLimitDepth);

    message_func_map_["@ybwc-invalid-moves"] =
    INSERT_MESSAGE_FUNCTION(SetYBWCInvalidMoves);

    message_func_map_["@enable-aspiration-windows"] =
    INSERT_MESSAGE_FUNCTION(SetEnableAspirationWindows);

    message_func_map_["@aspiration-windows-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetAspirationWindowsLimitDepth);

    message_func_map_["@aspiration-windows-delta"] =
    INSERT_MESSAGE_FUNCTION(SetAspirationWindowsDelta);

    message_func_map_["@enable-see"] =
    INSERT_MESSAGE_FUNCTION(SetEnableSEE);

    message_func_map_["@enable-history"] =
    INSERT_MESSAGE_FUNCTION(SetEnableHistory);

    message_func_map_["@enable-killer"] =
    INSERT_MESSAGE_FUNCTION(SetEnableKiller);

    message_func_map_["@enable-hash-table"] =
    INSERT_MESSAGE_FUNCTION(SetEnableHashTable);

    message_func_map_["@enable-iid"] =
    INSERT_MESSAGE_FUNCTION(SetEnableIID);

    message_func_map_["@iid-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetIIDLimitDepth);

    message_func_map_["@iid-search-depth"] =
    INSERT_MESSAGE_FUNCTION(SetIIDSearchDepth);

    message_func_map_["@enable-nmr"] =
    INSERT_MESSAGE_FUNCTION(SetEnableNMR);

    message_func_map_["@nmr-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetNMRLimitDepth);

    message_func_map_["@nmr-search-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetNMRSearchReduction);

    message_func_map_["@nmr-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetNMRReduction);

    message_func_map_["@enable-probcut"] =
    INSERT_MESSAGE_FUNCTION(SetEnableProbCut);

    message_func_map_["@probcut-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetProbCutLimitDepth);

    message_func_map_["@probcut-margin"] =
    INSERT_MESSAGE_FUNCTION(SetProbCutMargin);

    message_func_map_["@probcut-search-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetProbCutSearchReduction);

    message_func_map_["@enable-history-pruning"] =
    INSERT_MESSAGE_FUNCTION(SetEnableHistoryPruning);

    message_func_map_["@history-pruning-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetHistoryPruningLimitDepth);

    message_func_map_["@history-pruning-move-threshold"] =
    INSERT_MESSAGE_FUNCTION(SetHistoryPruningMoveThreshold);

    message_func_map_["@history-pruning-threshold"] =
    INSERT_MESSAGE_FUNCTION(SetHistoryPruningThreshold);

    message_func_map_["@history-pruning-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetHistoryPruningReduction);

    message_func_map_["@enable-lmr"] =
    INSERT_MESSAGE_FUNCTION(SetEnableLMR);

    message_func_map_["@lmr-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetLMRLimitDepth);

    message_func_map_["@lmr-move-threshold"] =
    INSERT_MESSAGE_FUNCTION(SetLMRMoveThreshold);

    message_func_map_["@lmr-invalid-moves"] =
    INSERT_MESSAGE_FUNCTION(SetLMRInvalidMoves);

    message_func_map_["@lmr-search-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetLMRSearchReduction);

    message_func_map_["@enable-futility-pruning"] =
    INSERT_MESSAGE_FUNCTION(SetEnableFutilityPruning);

    message_func_map_["@futility-pruning-depth"] =
    INSERT_MESSAGE_FUNCTION(SetFutilityPruningDepth);

    message_func_map_["@futility-pruning-margin"] =
    INSERT_MESSAGE_FUNCTION(SetFutilityPruningMargin);

    message_func_map_["@pawn-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<PAWN>);

    message_func_map_["@knight-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<KNIGHT>);

    message_func_map_["@bishop-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<BISHOP>);

    message_func_map_["@rook-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<ROOK>);

    message_func_map_["@queen-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<QUEEN>);

    message_func_map_["@king-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<KING>);

    message_func_map_["@pawn-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<PAWN>);

    message_func_map_["@knight-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<KNIGHT>);

    message_func_map_["@bishop-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<BISHOP>);

    message_func_map_["@rook-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<ROOK>);

    message_func_map_["@queen-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<QUEEN>);

    message_func_map_["@king-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<KING>);

    message_func_map_["@pawn-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<PAWN>);

    message_func_map_["@knight-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<KNIGHT>);

    message_func_map_["@bishop-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<BISHOP>);

    message_func_map_["@rook-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<ROOK>);

    message_func_map_["@queen-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<QUEEN>);

    message_func_map_["@king-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<KING>);

    message_func_map_["@pawn-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<PAWN>);

    message_func_map_["@knight-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<KNIGHT>);

    message_func_map_["@bishop-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<BISHOP>);

    message_func_map_["@rook-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<ROOK>);

    message_func_map_["@queen-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<QUEEN>);

    message_func_map_["@king-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<KING>);

    message_func_map_["@bishop-pin-table"] =
    INSERT_MESSAGE_FUNCTION(SetPinTable<BISHOP>);

    message_func_map_["@rook-pin-table"] =
    INSERT_MESSAGE_FUNCTION(SetPinTable<ROOK>);

    message_func_map_["@queen-pin-table"] =
    INSERT_MESSAGE_FUNCTION(SetPinTable<QUEEN>);

    message_func_map_["@pawn-shield-table"] =
    INSERT_MESSAGE_FUNCTION(SetPawnShieldTable);

    message_func_map_["@weight-pawn-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<PAWN>);
    message_func_map_["@weight-knight-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<KNIGHT>);
    message_func_map_["@weight-bishop-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<BISHOP>);
    message_func_map_["@weight-rook-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<ROOK>);
    message_func_map_["@weight-queen-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<QUEEN>);
    message_func_map_["@weight-king-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<KING>);

    message_func_map_["@weight-pawn-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<PAWN>);
    message_func_map_["@weight-knight-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<KNIGHT>);
    message_func_map_["@weight-bishop-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<BISHOP>);
    message_func_map_["@weight-rook-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<ROOK>);
    message_func_map_["@weight-queen-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<QUEEN>);
    message_func_map_["@weight-king-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<KING>);

    message_func_map_["@weight-pawn-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<PAWN>);
    message_func_map_["@weight-knight-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<KNIGHT>);
    message_func_map_["@weight-bishop-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<BISHOP>);
    message_func_map_["@weight-rook-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<ROOK>);
    message_func_map_["@weight-queen-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<QUEEN>);
    message_func_map_["@weight-king-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<KING>);

    message_func_map_["@weight-pawn-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<PAWN>);
    message_func_map_["@weight-knight-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<KNIGHT>);
    message_func_map_["@weight-bishop-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<BISHOP>);
    message_func_map_["@weight-rook-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<ROOK>);
    message_func_map_["@weight-queen-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<QUEEN>);
    message_func_map_["@weight-king-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<KING>);

    message_func_map_["@weight-pawn-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<PAWN>);
    message_func_map_["@weight-knight-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<KNIGHT>);
    message_func_map_["@weight-bishop-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<BISHOP>);
    message_func_map_["@weight-rook-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<ROOK>);
    message_func_map_["@weight-queen-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<QUEEN>);
    message_func_map_["@weight-king-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<KING>);

    message_func_map_["@weight-pawn-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<PAWN>);
    message_func_map_["@weight-knight-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<KNIGHT>);
    message_func_map_["@weight-bishop-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<BISHOP>);
    message_func_map_["@weight-rook-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<ROOK>);
    message_func_map_["@weight-queen-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<QUEEN>);
    message_func_map_["@weight-king-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<KING>);

    message_func_map_["@weight-pawn-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<PAWN>);
    message_func_map_["@weight-knight-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<KNIGHT>);
    message_func_map_["@weight-bishop-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<BISHOP>);
    message_func_map_["@weight-rook-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<ROOK>);
    message_func_map_["@weight-queen-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<QUEEN>);
    message_func_map_["@weight-king-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<KING>);

    message_func_map_["@weight-pawn-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<PAWN>);
    message_func_map_["@weight-knight-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<KNIGHT>);
    message_func_map_["@weight-bishop-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<BISHOP>);
    message_func_map_["@weight-rook-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<ROOK>);
    message_func_map_["@weight-queen-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<QUEEN>);
    message_func_map_["@weight-king-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<KING>);

    message_func_map_["@weight-bishop-pin"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPin<BISHOP>);
    message_func_map_["@weight-rook-pin"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPin<ROOK>);
    message_func_map_["@weight-queen-pin"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPin<QUEEN>);

    message_func_map_["@weight-pawn-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<PAWN>);
    message_func_map_["@weight-knight-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<KNIGHT>);
    message_func_map_["@weight-bishop-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<BISHOP>);
    message_func_map_["@weight-rook-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<ROOK>);
    message_func_map_["@weight-queen-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<QUEEN>);
    message_func_map_["@weight-king-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<KING>);

    message_func_map_["@weight-pass-pawn"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPassPawn);

    message_func_map_["@weight-protected-pass-pawn"] =
    INSERT_MESSAGE_FUNCTION(SetWeightProtectedPassPawn);

    message_func_map_["@weight-double-pawn"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDoublePawn);

    message_func_map_["@weight-iso-pawn"] =
    INSERT_MESSAGE_FUNCTION(SetWeightIsoPawn);

    message_func_map_["@weight-pawn-shield"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPawnShield);

    message_func_map_["@weight-bishop-pair"] =
    INSERT_MESSAGE_FUNCTION(SetWeightBishopPair);

    message_func_map_["@weight-bad-bishop"] =
    INSERT_MESSAGE_FUNCTION(SetWeightBadBishop);

    message_func_map_["@weight-rook-pair"] =
    INSERT_MESSAGE_FUNCTION(SetWeightRookPair);

    message_func_map_["@weight-rook-semiopen-fyle"] =
    INSERT_MESSAGE_FUNCTION(SetWeightRookSemiopenFyle);

    message_func_map_["@weight-rook-open-fyle"] =
    INSERT_MESSAGE_FUNCTION(SetWeightRookOpenFyle);

    message_func_map_["@weight-early-queen-starting"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEarlyQueenStarting);

    message_func_map_["@weight-weak-square"] =
    INSERT_MESSAGE_FUNCTION(SetWeightWeakSquare);

    message_func_map_["@weight-castling"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCastling);

    message_func_map_["@weight-abandoned-castling"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAbandonedCastling);
  }

  // 関数オブジェクト。
  DEF_LC_FUNCTION(EngineSuite::operator()) {
    // 準備。
    LObject* args_ptr = nullptr;
    Lisp::GetReadyForFunction(args, 1, &args_ptr);

    // メッセージシンボルを抽出。
    LPointer result = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*result, LType::SYMBOL);
    const std::string& symbol = result->symbol();

    if (message_func_map_.find(symbol) != message_func_map_.end()) {
      return message_func_map_.at(symbol)(symbol, self, caller, args);
    }

    throw Lisp::GenError("@engine-error",
    "'" + symbol + "' is not message symbol.");
  }

  // ====================== //
  // メッセージシンボル関数 //
  // ====================== //
  // %%% @get-white-pawn-position
  // %%% @get-white-knight-position
  // %%% @get-white-bishop-position
  // %%% @get-white-rook-position
  // %%% @get-white-queen-position
  // %%% @get-white-king-position
  // %%% @get-black-pawn-position
  // %%% @get-black-knight-position
  // %%% @get-black-bishop-position
  // %%% @get-black-rook-position
  // %%% @get-black-queen-position
  // %%% @get-black-king-position
  template<Side SIDE, PieceType PIECE_TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::GetPosition) {
    LPointerVec ret_vec;
    for (Bitboard bb = board_ptr_->position_[SIDE][PIECE_TYPE]; bb;
    NEXT_BITBOARD(bb)) {
      ret_vec.push_back(Lisp::NewSymbol
      (Sayulisp::SQUARE_MAP_INV[Util::GetSquare(bb)]));
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  template LPointer EngineSuite::GetPosition<WHITE, PAWN>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, KNIGHT>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, BISHOP>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, ROOK>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, QUEEN>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, KING>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, PAWN>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, KNIGHT>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, BISHOP>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, ROOK>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, QUEEN>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, KING>
  (const std::string&, LPointer, LObject*, const LObject&);

  // %%% @get-piece
  DEF_MESSAGE_FUNCTION(EngineSuite::GetPiece) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 引数のチェック。
    LPointer result = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckSquare(*result);
    Square square = result->number();

    // サイドと駒の種類を得る。
    LPointer ret_ptr = Lisp::NewList(2);
    ret_ptr->car(Lisp::NewSymbol
    (Sayulisp::SIDE_MAP_INV[board_ptr_->side_board_[square]]));
    ret_ptr->cdr()->car(Lisp::NewSymbol
    (Sayulisp::PIECE_MAP_INV[board_ptr_->piece_board_[square]]));

    return ret_ptr;
  }

  // %%% @get-all-pieces
  DEF_MESSAGE_FUNCTION(EngineSuite::GetAllPieces) {
    LPointerVec ret_vec(NUM_SQUARES);
    LPointerVec::iterator itr = ret_vec.begin();

    // 各マスの駒のリストを作る。
    LPointer elm_ptr;
    FOR_SQUARES(square) {
      elm_ptr = Lisp::NewList(2);
      elm_ptr->car(Lisp::NewSymbol
      (Sayulisp::SIDE_MAP_INV[board_ptr_->side_board_[square]]));
      elm_ptr->cdr()->car(Lisp::NewSymbol
      (Sayulisp::PIECE_MAP_INV[board_ptr_->piece_board_[square]]));

      *itr = elm_ptr;
      ++itr;
    }

    return Lisp::LPointerVecToList(ret_vec);
  }

  // %%% @set-fen
  DEF_MESSAGE_FUNCTION(EngineSuite::SetFEN) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // FENを得る。
    LPointer fen_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*fen_ptr, LType::STRING);

    // パースする。
    FEN fen;
    try {
      fen = FEN(fen_ptr->string());
    } catch (...) {
      throw Lisp::GenError("@engine-error", "Couldn't parse FEN.");
    }

    // キングの数をチェック。
    if ((Util::CountBits(fen.position()[WHITE][KING]) != 1)
    || (Util::CountBits(fen.position()[BLACK][KING]) != 1)) {
      throw Lisp::GenError("@engine-error", "This FEN is invalid position.");
    }

    engine_ptr_->LoadFEN(fen);
    return Lisp::NewBoolean(true);
  }

  // %%% @place-piece
  DEF_MESSAGE_FUNCTION(EngineSuite::PlacePiece) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 2, &args_ptr);

    // マスを得る。
    LPointer square_ptr = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckSquare(*square_ptr);
    Lisp::Next(&args_ptr);
    Square square = square_ptr->number();

    // 駒を得る。
    LPointer piece_ptr = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckPiece(*piece_ptr);
    Side side = piece_ptr->car()->number();
    PieceType piece_type = piece_ptr->cdr()->car()->number();

    // キングを置き換えることはできない。
    if (board_ptr_->piece_board_[square] == KING) {
      throw Lisp::GenError("@engine-error", "Couldn't overwrite King.");
    }

    // 前の駒のリストを作る。
    LPointer ret_ptr = Lisp::NewList(2);
    ret_ptr->car(Lisp::NewSymbol
    (Sayulisp::SIDE_MAP_INV[board_ptr_->side_board_[square]]));
    ret_ptr->cdr()->car(Lisp::NewSymbol
    (Sayulisp::PIECE_MAP_INV[board_ptr_->piece_board_[square]]));

    // 置き換える。
    engine_ptr_->PlacePiece(square, piece_type, side);

    return ret_ptr;
  }

  // %%% @get-candidate-moves
  DEF_MESSAGE_FUNCTION(EngineSuite::GetCandidateMoves) {
    // 指し手の生成。
    std::vector<Move> move_vec = engine_ptr_->GetLegalMoves();
    LPointer ret_ptr = Lisp::NewList(move_vec.size());

    // リストに代入していく。
    LObject* ptr = ret_ptr.get();
    for (auto move : move_vec) {
      ptr->car(Sayulisp::MoveToList(move));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @set-to-move
  DEF_MESSAGE_FUNCTION(EngineSuite::SetToMove) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 指し手を得る。
    LPointer to_move_ptr = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckSide(*to_move_ptr);
    Side to_move = to_move_ptr->number();

    // NO_SIDEはダメ。
    if (to_move == NO_SIDE) {
      throw Lisp::GenError("@engine-error", "NO_SIDE is not allowed.");
    }

    // 前の状態。
    LPointer ret_ptr =
    Lisp::NewSymbol(Sayulisp::SIDE_MAP_INV[board_ptr_->to_move_]);

    engine_ptr_->to_move(to_move);
    return ret_ptr;
  }

  // %%% @set-castling-rights
  DEF_MESSAGE_FUNCTION(EngineSuite::SetCastlingRights) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // キャスリングの権利を得る。
    LPointer castling_list_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckList(*castling_list_ptr);

    // キャスリングの権利フラグを作成。
    Castling rights = 0;
    LPointer result;
    int rights_number = 0;
    for (LObject* ptr = castling_list_ptr.get(); ptr->IsPair();
    Lisp::Next(&ptr)) {
      result = caller->Evaluate(*(ptr->car()));
      Sayulisp::CheckCastling(*result);

      rights_number = result->number();
      switch (rights_number) {
        case 1:
          rights |= WHITE_SHORT_CASTLING;
          break;
        case 2:
          rights |= WHITE_LONG_CASTLING;
          break;
        case 3:
          rights |= BLACK_SHORT_CASTLING;
          break;
        case 4:
          rights |= BLACK_LONG_CASTLING;
          break;
      }
    }

    // 前のキャスリングの権利を得る。
    Castling origin_rights = board_ptr_->castling_rights_;
    LPointerVec ret_vec;
    if ((origin_rights & WHITE_SHORT_CASTLING)) {
      ret_vec.push_back(Lisp::NewSymbol("WHITE_SHORT_CASTLING"));
    }
    if ((origin_rights & WHITE_LONG_CASTLING)) {
      ret_vec.push_back(Lisp::NewSymbol("WHITE_LONG_CASTLING"));
    }
    if ((origin_rights & BLACK_SHORT_CASTLING)) {
      ret_vec.push_back(Lisp::NewSymbol("BLACK_SHORT_CASTLING"));
    }
    if ((origin_rights & BLACK_LONG_CASTLING)) {
      ret_vec.push_back(Lisp::NewSymbol("BLACK_LONG_CASTLING"));
    }

    // セットして返す。
    engine_ptr_->castling_rights(rights);
    return Lisp::LPointerVecToList(ret_vec);
  }

  // %%% set-en-passant-square
  DEF_MESSAGE_FUNCTION(EngineSuite::SetEnPassantSquare) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // マスを得る。
    LPointer square_ptr = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckSquare(*square_ptr);
    Square square = square_ptr->number();

    // 前のアンパッサンのマスを作る。
    LPointer ret_ptr = Lisp::NewNil();
    if (board_ptr_->en_passant_square_) {
      ret_ptr = Lisp::NewNumber(board_ptr_->en_passant_square_);
    }

    // マスがアンパッサンのマスのチェック。
    if (board_ptr_->to_move_ == WHITE) {
      // 白番の時は黒側のアンパッサン。
      if (Util::SquareToRank(square) == RANK_6) {
        // 一つ上にポーンがいて、アンパッサンのマスが空の時にセットできる。
        if ((board_ptr_->piece_board_[square] == EMPTY)
        && (board_ptr_->side_board_[square - 8] == BLACK)
        && (board_ptr_->piece_board_[square - 8] == PAWN)) {
          engine_ptr_->en_passant_square(square);
          return ret_ptr;
        }
      }
    } else {
      // 黒番の時は白側のアンパッサン。
      if (Util::SquareToRank(square) == RANK_3) {
        // 一つ上にポーンがいて、アンパッサンのマスが空の時にセットできる。
        if ((board_ptr_->piece_board_[square] == EMPTY)
        && (board_ptr_->side_board_[square + 8] == WHITE)
        && (board_ptr_->piece_board_[square + 8] == PAWN)) {
          engine_ptr_->en_passant_square(square);
          return ret_ptr;
        }
      }
    }

    throw Lisp::GenError("@engine-error", "'" + square_ptr->ToString()
    + "' couldn't be en passant square.");
  }

  // %%% @set-play
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPly) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 手数を得る。
    LPointer ply_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*ply_ptr, LType::NUMBER);
    int ply = ply_ptr->number();

    // 手数はプラスでないとダメ。
    if (ply < 0) {
      throw Lisp::GenError("@engine-error", "Ply must be positive number.");
    }

    LPointer ret_ptr = Lisp::NewNumber(board_ptr_->ply_);
    engine_ptr_->ply(ply);
    return ret_ptr;
  }

  // %%% @set-clock
  DEF_MESSAGE_FUNCTION(EngineSuite::SetClock) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 手数を得る。
    LPointer clock_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*clock_ptr, LType::NUMBER);
    int clock = clock_ptr->number();

    // 手数はプラスでないとダメ。
    if (clock < 0) {
      throw Lisp::GenError("@engine-error", "Clock must be positive number.");
    }

    LPointer ret_ptr = Lisp::NewNumber(board_ptr_->clock_);
    engine_ptr_->clock(clock);
    return ret_ptr;
  }

  // %%% @play-move
  // %%% @play-note
  /** 手を指す。 */
  DEF_MESSAGE_FUNCTION(EngineSuite::PlayMoveOrNote) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 指し手を得る。
    LPointer result = caller->Evaluate(*(args_ptr->car()));
    Move move = 0;
    if (symbol == "@play-move") {  // @play-move
      move = Sayulisp::ListToMove(*result);
    } else {  // @play-note
      Lisp::CheckType(*result, LType::STRING);
      std::vector<Move> move_vec = engine_ptr_->GuessNote(result->string());
      if (move_vec.size() == 0) return Lisp::NewBoolean(false);
      move = move_vec[0];
    }

    return Lisp::NewBoolean(engine_ptr_->PlayMove(move));
  }

  // %%% @undo-move
  DEF_MESSAGE_FUNCTION(EngineSuite::UndoMove) {
    Move move = engine_ptr_->UndoMove();
    if (!move) return Lisp::NewNil();

    return Sayulisp::MoveToList(move);
  }

  // %%% @move->note
  DEF_MESSAGE_FUNCTION(EngineSuite::MoveToNote) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 指し手を得る。
    LPointer move_ptr = caller->Evaluate(*(args_ptr->car()));

    return Lisp::NewString(engine_ptr_->MoveToNote
    (Sayulisp::ListToMove(*move_ptr)));
  }

  // %%% @input-uci-command
  DEF_MESSAGE_FUNCTION(EngineSuite::InputUCICommand) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    LPointer command_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*command_ptr, LType::STRING);

    return Lisp::NewBoolean(shell_ptr_->InputCommand(command_ptr->string()));
  }

  // %%% @add-uci-output-listener
  DEF_MESSAGE_FUNCTION(EngineSuite::AddUCIOutputListener) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    LPointer result = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*result, LType::FUNCTION);

    // リストを作る。
    LPointer listener_ptr =
    Lisp::NewPair(result, Lisp::NewPair(Lisp::NewString(""), Lisp::NewNil()));

    // callerと同じスコープの関数オブジェクトを作る。
    LPointer caller_scope =
    Lisp::NewN_Function(LC_Function(), "", caller->scope_chain());

    // コールバック関数を作成。
    auto callback =
    [caller_scope, listener_ptr](const std::string& message) {
      listener_ptr->cdr()->car()->string(message);
      caller_scope->Evaluate(*listener_ptr);
    };

    // コールバック関数を登録。
    callback_vec_.push_back(callback);

    return Lisp::NewBoolean(true);
  }

  // %%% @run
  DEF_MESSAGE_FUNCTION(EngineSuite::RunEngine) {
    // 出力リスナー。
    auto callback = [](const std::string& message) {
      std::cout << message << std::endl;
    };
    callback_vec_.push_back(callback);

    // quitが来るまでループ。
    std::string input;
    while (true) {
      std::getline(std::cin, input);
      if (input == "quit") break;
      shell_ptr_->InputCommand(input);
    }

    return Lisp::NewBoolean(true);
  }

  // Go...()で使う関数。
  LPointer EngineSuite::GoFunc(std::uint32_t depth, std::uint64_t nodes,
  int thinking_time, const LObject& candidate_list) {
    // 候補手のリストを作成。
    std::vector<Move> candidate_vec(Lisp::CountList(candidate_list));
    std::vector<Move>::iterator candidate_itr = candidate_vec.begin();
    LPointer car;
    for (const LObject* ptr = &candidate_list; ptr->IsPair();
    ptr = ptr->cdr().get(), ++candidate_itr) {
      car = ptr->car();
      Sayulisp::CheckMove(*car);
      *candidate_itr = Sayulisp::ListToMove(*car);
    }

    // ストッパーを登録。
    engine_ptr_->SetStopper(Util::GetMin(depth, MAX_PLYS),
    Util::GetMin(nodes, MAX_NODES),
    Chrono::milliseconds(thinking_time), false);

    // テーブルの年齢を上げる。
    table_ptr_->GrowOld();

    // 思考開始。
    PVLine pv_line = engine_ptr_->Calculate(shell_ptr_->num_threads(),
    candidate_vec, *shell_ptr_);

    // 最善手、Ponderをアウトプットリスナーに送る。
    std::ostringstream oss;
    int len = pv_line.length();
    if (len) {
      oss << "bestmove " << Util::MoveToString(pv_line[0]);
      if (len >= 2) {
        oss << " ponder " << Util::MoveToString(pv_line[1]);
      }
      for (auto& callback : callback_vec_) callback(oss.str());
    }

    // PVラインのリストを作る。
    LPointer ret_ptr = Lisp::NewList(len + 2);
    LObject* ptr = ret_ptr.get();
    // スコア。
    ptr->car(Lisp::NewNumber(pv_line.score()));
    Lisp::Next(&ptr);

    // メイトイン。
    int mate_in = pv_line.mate_in();
    if (mate_in >= 0) {
      if ((mate_in % 2) == 1) {
        mate_in = (mate_in / 2) + 1;
      } else {
        mate_in = -1 * (mate_in / 2);
      }
      ptr->car(Lisp::NewNumber(mate_in));
    } else {
      ptr->car(Lisp::NewNil());
    }
    Lisp::Next(&ptr);

    // PVライン。
    for (int i = 0; i < len; ++i, Lisp::Next(&ptr)) {
      ptr->car(Sayulisp::MoveToList(pv_line[i]));
    }

    return ret_ptr;
  }

  // %%% @go-movetime
  DEF_MESSAGE_FUNCTION(EngineSuite::GoMoveTime) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 思考時間を得る。
    LPointer time_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*time_ptr, LType::NUMBER);
    int time = time_ptr->number();
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);
      candidate_list_ptr = result;
    }

    // GoFuncに渡して終わる。
    return GoFunc(MAX_PLYS, MAX_NODES, time, *candidate_list_ptr);
  }

  // %%% @go-timelimit
  DEF_MESSAGE_FUNCTION(EngineSuite::GoTimeLimit) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 持ち時間を得る。
    LPointer time_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*time_ptr, LType::NUMBER);
    int time = TimeLimitToMoveTime(time_ptr->number());
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);
      candidate_list_ptr = result;
    }

    // GoFuncに渡して終わる。
    return GoFunc(MAX_PLYS, MAX_NODES, time, *candidate_list_ptr);
  }

  // %%% @go-depth
  DEF_MESSAGE_FUNCTION(EngineSuite::GoDepth) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 深さを得る。
    LPointer depth_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*depth_ptr, LType::NUMBER);
    std::uint32_t depth = depth_ptr->number();
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);
      candidate_list_ptr = result;
    }

    // GoFuncに渡して終わる。
    return GoFunc(depth, MAX_NODES, INT_MAX, *candidate_list_ptr);
  }

  // %%% @go-nodes
  DEF_MESSAGE_FUNCTION(EngineSuite::GoNodes) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // ノード数を得る。
    LPointer node_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*node_ptr, LType::NUMBER);
    std::uint64_t node = node_ptr->number();
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);
      candidate_list_ptr = result;
    }

    // GoFuncに渡して終わる。
    return GoFunc(MAX_PLYS, node, INT_MAX, *candidate_list_ptr);
  }

  // %%% @set-hash-size
  DEF_MESSAGE_FUNCTION(EngineSuite::SetHashSize) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // サイズを得る。
    LPointer size_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*size_ptr, LType::NUMBER);
    std::size_t size = Util::GetMax(size_ptr->number(),
    TTEntry::TTENTRY_HARD_CODED_SIZE);

    // 古いサイズ。
    LPointer ret_ptr = Lisp::NewNumber(table_ptr_->GetSizeBytes());

    // サイズを更新。
    table_ptr_->SetSize(size);

    return ret_ptr;
  }

  // %%% @set-threads
  DEF_MESSAGE_FUNCTION(EngineSuite::SetThreads) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // スレッド数を得る。
    LPointer threads_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*threads_ptr, LType::NUMBER);
    int threads = Util::GetMax(threads_ptr->number(), 1);

    // 古いスレッド数。
    LPointer ret_ptr = Lisp::NewNumber(shell_ptr_->num_threads());

    // スレッド数を更新。
    shell_ptr_->num_threads(threads);

    return ret_ptr;
  }

  // %%% @material
  DEF_MESSAGE_FUNCTION(EngineSuite::SetMaterial) {
    // 古い設定を得る。
    const int (& material)[NUM_PIECE_TYPES] = search_params_ptr_->material();
    LPointerVec ret_vec(7);
    FOR_PIECE_TYPES(piece_type) {
      ret_vec[piece_type] = Lisp::NewNumber(material[piece_type]);
    }

    // もし引数があるなら設定。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      int len = Lisp::CountList(*result);
      if (len < 7) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "'"" requires List of 7 elements.");
      }

      int new_material[NUM_PIECE_TYPES];
      LObject* ptr = result.get();
      FOR_PIECE_TYPES(piece_type) {
        if (piece_type == EMPTY) {
          new_material[piece_type] = 0;
        } else {
          new_material[piece_type] = ptr->car()->number();
        }
        Lisp::Next(&ptr);
      }

      search_params_ptr_->material(new_material);
    }

    return Lisp::LPointerVecToList(ret_vec);
  }

  // @pawn-square-table-opening
  // @knight-square-table-opening
  // @bishop-square-table-opening
  // @rook-square-table-opening
  // @queen-square-table-opening
  // @king-square-table-opening
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_SQUARES);
    const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES] =
    eval_params_ptr_->opening_position_value_table();
    FOR_SQUARES(square) {
      ret_vec[square] = Lisp::NewNumber(table[TYPE][square]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_SQUARES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_SQUARES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_SQUARES(square) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->opening_position_value_table
        (TYPE, square, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  // インスタンス化。
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<PAWN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<KNIGHT>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<BISHOP>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<ROOK>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<QUEEN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<KING>);

  // @pawn-square-table-ending
  // @knight-square-table-ending
  // @bishop-square-table-ending
  // @rook-square-table-ending
  // @queen-square-table-ending
  // @king-square-table-ending
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_SQUARES);
    const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES] =
    eval_params_ptr_->ending_position_value_table();
    FOR_SQUARES(square) {
      ret_vec[square] = Lisp::NewNumber(table[TYPE][square]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_SQUARES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_SQUARES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_SQUARES(square) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->ending_position_value_table
        (TYPE, square, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  // インスタンス化。
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<PAWN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<KNIGHT>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<BISHOP>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<ROOK>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<QUEEN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<KING>);

  // %%% @pawn-attack-table
  // %%% @knight-attack-table
  // %%% @bishop-attack-table
  // %%% @rook-attack-table
  // %%% @queen-attack-table
  // %%% @king-attack-table
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_PIECE_TYPES);
    const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
    eval_params_ptr_->attack_value_table();
    FOR_PIECE_TYPES(piece_type) {
      ret_vec[piece_type] = Lisp::NewNumber(table[TYPE][piece_type]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_PIECE_TYPES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_PIECE_TYPES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_PIECE_TYPES(piece_type) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->attack_value_table
        (TYPE, piece_type, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<KING>);

  // %%% @pawn-defense-table
  // %%% @knight-defense-table
  // %%% @bishop-defense-table
  // %%% @rook-defense-table
  // %%% @queen-defense-table
  // %%% @king-defense-table
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_PIECE_TYPES);
    const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
    eval_params_ptr_->defense_value_table();
    FOR_PIECE_TYPES(piece_type) {
      ret_vec[piece_type] = Lisp::NewNumber(table[TYPE][piece_type]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_PIECE_TYPES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_PIECE_TYPES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_PIECE_TYPES(piece_type) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->defense_value_table
        (TYPE, piece_type, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<KING>);

  // @bishop-pin-tab
  // @rook-pin-table
  // @queen-pin-table
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPinTable) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_PIECE_TYPES);
    const double (& table)
    [NUM_PIECE_TYPES][NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
    eval_params_ptr_->pin_value_table();
    FOR_PIECE_TYPES(piece_type_1) {
      LPointerVec temp_vec(NUM_PIECE_TYPES);

      FOR_PIECE_TYPES(piece_type_2) {
        temp_vec[piece_type_2] =
        Lisp::NewNumber(table[TYPE][piece_type_1][piece_type_2]);
      }

      ret_vec[piece_type_1] = Lisp::LPointerVecToList(temp_vec);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_PIECE_TYPES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_PIECE_TYPES) + " x "
        + std::to_string(NUM_PIECE_TYPES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_PIECE_TYPES(piece_type_1) {
        const LPointer& car = ptr->car();

        // チェックする。
        Lisp::CheckList(*car);
        if (Lisp::CountList(*car) < static_cast<int>(NUM_PIECE_TYPES)) {
          throw Lisp::GenError("@engine-error",
          "'" + symbol + "' requires List of "
          + std::to_string(NUM_PIECE_TYPES) + "x"
          + std::to_string(NUM_PIECE_TYPES) + " elements.");
        }

        // 内側のループ。
        LObject* ptr_2 = car.get();
        FOR_PIECE_TYPES(piece_type_2) {
          Lisp::CheckType(*(ptr_2->car()), LType::NUMBER);

          eval_params_ptr_->pin_value_table
          (TYPE, piece_type_1, piece_type_2, ptr_2->car()->number());

          Lisp::Next(&ptr_2);
        }

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetPinTable<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetPinTable<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetPinTable<QUEEN>);

  DEF_MESSAGE_FUNCTION(EngineSuite::SetPawnShieldTable) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_SQUARES);
    const double (& table)[NUM_SQUARES] =
    eval_params_ptr_->pawn_shield_value_table();
    FOR_SQUARES(square) {
      ret_vec[square] = Lisp::NewNumber(table[square]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_SQUARES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_SQUARES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_SQUARES(square) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->pawn_shield_value_table
        (square, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }

#define SET_PIECE_WEIGHT(accessor) \
    const Weight& old = eval_params_ptr_->accessor()[TYPE];\
    std::size_t len = old.Size();\
    LPointerVec ret_vec(len);\
    for (std::size_t i = 0; i < len; ++i) {\
      ret_vec[i] = Lisp::NewPair(Lisp::NewNumber(std::get<0>(old.At(i))),\
      Lisp::NewPair(Lisp::NewNumber(std::get<1>(old.At(i))), Lisp::NewNil()));\
    }\
    \
    LObject* args_ptr = args.cdr()->cdr().get();\
    if (args_ptr->IsPair()) {\
      LPointer car = caller->Evaluate(*(args_ptr->car()));\
      \
      if (car->IsNumber()) {\
        const LPointer& cdr = args_ptr->cdr();\
        if (cdr->IsPair()) {\
          LPointer cdar = caller->Evaluate(*(cdr->car()));\
          Lisp::CheckType(*cdar, LType::NUMBER);\
          \
          eval_params_ptr_->accessor\
          (TYPE, Weight::CreateWeight(car->number(), cdar->number()));\
          \
          return Lisp::LPointerVecToList(ret_vec);\
        }\
      } else if (car->IsList()) {\
        Weight weight;\
        for (LObject* ptr = car.get(); ptr->IsPair(); Lisp::Next(&ptr)) {\
          const LPointer& temp = ptr->car();\
          \
          Lisp::CheckList(*temp);\
          if (Lisp::CountList(*temp) < 2) {\
            throw Lisp::GenError("@engine-error", "If you want to set weight, "\
            "the arguments are 2 numbers or List of Lists composed with 2 "\
            "numbers.");\
          }\
          \
          const LPointer& temp_car = temp->car();\
          Lisp::CheckType(*temp_car, LType::NUMBER);\
          const LPointer& temp_cdar = temp->cdr()->car();\
          Lisp::CheckType(*temp_cdar, LType::NUMBER);\
          \
          weight.Add(temp_car->number(), temp_cdar->number());\
        }\
        \
        eval_params_ptr_->accessor(TYPE, weight);\
        \
        return Lisp::LPointerVecToList(ret_vec);\
      }\
      \
      throw Lisp::GenError("@engine-error", "If you want to set weight, "\
      "the arguments are 2 numbers or List of Lists composed with 2 numbers.");\
    }\
    \
    return Lisp::LPointerVecToList(ret_vec)

  // %%% @weight-pawn-opening-position
  // %%% @weight-knight-opening-position
  // %%% @weight-bishop-opening-position
  // %%% @weight-rook-opening-position
  // %%% @weight-queen-opening-position
  // %%% @weight-king-opening-position
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition) {
    SET_PIECE_WEIGHT(weight_opening_position);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<KING>);

  // %%% @weight-pawn-ending-position
  // %%% @weight-knight-ending-position
  // %%% @weight-bishop-ending-position
  // %%% @weight-rook-ending-position
  // %%% @weight-queen-ending-position
  // %%% @weight-king-ending-position
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition) {
    SET_PIECE_WEIGHT(weight_ending_position);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<KING>);

  // %%% @weight-pawn-mobility
  // %%% @weight-knight-mobility
  // %%% @weight-bishop-mobility
  // %%% @weight-rook-mobility
  // %%% @weight-queen-mobility
  // %%% @weight-king-mobility
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility) {
    SET_PIECE_WEIGHT(weight_mobility);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<KING>);


  // %%% @weight-pawn-center-control
  // %%% @weight-knight-center-control
  // %%% @weight-bishop-center-control
  // %%% @weight-rook-center-control
  // %%% @weight-queen-center-control
  // %%% @weight-king-center-control
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll) {
    SET_PIECE_WEIGHT(weight_center_control);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<KING>);

  // %%% @weight-pawn-sweet-center-control
  // %%% @weight-knight-sweet-center-control
  // %%% @weight-bishop-sweet-center-control
  // %%% @weight-rook-sweet-center-control
  // %%% @weight-queen-sweet-center-control
  // %%% @weight-king-sweet-center-control
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll) {
    SET_PIECE_WEIGHT(weight_sweet_center_control);
  }
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<PAWN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<KNIGHT>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<BISHOP>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<ROOK>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<QUEEN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<KING>);

  // %%% @weight-pawn-development
  // %%% @weight-knight-development
  // %%% @weight-bishop-development
  // %%% @weight-rook-development
  // %%% @weight-queen-development
  // %%% @weight-king-development
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment) {
    SET_PIECE_WEIGHT(weight_development);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<KING>);

  // %%% @weight-pawn-attack
  // %%% @weight-knight-attack
  // %%% @weight-bishop-attack
  // %%% @weight-rook-attack
  // %%% @weight-queen-attack
  // %%% @weight-king-attack
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack) {
    SET_PIECE_WEIGHT(weight_attack);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<KING>);

  // %%% @weight-pawn-defense
  // %%% @weight-knight-defense
  // %%% @weight-bishop-defense
  // %%% @weight-rook-defense
  // %%% @weight-queen-defense
  // %%% @weight-king-defense
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense) {
    SET_PIECE_WEIGHT(weight_defense);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<KING>);

  // %%% @weight-bishop-pin
  // %%% @weight-rook-pin
  // %%% @weight-queen-pin
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPin) {
    SET_PIECE_WEIGHT(weight_pin);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPin<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPin<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPin<QUEEN>);

  // %%% @weight-pawn-attack-around-king
  // %%% @weight-knight-attack-around-king
  // %%% @weight-bishop-attack-around-king
  // %%% @weight-rook-attack-around-king
  // %%% @weight-queen-attack-around-king
  // %%% @weight-king-attack-around-king
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing) {
    SET_PIECE_WEIGHT(weight_attack_around_king);
  }
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<PAWN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<KNIGHT>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<BISHOP>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<ROOK>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<QUEEN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<KING>);

#define SET_WEIGHT(accessor) \
    const Weight& old = eval_params_ptr_->accessor();\
    std::size_t len = old.Size();\
    LPointerVec ret_vec(len);\
    for (std::size_t i = 0; i < len; ++i) {\
      ret_vec[i] = Lisp::NewPair(Lisp::NewNumber(std::get<0>(old.At(i))),\
      Lisp::NewPair(Lisp::NewNumber(std::get<1>(old.At(i))), Lisp::NewNil()));\
    }\
    \
    LObject* args_ptr = args.cdr()->cdr().get();\
    if (args_ptr->IsPair()) {\
      LPointer car = caller->Evaluate(*(args_ptr->car()));\
      \
      if (car->IsNumber()) {\
        const LPointer& cdr = args_ptr->cdr();\
        if (cdr->IsPair()) {\
          LPointer cdar = caller->Evaluate(*(cdr->car()));\
          Lisp::CheckType(*cdar, LType::NUMBER);\
          \
          eval_params_ptr_->accessor\
          (Weight::CreateWeight(car->number(), cdar->number()));\
          \
          return Lisp::LPointerVecToList(ret_vec);\
        }\
      } else if (car->IsList()) {\
        Weight weight;\
        for (LObject* ptr = car.get(); ptr->IsPair(); Lisp::Next(&ptr)) {\
          const LPointer& temp = ptr->car();\
          \
          Lisp::CheckList(*temp);\
          if (Lisp::CountList(*temp) < 2) {\
            throw Lisp::GenError("@engine-error", "If you want to set weight, "\
            "the arguments are 2 numbers or List of Lists composed with 2 "\
            "numbers.");\
          }\
          \
          const LPointer& temp_car = temp->car();\
          Lisp::CheckType(*temp_car, LType::NUMBER);\
          const LPointer& temp_cdar = temp->cdr()->car();\
          Lisp::CheckType(*temp_cdar, LType::NUMBER);\
          \
          weight.Add(temp_car->number(), temp_cdar->number());\
        }\
        \
        eval_params_ptr_->accessor(weight);\
        \
        return Lisp::LPointerVecToList(ret_vec);\
      }\
      \
      throw Lisp::GenError("@engine-error", "If you want to set weight, "\
      "the arguments are 2 numbers or List of Lists composed with 2 numbers.");\
    }\
    \
    return Lisp::LPointerVecToList(ret_vec)

  // %%% @weight-pass-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPassPawn) {
    SET_WEIGHT(weight_pass_pawn);
  }

  // %%% @weight-protected-pass-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightProtectedPassPawn) {
    SET_WEIGHT(weight_protected_pass_pawn);
  }

  // %%% @weight-double-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDoublePawn) {
    SET_WEIGHT(weight_double_pawn);
  }

  // %%% @weight-iso-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightIsoPawn) {
    SET_WEIGHT(weight_iso_pawn);
  }

  // %%% @weight-pawn-shield
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPawnShield) {
    SET_WEIGHT(weight_pawn_shield);
  }

  // %%% @weight-bishop-pair
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightBishopPair) {
    SET_WEIGHT(weight_bishop_pair);
  }

  // %%% @weight-bad-bishop
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightBadBishop) {
    SET_WEIGHT(weight_bad_bishop);
  }

  // %%% @weight-rook-pair
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightRookPair) {
    SET_WEIGHT(weight_rook_pair);
  }

  // %%% @weight-rook-semiopen-fyle
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightRookSemiopenFyle) {
    SET_WEIGHT(weight_rook_semiopen_fyle);
  }

  // %%% @weight-rook-open-fyle
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightRookOpenFyle) {
    SET_WEIGHT(weight_rook_open_fyle);
  }

  // %%% @weight-early-queen-starting
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEarlyQueenStarting) {
    SET_WEIGHT(weight_early_queen_starting);
  }

  // %%% @weight-weak-square
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightWeakSquare) {
    SET_WEIGHT(weight_weak_square);
  }

  // %%% @weight-castling
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCastling) {
    SET_WEIGHT(weight_castling);
  }

  // %%% @weight-abandoned-castling
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAbandonedCastling) {
    SET_WEIGHT(weight_abandoned_castling);
  }
}  // namespace Sayuri
