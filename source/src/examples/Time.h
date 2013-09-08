#include <unistd.h>					// sysconf
#include <sys/times.h>					// times
#include <time.h>

inline unsigned long long int Time() {
#if 1
    timespec ts;
    clock_gettime(
#if defined( __linux__ )
	 CLOCK_THREAD_CPUTIME_ID,
#elif defined( __freebsd__ )
	 CLOCK_PROF,
#elif defined( __solaris__ )
	 CLOCK_HIGHRES,
#else
    #error uC++ : internal error, unsupported architecture
#endif
	 &ts );
    return 1000000000LL * ts.tv_sec + ts.tv_nsec;
#endif

#if 0
    struct tms usage;
    long int usec_per_tck = 1000000 / sysconf(_SC_CLK_TCK);
    times( &usage );
    return ( usage.tms_utime + usage.tms_stime ) * usec_per_tck;
#endif

#if 0 // old BSD form
    struct rusage usage;
    getrusage( RUSAGE_SELF, &usage );
    return usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec +
	   usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;
#endif
} // Time
