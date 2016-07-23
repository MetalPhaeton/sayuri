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

;; Predicates next move with Naive Bayes.

;; PGN object.
(define pgn (gen-pgn ((input-stream (ref argv 1)) '@read)))

;; Engine.
(define engine (gen-engine))

;; Note to move.
(define (note->move engine note)
        (define candidates (chess->number(engine '@get-candidate-moves)))
        (define ret ())
        (for (move candidates)
             (if (equal? (engine '@move->note move) note) (set! ret move) ()))
        ret)

;; Current position and note to data.
(define (pos->data engine note)
        (define move (note->move engine note))
        (if (null? move)
            ()
            (chess->number (list (engine '@get-to-move)
                                 (engine '@get-all-pieces)
                                 move))))

;; Get FEN from PGN.
(define (get-fen pgn)
        (define headers (pgn '@get-current-game-headers))
        (define ret ())
        (for (elm headers)
             (if (equal? (car elm) "FEN") (set! ret (car (cdr elm))) ()))
        ret)

;; Make data list of one game.
(define (make-data-list engine pgn)
        ;; Set start position
        (define start-fen (get-fen pgn))
        (if (null? start-fen)
            (engine '@set-new-game)
            (engine '@set-fen start-fen))

        ;; Make data list.
        (define white-list ())
        (define black-list ())
        (define ret ())
        (define temp ())
        (while (not (null? (pgn '@current-move)))
               (set! temp (pos->data engine (pgn '@current-move)))
               (if (null? temp)
                   ()
                   (begin (if (equal? (car temp) WHITE)
                              (push-back! white-list temp)
                              (push-back! black-list temp))
                          (engine '@play-move (car (cdr (cdr temp))))
                          )
                   )
               (pgn '@next-move))
        (list white-list black-list))

;; Make all data list.
(define (make-all-data engine pgn)
        (define white-list ())
        (define black-list ())
        (define result ())
        (for (i (range (pgn '@length)))
             (pgn '@set-current-game i)
             (set! result (make-data-list engine pgn))
             (set! white-list (append white-list (car result)))
             (set! black-list (append black-list (car (cdr result))))
             )
        (list white-list black-list))

;; FEN to side and position.
(define (fen->pos engine)
        (chess->number
          (list (engine '@get-to-move) (engine '@get-all-pieces))))

;; Make condition func list.
(define (gen-conditions engine)
        (define side-and-pos (fen->pos engine))
        (define side (car side-and-pos))
        (define pos (car (cdr side-and-pos)))
        (define (gen-func side piece square)
                (define lambda-expr
                        `(lambda (data)
                                 (define data-side (car data))
                                 (define data-position (car (cdr data)))
                                 (equal? (ref data-position ,square)
                                         (quote ,piece))
                                 ))
                (eval lambda-expr))
        (define ret ())
        (for (square (range 64))
             (push-back! ret (gen-func side (car pos) square))
             (set! pos (cdr pos)))
        ret)

;; Make predicate.
(define (gen-predicates candidates)
        (define (gen-func move)
                (define lambda-expr
                        `(lambda (data)
                                 (define data-move (car (cdr (cdr data))))
                                 (equal? data-move (quote ,move))))
                (eval lambda-expr))
        (define ret ())
        (for (move candidates)
             (push-back! ret (gen-func move)))
        ret)

;; Move to PCN
(define square-str
        '("a1" "b1" "c1" "d1" "e1" "f1" "g1" "h1"
          "a2" "b2" "c2" "d2" "e2" "f2" "g2" "h2"
          "a3" "b3" "c3" "d3" "e3" "f3" "g3" "h3"
          "a4" "b4" "c4" "d4" "e4" "f4" "g4" "h4"
          "a5" "b5" "c5" "d5" "e5" "f5" "g5" "h5"
          "a6" "b6" "c6" "d6" "e6" "f6" "g6" "h6"
          "a7" "b7" "c7" "d7" "e7" "f7" "g7" "h7"
          "a8" "b8" "c8" "d8" "e8" "f8" "g8" "h8"))
(define piece-str '("" "p" "n" "b" "r" "q" "k"))
(define (move->pcn move)
        (string-append (ref square-str (car move))
                       (ref square-str (car (cdr move)))
                       (ref piece-str (car (cdr (cdr move))))))

;; Predicate for bubble sort.
(define (sort-order prev next)
        (> (car (cdr prev)) (car (cdr next))))

;; Main.
(stderr "Loading...")
(define data-list (make-all-data engine pgn))
(stderr "done.\n")

(define loop #t)
(define input "")
(define candidates ())
(define conditions ())
(define predicates ())
(define data-index 0)
(define logit-list ())
(while loop
       (set! input (stdin '@read-line))
       (if (null? input)
           (set! loop #f)
           (try ((engine '@set-fen input)
                 (set! candidates
                       (chess->number (engine '@get-candidate-moves)))
                 (set! conditions (gen-conditions engine))
                 (set! predicates (gen-predicates candidates))
                 (set! data-index (-- (chess->number (engine '@get-to-move))))
                 (display input)
                 (set! logit-list
                       (bayes (ref data-list data-index)
                              predicates
                              conditions))
                 (for (result (list-sort (zip candidates logit-list)
                                         sort-order))
                      (display (move->pcn (car result))
                               " "
                               (car (cdr result)))
                      )
                 (stdout "\n"))
                ())))

