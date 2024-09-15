#include <cassert>

#include "parse_input.h"

#ifndef VARIANT
#define VARIANT 3
#endif

/** Variants
 * 0: Naive greedy from paper
 * 1: Semi-naive greedy from rust impl
 * 2: Slightly improved 11
 * 3: Improved 12 with NST
 */

#if VARIANT <= 2
#define ROT_IMPROVED
#elif VARIANT >= 3
#define ROT_NST
#endif

#include "stt.h"

namespace greedy_stt {
	using namespace stt;

/// Access implementation

#if VARIANT == 0
	// Very naive Greedy impl from paper
	static inline void access( Node* v ) {
		while( v->parent ) {
			if( can_splay_step( v ) ) {
				splay_step( v );
			}
			else if( can_splay_step( v->parent ) ) {
				splay_step( v->parent );
			}
			else {
				assert( can_splay_step( v->parent->parent ) );
				splay_step( v->parent->parent );
			}
		}
	}
#elif VARIANT == 1
	// Naive Greedy impl from Rust lib
	static inline void access( Node* v ) {
		while( Node* p = v->parent ) {
			if( Node* g = p->parent ) {
				if( Node* gg = g->parent ) {
					bool v_sep = v->is_separator_hint( p );
					bool p_sep = p->is_separator_hint( g );
					bool g_sep = g->is_separator_hint( gg );
					if( ( v_sep && p_sep ) || !g_sep ) { // Can splay at v
						splay_step_full( v, p );
					}
					else { // Cannot splay at v
						if( Node* ggg = gg->parent ) {
							bool gg_sep = gg->is_separator_hint( ggg );
							if( ( p_sep && g_sep ) || !gg_sep ) { // Can splay at p
								splay_step_full( p, g );
							}
							else { // Cannot splay at p, so splaying at g must be allowed
								splay_step_full( g, gg );
							}
						}
						else { // ggg is root, splaying at p must be allowed
							splay_step_full( p, g );
						}
					}
				}
				else { // g is root, splaying at v must be allowed
					splay_step_full( v, p );
				}
			}
			else { // p is root
				v->rotate();
			}
		}
	}
#elif VARIANT == 2
	// Improved Greedy impl from Rust lib
	static inline void access( Node* v ) {
		while( Node* p = v->parent ) {
			if( Node* g = p->parent ) {
				bool v_sep = v->is_separator_hint( p );
				bool p_sep = p->is_separator_hint( g );
				if( v_sep && p_sep ) { // Can splay at v
					splay_step_full( v, p );
				}
				else if( Node* gg = g->parent ) { // !v_sep or !p_sep
					bool g_sep = g->is_separator_hint( gg );
					if( !g_sep ) { // Can splay at v
						splay_step_full( v, p );
					}
					else if( p_sep ) { // g_sep and p_sep => can splay at p
						splay_step_full( p, g );
					}
					else { // Cannot splay at v and g_sep and !p_sep
						if( Node* ggg = gg->parent ) {
							bool gg_sep = gg->is_separator_hint( ggg );
							if( !gg_sep ) { // Can splay at p
								splay_step_full( p, g );
							}
							else { // Cannot splay at p, so splaying at g must be allowed
								splay_step_full( g, gg );
							}
						}
						else { // ggg is root, splaying at p must be allowed
							splay_step_full( p, g );
						}
					}
				}
				else { // g is root, splaying at v must be allowed
					splay_step_full( v, p );
				}
			}
			else { // p is root
				v->rotate();
			}
		}
	}
#elif VARIANT == 3
	// Improved Greedy impl from Rust lib, using NodeSepType
	static inline void access( Node* v ) {
		while( Node* p = v->parent ) {
			if( Node* g = p->parent ) {
				NodeSepType v_sep = v->get_sep_type_hint( p );
				NodeSepType p_sep = p->get_sep_type_hint( g );
				
				// Try splaying at v without information about g's NodeSepType.
				if( v_sep != NOSEP && p_sep != NOSEP ) {
					splay_step_type_hint( v, v_sep, p, p_sep );
				}
				// Either v or p is not a separator
				else if( Node* gg = g->parent ) {
					NodeSepType g_sep = g->get_sep_type_hint( gg );
					if( g_sep == NOSEP ) { // Can splay at v
						splay_step_type_hint( v, v_sep, p, p_sep );
					}
					else if( p_sep != NOSEP ) { // g_sep and p_sep => can splay at p
						splay_step_type_hint( p, p_sep, g, g_sep );
					}
					else { // Cannot splay at v and g_sep and !p_sep
						Node* ggg = gg->parent; // Must exist, since g_sep
						assert( gg->parent );
						NodeSepType gg_sep = gg->get_sep_type_hint( ggg );
						if( gg_sep == NOSEP ) { // Can splay at p
							splay_step_type_hint( p, p_sep, g, g_sep );
						}
						else { // Cannot splay at p, so splaying at g must be allowed
							splay_step_type_hint( g, g_sep, gg, gg_sep );
						}
					}
				}
				else { // g is root, splaying at v must be allowed
					splay_step_type_hint( v, v_sep, p, p_sep );
				}
			}
			else { // p is root
				v->rotate();
			}
		}
	}
#else
#error "Invalid variant specified"
#endif
}

struct GreedyAccessImpl {
	static void access( stt::Node* v ) {
		greedy_stt::access( v );
	}
};

using GreedySTF = stt::STF<GreedyAccessImpl>;

void test() {
	std::cout << "Starting test" << std::endl;
	GreedySTF f( 10 );
	f.link( 1, 2 );
	f.link( 2, 3 );
	f.link( 3, 4 );
	f.link( 0, 2 );
//	std::cout << f << std::endl;
//	f.get_node( 0 )->access();
//	std::cout << f << std::endl;
	assert( f.is_connected( 0, 2 ) );
	assert( f.is_connected( 0, 1 ) );
	assert( f.is_connected( 0, 3 ) );
	assert( f.is_connected( 0, 4 ) );
	assert( !f.is_connected( 0, 5 ) );
//	std::cout << f << std::endl;
	std::cout << "----- CUT(1,2) -----\n";
	f.cut( 1, 2 );
	std::cout << f << std::endl;
	assert( f.is_connected( 0, 2 ) );
	assert( !f.is_connected( 0, 1 ) );
	assert( f.is_connected( 0, 3 ) );
	assert( f.is_connected( 0, 4 ) );
	assert( !f.is_connected( 0, 5 ) );
	std::cout << "Done.\n";
}

int main( int argc, const char** argv ) {
	main_connectivity<GreedySTF>( argc, argv );
}
