all : dtree_queries

dtree_queries: dtree_queries.cpp parse_input.o
	g++ -Wall -O3 --std=c++20 -Idtree-May_2014 dtree_queries.cpp parse_input.o -o dtree_queries

parse_input.o: parse_input.h parse_input.cpp
	g++ -c -Wall -O3 --std=c++20 parse_input.cpp

clean:
	rm -f dtree_queries
	rm -f *.o
