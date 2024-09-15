#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#ifndef PARSE_INPUT_H

enum QueryType { LINK, CUT, CUT_FROM_PARENT, LCA, PATH };

struct Query {
	QueryType type;
	long arg1;
	long arg2;
	long arg3;
	
	Query( QueryType type, long arg1, long arg2 = -1, long arg3=-1 );
};

std::ostream& operator<<( std::ostream& out, const Query& query );

bool read_query_file( const char* filename, size_t& num_vertices, std::vector<Query>& queries );


/* Requires class with the following methods:
void link( size_t u, size_t v );
void cut( size_t u, size_t v );
bool is_connected( size_t u, size_t v );
*/

template<typename T>
bool bench_queries( size_t num_vertices, const std::vector<Query>& queries, size_t repeat, bool json, const char* algo_name ) {
	auto start = std::chrono::high_resolution_clock::now();
	
	int total_cons = 0; // To avoid optimizing away path queries.
	for(size_t i = 0; i < repeat; i++ ) {
		T t( num_vertices );
		for( const auto& query : queries ) {
			if( query.type == LINK ) {
				t.link( query.arg1, query.arg2 );
			}
			else if( query.type == CUT ) {
				t.cut( query.arg1, query.arg2 );
			}
			else if( query.type == PATH ) {
				total_cons += t.is_connected( query.arg1, query.arg2 );
			}
			else {
				std::cerr << "Cannot execute query '" << query << "'\n";
				return false;
			}
		}
	}
	
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::high_resolution_clock::now() - start );
	if( json ) {
		std::cout << "{\"num_vertices\":" << num_vertices << ",\"num_queries\":" << queries.size() << ",\"name\":\"" << algo_name << "\",\"time_ns\":" << duration.count() * 1000 / repeat << "}" << std::endl;
	}
	else {
		std::cout << "Total yes-anwers: " << total_cons / repeat << "\n";
//		std::cout << "Rotations: " << num_rotations / repeat << "/run – " << num_rotations / repeat / queries.size() << "/query\n";
		std::cout << duration.count() << " us total\n";
		std::cout << duration.count() / repeat << " us/run\n";
		std::cout << duration.count() * 1. / repeat / queries.size() << " us/query\n";
	}
	return true;
}

template<typename T>
bool compute_queries( size_t num_vertices, const std::vector<Query>& queries ) {
	T t( num_vertices );
	for( const auto& query : queries ) {
		if( query.type == LINK ) {
			t.link( query.arg1, query.arg2 );
		}
		else if( query.type == CUT ) {
			t.cut( query.arg1, query.arg2 );
		}
		else if( query.type == PATH ) {
			std::cout << (int) t.is_connected( query.arg1, query.arg2 ) << "\n";
		}
		else {
			std::cerr << "Cannot execute query '" << query << "'\n";
			return false;
		}
	}
	
	return true;
}

template<typename T>
int main_connectivity( int argc, const char** argv ) {
	if( argc < 3 ) {
		std::cout << "usage: " << argv[0] << " <bench|compute> <...> <query-file>\n";
		return 1;
	}
	
	std::string cmd( argv[1] );
	if( cmd == "bench" ) {
		if( !( argc == 4 || ( argc == 5 && std::strcmp( argv[2], "--json" ) == 0 ) ) ) {
			std::cout << "usage: " << argv[0] << " bench [--json] <repeat> <query-file>\n";
			return 1;
		}
		size_t repeat = std::atol( argv[argc-2] );
		
		bool json = ( argc == 5 );
		
		size_t num_vertices;
		std::vector<Query> queries;
		if( !read_query_file( argv[argc-1], num_vertices, queries ) ) {
			std::cerr << "Failed parsing file' " << argv[argc-1] << "'\n";
			return 2;
		}
		
		if( !json ) {
			std::cout << "Successfully parsed file. Now executing " << queries.size() << " queries on " << num_vertices << " vertices " << repeat << " times." << std::endl;
		}
		
		if( ! bench_queries<T>( num_vertices, queries, repeat, json, argv[0] ) ) {
			return 3;
		}
	}
	else if( cmd == "compute" ) {
		size_t num_vertices;
		std::vector<Query> queries;
		if( !read_query_file( argv[2], num_vertices, queries ) ) {
			std::cerr << "Failed parsing file' " << argv[2] << "'\n";
			return 2;
		}
		if( ! compute_queries<T>( num_vertices, queries ) ) {
			return 3;
		}
	}
	else {
		std::cout << "usage: " << argv[0] << " <bench|compute> <...> <query-file>\n";
		return 1;
	}
	
	
	return 0;
}

#endif
