;; The MIT License (MIT)
;;
;; Copyright (c) 2015 Hironori Ishibashi
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

;;;;;;;;;;;;;;
;; Settings ;;
;;;;;;;;;;;;;; You can edit this section.
;;-----------------------------------------------------------------------------
;; Number of threads.
(define threads 2)

;; Size of hash table. (MB)
(define hash-size 512)

;; Number of repeat.
(define repeat 30)

;; Position. (FEN)
(define fen "r1bq1r1k/p1pnbpp1/1p2p3/6p1/3PB3/5N2/PPPQ1PPP/2KR3R w - - 0 1")

;; Depth. (Ply)
(define depth 13)
;;-----------------------------------------------------------------------------

;; Generate Engine.
(define engine (gen-engine))

;; Data.
(define output "")
(define data-time ())
(define data-nodes ())
(define data-nps ())
(define data-hashfull ())

;; If str is "info..", then update info-str.
(define (info?-update str)
  (if (equal? (ref (string-split str " ") 0) "info")
    (set! output str) ()))

;; If str is "bestmove..", then return #t. Otherwise return #f
(define (bestmove? str)
  (if (equal? (ref (string-split str " ") 0) "bestmove") #t #f))

;; Append time data.
(define (append-time li)
  (push-back! data-time
              (parse (ref li (++ (list-search "time" li))))))

;; Append nodes data.
(define (append-nodes li)
  (push-back! data-nodes
              (parse (ref li (++ (list-search "nodes" li))))))

;; Append nps data.
(define (append-nps li)
  (push-back! data-nps
              (parse (ref li (++ (list-search "nps" li))))))

;; Append hashfull data.
(define (append-hashfull li)
  (push-back! data-hashfull
              (parse (ref li (++ (list-search "hashfull" li))))))

;; Mean.
(define (mean li)
  (/ (apply '+ li) (length li)))

;; Dispersion.
(define (var li)
  (define m (mean li))
  (define (func x) (* (- x m) (- x m)))
  (mean (map 'func li)))

;; Standard Deviation.
(define (dev li) (sqrt (var li)))

;; Log.

;; Mode.
(define (mode li)
  (define (func x) (if (<= x 0.01) (log 0.01) (log x)))
  (define log-nums (map 'func li))
  (exp (- (mean log-nums) (var log-nums))))

;; Listener.
(define (output-listener message)
  (stderr (append message "\n"))
  (info?-update message))

;; Print result.
(define (print-result)
  (display "##########")
  (display "# Result #")
  (display "##########")

  (display "")

  ;; Configure.
  (display "Settings:")
  (display "      Threads: " threads)
  (display "    Hash Size: " hash-size)
  (display "       Repeat: " repeat)
  (display "     Position: " fen)
  (display "        Depth: " depth)

  (display "")

  ;; Time.
  (display "Time:")
  (display "    Mean: " (mean data-time))
  (display "     Max: " (apply 'max data-time))
  (display "     Min: " (apply 'min data-time))
  (display "    SDev: " (dev data-time))
  (display "    Mode: " (mode data-time))

  (display "")

  ;; Nodes.
  (display "Nodes:")
  (display "    Mean: " (mean data-nodes))
  (display "     Max: " (apply 'max data-nodes))
  (display "     Min: " (apply 'min data-nodes))
  (display "    SDev: " (dev data-nodes))
  (display "    Mode: " (mode data-nodes))

  (display "")

  ;; NPS
  (display "NPS:")
  (display "    Mean: " (mean data-nps))
  (display "     Max: " (apply 'max data-nps))
  (display "     Min: " (apply 'min data-nps))
  (display "    SDev: " (dev data-nps))
  (display "    Mode: " (mode data-nps))

  (display "")

  ;; Hash Full
  (display "Hash Full:")
  (display "    Mean: " (mean data-hashfull))
  (display "     Max: " (apply 'max data-hashfull))
  (display "     Min: " (apply 'min data-hashfull))
  (display "    SDev: " (dev data-hashfull))
  (display "    Mode: " (mode data-hashfull)))

;; --- Run --- ;;
;; Get ready.
(engine '@add-uci-output-listener output-listener)
(engine '@input-uci-command
        (append "setoption name threads value " (to-string threads)))
(engine '@input-uci-command
        (append "setoption name hash value " (to-string hash-size)))
;; Go.
(define str-list ())
(for (x (range repeat))
       (engine '@input-uci-command "ucinewgame")
       (engine '@input-uci-command (append "position fen " fen))
       (engine '@go-depth depth)
       (set! str-list (string-split output " "))
       (append-time str-list)
       (append-nodes str-list)
       (append-nps str-list)
       (append-hashfull str-list))
;; Result.
(stderr "\n")
(print-result)
