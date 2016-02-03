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

;; Generates EPD from PGN.

;; Generate engins.
(define engine (gen-engine))

;; Load PGN file from argv.
(define istream (input-stream (list-ref argv 1)))
(define pgn-txt (istream '@read))
(istream ()) ;; Closes file.

;; Generate PGN object.
(define pgn (gen-pgn pgn-txt))

;; Make FEN List.
(define (make-fen-list pgn)
  (define ret ())
  (while (not (equal? (pgn '@current-move) ""))
    (push-back! ret (list (engine '@get-fen) (pgn '@current-move)))
    (engine '@play-note (pgn '@current-move))
    (pgn '@next-move))
  ret)

;; Ommit last 2 elements from fen.
(define (ommit-last-2 fen)
  (define temp (string-split fen " "))
  (pop-back! temp)(pop-back! temp)
  (define ret "")
  (for (i (range (length temp)))
    (if (= i 0)
      (set! ret (string-append ret (list-ref temp i)))
      (set! ret (string-append ret " " (list-ref temp i)))))
  ret)

;; Make EPD list.
(define (make-epd-list pgn)
  (define ret ())
  (for (fen-tup (make-fen-list pgn))
    (push-back! ret
      (string-append
        (ommit-last-2 (list-ref fen-tup 0))
        " pm "
        (list-ref fen-tup 1)
        ";")))
  ret)

;; Set FEN starting position.
(define (find-fen-head pgn)
  (define ret ())
  (for (head (pgn '@get-current-game-headers))
    (if (equal? (list-ref head 0) "FEN") (set! ret (list-ref head 1)) ()))
  ret)
(define (set-starting-position pgn)
  (define fen (find-fen-head pgn))
  (if (null? fen) (engine '@set-new-game) (engine '@set-fen fen)))

;; Print EPD.
(define pgn-len (pgn '@length))
(define list-2 ())
(define list-2-len 0)
(for (i (range pgn-len))
  (pgn '@set-current-game i)
  (set-starting-position pgn)
  (set! list-2 (make-epd-list pgn))
  (set! list-2-len (length list-2))
  (for (j (range list-2-len))
    (display (list-ref list-2 j))))
