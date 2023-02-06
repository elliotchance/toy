clean:
	rm -f toy

toy: src/toy.c src/util.c src/lex.c src/parse.c src/run.c src/lib.c src/fmt.c
	clang -g src/toy.c src/util.c src/lex.c src/parse.c src/run.c src/lib.c src/fmt.c -o toy

benchmark-clang:
	node benchmark.js c /tmp/benchmark.c
	ls -lh /tmp/benchmark.c
	time clang /tmp/benchmark.c -o /tmp/benchmark && time /tmp/benchmark
	rm -f /tmp/benchmark.c /tmp/benchmark

benchmark-go:
	node benchmark.js go /tmp/benchmark.go
	ls -lh /tmp/benchmark.go
	time go run /tmp/benchmark.go
	rm -f /tmp/benchmark.go

benchmark-node:
	node benchmark.js node /tmp/benchmark.js
	ls -lh /tmp/benchmark.js
	time node /tmp/benchmark.js
	rm -f /tmp/benchmark.js

benchmark-perl:
	node benchmark.js perl /tmp/benchmark.pl
	ls -lh /tmp/benchmark.pl
	time perl /tmp/benchmark.pl
	rm -f /tmp/benchmark.pl

benchmark-toy: clean toy
	node benchmark.js toy /tmp/benchmark.toy
	ls -lh /tmp/benchmark.toy
	time ./toy /tmp/benchmark.toy
	rm -f /tmp/benchmark.toy
