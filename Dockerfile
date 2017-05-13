FROM alpine
MAINTAINER Hironori Ishibashi <phaeton2718@gmail.com>

RUN mkdir -p /usr/local/bin /usr/local/share/sayulisp /tmp/sayuri_build
ADD src/* /tmp/sayuri_build/

RUN apk update && apk --no-cache add g++

RUN g++ -Ofast -std=c++11 -fno-rtti -fexceptions -m64 -pthread -o /usr/local/bin/sayuri /tmp/sayuri_build/*.cpp

RUN rm -rf /tmp/sayuri_build

ADD Tools/Benchmark/benchmark.scm /usr/local/share/sayulisp/
ADD Tools/PlayWithSayuri/play-with-sayuri.scm /usr/local/share/sayulisp/

RUN adduser -Dh /home/user user
USER user
WORKDIR /home/user

RUN ln -s /usr/local/share/sayulisp/benchmark.scm benchmark
RUN ln -s /usr/local/share/sayulisp/play-with-sayuri.scm play-with-sayuri

ENTRYPOINT ["/usr/local/bin/sayuri"]
