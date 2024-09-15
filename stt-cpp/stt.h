#include <cassert>
#include <unordered_map>


#ifdef COUNT_ROTATIONS
static size_t num_rotations = 0;
#endif

namespace stt {
	enum NodeSepType {
		NOSEP, DSEP, ISEP
	};
	
	struct Node {
		Node* parent;
		Node* dsep_child;
		Node* isep_child;
	
		void attach( Node* p ) {
			assert( this->parent == nullptr );
			this->parent = p;
		}
	
		void detach() {
			assert( this->parent != nullptr && !this->is_separator_hint( this->parent ) );
			this->parent = nullptr;
		}
		
		[[nodiscard]] NodeSepType get_sep_type() const {
			Node* p = this->parent;
			if( p ) {
				if( p->dsep_child == this ) { return DSEP; }
				else if( p->isep_child == this ) { return ISEP; }
			}
			return NOSEP;
		}
		
		[[nodiscard]] NodeSepType get_sep_type_hint( Node* p ) const {
			if( p->dsep_child == this ) { return DSEP; }
			else if( p->isep_child == this ) { return ISEP; }
			else { return NOSEP; }
		}

#ifdef ROT_IMPROVED
		inline bool rotate() { // Returns whether this is a separator after the rotation
#ifdef COUNT_ROTATIONS
			num_rotations++;
#endif
			assert( this->parent != nullptr );
			assert( this->is_separator() || !this->parent->is_separator() );
			
			Node* v = this;
			Node* p = v->parent;
			Node* g = p->parent;
			Node* c = v->dsep_child;
			
			// Change parents
			v->parent = g;
			p->parent = v;
			
			// Changes related to c
			if( c ) {
				c->parent = p;
				std::swap( c->dsep_child, c->isep_child );
			}
			
			bool p_was_sep = false;
			// Change separator information for children of v and g
			if( g ) { // p was not root
				Node* old_p_dsep_child = p->dsep_child;
				
				// Change isep_child of p (stays null if p is the root)
				if( old_p_dsep_child && old_p_dsep_child != v ) {
					p->isep_child = old_p_dsep_child;
				}
				else if( p->isep_child == v ) {
					p->isep_child = nullptr;
				}
				
				if( p == g->dsep_child ) {
					p_was_sep = true;
					g->dsep_child = v;
				}
				else if( p == g->isep_child ) {
					p_was_sep = true;
					g->isep_child = v;
				}
				
				if( old_p_dsep_child != v ) {
					// p separates v and g
					v->dsep_child = p;
				}
				else {
					// v separates p and g
					v->dsep_child = v->isep_child;
					if( p_was_sep ) {
						v->isep_child = p;
					}
					else {
						v->isep_child = nullptr;
					}
				}
			}
			else { // p was root
				v->dsep_child = nullptr;
			}
			
			// Change dsep child of p
			p->dsep_child = c;
			
			return p_was_sep;
		}
		
#elif defined( ROT_NST )
		inline NodeSepType rotate() { // Returns separator type after the rotation
#ifdef COUNT_ROTATIONS
			num_rotations++;
#endif
			assert( this->parent != nullptr );
			assert( this->is_separator() || !this->parent->is_separator() );
			
			Node* v = this;
			Node* p = v->parent;
			Node* g = p->parent;
			Node* c = v->dsep_child;
			
			// Change parents
			v->parent = g;
			p->parent = v;
			
			// Changes related to c
			if( c ) {
				c->parent = p;
				std::swap( c->dsep_child, c->isep_child );
			}
			
			NodeSepType p_type = NOSEP;
			// Change separator information for children of v and g
			if( g ) { // p was not root
				Node* old_p_dsep_child = p->dsep_child;
				
				// Change isep_child of p (stays null if p is the root)
				if( old_p_dsep_child && old_p_dsep_child != v ) {
					p->isep_child = old_p_dsep_child;
				}
				else if( p->isep_child == v ) {
					p->isep_child = nullptr;
				}
				
				if( p == g->dsep_child ) {
					p_type = DSEP;
					g->dsep_child = v;
				}
				else if( p == g->isep_child ) {
					p_type = ISEP;
					g->isep_child = v;
				}
				
				if( old_p_dsep_child != v ) {
					// p separates v and g
					v->dsep_child = p;
				}
				else {
					// v separates p and g
					v->dsep_child = v->isep_child;
					if( p_type != NOSEP ) {
						v->isep_child = p;
					}
					else {
						v->isep_child = nullptr;
					}
				}
			}
			else { // p was root
				v->dsep_child = nullptr;
			}
			
			// Change dsep child of p
			p->dsep_child = c;
			
			return p_type;
		}
		
		inline NodeSepType rotate_dsep() { // Returns separator type after the rotation
#ifdef COUNT_ROTATIONS
			num_rotations++;
#endif
			assert( this->parent != nullptr );
			assert( this->parent->dsep_child == this );
			
			Node* v = this;
			Node* p = v->parent;
			Node* g = p->parent;
			Node* c = v->dsep_child;
			
			assert( g ); // this is dsep, so p is not the root.
			
			// Change parents
			v->parent = g;
			p->parent = v;
			
			// Changes related to c
			if( c ) {
				c->parent = p;
				std::swap( c->dsep_child, c->isep_child );
			}
			
			NodeSepType p_type = NOSEP;
			// Change separator information for children of v and g
			if( p == g->dsep_child ) {
				p_type = DSEP;
				g->dsep_child = v;
			}
			else if( p == g->isep_child ) {
				p_type = ISEP;
				g->isep_child = v;
			}
			
			// v separates p and g
			v->dsep_child = v->isep_child;
			if( p_type != NOSEP ) {
				v->isep_child = p;
			}
			else {
				v->isep_child = nullptr;
			}
			
			// Change dsep child of p
			p->dsep_child = c;
			
			return p_type;
		}
		
		inline NodeSepType rotate_isep() { // Returns separator type after the rotation
#ifdef COUNT_ROTATIONS
			num_rotations++;
#endif
			assert( this->parent != nullptr );
			assert( this->parent->isep_child == this );
			
			Node* v = this;
			Node* p = v->parent;
			Node* g = p->parent;
			Node* c = v->dsep_child;
			
			assert( g ); // this is isep, so p is not the root.
			
			// Change parents
			v->parent = g;
			p->parent = v;
			
			// Changes related to c
			if( c ) {
				c->parent = p;
				std::swap( c->dsep_child, c->isep_child );
			}
			
			// Change separator information for children of v and g
			NodeSepType p_type = NOSEP;
			Node* old_p_dsep_child = p->dsep_child;
			
			// Change isep_child of p (stays null if p is the root)
			p->isep_child = old_p_dsep_child;
			
			if( p == g->dsep_child ) {
				p_type = DSEP;
				g->dsep_child = v;
			}
			else if( p == g->isep_child ) {
				p_type = ISEP;
				g->isep_child = v;
			}
			
			// We know that p separates v and g
			v->dsep_child = p;
			
			// Change dsep child of p
			p->dsep_child = c;
			
			return p_type;
		}
		
		inline NodeSepType rotate_nosep() { // Returns separator type after the rotation
#ifdef COUNT_ROTATIONS
			num_rotations++;
#endif
			assert( this->parent != nullptr );
			assert( !this->is_separator() && !this->parent->is_separator() );
			assert( this->isep_child == nullptr );
			
			Node* v = this;
			Node* p = v->parent;
			Node* g = p->parent;
			Node* c = v->dsep_child;
			
			// Change parents
			v->parent = g;
			p->parent = v;
			
			// Changes related to c
			if( c ) {
				c->parent = p;
				std::swap( c->dsep_child, c->isep_child );
			}
			
			// Change separator information for children of v and g
			if( g ) { // p was not root
				Node* old_p_dsep_child = p->dsep_child;
				
				// Change isep_child of p (stays null if p is the root)
				if( old_p_dsep_child ) {
					p->isep_child = old_p_dsep_child;
				}
				
				// p cannot be a separator child of g (otherwise rotation wouldn't be valid)
				
				// We know that p separates v and g
				v->dsep_child = p;
			}
			else { // p was root
				v->dsep_child = nullptr;
			}
			
			// Change dsep child of p
			p->dsep_child = c;
			
			return NOSEP;
		}
		
		inline NodeSepType rotate_type_hint( const NodeSepType type ) {
			if( type == DSEP ) {
				return rotate_dsep();
			}
			else if( type == ISEP ) {
				return rotate_isep();
			}
			else {
				return rotate_nosep();
			}
		}
#else
		void rotate() {
#ifdef COUNT_ROTATIONS
			num_rotations++;
#endif
			assert( this->parent != nullptr );
			assert( this->is_separator() || !this->parent->is_separator() );
			
			Node* v = this;
			Node* p = v->parent;
			Node* g = p->parent;
			Node* c = v->dsep_child;
			
			// Change parents
			v->parent = g;
			p->parent = v;
			if( c ) { c->parent = p; }
			
			// Change separator information for children of gp
			bool p_was_sep = false;
			if( g ) {
				if( p == g->dsep_child ) {
					p_was_sep = true;
					g->dsep_child = v;
				}
				else if( p == g->isep_child ) {
					p_was_sep = true;
					g->isep_child = v;
				}
			}
			
			// Change separator information for children of p
			Node* old_p_dsep_child = p->dsep_child;
			p->dsep_child = c;
			if( old_p_dsep_child && old_p_dsep_child != v ) {
				p->isep_child = old_p_dsep_child;
			}
			else if( p->isep_child == v ) {
				p->isep_child = nullptr;
			}
			
			// Change separator information for children of v
			if( g ) { // p was not root
				if( old_p_dsep_child != v ) {
					// p separates v and g
					v->dsep_child = p;
				}
				else {
					// v separates p and g
					v->dsep_child = v->isep_child;
					if( p_was_sep ) {
						v->isep_child = p;
					}
					else {
						v->isep_child = nullptr;
					}
				}
			}
			else { // p was root
				v->dsep_child = nullptr;
			}
			
			// Change separator information for children of c (not affected by the rotation otherwise)
			if( c ) {
				std::swap( c->dsep_child, c->isep_child );
			}
		}
#endif
		
		bool is_separator_hint( Node* p ) {
			return p->dsep_child == this || p->isep_child == this;
		}
		
		bool is_separator() {
			return this->parent && this->is_separator_hint( this->parent );
		}
		
		Node* get_stt_root() {
			Node* v = this;
			while( v->parent ) {
				v = v->parent;
			}
			return v;
		}
	};
	
	
	
	// Splay-related stuff
	static inline void splay_step( Node* v ) {
		Node* p = v->parent;
		if( p->dsep_child == v ) {
			v->rotate();
		}
		else if( p->parent ) {
			p->rotate();
		}
		v->rotate();
	}
	
	static inline void splay_step_full( Node* v, Node* p ) {
		if( p->dsep_child == v ) {
			v->rotate();
		}
		else {
			p->rotate();
		}
		v->rotate();
	}
	
#ifdef ROT_NST
	static inline void splay_step_type_hint( Node* v, const NodeSepType v_type, Node* p, const NodeSepType p_type ) {
		if( v_type == DSEP ) {
			v->rotate_dsep();
			v->rotate_type_hint( p_type );
		}
		else {
			p->rotate_type_hint( p_type );
			v->rotate();
		}
	}
#endif
	
	static inline bool can_splay_step( Node* v ) {
		Node* p = v->parent;
		assert( p );
		Node* g = p->parent;
		return !g || !g->is_separator() || ( v->is_separator_hint( p ) && p->is_separator_hint( g ) );
	}
	
	
	// Forward declarations
	template<typename AccessImpl>
	class STF;
	template<typename AccessImpl>
	std::ostream& operator<<( std::ostream& os, STF<AccessImpl>& f );
	
	template<typename AccessImpl>
	class STF {
	public :
		explicit STF( size_t n ) : nodes( n ) {}
		
		inline Node* get_node( size_t idx ) { return &nodes[idx]; }
		
		[[nodiscard]] inline size_t num_nodes() const { return nodes.size(); }
		
		void link( size_t u_idx, size_t v_idx ) {
			Node* u = get_node( u_idx );
			Node* v = get_node( v_idx );
			AccessImpl::access( u );
			AccessImpl::access( v );
			u->attach( v );
		}
		
		void cut( size_t u_idx, size_t v_idx ) {
			Node* u = get_node( u_idx );
			Node* v = get_node( v_idx );
			AccessImpl::access( u );
			AccessImpl::access( v );
			u->detach();
		}
		
		bool is_connected( size_t u_idx, size_t v_idx ) {
			Node* u = get_node( u_idx );
			Node* v = get_node( v_idx );
			AccessImpl::access( u );
			AccessImpl::access( v );
			return u->get_stt_root() == v;
		}
		
		friend std::ostream& operator<< <>( std::ostream& os, stt::STF<AccessImpl>& f );
		
	private :
		std::vector<Node> nodes;
	};

	template<typename AccessImpl>
	void _write_tree( std::ostream& os, STF<AccessImpl>& f, size_t v_idx, const std::vector<std::vector<size_t>>& node_children, const std::string& indent = "" ) {
		if( indent.length() >= 1000 ) {
			std::cerr << "Refusing to write tree of depth >= 1000\n";
			exit( -1 );
		}
		auto v = f.get_node( v_idx );
		os << indent << v_idx;
		// os << "[" << v << "]";
		if( v->parent && v->parent->dsep_child == v ) { os << "d"; }
		else if( v->parent && v->parent->isep_child == v ) { os << "i"; }
		os << "\n";
		for( const size_t c_idx : node_children[v_idx] ) {
			_write_tree( os, f, c_idx, node_children, indent + "  " );
		}
	}

	template<typename AccessImpl>
	std::ostream& operator<<( std::ostream& os, STF<AccessImpl>& f ) {
		std::unordered_map<Node*, size_t> node_indices;
		for( size_t i = 0; i < f.num_nodes(); i++ ) {
			node_indices[f.get_node( i )] = i;
		}
		std::vector<std::vector<size_t>> node_children( f.num_nodes() );
		for( size_t i = 0; i < f.num_nodes(); i++ ) {
			Node* v = f.get_node( i );
			if( v->parent ) {
				node_children[node_indices[v->parent]].push_back( i );
			}
		}
		for( size_t i = 0; i < f.num_nodes(); i++ ) {
			if( f.get_node( i )->parent == nullptr ) {
				_write_tree( os, f, i, node_children );
			}
		}
		return os;
	}
}
