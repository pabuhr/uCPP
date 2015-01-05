class xxx {
    class bar {
	friend class foo;
    };
    friend int m( foo & );
    int m () { return w; }
    static int w;
  public:
    class ppp {
	int qqq;
      public:
	friend int t() { return 5; }
	int s() { return w; }
	ppp() { w += 1; }
    };
    xxx() { this->m(); }
};

xxx::ppp p;
int xxx::w = 0;
void yyy( foo &x ) {
    m(x);
    t();
}

class ostream;
extern ostream& lock(ostream& outs);
class __default_alloc_template {
    class lock {
    };
    friend class lock;
} ;


// Local Variables: //
// compile-command: "../../bin/u++ testFriend.cc" //
// End: //
