#include <uFuture.h>

void fred() {
    Future_ISM<int> f1, f2, f3, f4, f5;
    bool S, T, X, Y, Z, W;

    S = T = X = Y = Z = W = false;
#if 1
    _Select( f1 );

    _Select( f1 ) {
    }

    _Select( f1 || f2 ) {
    }

    _When( true ) _Select( f1 );

    _When( true ) _Select( f1 ) {
    }

    _Select( f1 );
    _Else _Select( f3 );

    _Select( f1 || f2 );
    _Else _Select( f3 );

    _When( true ) _Select( f1 );
    _Else _When( true ) _Select( f3 );

    _When( true ) _Select( f1 || f2 );
    _Else _When( true ) _Select( f3 );

    _Select( f1 ) {
    } _Else _Select( f3 ) {
    }

    _Select( f1 || f2 ) {
    } _Else _Select( f3 ) {
    }

    _When( true ) _Select( f1 ) {
    } _Else _When( true ) _Select( f3 ) {
    }

    _Select( f1 );
    _Else _Select( f3 ) {
    }

    _Select( f1 || f2 ) {
    } _Else _Select( f3 ) {
    }

    _When( true ) _Select( f1 ) {
    } _Else _When( true ) _Select( f3 ) {
    }

    _Select( f1 );
    _Else _Select( f3 ) {
    }

    _When( true ) _Select( f1 );
    _Else _When( true ) _Select( f3 ) {
    }

    _Select( f1 ) {
    } _Else _Select( f3 );

    _When( true ) _Select( f1 ) {
    } _Else _When( true ) _Select( f3 );

    // ******************************

    _Select( f1 );
    _Else;

    _When( true ) _Select( f1 );
    _Else;

    _When( true ) _Select( f1 || f2 );
    _Else;

    _Select( f1 ) {
    } _Else {
    }

    _When( true ) _Select( f1 ) {
    } _Else {
    }

    _Select( f1 );
    _Else {
    }

    _When( true ) _Select( f1 );
    _Else {
    }

    _When( true ) _Select( f1 || f2 );
    _Else {
    }

    _Select( f1 ) {
    } _Else;

    _When( true ) _Select( f1 ) {
    } _Else;

    // ******************************

    _Select( f1 );
    _When( true ) _Else;

    _When( true ) _Select( f1 );
    _When( true ) _Else;

    _When( true ) _Select( f1 || f2 );
    _When( true ) _Else;

    _Select( f1 ) {
    } _When( true ) _Else {
    }

    _When( true ) _Select( f1 ) {
    } _When( true ) _Else {
    }

    _Select( f1 );
    _When( true ) _Else {
    }

    _When( true ) _Select( f1 );
    _When( true ) _Else {
    }

    _When( true ) _Select( f1 || f2 );
    _When( true ) _Else {
    }

    _Select( f1 ) {
    } _When( true ) _Else;

    _When( true ) _Select( f1 ) {
    } _When( true ) _Else;

    // ******************************

    _Select( f1 );
    or _Timeout( uDuration( 5 + 6 ) );

    _Select( f1 || f2 );
    or _Timeout( uDuration( 5 + 6 ) );

    _When( true ) _Select( f1 );
    or _Timeout( uDuration( 5 + 6 ) );

    _Select( f1 ) {
    } or _Timeout( uDuration( 5 + 6 ) ) {
    }

    _When( true ) _Select( f1 ) {
    } or _Timeout( uDuration( 5 + 6 ) ) {
    }

    _When( true ) _Select( f1 || f2 ) {
    } or _Timeout( uDuration( 5 + 6 ) ) {
    }

    _When( true ) _Select( f1 ) {
    } or _When ( true ) _Timeout( uDuration( 5 + 6 ) ) {
    }

    _When( true ) _Select( f1 || f2 ) {
    } or _When ( true ) _Timeout( uDuration( 5 + 6 ) ) {
    }

    _Select( f1 );
    or _Timeout( uDuration( 5 + 6 ) ) {
    }

    _When( true ) _Select( f1 );
    or _Timeout( uDuration( 5 + 6 ) ) {
    }

    _When( true ) _Select( f1 );
    or _When( true ) _Timeout( uDuration( 5 + 6 ) ) {
    }

    _Select( f1 ) {
    } or _Timeout( uDuration( 5 + 6 ) );

    _When( true ) _Select( f1 ) {
    } or _Timeout( uDuration( 5 + 6 ) );

    _When( true ) _Select( f1 ) {
    } or _When( true ) _Timeout( uDuration( 5 + 6 ) );

    // ******************************

    _Select( f1 || f2 ) {}
    and _Select( f3 ) {}
    and _Select( f4 ) {}
    or _Select( f5 ) {}

    _Select( f1 ) {}
    and _Select( f2 ) {}
    and _Select( f3 ) {}
    and _Select( f4 ) {}
    or _Select( f5 ) {}

    _Select( f1 ) {}
    and _Select( f2 ) {}
    or _Select( f3 ) {}
    and _Select( f4 ) {}
    or _Select( f5 ) {}

    (   _Select( f1 ) {
	} or _Select( f2 ) {
	}
    ) and (
	_Select( f3 ) {
	} or _Select( f4 ) {
	}
    )

    _Select( f1 ) {}
    or _Select( f2 ) {}
    and (
	_Select( f3 ) {}
	or _Select( f4 ) {}
    )

    (   _Select( f1 ) {
	} and _Select( f2 ) {
	}
    ) or (
	_Select( f3 ) {
	} and _Select( f4 ) {
	}
    )

    _Select( f1 || f2 ) {
    } and (
	_Select( f3 ) {
	} or _Select( f4 ) {
	}
    )

    _Select( f1 ) {
    } or _Select( f2 ) {
    } or _Select( f3 ) {
    }

    _Select( f1 ) {
    } and _Select( f2 ) {
    }

    _When( X ) (
	_Select( f1 ) {}
	or _Select( f2 ) {}
	and _Select( f3 ){}
    ) {}

    _When( X ) (
	_When( Y ) _Select( f1 ) {}
	or _When( Z ) _Select( f2 ) {}
    ) and _When( W ) _Select( f3 ) {}

    _When ( 3 < 4 ) _Select( f1 ) {}
    or _Select( f2 ) {}
    and _Select( f3 ) {}
    and _Select( f4 ) {}
    and _Select( f5 ) {}

    _Select( f1 ) {
    } or _Select( f2 ) {
    } _Else {}

    _Select( f1 ) {
    } or _Select( f2 ) {
    } or _Timeout( uTime( 7 ) ) {}

    _When( X ) ( _When( Y ) _Select( f1 ) {}
    or _When( Z ) _Select( f2 ) {}
    ) and _When( W ) _Select( f3 ) {}
    _Else {}

    _When( X ) ( _When( Y ) _Select( f1 ) {}
    or _When( Z ) _Select( f2 ) {}
    ) and _When( W ) _Select( f3 ) {}
    _When( S ) _Else {}

    _When( X ) ( _When( Y ) _Select( f1 ) {}
    or _When( Z ) _Select( f2 ) {}
    ) and _When( W ) _Select( f3 ) {}
    or _Timeout( uDuration( 5 ) ) {}

    _When( X ) ( _When( Y ) _Select( f1 ) {}
    or _When( Z ) _Select( f2 ) {}
    ) and _When( W ) _Select( f3 ) {}
    or _When( S ) _Timeout( uDuration( 5 ) ) {}

    _When( X ) ( _When( Y ) _Select( f1 ) {}
    or _When( Z ) _Select( f2 ) {}
    ) and _When( W ) _Select( f3 ) {}
    or _Timeout( uDuration( 5 ) ) {}
    _When( S ) _Else {}

    _When( X ) ( _When( Y ) _Select( f1 ) {}
    or _When( Z ) _Select( f2 ) {}
    ) and _When( W ) _Select( f3 ) {}
    or _When( T ) _Timeout( uDuration( 5 ) ) {}
    _When( S ) _Else {}

    // ******************************

    _Select( f1 ) {
	_Select( f1 ) {
	} or _Select( f2 ) {
	} or _Select( f3 ) {
	}
    } or _Select( f2 ) {
	_Select( f1 ) {
	    _Select( f1 ) {
	    } or _Select( f2 ) {
	    } or _Select( f3 ) {
	    }
	} or _Select( f2 ) {
	} or _Select( f3 ) {
	}
    } or _Select( f3 ) {
	_Select( f1 ) {
	} or _Select( f2 ) {
	} or _Select( f3 ) {
	}
    }
#endif

#if defined( ERRORS )
    _Select;
    _When;
    _When( X );
    _Timeout;
    _Select( f1 ) {} or;
    _Select( f1 ) {} or _When;
    _Select( f1 ) {} or _When( X );
#endif
}

void uMain::main() {
}
