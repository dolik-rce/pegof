FROM alpine:latest AS build
RUN apk --no-cache add git bash libc++-dev g++ make cmake bats diffutils
COPY .git /pegof/.git/
COPY src /pegof/src/
COPY packcc /pegof/packcc/
COPY CMakeLists.txt README.md update_*.sh /pegof/
RUN set -ex && \
    cd /pegof && \
    cmake -B build.d  && \
    cmake --build build.d -j 4 -v && \
    cmake --install build.d
COPY tests /pegof/tests/
COPY benchmark /pegof/benchmark/
RUN set -ex && \
    cd /pegof && \
    cmake --build build.d -v --target test

FROM alpine:latest
RUN apk --no-cache add libc++ libstdc++
COPY --from=build /usr/local/bin/pegof /bin/pegof
ENTRYPOINT ["/bin/pegof"]
CMD ["--help"]
