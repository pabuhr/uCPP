struct E {};
struct W {};

int global;

static void HandlerRtn0( const E &e ) { global = 5; }
static void HandlerRtn1( W &e ) { global = 5; }
static void HandlerRtn2() { global = 5; }

class Obj {} obj;

const volatile int jack( char parm ) {
    int local1;
    double local2;

    try <const E, HandlerRtn0> <obj.W, HandlerRtn1> <..., HandlerRtn2> { // old style
	global = 5;
    } catch( E ) {
	global = 5;
    }

    try {
    } _CatchResume( const E e ) {			// new style
	global = 5;
#if defined( ERRORS )
#include "testCatchResume.h"
	global = Y;
#include "testCatchResume.h"
	global = Z;
	global = k;
#endif // ERRORS
    } _CatchResume( obj.W &e ) {
	global = 5;
    } _CatchResume( obj.W &e ) () {
	global = 5;
    } _CatchResume( E ) ( char parm, int local1, double local2 ) {
	global = 5;
	parm = 1;
	local1 = 3; local2 = 3.0;
#if defined( ERRORS )
	global = x;
#endif // ERRORS
    } _CatchResume( obj.W &e ) ( char parm, int local1, double local2 ) {
	global = 5;
	parm = 1;
	local1 = 3; local2 = 3.0;
    } _CatchResume( ... ) {
	global = 5;
    } catch( E ) {
	global = 5;
    }

    // check reference parameter
    int x;
    int (&(y)) = x;
    int *w;
    int *(&(z)) = w;
    try {
    } _CatchResume( E ) ( int (&(y)), int *(&(z)) ) {
	y = 3;
	z = NULL;
    }
    return 3;
}

// unnamed routine-pointer template parameter not supported
// template <double (*)()> void rtn() {};

template< typename T, int w, T k, template<typename H> class T2 > const int fred( T parm ) {
    int local1;
    double local2;

    try {
	global = 5;
	T t = 0; t += k; T2<int> t2; t2.c += w;
    } _CatchResume( E & ) {
	global = 5;
	T t = 0; t += k; T2<int> t2; t2.c += w;
    } _CatchResume( obj.W &e ) {
	global = 5;
	T t = 0; t += k; T2<int> t2; t2.c += w;
    } _CatchResume( E ) ( int parm, int local1, double local2 ) {
	global = 5;
	T t = 0; t += k; T2<int> t2; t2.c += w;
	parm = 1;
	local1 = 3; local2 = 3.0;
    } _CatchResume( ... ) {
	global = 5;
	T t = 0; t += k; T2<int> t2; t2.c += w;
    } catch( E ) {
    }
    return 3;
}

_Task B {
    struct PPP { int i; };

    int classGlobal1;
    int classGlobal2;
  public:
    void main();
    const volatile int tom( int parm );

    template<typename T, int w, T k, template<typename H> class T2> volatile const int mary( T parm ) {
	int local1;
	double local2;

	try {
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	} _CatchResume( E ) ( int classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	} _CatchResume( obj.W &e ) ( int classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	} _CatchResume( E ) ( int classGlobal1, int classGlobal2, int parm, int local1, double local2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	    parm = 1;
	    local1 = 3; local2 = 3.0;
	} _CatchResume( ... ) ( int classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	} catch( E ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; T2<int> t2;
	    t += k; t2.c += w;
	}
	return 3;
    }
};

void B::main() {
    int local1;
    double local2;

    try {
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    } _CatchResume( E ) {
	PPP ppp; ppp.i += 5;
	global = 5;
    } _CatchResume( obj.W &e ) {
	PPP ppp; ppp.i += 5;
	global = 5;
    } _CatchResume( E ) ( int classGlobal1, int classGlobal2, int local1, double local2 ) {
	PPP ppp; ppp.i += 5;
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
	local1 = 3; local2 = 3.0;
    } _CatchResume( ... ) ( int classGlobal1, int classGlobal2 ) {
	PPP ppp; ppp.i += 5;
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    } catch( E ) {
	PPP ppp; ppp.i += 5;
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    }
}

const volatile int B::tom( int parm ) {
    int local1;
    double local2;

    try {
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    } _CatchResume( E ) {
	PPP ppp; ppp.i += 5;
	global = 5;
    } _CatchResume( obj.W &e ) ( int classGlobal1, int classGlobal2 ) {
	PPP ppp; ppp.i += 5;
		global = 5;
		classGlobal1 = 2; classGlobal2 = 3;
    } _CatchResume( E ) ( int classGlobal1, int classGlobal2, int parm, int local1, double local2 ) {
	PPP ppp; ppp.i += 5;
	global = 5;
   	classGlobal1 = 2; classGlobal2 = 3;
   	parm = 1;
   	local1 = 3; local2 = 3.0;
    } _CatchResume( ... ) ( int classGlobal1, int classGlobal2 ) {
	PPP ppp; ppp.i += 5;
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    } catch( E ) {
	PPP ppp; ppp.i += 5;
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    }
    return 3;
}


template< typename WWW > class C;

template< typename WWW > class C {
    struct PPP { int i; };
    struct XXX;
    template<typename T, int w, T k, template<typename H> class T2> struct YYY;

    WWW classGlobal1;
    int classGlobal2;
  public:
    template<typename T, int w, T k, template<typename H> class T2> const volatile int tom( int parm );

    template<typename T, int w, T k, template<typename H> class T2> volatile const int mary( T parm ) {
	int local1;
	double local2;

	try {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	} _CatchResume( E ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	} _CatchResume( obj.W &e ) ( WWW classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	} _CatchResume( E ) ( WWW classGlobal1, int classGlobal2, int parm, int local1, double local2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	    parm = 1;
	    local1 = 3; local2 = 3.0;
	} _CatchResume( ... ) ( WWW classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	} catch( E ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = w; classGlobal2 = k;
	    T t = 0; t += k; T2<int> t2; t2.c += w;
	}
	return 3;
    }
};

template< typename WWW > class C;


template< typename WWW > struct C<WWW>::XXX {
    void xxx() {
	int local1;
	double local2;

	try {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	} _CatchResume( E ) ( WWW classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	} _CatchResume( obj.W &e ) ( WWW classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	} _CatchResume( E ) ( WWW classGlobal1, int classGlobal2, int local1, double local2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	    local1 = 3; local2 = 3.0;
	} _CatchResume( ... ) ( WWW classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	} catch( E ) {
	    PPP ppp; ppp.i += 5;
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	}
    }
    void mem() {}
};

template< typename WWW > template<typename T, int w, T k, template<typename H> class T2> struct C<WWW>::YYY {
    void yyy() {
	int local1;
	double local2;

	try {
	    PPP ppp; ppp.i += 5;
	    XXX xxx; xxx.mem();
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	} _CatchResume( E ) ( WWW classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    XXX xxx; xxx.mem();
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	} _CatchResume( obj.W &e ) ( WWW classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    XXX xxx; xxx.mem();
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	} _CatchResume( E ) ( WWW classGlobal1, int classGlobal2, int local1, double local2 ) {
	    PPP ppp; ppp.i += 5;
	    XXX xxx; xxx.mem();
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	    local1 = 3; local2 = 3.0;
	} _CatchResume( ... ) ( WWW classGlobal1, int classGlobal2 ) {
	    PPP ppp; ppp.i += 5;
	    XXX xxx; xxx.mem();
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	} catch( E ) {
	    PPP ppp; ppp.i += 5;
	    XXX xxx; xxx.mem();
	    global = 5;
	    classGlobal1 = 2; classGlobal2 = 3;
	}
    }
    void mem() {}
};


template<typename T> struct CC {
    T c;
};


template< typename WWW > template<typename T, int w, T k, template<typename H> class T2> const volatile int C<WWW>::tom( int parm ) {
    int local1;
    double local2;

    try {
	PPP ppp; ppp.i += 5;
	XXX xxx; xxx.mem();
	YYY<int, 3, 3, CC> yyy; yyy.mem();
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    } _CatchResume( E ) ( WWW classGlobal1, int classGlobal2 ) {
	PPP ppp; ppp.i += 5;
	XXX xxx; xxx.mem();
	YYY<int, 3, 3, CC> yyy; yyy.mem();
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    } _CatchResume( obj.W &e ) ( WWW classGlobal1, int classGlobal2 ) {
	PPP ppp; ppp.i += 5;
	XXX xxx; xxx.mem();
	YYY<int, 3, 3, CC> yyy; yyy.mem();
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    } _CatchResume( E ) ( WWW classGlobal1, int classGlobal2, int parm, int local1, double local2 ) {
	PPP ppp; ppp.i += 5;
	XXX xxx; xxx.mem();
	YYY<int, 3, 3, CC> yyy; yyy.mem();
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
	parm = 1;
	local1 = 3; local2 = 3.0;
    } _CatchResume( ... ) ( WWW classGlobal1, int classGlobal2 ) {
	PPP ppp; ppp.i += 5;
	XXX xxx; xxx.mem();
	YYY<int, 3, 3, CC> yyy; yyy.mem();
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    } catch( E ) {
	PPP ppp; ppp.i += 5;
	XXX xxx; xxx.mem();
	YYY<int, 3, 3, CC> yyy; yyy.mem();
	global = 5;
	classGlobal1 = 2; classGlobal2 = 3;
    }
    return 3;
}

void start() {
    jack(3);
    fred<int, 3, 3, CC>(3);
    B b;
    b.tom( 3 );
    b.mary<int, 3, 3, CC>( 3 );
    C<int> c;
    c.tom<int, 3, 3, CC>( 3 );
    c.mary<int, 3, 3, CC>( 3 );
#if defined( ERRORS )
    i = 3;
#endif // ERRORS
}

// Local Variables: //
// compile-command: "../../bin/u++ testCatchResume.cc -U++" //
// End: //
