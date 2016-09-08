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

;; Engine.
(define engine (gen-engine))

;; Generator of feature vector ------------------------------------------------
;; Material (PAWN KNIGHT BISHOP ROOK QUEEN)
(define (gen-material engine)
        (define sign
                (if (= (eval (engine '@get-to-move)) WHITE) 1 -1))
        (define diff-list
                (map (lambda (x) (* sign x)) (engine '@analyse-diff)))
        (cdr (pop-back diff-list)))

;; Mobility.
(define (gen-mobility engine position)
        (define to-move (eval (engine '@get-to-move)))
        (define enemy-side (if (= to-move WHITE) BLACK WHITE))
        (define my-mobility 0)
        (define enemy-mobility 0)
        (define square 0)
        (for (piece position)
             (cond ((= (car piece) to-move)
                    (add! my-mobility
                          (length (engine '@analyse-mobility square))))
                   ((= (car piece) enemy-side)
                    (add! enemy-mobility
                          (length (engine '@analyse-mobility square))))
                   )
             (inc! square))
        (- my-mobility enemy-mobility))

;; Make feature vector --------------------------------------------------------
;; (<Win?> <Feature vector>)
;; <Feature vector> := <Pawn>
;;                     <Knight>
;;                     <Bishop>
;;                     <Rook>
;;                     <Queen>
;;                     <Mobility>
;; Make feature vector.
(define (make-feature-vec engine epd-str)
        (define is-ok? #t)
        (try ((engine '@set-fen epd-str)) (set! is-ok? #f))
        (if is-ok?
            (begin
              (define ret ())
              (define superior ())
              (for (elm (parse-fen/epd epd-str))
                   (if (equal? (car elm) "superior")
                       (set! superior (cadr elm))
                       ()))
              (set! superior 
                      (if (= (eval (engine '@get-to-move)) WHITE)
                          (cond ((equal? superior "w") #t)
                                ((equal? superior "b") #f)
                                (else ()))
                          (cond ((equal? superior "w") #f)
                                ((equal? superior "b") #t)
                                (else ()))))
              (define position (chess->number (engine '@get-all-pieces)))
              (define material (gen-material engine))
              (define mobility (gen-mobility engine position))
              (set! ret material)
              (push-back! ret mobility)
              (if (null? superior) () (list superior ret))
              )
            ()))

;; Make list of feature vectors.
(define (make-feature-vec-list engine epd-text)
        (define ret ())
        (define temp ())
        (define epd-str-list (string-split epd-text "\n"))
        (for (epd-str (string-split epd-text "\n"))
             (try ((parse-fen/epd epd-str)) (set! epd-str ()))  ;; Test EPD.
             (if (null? epd-str) ()
                 (begin
                   (set! temp (make-feature-vec engine epd-str))
                   (if (null? temp) () (push-back! ret temp)))))
        ret)

;; Main -----------------------------------------------------------------------
(define epd-text (stdin '@read))
(define weights-file (input-stream (ref argv 1)))
(define weights (parse (weights-file '@read)))
(weights-file ())
(define cost 1)
(define pa2 (gen-pa2 weights))

;; Train and print.
(stderr "Start training.\n")
(for (feature-vec (make-feature-vec-list engine epd-text))
     (stderr (to-string
               (pa2 '@train (car feature-vec) cost (cadr feature-vec))))
     (stderr "\n"))
(stderr "Done.\n")
(stderr "----------------------------------------\n")
(display (pa2 '@get-weights))
