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
    b8a6 6.13750806231518
    a7a5 0.687812231472485
    h7h6 -0.312330710579701
    c8g4 -1.79113841288506
    a7a6 -4.10237147774551
    b7b6 -4.8060851496101
    b7b5 -5.57216043987761
    d6d5 -6.15560178200187
    f8e8 -6.70605048509202
    f6e4 -9.20394592967672
    d8a5 -11.9414970093864
    e7e5 -13.2909211632065
    f6d5 -15.4484404049009
    e7e6 -16.0394393503478
    c8d7 -16.4272456292174
    c6c5 -17.228835102323
    d8c7 -17.283123471267
    h7h5 -17.5822444275775
    f6h5 -20.8642832286062
    d8d7 -23.6476139655796
    c8f5 -24.5503467241813
    c8e6 -26.7194017746249
    g8h8 -26.8877983291517
    f6d7 -27.3061165426527
    d8b6 -32.1956885274665
    g6g5 -42.2378554347392
    d8e8 -54.5385998733888
    g7h6 -60.2472739497237
    b8d7 -71.2596246640096
    f6e8 -89.0667059228656
    f6g4 -98.2932677211238
    g7h8 -186.652255188352
    c8h3 -609.498406362403

    rnbqkbnr/pp2pppp/2p5/3p4/3PP3/8/PPP2PPP/RNBQKBNR w KQkq d6 sm exd5;
    g1f3 33.490533999915
    b1c3 25.4450643920725
    c2c4 20.5121165224085
    f1d3 5.85788361299489
    g2g3 4.84041474890144
    e4d5 1.92597270022715
    b1d2 1.60699696245538
    f1c4 -0.150027653821937
    e4e5 -1.22018147337763
    f1b5 -1.94619302133943
    f2f4 -6.05416028473775
    f2f3 -7.89219363057848
    c1e3 -7.92250499404898
    h2h3 -8.16934499895975
    c2c3 -8.32198864142307
    a2a3 -12.5055601976397
    c1g5 -18.0680565253959
    d1f3 -19.076575654753
    d1e2 -21.9472168274493
    f1e2 -24.117657696378
    b2b3 -26.8358490339778
    a2a4 -32.2186694806645
    d1d2 -33.6311532897358
    c1f4 -37.2397712144844
    g1e2 -45.0466580525277
    c1d2 -45.3909793952942
    b2b4 -49.5350696375634
    h2h4 -59.7914501192615
    d1d3 -63.2854844917056
    g2g4 -67.4066345217619
    e1e2 -126.352436356938
    d1g4 -137.469743249456
    c1h6 -262.272831556404
    e1d2 -264.270888984493
    b1a3 -330.78709679759
    d1h5 -342.164402167898
    f1a6 -399.021734905868
    g1h3 -617.537511849338


### Note! ###

Sayulisp is very slow. Please wait for calculating patiently.
