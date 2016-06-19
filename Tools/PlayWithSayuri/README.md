Play Chess with Sayuri
======================

Chess CUI for Sayuri made by Sayulisp.

Not implemented draw by threefold repetitions.

### Example ###

    ========================
    Play Chess with Sayuri!!
    ========================
    
    Input Command. ('help' shows help)
    (Command)>> play
    
    Input Command or Move. ('help' shows help)
    (Game)>> e2e4
      +---+---+---+---+---+---+---+---+  Move Number : 1 (Ply : 1)
    8 |-R-|-N-|-B-|-Q-|-K-|-B-|-N-|-R-|  Clock : 0
      +---+---+---+---+---+---+---+---+  To Move : Black
    7 |-P-|-P-|-P-|-P-|-P-|-P-|-P-|-P-|  Castling : KQkq
      +---+---+---+---+---+---+---+---+  En Passant Square : E3
    6 |   |   |   |   |   |   |   |   |
      +---+---+---+---+---+---+---+---+
    5 |   |   |   |   |   |   |   |   |
      +---+---+---+---+---+---+---+---+
    4 |   |   |   |   |<P>|   |   |   |
      +---+---+---+---+---+---+---+---+
    3 |   |   |   |   |   |   |   |   |
      +---+---+---+---+---+---+---+---+
    2 |<P>|<P>|<P>|<P>|   |<P>|<P>|<P>|
      +---+---+---+---+---+---+---+---+
    1 |<R>|<N>|<B>|<Q>|<K>|<B>|<N>|<R>|
      +---+---+---+---+---+---+---+---+
        a   b   c   d   e   f   g   h
    [Sayuri] Thinking...\

Usage
-----

    $ path/to/sayuri --sayulisp path/to/play-with-sayuri

| Symbol | Piece        |
|:------:|:------------:|
| `<P>`  | White Pawn   |
| `<N>`  | White Knight |
| `<B>`  | White Bishop |
| `<R>`  | White Rook   |
| `<Q>`  | White Queen  |
| `<K>`  | White King   |
| `-P-`  | Black Pawn   |
| `-N-`  | Black Knight |
| `-B-`  | Black Bishop |
| `-R-`  | Black Rook   |
| `-Q-`  | Black Queen  |
| `-K-`  | Black King   |
