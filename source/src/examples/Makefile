## Define the appropriate configuration variables.

PACKAGE = /u0/usystem/software/u++-6.1.0

## Include the architecture dependent definitions.

include ${PACKAGE}/CONFIG

## Define the path names of important directories.

SRCDIR = ${PACKAGE}/src/examples

ifneq (${CPP11},)
	CCFLAGS += -std=${CPP11}
endif

ifneq (${TCPU},ia64)			# gcc does not support -m64 flag ?
	CCFLAGS += -m${WORDSIZE}
endif

HOSTNAME = ${shell hostname}
ifeq (${HOSTNAME},plg2)
    CCFLAGS += -O2 -Wall -Wno-unused-label #
else
    CCFLAGS += -O2 #
endif

ifdef ALLOCATOR
ALLOCFLAGS += -DALLOCATOR=${ALLOCATOR}
CCFLAGS += -uAlloc${ALLOCATOR}
endif

.SILENT : all abortexit bench allocation features pthread EHM realtime multiprocessor

all : bench allocation features cobegin timeout pthread EHM realtime multiprocessor

errors : ownership abortexit

ownership :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	for filename in Ownership1 Ownership2 ; do \
		for ccflags in "" $${multi+"-multi"} ; do \
			${INSTALLBINDIR}/u++ ${CCFLAGS} $${ccflags} $${filename}.cc ; \
			./a.out ; \
		done ; \
	done ; \
	rm -f a.out core core.* ;

abortexit :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	if [ ${TOS} = freebsd ] ; then \
		pscmd="ps -U $${USER}" ; \
	else \
		pscmd="ps -u$${USER}" ; \
	fi ; \
	for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
		${INSTALLBINDIR}/u++ ${CCFLAGS} $${ccflags} AbortExit.cc ; \
		echo "************************** 0 Killed by SIGTERM **************************" ; \
		./a.out 0 & \
		sleep 1 ; \
		kill $$! ; \
		sleep 1 ; \
		$$pscmd | grep a.out ; \
		echo "************************** 1 Exit **************************" ; \
		./a.out 1 ; \
		echo "return code is $$? should be 1" ; \
		sleep 1 ; \
		$$pscmd | grep a.out ; \
		echo "************************** 2 uAbort **************************" ; \
		./a.out 2 ; \
		echo "return code is $$?" ; \
		sleep 1 ; \
		$$pscmd | grep a.out ; \
		echo "************************** 3 Child killed by SIGKILL **************************" ; \
		./a.out 3 ; \
		echo "return code is $$?" ; \
		sleep 1 ; \
		$$pscmd | grep a.out ; \
		echo "************************** 4 abort **************************" ; \
		./a.out 4 ; \
		echo "return code is $$?" ; \
		sleep 1 ; \
		$$pscmd | grep a.out ; \
		echo "************************** 5 Assertion failure **************************" ; \
		./a.out 5 ; \
		echo "return code is $$?" ; \
		sleep 1 ; \
		$$pscmd | grep a.out ; \
		echo "************************** 6 Return from uMain::main **************************" ; \
		./a.out 6 ; \
		echo "return code is $$? should be 6" ; \
		sleep 1 ; \
		$$pscmd | grep a.out ; \
		echo "************************** 7 pthread_shutdown **************************" ; \
		./a.out 7 ; \
		echo "return code is $$? should be 7" ; \
		sleep 1 ; \
		$$pscmd | grep a.out ; \
	done ; \
	rm -f a.out core core.* ;

bench :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	for filename in Bench ; do \
		for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
			${INSTALLBINDIR}/u++ ${CCFLAGS} $${ccflags} $${filename}.cc -lrt ; \
			./a.out ; \
		done ; \
	done ; \
	rm -f ./a.out ;

allocation :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	for filename in Allocation ; do \
		for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
			${INSTALLBINDIR}/u++ ${ALLOCFLAGS} ${CCFLAGS} $${ccflags} $${filename}.cc ; \
			./a.out ; \
		done ; \
	done ; \
	rm -f ./a.out ;

features :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	for filename in FloatTest CorFullProdCons CorFullProdConsStack BinaryInsertionSort Merger Locks Accept MonAcceptBB MonConditionBB SemaphoreBB TaskAcceptBB TaskConditionBB DeleteProcessor Sleep Atomic Migrate Migrate2 ; do \
		for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
			${INSTALLBINDIR}/u++ ${CCFLAGS} $${ccflags} $${filename}.cc ; \
			./a.out ; \
		done ; \
	done ; \
	rm -f ./a.out ;

cobegin :
	# needs -std=c++1y
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	tmpname="${TMPDIR}/uC_tmp$${$$}.cc" ; \
	echo 'int main() {}' > $${tmpname} ; \
	if ${CCAPP} -std=c++1y $${tmpname} > /dev/null 2>&1 ; then \
		for filename in Cobegin ; do \
			for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
				${INSTALLBINDIR}/u++ -std=c++1y ${CCFLAGS} $${ccflags} $${filename}.cc ; \
				./a.out ; \
			done ; \
		done ; \
	fi ; \
	rm -f ./a.out $${tmpname} ;

timeout :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	for filename in TimeSlice TimeoutAccept TimeoutCondLock TimeoutSem TimeoutProcessor TimeoutPoke ; do \
		for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
			${INSTALLBINDIR}/u++ ${CCFLAGS} $${ccflags} $${filename}.cc ; \
			./a.out ; \
		done ; \
	done ; \
	rm -f ./a.out ;

pthread :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	for filename in Pthread TimeoutPthreadsCond ; do \
		for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
			${INSTALLBINDIR}/u++ ${CCFLAGS} $${ccflags} $${filename}.cc ; \
			./a.out ; \
		done ; \
	done ; \
	rm -f ./a.out ;

EHM :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	for filename in EHM1 EHM2 EHM3 EHM4 EHM5 EHM6 EHM7 EHM8 EHM9 EHM10 CRAII UncaughtException ; do \
		for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
			${INSTALLBINDIR}/u++ ${CCFLAGS} $${ccflags} $${filename}.cc ; \
			./a.out ; \
		done ; \
	done ; \
	rm -f ./a.out ;

realtime :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	for filename in PeriodicTaskTest PeriodicTaskTest1 PeriodicTaskTestStatic RealTimePhilosophers RealTimePhilosophers1 RealTimePhilosophersStatic Disinherit Disinherit1 DisinheritStatic Disinherit1Static ; do \
		for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
			${INSTALLBINDIR}/u++ ${CCFLAGS} $${ccflags} $${filename}.cc ; \
			time -p ./a.out ; \
		done ; \
	done ; \
	rm -f ./a.out ;

multiprocessor :
	set -x ; \
	if [ ${MULTI} = TRUE ] ; then \
		multi=${MULTI} ; \
	fi ; \
	${INSTALLBINDIR}/u++ ${CCFLAGS} uCalibrate.cc -lrt ; \
	./a.out > uCalibrate.h ; \
	for ccflags in "" "-nodebug" $${multi+"-multi"} $${multi+"-multi -nodebug"} ; do \
		${INSTALLBINDIR}/u++ ${CCFLAGS} $${ccflags} Sim.cc ; \
		time -p ./a.out  1 100 500000 ; \
		time -p ./a.out  2 100 500000 ; \
		time -p ./a.out  4 100 500000 ; \
		time -p ./a.out  8 100 500000 ; \
		time -p ./a.out 16 100 500000 ; \
	done ; \
	rm -f ./a.out ;


## Local Variables: ##
## compile-command: "make install" ##
## End: ##
