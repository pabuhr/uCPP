struct Mallinfo {};
struct Mallinfo Mallinfo(void);

enum foo { red } x[10];
struct bar { int i; } y;
struct goo { int i; } z;
struct boo { int i; } w;
const foo (*hoo)[10];
const foo (&koo)[10] = *hoo;

enum foo bar( enum foo i ) { return red; }
enum foo const (*(goo)(enum foo f))[10] { return hoo; }

class test {
  public:
    foo bar( foo i ) { return red; }
    const foo (&((goo))(foo f))[10] { return koo; }
    int foo( boo ) { return 1; }
    boo foo2( int bar ) { return w; }
};

void fred() {
    int *registers = (int *)({ int *__value; __value; });
    registers += 1;
}

typedef long unsigned int jane;
extern "C" jane tom();
class C {
    friend jane ::tom();
};

class ken {
  public:
    operator int **() { return 0; }
};

// Local Variables: //
// compile-command: "../../bin/u++ testTypes.cc" //
// End: //
