class B1 {
  protected:
    typedef int X;
};

class B2 {
  protected:
    typedef int Y;
    _Mutex void mem();
};

class B3 : public B2 {
  protected:
    typedef int Y;
    _Mutex void mem();
};

class D : public B1, public B3 {
    X x;
    Y y;
    _Mutex void mem() {}
};


_Task foo {
  public:
    void m1() {}
    void m2() {}
};

_Task bar : public foo {
  public:
    void m3() {}
    void m4() {}
  private:
    void main() {
	_Accept(m1);
	or _Accept(m2);
	or _Accept(m3);
	or _Accept(m4);
    }
};


_Task MyTask {
    void main() {}
  public:
    void myFunc1( std::string &foobar ) {}
    void myFunc2( std::string foobar );
    void mem1() {
	_Accept( myFunc1 );
    }
    void mem2();
};

using namespace std;

void MyTask::myFunc2( string foobar ) {}

void MyTask::mem2( void ) {
    for(;;) {
        _Accept( myFunc2 ) {}
    }
}


_Mutex class mary {
    uCondition c;
  public:
    void x() {}
    void y() {}
    void z();
  private:
    int mem1() {
	_AcceptReturn( x ) 5 + 6;
	_AcceptReturn( x, y, z ) 5 + 6;
	_AcceptWait( x ) c;
	_AcceptWait( x, z ) c;
    }
    void mem2() {
    	_AcceptReturn( x );
	_AcceptReturn( x, z );
    }
};

_Mutex class fred {
    uCondition c;
  public:
    void x() {}
    void y() {}
    void z();

    fred() {
	_AcceptWait( x, z, ~fred ) c _With 5;
    }
    ~fred() {
	_AcceptWait( x, z, ~fred ) c;
    }
  private:
    void mem() {
#if 1
	_Accept( x );

	_Accept( x ) {
	}

	_Accept( x, z ) {
	}

	_When( true ) _Accept( x );

	_When( true ) _Accept( x ) {
	}

	_Accept( x );
	or _Accept( y );

	_Accept( x, z );
	or _Accept( y );

	_When( true ) _Accept( x );
	or _When( true ) _Accept( y );

	_When( true ) _Accept( x, z );
	or _When( true ) _Accept( y );

	_Accept( x ) {
	} or _Accept( y ) {
	}

	_Accept( x, z ) {
	} or _Accept( y ) {
	}

	_When( true ) _Accept( x ) {
	} or _When( true ) _Accept( y ) {
	}

	_Accept( x );
	or _Accept( y ) {
	}

	_When( true ) _Accept( x );
	or _When( true ) _Accept( y ) {
	}

	_Accept( x ) {
	} or _Accept( y );

	_When( true ) _Accept( x ) {
	} or _When( true ) _Accept( y );

	// ******************************

	_Accept( x );
	_Else;

	_When( true ) _Accept( x );
	_Else;

	_When( true ) _Accept( x, z );
	_Else;

	_Accept( x ) {
	} _Else {
	}

	_When( true ) _Accept( x ) {
	} _Else {
	}

	_Accept( x );
	_Else {
	}

	_When( true ) _Accept( x );
	_Else {
	}

	_When( true ) _Accept( x, z );
	_Else {
	}

	_Accept( x ) {
	} _Else;

	_When( true ) _Accept( x ) {
	} _Else;

	// ******************************

	_Accept( x );
	_When( true ) _Else;

	_When( true ) _Accept( x );
	_When( true ) _Else;

	_When( true ) _Accept( x, z );
	_When( true ) _Else;

	_Accept( x ) {
	} _When( true ) _Else {
	}

	_When( true ) _Accept( x ) {
	} _When( true ) _Else {
	}

	_Accept( x );
	_When( true ) _Else {
	}

	_When( true ) _Accept( x );
	_When( true ) _Else {
	}

	_When( true ) _Accept( x, z );
	_When( true ) _Else {
	}

	_Accept( x ) {
	} _When( true ) _Else;

	_When( true ) _Accept( x ) {
	} _When( true ) _Else;

	// ******************************

	_Timeout( uDuration( 5 + 6 ) );

	_Timeout( uDuration( 5 + 6 ) ) {
	}

	_When ( true ) _Timeout( uDuration( 5 + 6 ) );

	_When ( true ) _Timeout( uDuration( 5 + 6 ) ) {
	}

	_Accept( x );
	or _Timeout( uDuration( 5 + 6 ) );

	_Accept( x, z );
	or _Timeout( uDuration( 5 + 6 ) );

	_When( true ) _Accept( x );
	or _Timeout( uDuration( 5 + 6 ) );

	_Accept( x ) {
	} or _Timeout( uDuration( 5 + 6 ) ) {
	}

	_When( true ) _Accept( x ) {
	} or _Timeout( uDuration( 5 + 6 ) ) {
	}

	_When( true ) _Accept( x, z ) {
	} or _Timeout( uDuration( 5 + 6 ) ) {
	}

	_When( true ) _Accept( x ) {
	} or _When ( true ) _Timeout( uDuration( 5 + 6 ) ) {
	}

	_When( true ) _Accept( x, z ) {
	} or _When ( true ) _Timeout( uDuration( 5 + 6 ) ) {
	}

	_Accept( x );
	or _Timeout( uDuration( 5 + 6 ) ) {
	}

	_When( true ) _Accept( x );
	or _Timeout( uDuration( 5 + 6 ) ) {
	}

	_When( true ) _Accept( x );
	or _When( true ) _Timeout( uDuration( 5 + 6 ) ) {
	}

	_Accept( x ) {
	} or _Timeout( uDuration( 5 + 6 ) );

	_When( true ) _Accept( x ) {
	} or _Timeout( uDuration( 5 + 6 ) );

	_When( true ) _Accept( x ) {
	} or _When( true ) _Timeout( uDuration( 5 + 6 ) );

	// ******************************

	_Accept( x ) {
	} or _Timeout( uDuration( 5 + 6 ) ) {
	} _Else {}

	_When( true ) _Accept( x ) {
	} or _Timeout( uDuration( 5 + 6 ) ) {
	} _Else {}

	_Accept( x ) {
	} or _Timeout( uDuration( 5 + 6 ) ) {
	} _When( true ) _Else {}

	_Accept( x ) {
	} or _When( true ) _Timeout( uDuration( 5 + 6 ) ) {
	} _Else {}

	_When( true ) _Accept( x ) {
	} or _Timeout( uDuration( 5 + 6 ) ) {
	} _When( true ) _Else {}

	_Accept( x ) {
	} or _When( true ) _Timeout( uDuration( 5 + 6 ) ) {
	} _When( true ) _Else {}

	_When( true ) _Accept( x ) {
	} or _When( true ) _Timeout( uDuration( 5 + 6 ) ) {
	} _When( true ) _Else {}

#if defined( ERRORS )
	_Accept( x );
	or _Accept( x );

	_Accept( x, x );

	_Accept( x, y );
	or _Accept( x );

	_When( true ) _Accept( x ) {
	} or _When( true ) _Timeout( uDuration( 5 + 6 ) ) {
	} or _When( true ) _Timeout( uDuration( 5 + 6 ) );

	_When( true ) _Timeout( uDuration( 5 + 6 ) ) {
	} or _Timeout( uDuration( 5 + 6 ) );

	_When;
	_When( true );
	_When(;
	_Else;
	_Accept;
	_Accept( );
	_Accept( x;

	_Timeout;

	_Accept( ~mary );
#endif // ERRORS
#endif
    }
};

#if defined( ERRORS )
_Nomutex void fred::z() {}
#endif // ERRORS

void uMain::main() {
}

// Local Variables: //
// compile-command: "../../bin/u++ testAcceptStmt.cc" //
// End: //
