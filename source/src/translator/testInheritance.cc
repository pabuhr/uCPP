class err {
  public:
    _Mutex void mem();
};

_Mutex class jane;

_Mutex class jane;

_Mutex class jane : public err {
  public:
    _Nomutex void mem();
};


#if 0
#define MEM private : void mem0() {} public : void mem1() {} void mem2() {}
#else
#define MEM
#endif

class fred {
  public:
    _Mutex ~fred() {}
};

struct strct { MEM };
class clss { MEM };
_Coroutine coroutine { MEM };
typedef coroutine td_coroutine;
_Mutex class monitor { MEM };
typedef monitor td_monitor;
_Mutex _Coroutine cormonitor { MEM };
typedef cormonitor td_cormonitor;
_Task task { MEM };
typedef task td_task;


_Coroutine coroutine_struct1 : private strct { MEM };
_Coroutine coroutine_struct2 : protected strct { MEM };
_Coroutine coroutine_struct3 : public strct { MEM };

_Coroutine coroutine_class1 : private clss { MEM };
_Coroutine coroutine_class2 : protected clss { MEM };
_Coroutine coroutine_class3 : public clss { MEM };

_Coroutine coroutine_coroutine1 : private coroutine { MEM };
_Coroutine coroutine_coroutine2 : protected coroutine { MEM };
_Coroutine coroutine_coroutine3 : public coroutine { MEM };

_Coroutine coroutine_td_coroutine1 : private td_coroutine { MEM };
_Coroutine coroutine_td_coroutine2 : protected td_coroutine { MEM };
_Coroutine coroutine_td_coroutine3 : public td_coroutine { MEM };


_Mutex class monitor_struct1 : private strct { MEM };
_Mutex class monitor_struct2 : protected strct { MEM };
_Mutex class monitor_struct3 : public strct { MEM };

_Mutex class monitor_class1 : private clss { MEM };
_Mutex class monitor_class2 : protected clss { MEM };
_Mutex class monitor_class3 : public clss { MEM };

_Mutex class monitor_monitor1 : private monitor { MEM };
_Mutex class monitor_monitor2 : protected monitor { MEM };
_Mutex class monitor_monitor3 : public monitor { MEM };

_Mutex class monitor_td_monitor1 : private td_monitor { MEM };
_Mutex class monitor_td_monitor2 : protected td_monitor { MEM };
_Mutex class monitor_td_monitor3 : public td_monitor { MEM };


_Mutex _Coroutine cormonitor_struct1 : private strct { MEM };
_Mutex _Coroutine cormonitor_struct2 : protected strct { MEM };
_Mutex _Coroutine cormonitor_struct3 : public strct { MEM };

_Mutex _Coroutine cormonitor_class1 : private clss { MEM };
_Mutex _Coroutine cormonitor_class2 : protected clss { MEM };
_Mutex _Coroutine cormonitor_class3 : public clss { MEM };

_Mutex _Coroutine cormonitor_coroutine1 : private coroutine { MEM };
_Mutex _Coroutine cormonitor_coroutine2 : protected coroutine { MEM };
_Mutex _Coroutine cormonitor_coroutine3 : public coroutine { MEM };

_Mutex _Coroutine cormonitor_td_coroutine1 : private td_coroutine { MEM };
_Mutex _Coroutine cormonitor_td_coroutine2 : protected td_coroutine { MEM };
_Mutex _Coroutine cormonitor_td_coroutine3 : public td_coroutine { MEM };

_Mutex _Coroutine cormonitor_monitor1 : private monitor { MEM };
_Mutex _Coroutine cormonitor_monitor2 : protected monitor { MEM };
_Mutex _Coroutine cormonitor_monitor3 : public monitor { MEM };

_Mutex _Coroutine cormonitor_td_monitor1 : private td_monitor { MEM };
_Mutex _Coroutine cormonitor_td_monitor2 : protected td_monitor { MEM };
_Mutex _Coroutine cormonitor_td_monitor3 : public td_monitor { MEM };

_Mutex _Coroutine cormonitor_cormonitor1 : private cormonitor { MEM };
_Mutex _Coroutine cormonitor_cormonitor2 : protected cormonitor { MEM };
_Mutex _Coroutine cormonitor_cormonitor3 : public cormonitor { MEM };

_Mutex _Coroutine cormonitor_td_cormonitor1 : private td_cormonitor { MEM };
_Mutex _Coroutine cormonitor_td_cormonitor2 : protected td_cormonitor { MEM };
_Mutex _Coroutine cormonitor_td_cormonitor3 : public td_cormonitor { MEM };


_Task task_struct1 : private strct { MEM };
_Task task_struct2 : protected strct { MEM };
_Task task_struct3 : public strct { MEM };

_Task task_class1 : private clss { MEM };
_Task task_class2 : protected clss { MEM };
_Task task_class3 : public clss { MEM };

_Task task_monitor1 : private monitor { MEM };
_Task task_monitor2 : protected monitor { MEM };
_Task task_monitor3 : public monitor { MEM };

_Task task_monitor4 : public task_monitor3 { MEM };	// check additional level

_Task task_td_monitor1 : private td_monitor { MEM };
_Task task_td_monitor2 : protected td_monitor { MEM };
_Task task_td_monitor3 : public td_monitor { MEM };

_Task task_task1 : private task { MEM };
_Task task_task2 : protected task { MEM };
_Task task_task3 : public task { MEM };

_Task task_task4 : public task_task3 { MEM };		// check additional level

_Task task_td_task1 : private td_task { MEM };
_Task task_td_task2 : protected td_task { MEM };
_Task task_td_task3 : public td_task { MEM };

// multiple inheritance

_Coroutine coroutine_struct_class1 : private strct, private clss { MEM };
_Coroutine coroutine_struct_class2 : protected strct, protected clss { MEM };
_Coroutine coroutine_struct_class3 : public strct, protected clss { MEM };

_Mutex class monitor_struct_class1 : private strct, private clss { MEM };
_Mutex class monitor_struct_class2 : protected strct, protected clss { MEM };
_Mutex class monitor_struct_class3 : public strct, protected clss { MEM };

_Mutex _Coroutine cormonitor_struct_class1 : private strct, private clss { MEM };
_Mutex _Coroutine cormonitor_struct_class2 : protected strct, protected clss { MEM };
_Mutex _Coroutine cormonitor_struct_class3 : public strct, protected clss { MEM };

_Task task_struct_class1 : private strct, private clss { MEM };
_Task task_struct_class2 : protected strct, protected clss { MEM };
_Task task_struct_class3 : public strct, protected clss { MEM };


#if 0							// error checking
struct struct_coroutine1 : private coroutine { MEM };
struct struct_coroutine2 : protected coroutine { MEM };
struct struct_coroutine3 : public coroutine { MEM };

struct struct_td_coroutine1 : private td_coroutine { MEM };
struct struct_td_coroutine2 : protected td_coroutine { MEM };
struct struct_td_coroutine3 : public td_coroutine { MEM };

struct struct_monitor1 : private monitor { MEM };
struct struct_monitor2 : protected monitor { MEM };
struct struct_monitor3 : public monitor { MEM };

struct struct_td_monitor1 : private td_monitor { MEM };
struct struct_td_monitor2 : protected td_monitor { MEM };
struct struct_td_monitor3 : public td_monitor { MEM };

struct struct_cormonitor1 : private cormonitor { MEM };
struct struct_cormonitor2 : protected cormonitor { MEM };
struct struct_cormonitor3 : public cormonitor { MEM };

struct struct_td_cormonitor1 : private td_cormonitor { MEM };
struct struct_td_cormonitor2 : protected td_cormonitor { MEM };
struct struct_td_cormonitor3 : public td_cormonitor { MEM };

struct struct_task1 : private task { MEM };
struct struct_task2 : protected task { MEM };
struct struct_task3 : public task { MEM };

struct struct_td_task1 : private td_task { MEM };
struct struct_td_task2 : protected td_task { MEM };
struct struct_td_task3 : public td_task { MEM };


class class_coroutine1 : private coroutine { MEM };
class class_coroutine2 : protected coroutine { MEM };
class class_coroutine3 : public coroutine { MEM };

class class_td_coroutine1 : private td_coroutine { MEM };
class class_td_coroutine2 : protected td_coroutine { MEM };
class class_td_coroutine3 : public td_coroutine { MEM };

class class_monitor1 : private monitor { MEM };
class class_monitor2 : protected monitor { MEM };
class class_monitor3 : public monitor { MEM };

class class_td_monitor1 : private td_monitor { MEM };
class class_td_monitor2 : protected td_monitor { MEM };
class class_td_monitor3 : public td_monitor { MEM };

class class_cormonitor1 : private cormonitor { MEM };
class class_cormonitor2 : protected cormonitor { MEM };
class class_cormonitor3 : public cormonitor { MEM };

class class_td_cormonitor1 : private td_cormonitor { MEM };
class class_td_cormonitor2 : protected td_cormonitor { MEM };
class class_td_cormonitor3 : public td_cormonitor { MEM };

class class_task1 : private task { MEM };
class class_task2 : protected task { MEM };
class class_task3 : public task { MEM };

class class_td_task1 : private td_task { MEM };
class class_td_task2 : protected td_task { MEM };
class class_td_task3 : public td_task { MEM };


_Coroutine coroutine_monitor1 : private monitor { MEM };
_Coroutine coroutine_monitor2 : protected monitor { MEM };
_Coroutine coroutine_monitor3 : public monitor { MEM };

_Coroutine coroutine_td_monitor1 : private td_monitor { MEM };
_Coroutine coroutine_td_monitor2 : protected td_monitor { MEM };
_Coroutine coroutine_td_monitor3 : public td_monitor { MEM };

_Coroutine coroutine_cormonitor1 : private cormonitor { MEM };
_Coroutine coroutine_cormonitor2 : protected cormonitor { MEM };
_Coroutine coroutine_cormonitor3 : public cormonitor { MEM };

_Coroutine coroutine_td_cormonitor1 : private td_cormonitor { MEM };
_Coroutine coroutine_td_cormonitor2 : protected td_cormonitor { MEM };
_Coroutine coroutine_td_cormonitor3 : public td_cormonitor { MEM };

_Coroutine coroutine_task1 : private task { MEM };
_Coroutine coroutine_task2 : protected task { MEM };
_Coroutine coroutine_task3 : public task { MEM };

_Coroutine coroutine_td_task1 : private td_task { MEM };
_Coroutine coroutine_td_task2 : protected td_task { MEM };
_Coroutine coroutine_td_task3 : public td_task { MEM };


_Mutex class monitor_coroutine1 : private coroutine { MEM };
_Mutex class monitor_coroutine2 : protected coroutine { MEM };
_Mutex class monitor_coroutine3 : public coroutine { MEM };

_Mutex class monitor_cormonitor1 : private cormonitor { MEM };
_Mutex class monitor_cormonitor2 : protected cormonitor { MEM };
_Mutex class monitor_cormonitor3 : public cormonitor { MEM };

_Mutex class monitor_task1 : private task { MEM };
_Mutex class monitor_task2 : protected task { MEM };
_Mutex class monitor_task3 : public task { MEM };


_Mutex _Coroutine cormonitor_task1 : private task { MEM };
_Mutex _Coroutine cormonitor_task2 : protected task { MEM };
_Mutex _Coroutine cormonitor_task3 : public task { MEM };


_Task task_coroutine1 : private coroutine { MEM };
_Task task_coroutine2 : protected coroutine { MEM };
_Task task_coroutine3 : public coroutine { MEM };

_Task task_cormonitor1 : private cormonitor { MEM };
_Task task_cormonitor2 : protected cormonitor { MEM };
_Task task_cormonitor3 : public cormonitor { MEM };


// multiple inheritance

_Coroutine mhcoroutine1 : private coroutine_struct1, private coroutine_struct2 { MEM };
_Coroutine mhcoroutine2 : protected coroutine_struct1, protected monitor_struct1 { MEM };
_Coroutine mhcoroutine3 : public cormonitor_struct1, public coroutine_struct1 { MEM };

_Mutex class mhmonitor1 : private monitor_struct1, private coroutine_struct1 { MEM };
_Mutex class mhmonitor2 : protected monitor_struct1, protected cormonitor_struct1 { MEM };
_Mutex class mhmonitor3 : public cormonitor_struct1, public monitor_struct1 { MEM };

_Mutex _Coroutine mhcormonitor1 : private cormonitor_struct1, private task_struct1 { MEM };
_Mutex _Coroutine mhcormonitor2 : protected cormonitor_struct1, protected coroutine_struct1 { MEM };
_Mutex _Coroutine mhcormonitor3 : public task_struct1, public monitor_struct1, protected cormonitor_struct1 { MEM };

_Task mhtask1 : private task_struct1, private coroutine_struct1 { MEM };
_Task mhtask2 : protected cormonitor_struct1, protected task_struct1 { MEM };
_Task mhtask3 : public monitor_struct1, public task_struct1 { MEM };
#endif


// Local Variables: //
// compile-command: "../../bin/u++ testInheritance.cc" //
// End: //
