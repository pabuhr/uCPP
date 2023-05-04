#include <iostream>
using namespace std;
#include <omp.h>

unsigned int uDefaultPreemption() { return 0; }			// uC++ disable time slicing

enum { Times = 1'000'000'000 };
int w, x, y, z;

void p1( int i ) { osacquire( cout ) << "p1 start" << endl; for ( x = 0; x < Times; x += 1 ); osacquire( cout ) << "p1 end " << x << endl; }
void p2( int i ) { osacquire( cout ) << "p2 start" << endl; for ( y = 0; y < Times; y += 1 ); osacquire( cout ) << "p2 end " << y << endl; }
void p3( int i ) { osacquire( cout ) << "p3 start" << endl; for ( z = 0; z < Times; z += 1 ); osacquire( cout ) << "p3 end " << z << endl; }

int main() {
	#pragma omp parallel sections num_threads( 4 )
	{
		#pragma omp section
		{ osacquire( cout ) << "p0 start" << endl; for ( w = 0; w < Times; w += 1 ); osacquire( cout ) << "p0 end " << w << endl; }
		#pragma omp section
		{ p1( 5 ); }
		#pragma omp section
		{ p2( 7 ); }
		#pragma omp section
		{ p3( 9 ); }
	}
	cout << w << ' ' << x << ' ' << y << ' ' << z << endl;
}

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work OpenMP_COBEGIN.cc -O3 -multi -fopenmp" //
// End: //
