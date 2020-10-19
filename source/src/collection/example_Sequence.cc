#include "uSequence.h"
#include <iostream>
using namespace std;

class fred: public uSeqable {
  public:
    int i;
    fred( int p ) { i = p; }
};

class mary: public fred {
  public:
    mary( int i ) : fred( i ) {}
};

int main() {
    uSequence<fred> foo;
    uSeqIter<fred> fooIter( foo );
    uSequence<mary> bar;
    uSequence<mary> baz;
    uSeqIter<mary> barIter( bar );
    fred *f;
    mary *m;
    int i;

    // fred test

    for ( ; fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout<< endl;
    
    for ( i = 0; i < 10; i += 1 ) {
	foo.add( new fred( 2 * i ) );
    }
    for ( fooIter.over( foo ); fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout<< endl;

    for ( i = 0; i < 9; i += 1 ) {
	foo.dropHead();
    }
    for ( fooIter.over( foo ); fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout<< endl;
    
    for ( i = 0; i < 10; i += 1 ) {
	foo.addTail( new fred( 2 * i + 1 ) );
    }
    for ( fooIter.over( foo ); fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout<< endl;

    for ( i = 0; i < 9; i += 1 ) {
	foo.dropTail();
    }
    for ( fooIter.over( foo ); fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout<< endl;

    // mary test

    for ( ; barIter >> m; ) {
	cout << m->i << " ";
    }
    cout<< endl;
    
    for ( i = 0; i < 10; i += 1 ) {
	bar.add( new mary( 2 * i ) );
	baz.add( new mary( 2 * i ) );
    }
    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout<< endl;
    
    for ( i = 0; i < 9; i += 1 ) {
	bar.dropHead();
    }
    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout<< endl;
    
    for ( i = 0; i < 10; i += 1 ) {
	bar.addTail( new mary( 2 * i + 1 ) );
    }
    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout<< endl;

    for ( i = 0; i < 9; i += 1 ) {
	bar.dropTail();
    }
    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout<< endl;

    bar.transfer( baz );
    for ( barIter.over( baz ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout<< endl;
    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout<< endl;
}

// Local Variables: //
// compile-command: "g++ example_Sequence.cc" //
// End: //
