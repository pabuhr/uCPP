#include "uBitSet.h"
#include <iostream>

#define NBITS 40

template< int n > void printBitSet( uBitSet< n > set ) {
    for( int i = 0; i < n; ++i ) {
        if ( set.isSet( i ) ) {
            std::cout << "T";
        } else {
            std::cout << "F";
        } // if
    } // for
    std::cout << std::endl;
}

#define SANITY_CHECK(set) if ( !set.isSet( set.findFirstSet() ) ) abort()

int main() {
    uBitSet< NBITS > a, b;
    a.setAll();
    std::cout << "a: ";
    printBitSet( a );
    std::cout << "first set is " << a.findFirstSet() << std::endl;
    a.clr( 0 );
    a.clr( 4 );
    a.clr( NBITS - 1 );
    a.clr( 31 );
    a.clr( 32 );
    std::cout << "a: ";
    printBitSet( a );
    std::cout << "first set is " << a.findFirstSet() << std::endl;
    
    b.clrAll();
    std::cout << "b: ";
    printBitSet( b );
    std::cout << "first set is " << b.findFirstSet() << std::endl;
    b.set( NBITS - 1 );
    b.set( 32 );
    std::cout << "b: ";
    printBitSet( b );
    std::cout << "first set is " << b.findFirstSet() << std::endl;
    b.clr( 32 );
    std::cout << "b: ";
    printBitSet( b );
    std::cout << "first set is " << b.findFirstSet() << std::endl;
}

// Local Variables: //
// compile-command: "g++ -g -I../library -I../kernel -DNDEBUG example_BitSet.cc" //
// End: //
