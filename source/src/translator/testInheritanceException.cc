#if 0
#define MEM private : void mem0() {} public : void mem1() {} void mem2() {}
#else
#define MEM
#endif

struct strct { MEM };
class clss { MEM };
_Event event { MEM };
_Coroutine coroutine { MEM };
_Monitor monitor { MEM };
_Cormonitor cormonitor { MEM };
_Task task { MEM };

_Event event_struct1 : private strct { MEM };
_Event event_struct2 : protected strct { MEM };
_Event event_struct3 : public strct { MEM };

_Event event_class1 : private clss { MEM };
_Event event_class2 : protected clss { MEM };
_Event event_class3 : public clss { MEM };

_Event event_event3 : public event { MEM };


#if 0							// error checking
struct struct_event1 : private event { MEM };
struct struct_event2 : protected event { MEM };
struct struct_event3 : public event { MEM };

class class_event1 : private event { MEM };
class class_event2 : protected event { MEM };
class class_event3 : public event { MEM };

_Event event_event1 : private event { MEM };
_Event event_event2 : protected event { MEM };

_Event event_coroutine1 : private coroutine { MEM };
_Event event_coroutine2 : protected coroutine { MEM };
_Event event_coroutine3 : public coroutine { MEM };

_Event event_monitor1 : private monitor { MEM };
_Event event_monitor2 : protected monitor { MEM };
_Event event_monitor3 : public monitor { MEM };

_Event event_cormonitor1 : private cormonitor { MEM };
_Event event_cormonitor2 : protected cormonitor { MEM };
_Event event_cormonitor3 : public cormonitor { MEM };

_Event event_task1 : private task { MEM };
_Event event_task2 : protected task { MEM };
_Event event_task3 : public task { MEM };

// multiple inheritance

_Event mhevent_event_event3 : private event, private event { MEM };
_Event mhevent_event_event3 : protected event, protected event { MEM };
_Event mhevent_event_event3 : public event, public event { MEM };

_Event mhevent_coroutine_event3 : private coroutine, private event { MEM };
_Event mhevent_coroutine_event3 : protected event, protected coroutine { MEM };
_Event mhevent_coroutine_event3 : public coroutine, public event { MEM };

_Event mhevent_monitor_event3 : private monitor, private event { MEM };
_Event mhevent_monitor_event3 : protected event, protected monitor { MEM };
_Event mhevent_monitor_event3 : public monitor, public event { MEM };

_Event mhevent_cormonitor_event3 : private cormonitor, private event { MEM };
_Event mhevent_cormonitor_event3 : protected event, protected cormonitor { MEM };
_Event mhevent_cormonitor_event3 : public cormonitor, public event { MEM };

_Event mhevent_task_event3 : private task, private event { MEM };
_Event mhevent_task_event3 : protected event, protected task { MEM };
_Event mhevent_task_event3 : public task, public event { MEM };

// opposite direction

_Coroutine coroutine1 : private event { MEM };
_Coroutine coroutine2 : protected event { MEM };
_Coroutine coroutine3 : public event { MEM };

_Mutex class monitor1 : private event { MEM };
_Mutex class monitor2 : protected event { MEM };
_Mutex class monitor3 : public event { MEM };

_Mutex _Coroutine cormonitor1 : private event { MEM };
_Mutex _Coroutine cormonitor2 : protected event { MEM };
_Mutex _Coroutine cormonitor3 : public event { MEM };

_Task task1 : private event { MEM };
_Task task2 : protected event { MEM };
_Task task3 : public event { MEM };

// multiple inheritance

_Coroutine mhcoroutine1 : private coroutine, private event_struct1 { MEM };
_Coroutine mhcoroutine2 : protected event_struct1, protected coroutine { MEM };
_Coroutine mhcoroutine3 : public event_struct1, public coroutine { MEM };

_Mutex class mhmonitor1 : private monitor, private event_struct1 { MEM };
_Mutex class mhmonitor2 : protected event_struct1, protected monitor { MEM };
_Mutex class mhmonitor3 : public event_struct1, public monitor { MEM };

_Mutex _Coroutine mhcormonitor1 : private cormonitor, private event_struct1 { MEM };
_Mutex _Coroutine mhcormonitor2 : protected event_struct1, protected cormonitor { MEM };
_Mutex _Coroutine mhcormonitor3 : public event_struct1, public monitor, protected cormonitor { MEM };

_Task mhtask1 : private task, private event_struct1 { MEM };
_Task mhtask2 : protected event_struct1, protected task { MEM };
_Task mhtask3 : public event_struct1, public task { MEM };

#endif


// Local Variables: //
// compile-command: "../../bin/u++ testInheritanceException.cc" //
// End: //
