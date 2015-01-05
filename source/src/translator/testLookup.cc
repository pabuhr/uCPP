namespace fred {
    typedef int X;
}
namespace tom {
    typedef int Y;
    typedef int X;
}
namespace mary {
    using namespace fred;
    using tom::Y;
    Y y1;
    X x1;
}
namespace mary {
    Y y2;
    X x2;
}
void jake() {
    typedef int Z;
    typedef int X;
    {
	using namespace mary;
	X x = 0;
	Y y = 0;
	Z z = 0;
	x += 1; y += 1; z += 1;
	{
	    typedef int X;
	    Z z = 0;
	    X x = 0;
	    x += 1; z += 1;
	}
    }
    {
	Z z = 0;
	X x = 0;
	x += 1; z += 1;
    }
//    y = 1;
}
class B {
    typedef int X;
};

class C : public B {
    typedef int X;
    template< class T > T bar(X);
};

template< class T >
T C::bar(X) {
    {
//	typedef int X;
	{
	    X x;
	}
    }
}


// qualified return types

typedef unsigned int Size_t;

namespace std {
    using ::Size_t;
}

namespace __gnu_cXX {
    template<typename _CharT>
    struct char_traits {
	static std::Size_t length(const char* __s);
    };

    template<typename _CharT>
    std::Size_t char_traits<_CharT>::length(const char* __p) { return 1; }
}

struct F {
    void f();
};

void F::f() {}


// shadowing of names with inheritance

namespace std {
    template <class _Tp, class _Allocator>
    struct _Alloc_traits {
	static const bool _S_instanceless = false;
	typedef typename _Allocator::template rebind<_Tp>::other Allocator_type;
    };
}

template<typename _Tp>
class new_Allocator {
    struct rebind;
};

namespace std {
    template<typename _Tp>
    class Allocator: public new_Allocator<_Tp> {
      public:
	template<typename _Tp1>
        struct rebind { typedef Allocator<_Tp1> other; };
    };
}


namespace stk {
    class group;

    int sync(group*);
}

int stk::sync(group *g) { return 0; }

struct jane { enum red { R }; int i; };

namespace xxx {
    typedef int X;
    template<class T> T jane( X g ) {}
}

namespace www {
    class fred {
      public:
	fred( www::fred & );
    };
};

class uEHM2 {
    class uResumptionHandlers;
    class uHandlerBase;
}; // uEHM2

class uEHM2::uResumptionHandlers {
  public:
    uResumptionHandlers( uHandlerBase *const table[], const unsigned int size );
    ~uResumptionHandlers();
}; // uEHM2::uResumptionHandlers

uEHM2::uResumptionHandlers::uResumptionHandlers( uHandlerBase *const table[], const unsigned int size ) {
} // uEHM2::uResumptionHandlers::uResumptionHandlers


// typedef

typedef int tdname1;
tdname1 typedef tdname2;

typedef __builtin_va_list __gnuc_va_list;// fake builtin types


// pointers to member routines

template <class _Ret, class _Tp>
class const_mem_fun_t {
    _Ret (_Tp::*_M_f)() const;
};


class D {};

template <class _Tp>
class qqq1 {
    D (_Tp::*_M_f)() const;
};


template <class _Tp>
class qqq2 {
    static D (_Tp::*_M_f)() const;
};

template <class _Tp>
D (_Tp::*qqq2<_Tp>::_M_f)() const = 0;


// Local Variables: //
// compile-command: "../../bin/u++ testLookup.cc" //
// End: //
