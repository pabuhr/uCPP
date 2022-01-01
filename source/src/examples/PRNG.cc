//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2021
// 
// PRNG.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sat Dec 25 20:47:53 2021
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Jan  1 14:27:42 2022
// Update Count     : 17
// 

#include <iostream>
#include <iomanip>
#include <locale>
#include <cmath>
#include <uPRNG.h>
using namespace std;

enum : unsigned int { BUCKETS = 100'000, TRIALS = 1'000'000'000 };

void avgstd( unsigned int buckets[] ) {
	unsigned int min = UINT_MAX, max = 0;
	double sum = 0.0;
	for ( unsigned int r = 0; r < BUCKETS; r += 1 ) {
		if ( buckets[r] < min ) min = buckets[r];
		if ( buckets[r] > max ) max = buckets[r];
		sum += buckets[r];
	} // for
	double avg = sum / BUCKETS;							// average
	sum = 0.0;
	for ( unsigned int r = 0; r < BUCKETS; r += 1 ) {	// sum squared differences from average
		double diff = buckets[r] - avg;
		sum += diff * diff;
		buckets[r] = 0;									// reset buckets
	} // for
	double std = sqrt( sum / BUCKETS );
	osacquire( cout ) << fixed << setprecision(1) << "trials " << TRIALS << " buckets " << BUCKETS
					  << " min " << min << " max " << max
					  << " avg " << avg << " std " << std << " rstd " << (avg == 0 ? 0.0 : std / avg * 100) << "%" << endl;
} // avgstd

unsigned int buckets[BUCKETS];

int main() {
	uTime start;
	uint32_t seed = 1009;
#if 0
	cout << "rand" << endl;
	srand( seed );
	for ( unsigned int i = 0; i < 20; i += 1 ) {
		cout << rand() << endl;
	} // for
	cout << "seed " << seed << endl;

	start = uClock::getCPUTime();
 	for ( unsigned int i = 0; i < TRIALS; i += 1 ) {
 		buckets[rand() % BUCKETS] += 1;					// sequential
 	} // for
	avgstd( buckets );
	cout << (uClock::getCPUTime() - start).nanoseconds() / 1'000'000'000.0 << " seconds" << endl;
#endif // 0
	cout << endl << "PRNG" << endl;
	PRNG sprng;
	sprng.set_seed( seed );
	for ( unsigned int i = 0; i < 20; i += 1 ) {
		cout << setw(10) << sprng() << ' ' << sprng( 5 ) << ' ' << sprng( 0, 5 ) << endl;
	} // for
	cout << "seed " << sprng.get_seed() << endl;
		
	start = uClock::getCPUTime();
 	for ( unsigned int i = 0; i < TRIALS; i += 1 ) {
		buckets[sprng() % BUCKETS] += 1;				// sequential
 	} // for
	avgstd( buckets );
	cout << (uClock::getCPUTime() - start).nanoseconds() / 1'000'000'000.0 << " seconds" << endl;

	cout << endl << "prng" << endl;
	set_seed( seed );
	for ( unsigned int i = 0; i < 20; i += 1 ) {
		cout << setw(10) << prng() << ' ' << prng( 5 ) << ' ' << prng( 0, 5 ) << endl;
	} // for
	cout << "seed " << get_seed() << endl;

	start = uClock::getCPUTime();
	{
		enum { TASKS = 4 };
		uProcessor p[TASKS - 1];						// already 1 processor
		// uThisProcessor().setAffinity( 0 );				// affinity cores 0 to TASK-1
		// for ( unsigned int i = 0; i < TASKS - 1; i += 1 ) p[i].setAffinity( i + 1 );
		{
			_Task T {
				void main() {
					unsigned int * buckets = (unsigned int *)calloc( BUCKETS, sizeof(unsigned int) ); // too big for task stack
					for ( unsigned int i = 0; i < TRIALS; i += 1 ) {
						buckets[prng() % BUCKETS] += 1;	// concurrent
					} // for
					avgstd( buckets );
					free( buckets );
				} // main
			} t[TASKS];
		} // wait for tasks to complete
	}
	cout << (uClock::getCPUTime() - start).nanoseconds() / 1'000'000'000.0 << " seconds" << endl;
} // main


// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work -Wall -g -O2 -multi -nodebug PRNG.cc" //
// End: //
