Machine Learning Test
=====================

This is test application for machine learning of evaluation weights.  
The algorithm is PA-2.

Weights are

* Material weight of Pawn.
* Material weight of Knight.
* Material weight of Bishop.
* Material weight of Rook.
* Material weight of Queen.
* Weight of mobility.

Usage
-----

(1) Run by the following command.

    $ sayuri --sayulisp machine-learning-test.scm <File name that inital weights is written on>

(2) Input EPD with "`superior <w | b>`" opcode into Standard Input.

* "`superior w`" means White has advantage.
* "`superior b`" means Black has advantage.

    rn1qkbnr/ppp2ppp/4b3/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - superior w ;
    rn1qkbnr/ppp2ppp/4b3/8/8/5N2/PPPP1PPP/RNBQKB1R b KQkq - superior w ;
    r1bqkb1r/ppp2ppp/2n5/4p3/4p3/1B3N2/PPPP1PPP/R1BQK2R w KQkq - superior b ;
    r1bqkb1r/ppp2ppp/2n5/4p3/4p3/1B6/PPPP1PPP/R1BQK1NR b KQkq - superior b ;

(3) The application prints information to Standard Error
and result(the last of lines) to Standard Output.

    Start training.
    (0.00388349514563107 0 0 0 0 -0.0621359223300971)
    (0.00388349514563107 0 0 0 0 -0.0621359223300971)
    (0.0133316947277867 0 0 0 0 0.0701388718200811)
    (0.0137707737401688 0 0 0 0 0.0758468989810492)
    Done.
    ----------------------------------------
    (0.0137707737401688 0 0 0 0 0.0758468989810492)
