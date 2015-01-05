// template function, use of type parameter
template<class T> T f( T ) { T x; }

// relational expression in template argument-list
template<int i> class X { /* ... */ };
//X< 1>2 > x1; 			// syntax error
X<(1>2)> x2; 			// OK

template<class T> class Y { /* ... */ };
Y< X<1> > x3; 			// OK
#if __cplusplus > 199711L 	// C++11?
Y<X<(6>> 1)>> x4; 		// OK: Y< X< (6>>1) > >
#else
Y<X<6>> 1> > x4; 		// OK: Y< X< (6>>1) > >
#endif

template< int > class _Base_bitset {};
template<int _Nb>
class bitset {
    typedef _Base_bitset<((_Nb) < 3)> _Base;
};

template< typename T, int __w, bool = (__w < static_cast<int> (10))> struct S1 {};
template< typename T, int __w, bool = __w < static_cast<int> (10)> struct S2 {};

// template type used in parameter list
template<typename _Tp, _Tp __m, bool> struct _Mod;

// template base class (base specifier usage), check generated constructors
template<class T> _Task ts11 {
    T t;
    void main(){}
};
template<class T> _Task ts2 : public ts11<int> {
    void main() {}
};
_Task ts3 : public ts2<double> {
    void main() {}
  public:
    ts3();
};
_Task ts4 : public ts2<int> {
    void main() {}
};
ts3::ts3() {}


// "using" check
namespace foo {
    template<typename _Iterator>
    struct iterator_traits {
	typedef typename _Iterator::iterator_category iterator_category;
	typedef typename _Iterator::difference_type difference_type;
	typedef typename _Iterator::reference reference;
    };
    template<typename _Category, typename _Tp, typename _Distance>
    struct iterator {};
}

using foo::iterator_traits;
using foo::iterator;
template<typename _Iterator, typename _Container>
class __normal_iterator
    : public iterator<typename iterator_traits<_Iterator>::iterator_category,
		      typename iterator_traits<_Iterator>::difference_type,
		      typename iterator_traits<_Iterator>::reference>
{
  public:
    typedef typename iterator_traits<_Iterator>::difference_type x;
    typedef typename iterator_traits<_Iterator>::reference y;
};


template< class T > struct A {
	template< class X > void g( T, X );
};

template<> template< class X > void A< int >::g( int, X );


// Local Variables: //
// compile-command: "../../bin/u++ testTemplate.cc" //
// End: //
