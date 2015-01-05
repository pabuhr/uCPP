// C++11 additions : http://en.wikipedia.org/wiki/C++11
// C++11 grammar : http://www.externsoft.ch/media/swf/cpp11-iso.html


// constexpr
//
// C++11 introduced the keyword constexpr, which allows the user to guarantee that a function or object constructor is a
// compile-time constant.

constexpr int get_five() { return 5; }
int some_value[get_five() + 7];

constexpr double earth_gravitational_acceleration = 9.8;
constexpr double moon_gravitational_acceleration = earth_gravitational_acceleration / 6.0;

class conststr {
    int sz;
  public:
    template<int N> constexpr conststr() : sz( N - 1 ) {}
    constexpr int inclassConstexprFn() { return 0; }
};
 
constexpr int constFn() { return 1; }
template<int N> constexpr int constFnTemp() { return N; }


// extern template
//
// C++11 introduced extern template declarations, analogous to extern data declarations, which tells the compiler not to
// instantiate the template in this translation unit.

#include <vector>
extern template class std::vector<double>;


// initializer list
//
// C++03 inherited the initializer-list feature from C. A struct or array is given a list of arguments in braces, in the
//      order of the members' definitions in the struct.  These initializer-lists are recursive, so an array of structs
//      or struct containing other structs can use them.  However C++03 allows initializer-lists only on structs and
//      classes that conform to the Plain Old Data (POD) definition.
//
// C++11 extends initializer-lists, so they can be used for all classes including standard containers like std::vector.
//      C++11 binds the concept to a template, called std::initializer_list. This allows constructors and other
//      functions to take initializer-lists as parameters.

#include <initializer_list>
#include <string>

class SequenceClass {
  public:
    SequenceClass(std::initializer_list<int> list) {}
};

SequenceClass some_var = {1, 4, 5, 6};
void function_name( std::initializer_list<float> list ) {}
void dummy() {
	function_name( {1.0f, -3.45f, -0.4f} );
}

std::vector<std::string> vstr1 = { "xyzzy", "plugh", "abracadabra" };
std::vector<std::string> vstr2({ "xyzzy", "plugh", "abracadabra" });
std::vector<std::string> vstr3{ "xyzzy", "plugh", "abracadabra" };


// uniform initialization
//
// C++11 provides a syntax that allows for fully uniform type initialization that works on any object.

#ifdef __U_CPLUSPLUS
_Coroutine
#else
class
#endif
BasicStruct {
    int x;
    double y;
	void main() {}
  public:
	BasicStruct( int x, double y ) : x{x}, y{y} {}
};

#ifdef __U_CPLUSPLUS
_Coroutine
#else
class
#endif
AltStruct : public BasicStruct {
	void main() {}
    int x_;
    double y_;
  public:
    AltStruct(int x, double y) : BasicStruct{5,6}, x_{x}, y_{y} {}
};
BasicStruct var1{5, 3.2};
AltStruct var2{2, 4.3};

struct IdString {
    std::string name;
    int identifier;
};

IdString get_string() {
    return {"foo", 42}; //Note the lack of explicit type.
}


// In C++03 (and C), the type of a variable must be explicitly specified in order to use it.  C++11 allows this to be
// mitigated in two ways.

// auto
//
// C++11 allows type inference of a variable based on the result type of an expression.

int auto_x = 1;
auto auto_y = auto_x;
void auto_rtn() {
	std::vector<int> myvec;
	for ( auto itr = myvec.cbegin(); itr != myvec.cend(); ++itr );
}

// decltype
//
// C++11 allows the type of a previously declared variable to be used as a type (gcc typeof).

#include <iostream>

int decltype_x = 1;
decltype(decltype_x) decltype_y = decltype_x;   // decltype_y should be 1
void decltype_rtn() {
    const std::vector<int> v(1);
    auto a = v[0];        // a has type int
    decltype(v[1]) b = 1; // b has type const int&, the return type of std::vector<int>::operator[](size_type) const
    auto c = 0;           // c has type int
    auto d = c;           // d has type int
    decltype(c) e = 1;    // e has type int, the type of the entity named by c
    decltype((c)) f = c;  // f has type int&, because (c) is an lvalue
    decltype(0) g = 1;    // g has type int, because 0 is an rvalue
	std::cerr << a << b << c << d << e << f << g;
}


// range-based for loop
//
// C++11 extends the syntax of the for statement to allow for easy iteration over a range of elements.

void range() {
	int array[5] = { 2, 2, 2, 2, 2 };
	for ( int &x : array ) { x = 0; }
	for ( auto &x : array ) { x = 0; }
}


// lambda functions and expressions
//
// C++11 provides the ability to create anonymous functions, called lambda functions.

auto func1 = [](int i) { return i+4; };
auto func_noreturn_1 = [] { return 4; };	// can eliminate () if no parameter
auto func_noreturn_2 = []() mutable throw() { return 4; };	// can eliminate () if no parameter
auto func_noreturn_3 = [](void) { return 4; };	// can eliminate () if no parameter

auto func_explicit_return_type = []() -> int{ return 4; };	// explictly specifying return type, () has to be there
auto func_no_exception = [] () throw (){};	// it does not throw exception!

// call lambda functions in initialization
double pi = []{ return 3.14159; }();
bool is_even = [](int n){ return n % 2 == 0; }( 41 ); 

struct SomeStruct  {
    auto func_name(int x, int y) -> int;
};
auto SomeStruct::func_name(int x, int y) -> int {
    return x + y;
}

template<class Lhs, class Rhs>
auto adding_func( const Lhs &lhs, const Rhs &rhs ) -> decltype( lhs+rhs ) { return lhs + rhs; }
auto multiply( int x, int y ) -> volatile const int;

#include <algorithm>

/* Syntax
   [] = lambda introducer
   [] - Capture nothing.
   [=] - Capture everything by value.
   [&] - Capture everything by reference.
   [var] - Capture var by value; nothing else, in either mode, is captured.
   [&var] - Capture var by reference; nothing else, in either mode, is captured.
   {} = definition of lambda
   it can contain anything a function can have (including lambdas itself)
*/

void lamb() {
	std::vector<int> v;
	int evenCount = 0;

	std::for_each( v.begin(), v.end(), [&evenCount]( int n ) {
		if ( n % 2 == 0 ) {
			++evenCount;
		}
	});
}


// Object construction improvement
//
// In C++03, constructors of a class are not allowed to call other constructors of that class; each constructor must
//      construct all of its class members itself or call a common member function Constructors for base classes cannot
//      be directly exposed to derived classes; each derived class must implement constructors even if a base class
//      constructor would be appropriate.  Non-constant data members of classes cannot be initialized at the site of the
//      declaration of those members. They can be initialized only in a constructor.
//
// C++11 provides solutions to all of these problems.  C++11 allows constructors to call other peer constructors (known
//      as delegation).  This allows constructors to utilize another constructor's behavior with a minimum of added
//      code.

class BaseClass {
  public:
    BaseClass(int value);
};
class DerivedClass : public BaseClass {
  public:
    using BaseClass::BaseClass;
};
class SomeClass {
  public:
    SomeClass() {}
    explicit SomeClass(int new_value) : value(new_value) {}
  private:
    int value = 5;
};


// "final" override qualifier

struct Base {
	virtual void f() final;
    virtual void some_func(float);
};
struct Derived : Base {
    virtual void some_func(float) override;
};
struct Final final {
	virtual void f() final;
    virtual void some_func(float);
};


// alignof, alignas, [[...]]
//
// C++11 allows variable alignment to be queried and controlled with alignof, and genralized attributes using alignas
// and "[[...]]" (like gcc __attribute__).

int alignof_x = alignof(int *);
int alignof_y = alignof(char);
struct alignas(16) alignas(16) sse_t {
	float sse_data[4];
};
alignas(128) char cacheline1[128];
[[ ]] [[ ]] char cacheline2[128]; // empty attributes


// nullptr
//
// C++11 replaces the NULL macro (0) by introducing a new keyword to serve as a distinguished null pointer constant:
// nullptr. It is of type nullptr_t, which is implicitly convertible and comparable to any pointer type or
// pointer-to-member type. It is not implicitly convertible or comparable to integral types, except for bool.

char *npc = nullptr;
int *npi = nullptr;
bool b = nullptr;
struct mystruct{};
mystruct *mys = nullptr;
 
void nfoo( char * ) {}
void nfoo( int ) {}
void null() {
	nfoo( nullptr );  // calls foo( nullptr_t ), which is foo( char * ) in this context
}


// default and delete
//
// In C++03, the compiler provides, for classes that do not provide them for themselves, a default constructor, a copy
//      constructor, a copy assignment operator (operator=), and a destructor.  The programmer can override these
//      defaults by defining custom versions. C++ also defines several global operators (such as operator new) that work
//      on all classes, which the programmer can override.  However, there is very little control over the creation of
//      these defaults. Making a class inherently non-copyable, for example, requires declaring a private copy
//      constructor and copy assignment operator and not defining them. Attempting to use these functions is a violation
//      of the One Definition Rule (ODR). While a diagnostic message is not required,[10] violations may result in a
//      linker error.  In the case of the default constructor, the compiler will not generate a default constructor if a
//      class is defined with any constructors. This is useful in many cases, but it is also useful to be able to have
//      both specialized constructors and the compiler-generated default.
//
//  C++11 allows the explicit defaulting and deleting of these special member functions

class X {
  public:
	X() = default;
	X(int) = delete;
};

// explicit conversion operator
//
// C++98 added the explicit keyword as a modifier on constructors to prevent single-argument constructors from being
//      used as implicit type conversion operators.  However, this does nothing for actual conversion operators.
//
// In C++11, the explicit keyword can now be applied to conversion operators. As with constructors, it prevents the use
//      of those conversion functions in implicit conversions.

struct T {
	explicit operator bool() { return true; };   //explicit bool conversion operator
};
int fred() {
	T t1;
	bool t2 = true;
	t1 && t2; 	// compiler implicitly converts t1 to bool type through explicit bool conversion operator
	return 0;
}

template <class T> struct S {
	operator bool() const { return false; }   // conversion function
};
void func(S<int>& s) {
	if (s) { }	// compiler implicitly converts s to bool type through the conversion function
}

void rtn(S<int>& p1, S<float>& p2) {
	// compiler implicitly converts both p1 and p2 to bool type through the conversion function.
	(void)(p1 + p2); 
  
	// compiler implicitly converts both p1 and p2 to bool type through the conversion function and compares results.
	if ( p1 == p2 ) { }  
}


// Local and Unnamed types as template arguments

template<typename T> void f(T) { }

struct { int i; } xx;

void fn() {
	f(xx); 
}

// long long
//
// In C++03, the largest integer type is long int. It is guaranteed to have at least as many usable bits as int.  This
//      resulted in long int having size of 64 bits on some popular implementations and 32 bits on others.
//
// C++11 adds a new integer type long long int to address this issue.  It is guaranteed to be at least as large as a
//      long int, and have no fewer than 64 bits.

unsigned long long llx = 184467442; // 2^64 - 1
long long int lly;


// XXX

struct B {};
struct A { B b; };
int sb = sizeof(A::b);

// static assert
//
// C++11 introduces a new way to test assertions at compile-time, using the new keyword static_assert.

#include <type_traits>

template<class T> struct Check  {
	static_assert(sizeof(int) <= sizeof(T), "T is not big enough!");
};
template<class Integral> Integral foo(Integral x, Integral y) {
	static_assert(std::is_integral<Integral>::value, "foo() parameter must be an integral type.");
}
void sa() {
	const double GREEKPI = 3.14159;
	static_assert((GREEKPI > 3.14) && (GREEKPI < 3.15), "GREEKPI is inaccurate!");


	static_assert( true, "error msg" );
}


int f() noexcept { return 3; }

/* Fail */
void baz() noexcept { throw 42; }  // noexcept is the same as noexcept(true)
/* Pass */
template <class T>
void self_assign(T& t) noexcept(noexcept(t = t)) {}
// whether foo is declared noexcept depends on if the expression
// T() will throw any exceptions
template <class T> void foo() noexcept(noexcept(T())) {}
void www() noexcept(true) {}
class exception_ptr {
    exception_ptr() noexcept;
    exception_ptr(exception_ptr&) noexcept;
};


// strongly typed enumerations
//
// In C++03, enumerations are not type-safe. They are effectively integers, even when the enumeration types are
// distinct. This allows the comparison between two enum values of different enumeration types. The only safety that
// C++03 provides is that an integer or a value of one enum type does not convert implicitly to another enum
// type. Additionally, the underlying integral type is implementation-defined; code that depends on the size of the
// enumeration is therefore non-portable. Lastly, enumeration values are scoped to the enclosing scope. Thus, it is not
// possible for two separate enumerations to have matching member names.
//
// C++11 allows a special classification of enumeration that has none of these issues. This is expressed using the enum
// class (enum struct is also accepted as a synonym) declaration:

enum class Enumeration {
    Val1,
    Val2,
    Val3 = 100,
    Val4 // = 101
};
enum class Enum1 : unsigned int { Val1, Val2 };
enum Enum2 : unsigned long { Val1 = 1, Val2 };
enum Enum3 : unsigned int;       // Legal in C++11, the underlying type is explicitly specified.
enum class Enum4;                // Legal in C++11, the underlying type is int.
enum class Enum5 : unsigned int; // Legal in C++11.


double d = 0;
int &&i = d;

// right angle bracket
//
// C++03's parser defines ">>" as the right shift operator in all cases. However, with nested template declarations,
// there is a tendency for the programmer to neglect to place a space between the two right angle brackets, thus causing
// a compiler syntax error.
//
// C++11 improves the specification of the parser so that multiple right angle brackets is interpreted as closing the
// template argument list where it is reasonable, which can be overridden by using parentheses around parameter
// expressions using the ">", ">=" or ">>" binary operators:

template<typename T, int i = 3> class Vector {};
Vector<Vector<int> > v1;
Vector<Vector<int>> v2;

Vector<Vector<Vector<int> > > v3;
Vector<Vector<Vector<int>> > v4;
Vector<Vector<Vector<int> >> v5;
Vector<Vector<Vector<int>>> v6;
Vector<Vector<Vector<Vector<int>>>> v7;

template<int T> class Bar {};
Bar<3> bar;
Vector<Bar<(3>4)>> m4;
Vector<Bar<(3>4)> > m5;
Vector<Bar<(3>>4)> > m6;
Vector<Bar<(3>>4>>2)> > m7;


struct Point {
    Point() {}
    Point(int x, int y): x_(x), y_(y) {}
    int x_, y_;
};
 
#include <new> // Required for placement 'new'.
 
union U {
    int z;
    double w;
    Point p; // Illegal in C++03; legal in C++11.
    U() {new(&p) Point();} // Due to the Point member, a constructor definition is now required.
};


// Function with variadic templates

template<typename... Args> int maximum( int n, Args... args ) {
    return max( n, maximum( args... ) );
}

// Class with variadic template

template<typename ... Arguments> class VariadicTemplate {};
template<volatile int ...> class VariadicTemplate2 {};


// using alias (new typedef)

using VCLI = volatile const long int;
VCLI vcli = 3;

using IP = int *;
IP ip;

using MIPR1 =
#ifdef __U_CPLUSPLUS__
	_Mutex												// uC++ error
#endif // __U_CPLUSPLUS__
	int *&;
using NMIPR1 =
#ifdef __U_CPLUSPLUS__
	_Nomutex											// uC++ error
#endif // __U_CPLUSPLUS__
	int *&;

using A3D = int ([][5][5]);
A3D *a3d;

using RGB = enum { R, G, B };
RGB xxx = xxx;

using STRCT = struct S1 { int i; };
STRCT strct;

using CLSS = class C { int i; void main() {}; };
CLSS clss;

using FP1 = void (*)(double);
FP1 ft1;
using FP2 = long int (* const volatile)(double[]);
long int func( double[] ) { return 3; }
FP2 ft2 = func;
using FP3 = int (*((*)[])[5]);
FP3 fp3;

struct S2 { class P { int x; }; };
using CVNCP = const volatile typename S2::P *;
CVNCP cvncp;

template<typename T> struct W { struct P { T x; }; };
template<typename T> using CVTNSP = const volatile typename W<T>::P *;
CVTNSP<int> cvtnsp;

using fmtfl = std::ios_base::fmtflags;
fmtfl fl_orig = std::cout.flags();
fmtfl fl_hex = (fl_orig & ~std::cout.basefield) | std::cout.showbase | std::cout.hex;
fmtfl fl_orig2 = std::cout.flags(fl_hex);

template<typename _R1, typename _R2> struct __ratio_multiply {
    typedef int type;
    static constexpr int num = 3;
    static constexpr int den = 4;
};

template<typename _R1, typename _R2>
constexpr int __ratio_multiply<_R1, _R2>::num;

template<typename _R1, typename _R2>
constexpr int __ratio_multiply<_R1, _R2>::den;

template<typename _R1, typename _R2> using ratio_multiply = typename __ratio_multiply<_R1, _R2>::type;
ratio_multiply<int,int> ppp;

template<typename...> class tuple;
template<class _Tp> class tuple_size;
template<int _Int, class _Tp> class tuple_element;
template<int _Ind, typename... _Tp> inline auto __volget( int& __tuple) -> typename tuple_element<_Ind, tuple<_Tp...>>::type & {}


//  inline Namespace

namespace L {
	inline namespace M {				// breaks here because of inline keyword
		template <typename T> class C; 
	}
	template <typename T> void f(T) { };
}

/* 
   Wiki: http://en.wikipedia.org/wiki/C++11
   Paper: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2442.htm
*/

// unicode string literals
const char u1[] = u8"This is a Unicode Character: \u2018.";
const char16_t u2[] = u"This is a bigger Unicode Character: \u2018.";
const char32_t u3[] = U"This is a Unicode Character: \U00002018.";

// wide string
wchar_t w1[] = L"The String Data Stuff";	// no lowercase L

// raw string
char rempty[] = R"()";
char rnodelimiter[] = R"(The String Data \ Stuff " )";
char rwithdelimiter[] = R"delimiter(The String Data \ Stuff " )delimiter";

// wide raw string
wchar_t wr1[] = LR"(The String Data \ Stuff " )";   // note we can have LR but not RL
wchar_t wr2[] = LR"X(The String Data \ Stuff " )X";   // note we can have LR but not RL

// unicode raw string
const char ur1[] = u8R"(This is a Unicode Character: \u2018.)";
const char16_t ur2[] = uR"(This is a bigger Unicode Character: \u2018.)";
const char32_t ur3[] = UR"(This is a Unicode Character: \U00002018.)";
char ur4[] = u8R"X(This is a Unicode )Y: \U00002018.)X";
char16_t ur5[] = uR"XR8(This is a Unicode )XY
	X: '"\U00002018.)XR8";
//"
char32_t ur6[] = UR"XXX(This is a Unicode )XXY: \U00002018.)XXX";
char32_t ur7[] = UR"0123456789012345(This is a Unicode )XXY: \U00002018.)0123456789012345";

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work testC++11.cc -std=c++0x -no-u++-include" //
// End: //
