#include "parse_input.h"

#include <cassert>
#include <cstdio>

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Query::Query( QueryType type, long arg1, long arg2, long arg3 ) : type( type ), arg1( arg1 ), arg2( arg2 ), arg3( arg3 ) {}

std::ostream& operator<<( std::ostream& out, const Query& query ) {
	return out << query.type << " " << query.arg1 << " " << query.arg2 << " " << query.arg3;
}

bool read_query_file( const char* filename, size_t& num_vertices, std::vector<Query>& queries ) {
	std::ifstream ifs( filename, std::ios::in );
	if( !ifs.is_open() ) {
		std::cerr << "ERROR: Cannot open file '" << filename << "'\n";
		return false;
	}
	
	std::string line;
	bool found_header = false;
	while( getline( ifs, line ) ) {
//		std::cout << "Reading line '" << line << "'\n";
		long arg1;
		long arg2;
		//long arg3;
		if( sscanf( line.c_str(), "lca %ld %ld", &arg1, &arg2 ) == 2
				|| sscanf( line.c_str(), "queries %ld %ld", &arg1, &arg2 ) == 2
				|| sscanf( line.c_str(), "con %ld %ld", &arg1, &arg2 ) == 2 ) {
			if( found_header ) {
				std::cerr << "ERROR: Found second header '" << line << "'\n";
				return false;
			}
			assert( arg1 >= 0 );
			num_vertices = arg1;
			found_header = true;
			continue;
		}
		
		if( line.empty() || line[0] == 'c' ) {
			// Comment
			continue;
		}
		
		if( !found_header ) {
			std::cerr << "ERROR: Missing header before line '" << line << "'\n";
			return false;
		}
		
		if( sscanf( line.c_str(), "i %ld %ld", &arg1, &arg2 ) == 2 ) {
			queries.push_back( Query( LINK, arg1, arg2 ) );
		}
		else if( sscanf( line.c_str(), "d %ld %ld", &arg1, &arg2 ) == 2 ) {
			queries.push_back( Query( CUT, arg1, arg2 ) );
		}
		else if( sscanf( line.c_str(), "d %ld", &arg1 ) == 1 ) {
			queries.push_back( Query( CUT_FROM_PARENT, arg1 ) );
		}
		else if( sscanf( line.c_str(), "a %ld %ld", &arg1, &arg2 ) == 2 ) {
			queries.push_back( Query( LCA, arg1, arg2 ) );
		}
		else if( sscanf( line.c_str(), "p %ld %ld", &arg1, &arg2 ) == 2 ) {
			queries.push_back( Query( PATH, arg1, arg2 ) );
		}
		else {
			std::cerr << "ERROR: Cannot parse line '" << line << "'\n";
			return false;
		}
	}
	return true;
}
