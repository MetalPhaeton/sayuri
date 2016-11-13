Bayesian Predictor
==================

Predicts logit of candidate moves with Naive Bayes.  
A move of the greatest logit is the best move predicted by Naive Bayes.

Usage
-----

(1) Run with the following command.

    $ sayuri --sayulisp bayesian-predictor.scm <PGN - training data>

(2) Input FEN(EPD) of target positions into Standard Input.

(3) This application prints each move and logit.

### Example ###

(1) Prepare PGN that is recorded 100 games of a grand master. (games100.pgn)

(2) Prepare EPD of target positions. (positions.epd)

    rnbq1rk1/pp2ppbp/2pp1np1/8/2PP4/2N2NP1/PP2PPBP/R1BQ1RK1 b - - sm Bf5;
    rnbqkbnr/pp2pppp/2p5/3p4/3PP3/8/PPP2PPP/RNBQKBNR w KQkq d6 sm exd5;

(3) Run the application and input EPD.

    $ sayuri --sayulisp bayesian-predictor.scm games100.pgn < positions.epd

(4) The application prints candidate moves of target positions and their logit.

    rnbq1rk1/pp2ppbp/2pp1np1/8/2PP4/2N2NP1/PP2PPBP/R1BQ1RK1 b - - sm Bf5;
    b8a6 8.87307867321218
    b8d7 5.43718534870303
    a7a5 5.42525352569573
    b7b6 2.35177735240213
    a7a6 1.96173020911842
    b7b5 -0.315286017369793
    c8g4 -0.79696600505315
    h7h6 -2.33320869622204
    f8e8 -2.62079430998293
    e7e6 -4.40709271607759
    c8e6 -4.60973164866542
    e7e5 -4.62365465243158
    f6e4 -6.00316360731918
    c8d7 -6.03973060272443
    f6d7 -10.3726810083573
    d8c7 -11.0677645616713
    d8d7 -11.9084690289923
    c8f5 -13.0239467410547
    h7h5 -14.0302980595905
    c6c5 -15.784082941164
    f6d5 -16.6320233548506
    d8b6 -18.4339959476821
    d6d5 -19.0953846228091
    g8h8 -25.4841260427582
    d8e8 -25.755837560091
    d8a5 -31.5460732652668
    f6h5 -36.0403390767925
    g6g5 -40.3594563758887
    g7h6 -51.879051527182
    f6e8 -58.7400688928571
    f6g4 -61.3119501259547
    g7h8 -100.766748993055
    c8h3 -246.881850535797
    
    rnbqkbnr/pp2pppp/2p5/3p4/3PP3/8/PPP2PPP/RNBQKBNR w KQkq d6 sm exd5;
    g1f3 31.6381711120375
    b1c3 25.2505087221749
    c2c4 21.0858535562359
    f1d3 10.2919754806382
    b1d2 8.49762994897016
    g2g3 7.84949327446007
    f1c4 7.76220097423965
    f1b5 6.58662725178827
    e4d5 6.00735860375455
    e4e5 1.37730237172813
    f2f4 1.07003803811872
    c2c3 0.203342824155712
    c1e3 -0.673942614448464
    h2h3 -4.42080522766902
    a2a3 -4.43873561555837
    f2f3 -4.49634144941885
    d1f3 -6.06695279593275
    f1e2 -11.2247289222239
    c1g5 -15.150900912671
    c1f4 -16.8810133214114
    a2a4 -17.8293628239715
    b2b3 -18.6675783849145
    d1e2 -21.7671674532762
    c1d2 -25.4263292553627
    g1e2 -26.4236712365485
    b2b4 -34.5072671416013
    d1d2 -38.4905654558122
    d1d3 -49.1106051396921
    g2g4 -59.6441129662521
    d1g4 -60.5157972567367
    h2h4 -60.6684095099909
    e1e2 -93.1896349405207
    e1d2 -103.022023798502
    b1a3 -215.225390349762
    c1h6 -231.696497352152
    d1h5 -256.403678979122
    g1h3 -442.476252208069
    f1a6 -442.476252208069


### Note! ###

Sayulisp is very slow. Please wait for calculating patiently.
