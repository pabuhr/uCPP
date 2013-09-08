#include "uQueue.h"
#include "iostream.h"

class fred : public uColable {
  public:
    int i;
    fred( int p ) { i = p; }
};

class mary: public fred {
  public:
    mary( int i ) : fred(i) {}
};

main() {
    uQueue<fred> foo;
    uQueueIter<fred> fooIter( foo );
    uQueue<mary> bar;
    uQueueIter<mary> barIter( bar );
    fred *f;
    mary *m;
    int i;

    // fred test

    for ( ; fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 10; i += 1 ) {
	foo.add( new fred( 2 * i ) );
    }

    for ( fooIter.over(foo); fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 9; i += 1 ) {
	foo.drop();
    }

    for ( fooIter.over( foo ); fooIter >> f; ) {
	cout << f->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 10; i += 1 ) {
	foo.add( new fred( 2 * i + 1 ) );
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
	bar.add( new mary( 2 * i ) );
    }

    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 9; i += 1 ) {
	bar.drop();
    }

    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout << "\n";
    
    for ( i = 0; i < 10; i += 1 ) {
	bar.add( new mary( 2 * i + 1 ) );
    }
    for ( barIter.over( bar ); barIter >> m; ) {
	cout << m->i << " ";
    }
    cout << "\n";
}

// Local Variables: //
// compile-command: "g++ example_Queue.cc" //
// End: //
