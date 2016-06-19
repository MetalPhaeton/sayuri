;; The MIT License (MIT)
;;
;; Copyright (c) 2016 Hironori Ishibashi
;;
;; Permission is hereby granted, free of charge, to any person obtaining a copy
;; of this software and associated documentation files (the "Software"), to
;; deal in the Software without restriction, including without limitation the
;; rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
;; sell copies of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:
;;
;; The above copyright notice and this permission notice shall be included in
;; all copies or substantial portions of the Software.
;;
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
;; IN THE SOFTWARE.

;; Settings.
(define player-side WHITE)
(define thinking-time (* 15 1000))
(define num-threads 1)
(define hash-size (* 512 1024 1024))

;; Engine.
(define engine (gen-engine))

;; Mode.
(define COMMAND 0)
(define GAME 1)
(define mode COMMAND)

;; Template of chess board.
(define (board-template side)
        (if (= side WHITE)
            (quote ("  +---+---+---+---+---+---+---+---+\n"
                    "8 |" A8 "|" B8 "|" C8 "|" D8 "|"
                    E8 "|" F8 "|" G8 "|" H8 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "7 |" A7 "|" B7 "|" C7 "|" D7 "|"
                    E7 "|" F7 "|" G7 "|" H7 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "6 |" A6 "|" B6 "|" C6 "|" D6 "|"
                    E6 "|" F6 "|" G6 "|" H6 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "5 |" A5 "|" B5 "|" C5 "|" D5 "|"
                    E5 "|" F5 "|" G5 "|" H5 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "4 |" A4 "|" B4 "|" C4 "|" D4 "|"
                    E4 "|" F4 "|" G4 "|" H4 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "3 |" A3 "|" B3 "|" C3 "|" D3 "|"
                    E3 "|" F3 "|" G3 "|" H3 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "2 |" A2 "|" B2 "|" C2 "|" D2 "|"
                    E2 "|" F2 "|" G2 "|" H2 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "1 |" A1 "|" B1 "|" C1 "|" D1 "|"
                    E1 "|" F1 "|" G1 "|" H1 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "    a   b   c   d   e   f   g   h\n"))
            (quote ("  +---+---+---+---+---+---+---+---+\n"
                    "1 |" H1 "|" G1 "|" F1 "|" E1 "|"
                    D1 "|" C1 "|" B1 "|" A1 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "2 |" H2 "|" G2 "|" F2 "|" E2 "|"
                    D2 "|" C2 "|" B2 "|" A2 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "3 |" H3 "|" G3 "|" F3 "|" E3 "|"
                    D3 "|" C3 "|" B3 "|" A3 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "4 |" H4 "|" G4 "|" F4 "|" E4 "|"
                    D4 "|" C4 "|" B4 "|" A4 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "5 |" H5 "|" G5 "|" F5 "|" E5 "|"
                    D5 "|" C5 "|" B5 "|" A5 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "6 |" H6 "|" G6 "|" F6 "|" E6 "|"
                    D6 "|" C6 "|" B6 "|" A6 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "7 |" H7 "|" G7 "|" F7 "|" E7 "|"
                    D7 "|" C7 "|" B7 "|" A7 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "8 |" H8 "|" G8 "|" F8 "|" E8 "|"
                    D8 "|" C8 "|" B8 "|" A8 "|\n"
                    "  +---+---+---+---+---+---+---+---+\n"
                    "    h   g   f   e   d   c   b   a\n"))))

;; Piece string.
(define (piece-string piece)
        (cond ((equal? piece '(WHITE PAWN)) "<P>")
              ((equal? piece '(WHITE KNIGHT)) "<N>")
              ((equal? piece '(WHITE BISHOP)) "<B>")
              ((equal? piece '(WHITE ROOK)) "<R>")
              ((equal? piece '(WHITE QUEEN)) "<Q>")
              ((equal? piece '(WHITE KING)) "<K>")
              ((equal? piece '(BLACK PAWN)) "-P-")
              ((equal? piece '(BLACK KNIGHT)) "-N-")
              ((equal? piece '(BLACK BISHOP)) "-B-")
              ((equal? piece '(BLACK ROOK)) "-R-")
              ((equal? piece '(BLACK QUEEN)) "-Q-")
              ((equal? piece '(BLACK KING)) "-K-")
              (else "   "))) 

;; Make chess board.
(define (board-string player-side position-list)
        ;; For walk function in board-string.
        (define (board-string-core elm path)
                (if (symbol? elm)
                    `(@replace ,(piece-string (ref position-list (eval elm))))
                    ()))
        (apply string-append
               (walk board-string-core (board-template player-side))))

;; Generate move.
(define reg "([a-h])([1-8])([a-h])([1-8])([nbrq]?)")
(define (to-fyle char)
        (cond ((equal? char "a") FYLE_A)
              ((equal? char "b") FYLE_B)
              ((equal? char "c") FYLE_C)
              ((equal? char "d") FYLE_D)
              ((equal? char "e") FYLE_E)
              ((equal? char "f") FYLE_F)
              ((equal? char "g") FYLE_G)
              (else FYLE_H)))
(define (to-rank char)
        (cond ((equal? char "1") RANK_1)
              ((equal? char "2") RANK_2)
              ((equal? char "3") RANK_3)
              ((equal? char "4") RANK_4)
              ((equal? char "5") RANK_5)
              ((equal? char "6") RANK_6)
              ((equal? char "7") RANK_7)
              (else RANK_8)))
(define (to-square fyle rank) (+ (* rank 8) fyle))
(define (to-move str)
        (define reg-list (regex-search reg str))
        (if (null? reg-list)
            ()
            (begin (define square-from
                           (to-square (to-fyle (ref reg-list 1))
                                      (to-rank (ref reg-list 2))))
                   (define square-to
                           (to-square (to-fyle (ref reg-list 3))
                                      (to-rank (ref reg-list 4))))
                   (define promotion-str (ref reg-list 5))
                   (define promotion
                           (cond ((equal? promotion-str "n") KNIGHT)
                                 ((equal? promotion-str "b") BISHOP)
                                 ((equal? promotion-str "r") ROOK)
                                 ((equal? promotion-str "q") QUEEN)
                                 (else EMPTY)))
                   (list square-from square-to promotion))))

;; Board status.
(define (status-string engine)
        (define temp (engine '@get-ply))
        (define ply (to-string temp))
        (define move-num (to-string (ceil (/ temp 2))))
        (define clock (to-string (engine '@get-clock)))
        (define to-move
                (if (equal? (engine '@get-to-move) 'WHITE) "White" "Black"))
        (define (gen-castling-rights li)
                (if (null? li)
                    ""
                    (string-append (cond ((equal? (car li)
                                                  'WHITE_SHORT_CASTLING)
                                          "K")
                                         ((equal? (car li)
                                                  'WHITE_LONG_CASTLING)
                                          "Q")
                                         ((equal? (car li)
                                                  'BLACK_SHORT_CASTLING)
                                          "k")
                                         ((equal? (car li)
                                                  'BLACK_LONG_CASTLING)
                                          "q")
                                         (else ""))
                                   (gen-castling-rights (cdr li)))))
        (define castling-rights
                (gen-castling-rights (engine '@get-castling-rights)))
        (define en-passant-square (engine '@get-en-passant-square))
        (string-append "Move Number : " move-num " (Ply : " ply ")\n"
                       "Clock : " clock "\n"
                       "To Move : " to-move "\n"
                       "Castling : " castling-rights "\n"
                       "En Passant Square : "
                       (if (null? en-passant-square)
                           ""
                           (to-string en-passant-square))
                       ))

;; Combine board and status.
(define (combine-board-status board-string status-string)
        (define board-string-list (string-split board-string "\n"))
        (define status-string-list (string-split status-string "\n"))
        (define separator '("  " "  " "  " "  " "  "))
        (define new-line '("\n" "\n" "\n" "\n" "\n" "\n" "\n" "\n" "\n" "\n"
                           "\n" "\n" "\n" "\n" "\n" "\n" "\n" "\n" ""))
        (define zipped-list
                (zip board-string-list separator status-string-list new-line))
        (define (core li) (apply string-append li))
        (apply string-append (map core zipped-list)))

;; Print board
(define (print-board engine)
        (stdout (combine-board-status (board-string player-side
                                                     (engine '@get-all-pieces))
                                       (status-string engine))))

;; Rotate bar.
(define bar-list '("|" "\\" "-" "/"))
(define bar-index 0)
(define (print-bar)
        (stdout (string-append "\b" (ref bar-list bar-index)))
        (if (>= bar-index 3) (set! bar-index 0) (inc! bar-index)))
(define rotate-bar-loop #f)
(define (rotate-bar-core)
        (while rotate-bar-loop
               (sleep 0.5)
               (print-bar)))
(define rotate-bar-thread (gen-thread rotate-bar-core))

;; Common commands.
(define (show-help)
        (if (= mode COMMAND)
            (stdout (string-append
                      "help :\n"
                      "    Shows help.\n"
                      "current-settings :\n"
                      "    Shows the current settings.\n"
                      "quit :\n"
                      "    Exits.\n"
                      "set-side <w or b> :\n"
                      "    Sets your side. 'w' is White, 'b' is Black.\n"
                      "set-thinking-time <Seconds> :\n"
                      "    Sets the computer thinking time(1 and more).\n"
                      "set-threads <Number> :\n"
                      "    Sets number of threads(1 and more).\n" 
                      "set-hash <Megabyte> :\n"
                      "    Sets the hash table size(1 and more).\n"
                      "import-sayulisp <File name> :\n"
                      "    Reads Sayulisp file and executes it.\n"
                      "play :\n"
                      "    Plays game.\n"))
            (stdout (string-append
                      "help :\n"
                      "    Shows help.\n"
                      "current-settings :\n"
                      "    Shows the current settings.\n"
                      "quit :\n"
                      "    Exits.\n"
                      "return :\n"
                      "    Exits from the game.\n"
                      "print :\n"
                      "    Prints the current board.\n"
                      "sayuri-last-move :\n"
                      "    Prints Sayuri's last move.\n"
                      "<From><To>[<Promotion>] :\n"
                      "    Makes move. Pure coordinate notation.\n"
                      "    From e2 to e4 => e2e4\n"
                      "    From d7 to d8 and promote to Queen => d7d8q\n"
                      ))))
(define (quit-app) (display "Bye.") (exit))
(define (print-current-settings)
        (display "Player's side : " (if (= player-side WHITE) "w" "b"))
        (display "Computer's thinking time : " (/ thinking-time 1000))
        (display "Number of threads : " num-threads)
        (display "Hash size : " (/ hash-size  1024 1024)))

;; command mode commands.
(define (set-side str)
        (cond ((equal? str "w") (set! player-side WHITE))
              ((equal? str "b") (set! player-side BLACK))
              (else (display "Couldn't change your side."))))
(define (set-thinking-time str)
        (define var (parse str))
        (if (and (number? var) (>= var 1))
            (set! thinking-time (* var 1000))
            (display "Couldn't change computer's thinking time.")))
(define (set-threads str)
        (define var (parse str))
        (if (and (number? var) (>= var 1))
            (set! num-threads var)
            (display "Couldn't change number of threads.")))
(define (set-hash str)
        (define var (parse str))
        (if (and (number? var) (>= var 1))
            (set! hash-size (* var 1024 1024))
            (display "Couldn't change hash table size.")))
(define (play-command)
        (set! mode GAME)
        (engine '@set-threads num-threads)
        (engine '@set-hash-size hash-size)
        (engine '@set-new-game)
        (if (= player-side BLACK)
            (begin (print-board engine)
                   (set! rotate-bar-loop #t)
                   (stdout "[Sayuri] Thinking.../")
                   (rotate-bar-thread '@start)
                   (set-engine-result (engine '@go-movetime thinking-time))
                   (set! rotate-bar-loop #f)
                   (rotate-bar-thread '@join)
                   (stdout "\n")
                   (engine '@play-move (chess->number engine-last-move))
                   (print-board engine)
                   (print-engine-last-move)
                   (if (null? engine-mate-in)
                       (display "[Sayuri] Sayuri's score : " engine-score)
                       (display "[Sayuri] Mate in " engine-mate-in)))
            ()))

;; Execute command mode command.
(define (do-command-mode-command str)
        (define str-list (string-split str " "))
        (define first
                (if (>= (length str-list) 1)
                    (car str-list)
                    "()"))
        (define second
                (if (>= (length str-list) 2)
                    (car (cdr str-list))
                    "()"))
        (cond ((equal? first "help") (show-help))
              ((equal? first "current-settings") (print-current-settings))
              ((equal? first "quit") (quit-app))
              ((equal? first "set-side") (set-side second))
              ((equal? first "set-thinking-time") (set-thinking-time second))
              ((equal? first "set-threads") (set-threads second))
              ((equal? first "set-hash") (set-hash second))
              ((equal? first "import-sayulisp") (import second))
              ((equal? first "play") (play-command))
              (else (display "Couldn't understand '" first "'."))))

;; Execute game mode.

;; Engine results.
(define engine-last-move ())
(define engine-score 0)
(define engine-mate-in ())
(define move-list ())
(define (set-engine-result result)
        (set! engine-score (ref result 0))
        (set! engine-mate-in (ref result 1))
        (set! engine-last-move (ref result 2))
        (push-back! move-list engine-last-move))
(define (print-engine-last-move)
        (if (null? engine-last-move)
            (display "[Sayuri] No last move.")
            (display "[Sayuri] "
                     (to-string (ref engine-last-move 0))
                     " -> "
                     (to-string (ref engine-last-move 1))
                     " | Promotion : "
                     (to-string (ref engine-last-move 2)))))

;; Thinking function level 1.
(define (think engine)
        (set! rotate-bar-loop #t)
        (stdout "[Sayuri] Thinking.../")
        (rotate-bar-thread '@start)
        (set-engine-result (engine '@go-movetime thinking-time))
        (set! rotate-bar-loop #f)
        (rotate-bar-thread '@join)
        (stdout "\n")
        (engine '@play-move (chess->number engine-last-move))
        (print-board engine)
        (print-engine-last-move)
        (if (null? engine-mate-in)
            (display "[Sayuri] Sayuri's score : " engine-score)
            (display "[Sayuri] Mate in " engine-mate-in)))

;; Thinking function level 2.
(define (move-and-think engine move)
        (define move-ok (engine '@play-move move))
        (if move-ok
            (begin (push-back! move-list move)
                   (print-board engine)
                   (cond ((>= (engine '@get-clock) 100)
                          (display "[Sayuri] 50 moves rule."))
                         (else (think engine))))
            (display "Couldn't make a move.")))

;; Thinking function level 3.
(define (check-and-move-and-think engine move)
        (cond ((engine '@checkmated?)
               (display "[Sayuri] Already checkmated."))
              ((engine '@stalemated?)
               (display "[Sayuri] Already stalemated."))
              ((>= (engine '@get-clock) 100)
               (display "[Sayuri] Already 50 moves rule."))
              (else (begin (move-and-think engine move)
                           (cond ((engine '@checkmated?)
                                  (display "[Sayuri] Checkmate!"))
                                 ((engine '@stalemated?)
                                  (display "[Sayuri] Stalemate..."))
                                 ((>= (engine '@get-clock) 100)
                                  (display "[Sayuri] 50 moves rule.")))))))

;; Execute game mode command.
(define (do-game-mode-command str)
        (define str-list (string-split str " "))
        (define first
                (if (>= (length str-list) 1)
                    (car str-list)
                    "()"))
        (cond ((equal? first "help") (show-help))
              ((equal? first "current-settings") (print-current-settings))
              ((equal? first "quit") (quit-app))
              ((equal? first "print") (print-board engine))
              ((equal? first "sayuri-last-move") (print-engine-last-move))
              ((equal? first "return") (set! mode COMMAND))
              (else (begin (define move (to-move str))
                           (if (null? move)
                               (display "Couldn't understand '" first "'.")
                               (check-and-move-and-think engine move))))))

;; Main loop.
(display "========================")
(display "Play Chess with Sayuri!!")
(display "========================")
(while #t 
       (if (= mode COMMAND)
           (begin (display "")
                  (display "Input Command. ('help' shows help)")
                  (stdout "(Command)>> ")
                  (do-command-mode-command (stdin '@read-line)))
           (begin (display "")
                  (display "Input Command or Move. ('help' shows help)")
                  (stdout "(Game)>> ")
                  (do-game-mode-command (stdin '@read-line)))))
