class OBJ {
};
struct COMP {
    static OBJ obj;
    OBJ foo;
} comp;
OBJ COMP::obj;
COMP *fobj() { return &comp; }

_Event Exn1 {};
_Event Exn2 {};

void functor_without_env( Exn1 &exn1 ) {}

struct functor_with_env {
    int &i;
    functor_with_env( int &i ) : i( i ) {}
    void operator()( Exn2 &exn2 ) {}
};

void fred() {
    int i;
    functor_with_env exn2functor( i );
    try	< COMP::obj.Exn1 >
	< Exn1, functor_without_env >
        < Exn2, exn2functor >
	< Exn2 >
	< fobj()->foo.Exn1, functor_without_env >
	{
    }
}


// Local Variables: //
// compile-command: "../../bin/u++ testResumption.cc" //
// End: //
