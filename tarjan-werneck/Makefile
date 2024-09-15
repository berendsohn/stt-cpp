all: connectivity_st_v connectivity_st_e

connectivity_st_v: parse_input.o dyntrees connectivity_st_v.cpp
	g++ -Wall -O3 -pedantic -fpermissive --std=c++20 -Idyntrees connectivity_st_v.cpp parse_input.o dyntrees/sttrees/st_splay.o -o connectivity_st_v

connectivity_st_e: parse_input.o dyntrees connectivity_st_e.cpp
	g++ -Wall -O3 -pedantic -fpermissive --std=c++20 -Idyntrees connectivity_st_e.cpp parse_input.o dyntrees/sttrees/st_splay.o -o connectivity_st_e

parse_input.o: parse_input.h parse_input.cpp
	g++ -c -Wall -O3 parse_input.cpp

dyntrees:
	$(MAKE) -C dyntrees sttrees/st_splay.o

clean:
	rm -f connectivity_st_v
	rm -f connectivity_st_e
	rm -f *.o
	$(MAKE) -C dyntrees clean

.PHONY: dyntrees
