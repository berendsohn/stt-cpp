#include <cassert>

#include "parse_input.h"

#ifndef VARIANT
#define VARIANT 8
#endif

/**
 * Local Two-Pass SplayTT
 * Two basic variants:
 * LTP-A: As in original rust implementation/ALENEX paper; almost the same as Two-Pass SplayTT
 * LTP-B: Variant with less lookahead, as in thesis (replace splay_step at grandparent with single rotation when possible)
 * 
 * Variants:
 * 0: Shortest possible LTB-A implementation
 * 1/2: Improved LTB-A/B impl from rust lib
 * 3/4: LTB-A/B impl using functions with NodeSepType
 * 5/6: Variant of 3/4 that reuses computed NodeSepType between loop runs, if possible
 * 7/8: Variant of 3/4 that tries to reduce re-checking of separator types with a helper loop
 * 9: Variant of 8 that reuses computed NodeSepType in the helper loop
 */

#if VARIANT <= 2
#define ROT_IMPROVED
#else
#define ROT_NST
#endif

#include "stt.h"

namespace ltp_stt {
	using namespace stt;

/// Access implementation

#if VARIANT == 0
	// Very naive LTP impl
	static inline void access( Node* v ) {
		while( v->parent ) {
			if( can_splay_step( v ) ) {
				splay_step( v );
			}
			else if( v->parent->is_separator() ) {
				splay_step( v->parent );
			}
			else {
				auto* g = v->parent->parent;
				if( can_splay_step( g ) ) {
					splay_step( g );
				}
				else {
					g->rotate();
				}
			}
		}
	}
#elif VARIANT == 1 || VARIANT == 2
	// Naive impl from old Rust lib
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
					else if( p_sep ) {
						splay_step_full( p, g );
					}
					else {
						Node* ggg = gg->parent; // Must exist, since g_sep
						assert( ggg );
#if VARIANT == 1
						if( gg->is_separator_hint( ggg ) || ! ggg->is_separator() ) {
#else
						if( gg->is_separator_hint( ggg ) ) {
#endif
							splay_step_full( g, gg );
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
#elif VARIANT == 3 || VARIANT == 4
	// Improved impl with NodeSepType
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
					else { // !p_sep and g_sep
						Node* ggg = gg->parent; // Must exist, since g_sep
						assert( ggg );
						NodeSepType gg_sep = gg->get_sep_type_hint( ggg );
#if VARIANT == 3
						if( gg_sep != NOSEP || ggg->get_sep_type() == NOSEP ) { // Can splay at g
#else
						if( gg_sep != NOSEP ) { // Can splay at g
#endif
							splay_step_type_hint( g, g_sep, gg, gg_sep );
						}
						else {
							g->rotate_type_hint( g_sep );
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
#elif VARIANT == 5 || VARIANT == 6
	// Improved impl with remembered NodeSepType
	static inline void access( Node* v ) {
		if( Node* p = v->parent ) {
			if( Node* g = p->parent ) {
				NodeSepType v_sep = v->get_sep_type_hint( p );
				NodeSepType p_sep = p->get_sep_type_hint( g );
				
				while( true ) {
					// Try splaying at v without information about g's NodeSepType.
					if( v_sep != NOSEP && p_sep != NOSEP ) {
						splay_step_type_hint( v, v_sep, p, p_sep );
						// Recompute NSTs or finish
						if( (p = v->parent) ) {
							if( (g = p->parent) ) {
								v_sep = v->get_sep_type_hint( p );
								p_sep = p->get_sep_type_hint( g );
							}
							else {
								v->rotate();
								return;
							}
						}
						else { return; }
					}
					// Either v or p is not a separator
					else if( Node* gg = g->parent ) {
						NodeSepType g_sep = g->get_sep_type_hint( gg );
						if( g_sep == NOSEP ) { // Can splay at v
							splay_step_type_hint( v, v_sep, p, p_sep );
							// Recompute NSTs or finish
							if( (p = v->parent) ) {
								if( (g = p->parent) ) {
									v_sep = g_sep;
									p_sep = p->get_sep_type_hint( g );
								}
								else {
									v->rotate();
									return;
								}
							}
							else { return; }
						}
						else if( p_sep != NOSEP ) { // g_sep and p_sep => can splay at p
							splay_step_type_hint( p, p_sep, g, g_sep );
							// Recompute NSTs or finish
							if( (g = p->parent) ) {
								p_sep = p->get_sep_type_hint( g );
							}
							else {
								v->rotate();
								return;
							}
						}
						else { // !p_sep and g_sep
							Node* ggg = gg->parent; // Must exist, since g_sep
							assert( ggg );
							NodeSepType gg_sep = gg->get_sep_type_hint( ggg );
#if VARIANT == 5
							if( gg_sep != NOSEP || ggg->get_sep_type() == NOSEP ) { // Can splay at g
#else
								if( gg_sep != NOSEP ) { // Can splay at g
#endif
								splay_step_type_hint( g, g_sep, gg, gg_sep );
								// v and p stay the same, since !p_sep
							}
							else {
								g->rotate_type_hint( g_sep );
								// v and p stay the same, since !p_sep
							}
						}
					}
					else { // g is root, splaying at v must be allowed
						splay_step_type_hint( v, v_sep, p, p_sep );
						return;
					}
				}
			}
			else { // p is root
				v->rotate();
			}
		}
	}
#elif VARIANT == 7 || VARIANT == 8
	// Improved impl with NodeSepType and less re-trying
	inline void move_branching_node( Node* v ) {
		// Rotate branching node up until it's not a branching node anymore
		while( Node* p = v->parent ) {
			auto v_sep = v->get_sep_type_hint( p );
			if( v_sep == NOSEP ) {
				return;
			}
			Node* g = p->parent; // Must exist, since v is separator
			auto p_sep = p->get_sep_type_hint( g );
			if( p_sep != NOSEP ) { // p is separator, can splay
				splay_step_type_hint( v, v_sep, p, p_sep );
			}
#if VARIANT == 7
			else if( g->get_sep_type() == NOSEP ) { // g is no separator, can splay and stop afterwards
				splay_step_type_hint( v, v_sep, p, p_sep );
				return; // v is no separator anymore
			}
#endif
			else {
				v->rotate_type_hint( v_sep );
				return; // v is no separator anymore
			}
		}
	}
	
	inline void access( Node* v ) {
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
					else { // !p_sep and g_sep
						move_branching_node( g );
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
#elif VARIANT == 9
	// Variant of LTB-B that remembers NodeSepType in move_branching_node
	inline void move_branching_node( Node* v, NodeSepType v_sep ) {
		// Rotate branching node up until it's not a branching node anymore
		Node* p = v->parent;
		while( v_sep != NOSEP ) {
			Node* g = p->parent; // Must exist, since v is separator
			auto p_sep = p->get_sep_type_hint( g );
			if( p_sep != NOSEP ) { // p is separator, can splay
				splay_step_type_hint( v, v_sep, p, p_sep );
				p = v->parent;
				if( p ) {
					v_sep = v->get_sep_type_hint( p );
				}
				else { return; }
			}
			else {
				v->rotate_type_hint( v_sep );
				return; // v is no separator anymore
			}
		}
	}
	
	inline void access( Node* v ) {
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
					else { // !p_sep and g_sep
						move_branching_node( g, g_sep );
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

struct LTPAccessImpl {
	static void access( stt::Node* v ) {
		ltp_stt::access( v );
	}
};

using LTPSTF = stt::STF<LTPAccessImpl>;

int main( int argc, const char** argv ) {
	main_connectivity<LTPSTF>( argc, argv );
}
