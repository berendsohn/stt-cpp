#include <cassert>

#include "parse_input.h"

#ifndef VARIANT
#define VARIANT 6
#endif

/** Variants
 * 1-6: MTR
 */

#if ( VARIANT >= 1 && VARIANT <= 3 )
#define ROT_IMPROVED
#elif VARIANT >= 4
#define ROT_NST
#endif

#include "stt.h"

namespace mtr_stt {
	using namespace stt;

#if VARIANT == 0
	// Naive MTR impl
	static inline void access( Node* v ) {
		while( Node* p = v->parent ) {
			if( !v->is_separator_hint( p ) ) {
				while( Node* g = p->parent ) {
					if( p->is_separator_hint( g ) ) {
						p->rotate();
//							std::cout << *this << "\n";
						continue;
					}
					break;
				}
			}
			// Now either v is a separator, or p is not, meaning we are allowed to rotate at v.
			v->rotate();
		}
	}
#elif VARIANT == 1
	inline void access( Node* v ) {
		while( Node* p = v->parent ) {
			if( !v->is_separator_hint( p ) ) {
				// Rotate at p as long as p is a separator
				if( Node* g = p->parent ) {
					bool is_p_sep = p->is_separator_hint( g );
					while( is_p_sep ) {
						is_p_sep = p->rotate();
					}
				}
			}
			// Now either v is a separator, or p is not, meaning we are allowed to rotate at v.
			v->rotate();
		}
	}
#elif VARIANT == 2
	inline void access( Node* v ) {
		Node* p = v->parent;
		
		if( p == nullptr ) {
			return;
		}
		
		bool is_v_sep = v->is_separator_hint( p );
		while( true ) {
			if( !is_v_sep ) {
				// Rotate at p as long as p is a separator
				if( Node* g = p->parent ) {
					bool is_p_sep = p->is_separator_hint( g );
					while( is_p_sep ) {
						is_p_sep = p->rotate();
					}
				}
			}
			// Now either v is a separator, or p is not, meaning we are allowed to rotate at v.
			is_v_sep = v->rotate();
			// Now v may not be a separator anymore
			p = v->parent;
			if( p == nullptr ) {
				return;
			}
		}
	}
#elif VARIANT == 3
	static inline void access( Node* v ) {
		bool is_v_sep = v->is_separator();
		while( Node* p = v->parent ) {
			if( !is_v_sep ) {
				// Rotate at p as long as p is a separator
				if( Node* g = p->parent ) {
					bool is_p_sep = p->is_separator_hint( g );
					while( is_p_sep ) {
						is_p_sep = p->rotate();
					}
				}
			}
			// Now either v is a separator, or p is not, meaning we are allowed to rotate at v.
			is_v_sep = v->rotate();
		}
	}
#elif VARIANT == 4
	static inline void access( Node* v ) {
		NodeSepType v_sep_type = v->get_sep_type();
		while( Node* p = v->parent ) {
			if( v_sep_type == NOSEP ) {
				// Rotate at p as long as p is a separator
				auto p_sep_type = p->get_sep_type();
				while( p_sep_type != NOSEP ) {
					if( p_sep_type == DSEP ) {
						p_sep_type = p->rotate();
					}
					else { // p_sep_type == ISEP
						assert( p_sep_type == ISEP );
						p_sep_type = p->rotate();
					}
					assert( p_sep_type == p->get_sep_type() );
				}
				assert( !p->is_separator() );
				
				// Now both v and p are NOSEP
				v_sep_type = v->rotate();
			}
			else if( v_sep_type == DSEP ) {
				v_sep_type = v->rotate();
			}
			else {
				assert( v_sep_type == ISEP );
				v_sep_type = v->rotate();
			}
		}
	}
#elif VARIANT == 5
	static inline void access( Node* v ) {
		NodeSepType v_sep_type = v->get_sep_type();
		while( Node* p = v->parent ) {
			if( v_sep_type == NOSEP ) {
				// Rotate at p as long as p is a separator
				auto p_sep_type = p->get_sep_type();
				while( p_sep_type != NOSEP ) {
					if( p_sep_type == DSEP ) {
						p_sep_type = p->rotate_dsep();
					}
					else { // p_sep_type == ISEP
						assert( p_sep_type == ISEP );
						p_sep_type = p->rotate_isep();
					}
					assert( p_sep_type == p->get_sep_type() );
				}
				assert( !p->is_separator() );
				
				// Now both v and p are NOSEP
				v_sep_type = v->rotate_nosep();
				// Note: from here on, v_sep_type can never become anything else than nosep
			}
			else if( v_sep_type == DSEP ) {
				v_sep_type = v->rotate_dsep();
			}
			else {
				assert( v_sep_type == ISEP );
				v_sep_type = v->rotate_isep();
			}
		}
	}
#elif VARIANT == 6
	static inline void access( Node* v ) {
		NodeSepType v_sep_type = v->get_sep_type();
		while( v_sep_type != NOSEP ) {
			if( v_sep_type == DSEP ) {
				v_sep_type = v->rotate_dsep();
			}
			else {
				assert( v_sep_type == ISEP );
				v_sep_type = v->rotate_isep();
			}
		}
		
		while( Node* p = v->parent) {
			assert( !v->is_separator() );
			
			// Rotate at p as long as p is a separator
			auto p_sep_type = p->get_sep_type();
			while( p_sep_type != NOSEP ) {
				if( p_sep_type == DSEP ) {
					p_sep_type = p->rotate_dsep();
				}
				else { // p_sep_type == ISEP
					assert( p_sep_type == ISEP );
					p_sep_type = p->rotate_isep();
				}
				assert( p_sep_type == p->get_sep_type() );
			}
			assert( !p->is_separator() );
			
			// Now both v and p are NOSEP
			v->rotate_nosep();
		}
	}
#else
#error "Invalid variant specified"
#endif
}

struct MTRAccessImpl {
	static void access( stt::Node* v ) {
		mtr_stt::access( v );
	}
};

using MTRSTF = stt::STF<MTRAccessImpl>;

int main( int argc, const char** argv ) {
	main_connectivity<MTRSTF>( argc, argv );
}
