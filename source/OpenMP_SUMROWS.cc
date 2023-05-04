#include <iostream>
using namespace std;
#include <omp.h>

unsigned int uDefaultPreemption() { return 0; }			// uC++ disable time slicing

int main() {
	enum { cols = 200'000 };
	const unsigned int rows = 1000;

	size_t (* matrix)[cols] = new size_t[rows][cols], total = 0;
	size_t * subtotals = new size_t[rows];

	for ( size_t r = 0; r < rows; r += 1 )				// sequential
		for ( size_t c = 0; c < cols; c += 1 )
			matrix[r][c] = 1;

	#pragma omp parallel for
	for ( size_t r = 0; r < rows; r += 1 ) {			// concurrent
		subtotals[r] = 0;
		for ( size_t c = 0; c < cols; c += 1 )
			subtotals[r] += matrix[r][c];
	}

	for ( size_t r = 0; r < rows; r += 1 )				// sequential
		total += subtotals[r];
	cout << total << endl;

	delete [] subtotals;
	delete [] matrix;
} // main

// setenv OMP_NUM_THREADS 4
// set env OMP_NUM_THREADS 4 (gdb)

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work OpenMP_SUMROWS.cc -O3 -nodebug -multi -fopenmp" //
// End: //
