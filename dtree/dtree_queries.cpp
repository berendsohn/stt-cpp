// Mostly copied from https://www.davideisenstat.com/dtree

#include <iostream>
#include <chrono>

#include "dtree/tree.h"
#include "dtree/tree-inl.h"

#include "parse_input.h"

using namespace std;

class Forest {
 private:
  typedef dtree::WithEvert<dtree::Begin<>> E;
  typedef dtree::EndTree<E> Node;

 public:
  typedef size_t NodeId;
  Forest();
  ~Forest();
  // creates a forest with node ids 0..size-1
  void Initialize(size_t size);
  // as in [ST85]
  NodeId FindRoot(NodeId v) { return Root(&node_[v]) - node_; }
  void Link(NodeId v, NodeId w) { dtree::Link(&node_[v], &node_[w]); }
  void Cut(NodeId v) { dtree::Cut(&node_[v]); }
  void Evert(NodeId v) { E::Evert(&node_[v]); }

 private:
  Node* node_;
  // disallows the copy constructor and the assignment operator
  Forest(const Forest&);
  void operator=(const Forest&);
};

Forest::Forest() : node_(NULL) {
}

Forest::~Forest() {
  delete[] node_;
}

void Forest::Initialize(size_t size) {
  Node* old_node = node_;
  // avoids a double delete[] if new[] throws
  node_ = NULL;
  delete[] old_node;
  node_ = new Node[size];
}

int main_old( int argc, char** argv ) {
	if( argc != 2 ) {
		cout << "usage: dtree_queries <query-file>\n";
		return -1;
	}
	size_t num_vertices;
	vector<Query> queries;
	
	if( !read_query_file( argv[1], num_vertices, queries ) ) {
		cerr << "Failed parsing file\n";
		return 1;
	}
	
	const size_t REPEATS = 20;
	
	cout << "Successfully parsed file. Now executing " << queries.size() << " queries on " << num_vertices << " vertices " << REPEATS << " times." << endl;
	
	auto start = chrono::high_resolution_clock::now();
	
	int total_cons = 0; // To avoid optimizing away path queries.
    for( size_t i = 0; i < REPEATS; i++ ) {
		Forest lc;
		lc.Initialize( num_vertices );
		for( const auto& query : queries ) {
			if( query.type == LINK ) {
				lc.Evert( query.arg1 );
				lc.Link( query.arg1, query.arg2 );
			}
			else if( query.type == CUT ) {
				lc.Evert( query.arg2 );
				lc.Cut( query.arg1 );
			}
			else if( query.type == PATH ) {
				lc.Evert( query.arg1 );
				bool con = ( lc.FindRoot( query.arg2 ) == (size_t) query.arg1 );
				lc.Evert( query.arg2 );
				total_cons += con;
			}
			else if( query.type == CUT_FROM_PARENT ) {
				lc.Cut( query.arg1 );
			}
			else if( query.type == LCA ) {
				assert( false );
			}
			else {
				cerr << "Cannot execute query '" << query << "'\n";
			}
		}
	}
	
	auto duration = chrono::duration_cast<chrono::microseconds>( chrono::high_resolution_clock::now() - start );
	cout << "Total yes-anwers: " << total_cons / REPEATS << "\n";
	cout << duration.count() << " us total\n";
	cout << duration.count() / REPEATS << " us/run\n";
	cout << duration.count() * 1.0 / REPEATS / queries.size() << " us/query\n";
	
	//Test();
	return 0;
}

class DTreeSTF {
public :
	typedef size_t NodeIdx;
	DTreeSTF( size_t num_nodes ) {
		lc.Initialize( num_nodes );
	}
	
	void link( NodeIdx u, NodeIdx v ) {
		lc.Evert( u );
		lc.Link( u, v );
	}
	
	void cut( NodeIdx u, NodeIdx v ) {
		lc.Evert( v );
		lc.Cut( u );
	}
	
	bool is_connected( NodeIdx u, NodeIdx v ) {
		lc.Evert( u );
		bool con = ( lc.FindRoot( v ) == (size_t) u );
		lc.Evert( v );
		return con;
	}
private :
	Forest lc;
};

int main( int argc, const char** argv ) {
	main_connectivity<DTreeSTF>( argc, argv );
}