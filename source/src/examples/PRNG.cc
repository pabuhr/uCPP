// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2021
// 
// PRNG.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sat Dec 25 20:47:53 2021
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Dec 13 23:08:27 2022
// Update Count     : 273
// 

#include <iostream>
#include <iomanip>
#include <locale>
#include <cmath>
#include <uPRNG.h>
using namespace std;

#define xstr(s) str(s)
#define str(s) #s

#ifdef __x86_64__										// 64-bit architecture
#define PRNG PRNG64
#else													// 32-bit architecture
#define PRNG PRNG32
#endif // __x86_64__

#define TIME

#ifdef TIME												// use -O2 -nodebug
#define STARTTIME start = uClock::getCPUTime();
#define ENDTIME( extra ) cout << (uClock::getCPUTime() - start).nanoseconds() / (double)TIMEGRAN << extra " seconds" << endl;
enum { BUCKETS = 100'000, TRIALS = 1'000'000'000 };
#else
#define STARTTIME
#define ENDTIME( extra )
enum { BUCKETS = 100'000, TRIALS = 100'000'000 };
#endif // TIME

static void avgstd( unsigned int buckets[] ) {
	unsigned int min = UINT_MAX, max = 0;
	double sum = 0.0, diff;
	for ( unsigned int i = 0; i < BUCKETS; i += 1 ) {
		if ( buckets[i] < min ) min = buckets[i];
		if ( buckets[i] > max ) max = buckets[i];
		sum += buckets[i];
	} // for

	double avg = sum / BUCKETS;							// average
	sum = 0.0;
	for ( unsigned int i = 0; i < BUCKETS; i += 1 ) {	// sum squared differences from average
		diff = buckets[i] - avg;
		sum += diff * diff;
	} // for
	double std = sqrt( sum / BUCKETS );
	osacquire( cout ) << fixed << setprecision(1) << "trials " << TRIALS << " buckets " << BUCKETS
					  << " min " << min << " max " << max
					  << " avg " << avg << " std " << std << " rstd " << (avg == 0 ? 0.0 : std / avg * 100) << "%" << endl;
} // avgstd

size_t seed = 1009;


_Task T1 {
	void main() {
		unsigned int * buckets = (unsigned int *)calloc( BUCKETS, sizeof(unsigned int) ); // too big for task stack
		for ( unsigned int i = 0; i < TRIALS / 100; i += 1 ) {
			buckets[rand() % BUCKETS] += 1;				// concurrent
		} // for
		avgstd( buckets );
		free( buckets );
	} // main
}; // T1

_Task T2 {
	void main() {
		PRNG prng;
		if ( seed != 0 ) prng.set_seed( seed );
		unsigned int * buckets = (unsigned int *)calloc( BUCKETS, sizeof(unsigned int) ); // too big for task stack
		for ( unsigned int i = 0; i < TRIALS; i += 1 ) {
			buckets[prng() % BUCKETS] += 1;				// concurrent
		} // for
		avgstd( buckets );
		free( buckets );
	} // main
}; // T2

_Task T3 {
	void main() {
		unsigned int * buckets = (unsigned int *)calloc( BUCKETS, sizeof(unsigned int) ); // too big for task stack
		for ( unsigned int i = 0; i < TRIALS; i += 1 ) {
			buckets[::prng() % BUCKETS] += 1;			// concurrent
		} // for
		avgstd( buckets );
		free( buckets );
	} // main
}; // T3

_Task T4 {
	void main() {
		unsigned int * buckets = (unsigned int *)calloc( BUCKETS, sizeof(unsigned int) ); // too big for task stack
		for ( unsigned int i = 0; i < TRIALS; i += 1 ) {
			buckets[prng() % BUCKETS] += 1;				// concurrent
		} // for
		avgstd( buckets );
		free( buckets );
	} // main
}; // T4

// Compiler bug requires hiding declaration of th from the bucket access, otherwise the compiler thinks th is aliased
// and continually reloads it from memory, which doubles the cost.
static void dummy( uBaseTask & th ) __attribute__(( noinline ));
static void dummy( uBaseTask & th ) {
	unsigned int * buckets = (unsigned int *)calloc( BUCKETS, sizeof(unsigned int) ); // too big for task stack
	for ( unsigned int i = 0; i < TRIALS; i += 1 ) {
		buckets[th.prng() % BUCKETS] += 1;				// sequential
	} // for
	avgstd( buckets );
	free( buckets );
} // dummy

size_t malloc_unfreed() { return 16621; }				// unfreed storage from locale

int main() {
	locale loc( getenv("LANG") );
	cout.imbue( loc );

	cout << xstr(PRNG_NAME_64) << endl << endl;
	cout << LONG_MAX-1 << ' ' << LONG_MAX-1-25ull << ' ' << 256-5 << endl;

	enum { TASKS = 4 };
	uTime start;
#ifdef TIME												// too slow for test and generates non-repeatable results
#if 1
	unsigned int rseed;
	if ( seed != 0 ) rseed = seed;
	else rseed = uRdtsc();
	srand( rseed );

	cout << setw(26) << "rand()" << setw(12) << "rand(5)" << setw(12) << "rand(0,5)" << endl;
	for ( unsigned int i = 0; i < 20; i += 1 ) {
		cout << setw(26) << rand();
		cout << setw(12) << rand() % 5;
		cout << setw(12) << rand() % (5 - 0 + 1) + 0 << endl;
	} // for
	cout << "seed " << rseed << endl;

	cout << endl << "Sequential" << endl;
	STARTTIME;
	{
		unsigned int * buckets = (unsigned int *)calloc( BUCKETS, sizeof(unsigned int) ); // too big for task stack
		for ( unsigned int i = 0; i < TRIALS / 10; i += 1 ) {
			buckets[rand() % BUCKETS] += 1;				// sequential
		} // for
		avgstd( buckets );
		free( buckets );
	}
	ENDTIME( " x 10" );

	cout << endl << "Concurrent" << endl;
	STARTTIME;
	{
		uProcessor p[TASKS - 1];						// already 1 processor
		// uThisProcessor().setAffinity( 0 );				// affinity cores 0 to TASK-1
		// for ( unsigned int i = 0; i < TASKS - 1; i += 1 ) p[i].setAffinity( i + 1 );
		{
			T1 t[TASKS];
		} // wait for tasks to complete
	}
	ENDTIME( " x 100" );
#endif // 0
#endif // TIME
#if 1
	PRNG sprng;
	if ( seed != 0 ) sprng.set_seed( seed );

	cout << endl << setw(26) << "PRNG()" << setw(12) << "PRNG(5)" << setw(12) << "PRNG(0,5)" << endl;
	for ( unsigned int i = 0; i < 20; i += 1 ) {
		cout << setw(26) << sprng();					// cascading => side-effect functions called in arbitary order
		cout << setw(12) << sprng( 5 );
		cout << setw(12) << sprng( 0, 5 ) << endl;
	} // for
	cout << "seed " << sprng.get_seed() << endl;
		
	cout << endl << "Sequential" << endl;
	STARTTIME;
	{
		unsigned int * buckets = (unsigned int *)calloc( BUCKETS, sizeof(unsigned int) ); // too big for task stack
		for ( unsigned int i = 0; i < TRIALS; i += 1 ) {
			buckets[sprng() % BUCKETS] += 1;			// sequential
		} // for
		avgstd( buckets );
		free( buckets );
	}
	ENDTIME();

	cout << endl << "Concurrent" << endl;
	STARTTIME;
	{
		uProcessor p[TASKS - 1];						// already 1 processor
		// uThisProcessor().setAffinity( 0 );				// affinity cores 0 to TASK-1
		// for ( unsigned int i = 0; i < TASKS - 1; i += 1 ) p[i].setAffinity( i + 1 );
		{
			T2 t[TASKS];
		} // wait for tasks to complete
	}
	ENDTIME();
#endif // 0
#if 1
	if ( seed != 0 ) set_seed( seed );

	cout << endl << setw(26) << "prng()" << setw(12) << "prng(5)" << setw(12) << "prng(0,5)" << endl;
	for ( unsigned int i = 0; i < 20; i += 1 ) {
		cout << setw(26) << prng();						// cascading => side-effect functions called in arbitary order
		cout << setw(12) << prng( 5 );
		cout << setw(12) << prng( 0, 5 ) << endl;
	} // for
	cout << "seed " << get_seed() << endl;
		
	cout << endl << "Sequential" << endl;
	STARTTIME;
	{
		unsigned int * buckets = (unsigned int *)calloc( BUCKETS, sizeof(unsigned int) ); // too big for task stack
		for ( unsigned int i = 0; i < TRIALS; i += 1 ) {
			buckets[prng() % BUCKETS] += 1;				// sequential
		} // for
		avgstd( buckets );
		free( buckets );
	}
	ENDTIME();

	cout << endl << "Concurrent" << endl;
	STARTTIME;
	{
		uProcessor p[TASKS - 1];						// already 1 processor
		// uThisProcessor().setAffinity( 0 );				// affinity cores 0 to TASK-1
		// for ( unsigned int i = 0; i < TASKS - 1; i += 1 ) p[i].setAffinity( i + 1 );
		{
			T3 t[TASKS];
		} // wait for tasks to complete
	}
	ENDTIME();
#endif // 0
#if 1
	if ( seed != 0 ) set_seed( seed );
	uBaseTask & th = uThisTask();

	cout << endl << setw(26) << "prng(t)" << setw(12) << "prng(t,5)" << setw(12) << "prng(t,0,5)" << endl;
	for ( unsigned int i = 0; i < 20; i += 1 ) {
		cout << setw(26) << th.prng();					// cascading => side-effect functions called in arbitary order
		cout << setw(12) << th.prng( 5 );
		cout << setw(12) << th.prng( 0, 5 ) << endl;
	} // for
	cout << "seed " << get_seed() << endl;

	cout << endl << "Sequential" << endl;
	STARTTIME;
	{
		dummy( th );
	}
	ENDTIME();

	cout << endl << "Concurrent" << endl;
	STARTTIME;
	{
		uProcessor p[TASKS - 1];						// already 1 processor
		// uThisProcessor().setAffinity( 0 );				// affinity cores 0 to TASK-1
		// for ( unsigned int i = 0; i < TASKS - 1; i += 1 ) p[i].setAffinity( i + 1 );
		{
			T4 t[TASKS];
		} // wait for tasks to complete
	} // wait for tasks to complete
	ENDTIME();
#endif // 0
//	malloc_stats();
} // main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work -Wall -DTIME -g -O2 -multi -nodebug PRNG.cc" //
// End: //
