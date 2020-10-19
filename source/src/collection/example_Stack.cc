#include "uStack.h"
#include <iostream>
using namespace std;

class fred : public uColable {
  public:
    int i;
    fred( int p ) { i = p; }
};

class mary: public fred {
  public:
    mary( int i ) : fred( i ) {}
};

int main() {
    uStack<fred> foo;
    uStackIter<fred> fooIter( foo );
    uStack<mary> bar;
    uStackIter<mary> barIter( bar );
    fred *f;
    mary *m;
    int i;

    // fred test

    for ( ; fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 10; i += 1 ) {
	foo.push( new fred( 2 * i ) );
    }

    for ( fooIter.over( foo ); fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 9; i += 1 ) {
	foo.pop();
    }

    for ( fooIter.over( foo ); fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 10; i += 1 ) {
	foo.push( new fred( 2 * i + 1 ) );
    }
    for ( fooIter.over( foo ); fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout << "\n";

    // mary test

    for ( ; barIter >> m; ) {
	cout << m->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 10; i += 1 ) {
	bar.push( new mary( 2 * i ) );
    }

    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 9; i += 1 ) {
	bar.pop();
    }

    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 10; i += 1 ) {
	bar.push( new mary( 2 * i + 1 ) );
    }
    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout << "\n";
}

// Local Variables: //
// compile-command: "g++ example_Stack.cc" //
// End: //
