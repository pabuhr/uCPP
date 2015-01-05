// Mutex qualifiers


class nomutex_class1;					// basic forward declarations
_Coroutine nomutex_coroutine1;
_Task mutex_task1;

class nomutex_class1;					// repeat basic forward declarations
_Coroutine nomutex_coroutine1;
_Task mutex_task1;

class nomutex_class1 {					// actual declarations, forward must match
  public:
    void mem() {}
};
_Coroutine nomutex_coroutine1 {
  public:
    void mem() {}
};
_Task mutex_task1 {
  public:
    void mem() {}
};

class nomutex_class1;					// repeat basic forward declarations
_Coroutine nomutex_coroutine1;
_Task mutex_task1;


class mutex_class;					// basic forward declarations
_Coroutine mutex_coroutine;
_Task nomutex_task1;

_Mutex class mutex_class;				// add mutex qualifier
_Mutex _Coroutine mutex_coroutine;
_Nomutex _Task nomutex_task1;

class mutex_class;					// repeat basic forward declarations
_Coroutine mutex_coroutine;
_Task nomutex_task1;

_Mutex class mutex_class {				// actual declarations, forward must match
  public:
    void mem() {}
};
_Mutex _Coroutine mutex_coroutine {
  public:
    void mem() {}
};
_Nomutex _Task nomutex_task1 {
  public:
    void mem() {}
};

class mutex_class;					// repeat basic forward declarations
_Coroutine mutex_coroutine;
_Task nomutex_task1;

_Mutex class mutex_class;				// add mutex qualifier
_Mutex _Coroutine mutex_coroutine;
_Nomutex _Task nomutex_task1;

class mutex_class;					// repeat basic forward declarations
_Coroutine mutex_coroutine;
_Task nomutex_task1;


_Mutex class over_mutex_class1 {
    _Mutex void mem(int) {}
  public:
    void mem(double);
};
void over_mutex_class1::mem(double) {}

_Mutex _Coroutine over_mutex_coroutine1 {
    _Mutex void mem(int) {}
  public:
    void mem(double);
};
void over_mutex_coroutine1::mem(double) {}

_Task over_mutex_task1 {
    _Mutex void mem(int) {}
  public:
    void mem(double);
};
void over_mutex_task1::mem(double) {}


#if defined( ERRORS )
_Mutex class mutex_class2;				// add mutex qualifier
_Mutex _Coroutine mutex_coroutine2;
_Nomutex _Task nomutex_task2;

class mutex_class2 {					// actual declarations, forward must match
  public:
    void mem() {}
};
_Coroutine mutex_coroutine2 {
  public:
    void mem() {}
};
_Task nomutex_task2 {
  public:
    void mem() {}
};
#endif // ERRORS


#if defined( ERRORS )
class mutex_class3 {					// actual declarations, forward must match
  public:
    void mem() {}
};
_Coroutine mutex_coroutine3 {
  public:
    void mem() {}
};
_Task nomutex_task3 {
  public:
    void mem() {}
};

_Mutex class mutex_class3;				// add mutex qualifier
_Mutex _Coroutine mutex_coroutine3;
_Nomutex _Task nomutex_task3;
#endif // ERRORS


#if defined( ERRORS )
_Mutex class fwd_mutex_class;
_Nomutex class fwd_mutex_class;

_Mutex _Coroutine fwd_mutex_coroutine;
_Nomutex _Coroutine fwd_mutex_coroutine;

_Mutex _Task fwd_mutex_task;
_Nomutex _Task fwd_mutex_task;

_Nomutex class fwd_nomutex_class;
_Mutex class fwd_nomutex_class;

_Nomutex _Coroutine fwd_nomutex_coroutine;
_Mutex _Coroutine fwd_nomutex_coroutine;

_Nomutex _Task fwd_nomutex_task;
_Mutex _Task fwd_nomutex_task;
#endif // ERRORS


#if defined( ERRORS )
_Mutex class over_mutex_class2 {
    void mem(int) {}
  public:
    void mem(double);
};
void over_mutex_class2::mem(double) {}

_Mutex _Coroutine over_mutex_coroutine2 {
    void mem(int) {}
  public:
    void mem(double);
};
void over_mutex_coroutine2::mem(double) {}

_Task over_mutex_task2 {
    void mem(int) {}
  public:
    void mem(double);
};
void over_mutex_task2::mem(double) {}
#endif // ERRORS


// Mutex parameters


class P : public uBasePIQ {
  public:
    int getHighestPriority() { return 3; }
};
class X : public uBasePrioritySeq {};
class Y : public uBasePrioritySeq {};

_Mutex<X,Y> _Task<P> T1 {
  public:
    void mem() {}
};

_Nomutex<X,Y> _Task<P> T2 {
  public:
    void mem() {}
};

_Mutex<X,Y> _Task T3 {
  public:
    void mem() {}
};

_Nomutex<X,Y> _Task T4 {
  public:
    void mem() {}
};

_Mutex _Task<P> T5 {
  public:
    void mem() {}
};

_Nomutex _Task<P> T6 {
  public:
    void mem() {}
};

_Task<P> T7 {
  public:
    void mem() {}
};

_Task T8 {
  public:
    void mem() {}
};

_Nomutex _Task T9 {
  public:
    void mem() {}
};

_Task TD1 : public T1 {
  public:
    void mem() {}
};


_Mutex<X,Y> class M1 {
  public:
    void mem() {}
};

_Mutex class M2 {
  public:
    void mem() {}
};

_Nomutex class M3 {
  public:
    void mem() {}
};

_Nomutex<X,Y> class M4 {
  public:
    _Mutex void mem() {}
};


#if defined( ERRORS )
_Mutex<X2,Y2> _Task<P2> T10 : public T1 {
  public:
    void mem() {}
};

_Nomutex class M11 : public M1 {
  public:
    void mem() {}
    void mem1() {}
};

_Mutex<X,Y> class M12 : public M11 {
  public:
    void mem() {}
};


_Mutex<,> _Task T11 {
  public:
    void mem() {}
};

_Mutex _Task<,> T12 {
  public:
    void mem() {}
};


_Nomutex<X,Y> class M5 {
  public:
    void mem() {}
};

_Nomutex<X,Y> struct M6 {
  public:
    void mem() {}
};

_Nomutex<X,Y> union M7 {
  public:
    void mem() {}
};
#endif // ERRORS


// Local Variables: //
// compile-command: "../../bin/u++ testMutexTaskParms.cc" //
// End: //
