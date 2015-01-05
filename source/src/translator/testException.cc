#include <iostream>

void f() throw(int);             // OK
void (*fp)() throw (int);        // OK
void g(void pfa() throw(int));   // OK
//typedef int (*pf)() throw(int);  // ill-formed

class fred {
  public:
    fred( int i ) { std::cout << "fred" << std::endl; }
    ~fred() { std::cout << "~fred" << std::endl; }
};

class mary : public fred {
  public:
    mary() : fred( 3 ) {}
#if defined( ERRORS )
    mary( int i ) try : fred( 3 ) { std::cout << "mary" << std::endl; } catch (...) { 5; }
#endif // ERRORS
    ~mary() { std::cout << "~mary" << std::endl; }
};

int main() {
    mary m;
}


// Local Variables: //
// compile-command: "../../bin/u++ testException.cc" //
// End: //
