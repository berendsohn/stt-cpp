#include <cstddef>

#ifndef TARJAN_WERNECK_CONNECTIVITY_H
#define TARJAN_WERNECK_CONNECTIVITY_H

template <typename T>
class TWSTF {
public :
	TWSTF( size_t n ) : t( n ) {}
	
	void link( size_t u, size_t v ) {
		t.evert( u );
		t.evert( v ); // Only for amortization!
		t.link( u, v, false );
	}
	
	void cut( size_t u, size_t v ) {
		t.evert( v );
		t.cut( u );
	}
	
	bool is_connected( size_t u, size_t v ) {
		t.evert( u );
		bool con = ( t.getRoot( v ) == u );
		t.evert( v );
		return con;
	}
	
private :
	T t;
};

#endif //TARJAN_WERNECK_CONNECTIVITY_H
