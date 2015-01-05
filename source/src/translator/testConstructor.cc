class Base {
  public:
    Base( ) {}
    Base( int i ) {}
};

class mary {
  public:
    mary( ) {}
    mary( int i ) {}
};

_Task fred : public Base {
    mary m;
  public:
    fred() {}
    fred( int ) : uBaseTask( 100 ) {}
    fred( float ) : uBaseTask( 100 ), m( 3 ) {}
    fred( bool ) : m( 3 ) {}
    fred( char ) : Base( 3 ) {}
    fred( int * ) : Base( 3 ), m( 3 ) {}
    fred( char * ) : uBaseTask( 100 ), Base( 3 ), m( 3 ) {}
};

// generated constructor

_Task jane {};

_Task tom : public jane {};

_Coroutine coroutine {};

_Mutex _Coroutine CM : private coroutine {};

