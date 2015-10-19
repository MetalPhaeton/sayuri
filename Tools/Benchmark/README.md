Benchmark
=========

'`benchmark.scm`' file is Sayulisp that measures benchmark.

Usage
-----

Run the following command.
    $ /path/to/sayuri --sayulisp /path/to/benchmark.scm

It prints statistics to Standard Output
and prints engine's output to Standard Error.

Configure
---------

If you want to configure this measurement, you can edit 27..40th lines.

* `(define threads <Number>)`
    + A number how many threads.
* `(define hash-size <Number>)`
    + Size of hash table.
    + The unit of size is 'MB'.
* `(define repeat <Number>)`
    + A number how many times to measure data.
* `(define fen <String>)`
    + A starting position for calculating.
* `(define depth <Number>)`
    + Depth for searching.
