_Event mary {
  public:
    int **y[10];
    void mem();
};

_Event fred : public mary {
  public:
    void mem();
};

fred *f, g, h;

_Event err {};
_Event b_err {};
_Event d_err {};
_Event f_err {};

void do_something() {}

void rtn() {
    int x, y;

    try {
    } catch( *f.fred ) {
    } catch( *((*&((g.y[(10-3)])))).fred ((&((g)))) ) {
    } catch( *((*&((g.y[(10-3)])))).fred &(g) ) {
    } catch( *((*&((g.y[(10-3)])))).fred::mary &(g) ) {
    } catch( fred &(g) ) {
    } // try

    try {
	do_something();
    } catch(d_err) {
	//B
	throw;
    } catch(g.f_err) {
	//C
	if ( x > y ) {
	    throw;
	} else throw;
    } catch(h.err &e) {
	//D
	if ( x > y ) {
	    _Throw;
	} else _Throw;
    } catch(err) {
	//A
    } catch(b_err) {
	//E
    } catch(g.d_err) {
	//G
    } catch(d_err) {
	//F
    } // try
}


class T0 {};
class T1 {};

class T2 : public T1 {
  public:
    class mary : public T0 {};
    _Event T3 {};
};

void tom() {
    T2 f;
    try {
    } catch( const (f).T2::T3 ) {
    }
}


// Local Variables: //
// compile-command: "../../bin/u++ testTermination.cc" //
// End: //
