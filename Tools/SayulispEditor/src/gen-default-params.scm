; The MIT License (MIT)
; 
; Copyright (c) 2016 Hironori Ishibashi
; 
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
; 
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
; THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.

; Default params generator for Sayulisp Editor
; Usage:
;    $ <path/to/sayuri> --sayulisp gen-default-params.scm <file name>
;        <path/to/sayuri> : Path to sayuri.
;        <file name> : Saves params as <file name>.

(if (equal? (ref argv 1) "--help") (begin (display
"Usage:\n"
"    $ <path/to/sayuri> --sayulisp gen-default-params.scm <file name>\n"
"        <path/to/sayuri> : Path to sayuri.\n"
"        <file name> : Saves params as <file name>.") (exit)) ())

(define symbol-list '(@material
                      @enable-quiesce-search
                      @enable-repetition-check
                      @enable-check-extension
                      @ybwc-limit-depth
                      @ybwc-invalid-moves
                      @enable-aspiration-windows
                      @aspiration-windows-limit-depth
                      @aspiration-windows-delta
                      @enable-see
                      @enable-history
                      @enable-killer
                      @enable-hash-table
                      @enable-iid
                      @iid-limit-depth
                      @iid-search-depth
                      @enable-nmr
                      @nmr-limit-depth
                      @nmr-search-reduction
                      @nmr-reduction
                      @enable-probcut
                      @probcut-limit-depth
                      @probcut-margin
                      @probcut-search-reduction
                      @enable-history-pruning
                      @history-pruning-limit-depth
                      @history-pruning-move-threshold
                      @history-pruning-invalid-moves
                      @history-pruning-threshold
                      @history-pruning-reduction
                      @enable-lmr
                      @lmr-limit-depth
                      @lmr-move-threshold
                      @lmr-invalid-moves
                      @lmr-search-reduction
                      @enable-futility-pruning
                      @futility-pruning-depth
                      @futility-pruning-margin
                      @pawn-square-table-opening
                      @knight-square-table-opening
                      @bishop-square-table-opening
                      @rook-square-table-opening
                      @queen-square-table-opening
                      @king-square-table-opening
                      @pawn-square-table-ending
                      @knight-square-table-ending
                      @bishop-square-table-ending
                      @rook-square-table-ending
                      @queen-square-table-ending
                      @king-square-table-ending
                      @pawn-attack-table
                      @knight-attack-table
                      @bishop-attack-table
                      @rook-attack-table
                      @queen-attack-table
                      @king-attack-table
                      @pawn-defense-table
                      @knight-defense-table
                      @bishop-defense-table
                      @rook-defense-table
                      @queen-defense-table
                      @king-defense-table
                      @bishop-pin-table
                      @rook-pin-table
                      @queen-pin-table
                      @pawn-shield-table
                      @weight-pawn-opening-position
                      @weight-knight-opening-position
                      @weight-bishop-opening-position
                      @weight-rook-opening-position
                      @weight-queen-opening-position
                      @weight-king-opening-position
                      @weight-pawn-ending-position
                      @weight-knight-ending-position
                      @weight-bishop-ending-position
                      @weight-rook-ending-position
                      @weight-queen-ending-position
                      @weight-king-ending-position
                      @weight-pawn-mobility
                      @weight-knight-mobility
                      @weight-bishop-mobility
                      @weight-rook-mobility
                      @weight-queen-mobility
                      @weight-king-mobility
                      @weight-pawn-center-control
                      @weight-knight-center-control
                      @weight-bishop-center-control
                      @weight-rook-center-control
                      @weight-queen-center-control
                      @weight-king-center-control
                      @weight-pawn-sweet-center-control
                      @weight-knight-sweet-center-control
                      @weight-bishop-sweet-center-control
                      @weight-rook-sweet-center-control
                      @weight-queen-sweet-center-control
                      @weight-king-sweet-center-control
                      @weight-pawn-development
                      @weight-knight-development
                      @weight-bishop-development
                      @weight-rook-development
                      @weight-queen-development
                      @weight-king-development
                      @weight-pawn-attack
                      @weight-knight-attack
                      @weight-bishop-attack
                      @weight-rook-attack
                      @weight-queen-attack
                      @weight-king-attack
                      @weight-pawn-defense
                      @weight-knight-defense
                      @weight-bishop-defense
                      @weight-rook-defense
                      @weight-queen-defense
                      @weight-king-defense
                      @weight-bishop-pin
                      @weight-rook-pin
                      @weight-queen-pin
                      @weight-pawn-attack-around-king
                      @weight-knight-attack-around-king
                      @weight-bishop-attack-around-king
                      @weight-rook-attack-around-king
                      @weight-queen-attack-around-king
                      @weight-king-attack-around-king
                      @weight-pass-pawn
                      @weight-protected-pass-pawn
                      @weight-double-pawn
                      @weight-iso-pawn
                      @weight-pawn-shield
                      @weight-bishop-pair
                      @weight-bad-bishop
                      @weight-rook-pair
                      @weight-rook-semiopen-fyle
                      @weight-rook-open-fyle
                      @weight-early-queen-starting
                      @weight-weak-square
                      @weight-castling
                      @weight-abandoned-castling))

(define ostream (output-stream (ref argv 1)))
(define my-engine (gen-engine))

(ostream ";; Generates Engine.\n(define my-engine (gen-engine))\n\n")
(ostream ";; Configures Engine.\n")

(for (symbol symbol-list)
     (ostream
       (string-append
         (to-string `(my-engine (quote ,symbol) (quote ,(my-engine symbol))))
         "\n")))

(ostream "\n;; Runs Engine.\n(my-engine (quote @run))")
(ostream ())
