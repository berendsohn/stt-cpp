CC_RELEASE=g++ -Wall -O4 -pedantic -std=c++11 -DNDEBUG

all: bin/mtr_stt bin/greedy_stt bin/ltp_stt #bin/greedy_stt_debug

bin/mtr_stt: mtr_stt.cpp stt.h parse_input.o
	mkdir -p bin
	$(CC_RELEASE) mtr_stt.cpp parse_input.o -o $@

bin/mtr_stt_var%: mtr_stt.cpp stt.h parse_input.o
	mkdir -p bin
	$(CC_RELEASE) mtr_stt.cpp parse_input.o -DVARIANT=$* -o $@

bin/greedy_stt: greedy_stt.cpp stt.h parse_input.o
	mkdir -p bin
	$(CC_RELEASE) greedy_stt.cpp parse_input.o -o $@

bin/greedy_stt_var%: greedy_stt.cpp stt.h parse_input.o
	mkdir -p bin
	$(CC_RELEASE) greedy_stt.cpp parse_input.o -DVARIANT=$* -o $@

bin/ltp_stt: ltp_stt.cpp stt.h parse_input.o
	mkdir -p bin
	$(CC_RELEASE) ltp_stt.cpp parse_input.o -o $@

bin/ltp_stt_var%: ltp_stt.cpp stt.h parse_input.o
	mkdir -p bin
	$(CC_RELEASE) ltp_stt.cpp parse_input.o -DVARIANT=$* -o $@

parse_input.o: parse_input.h parse_input.cpp
	$(CC_RELEASE) -c parse_input.cpp


bin/greedy_stt_debug: greedy_stt.cpp stt.h parse_input_debug.o
	g++ -Wall -g -pedantic -std=c++20 greedy_stt.cpp parse_input_debug.o -o bin/greedy_stt_debug

parse_input_debug.o: parse_input.h parse_input.cpp
	g++ -c -Wall -g -pedantic -std=c++20 parse_input.cpp -o parse_input_debug.o

clean:
	rm -rf *.o
	rm -rf bin
