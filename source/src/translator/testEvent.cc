template<class EntryList, class MemberQueue> struct uSerialTemp  {
    _Event uFailure;
}; // uSerialTemp

template<> class uSerialTemp<int,int> {
  public:
    _Event uFailure;
}; // uSerialTemp

_Event uSerialTemp<int,int>::uFailure {
};

class uSerial1 : public uSerialTemp<int, int> {
};

class uCondition1 {
    _Event uWaitingFailure;
};

_Event uCondition1::uWaitingFailure : public uSerial1::uFailure {
}; // uCondition::uWaitingFailure

_Event uIOFailure1 {
};

class uFileAccOps {
    _Mutex int me_write( char *buf, int len );
  public:
    _Event uFailure;
};

_Event uFileAccOps::uFailure : public uIOFailure1 {
};

class uFile {
  public:
    _Event uFailure;
};

_Event uFile::uFailure : public uIOFailure1 {
};

_Mutex class uFileAccess : public uFileAccOps {
  public:
    _Event uFailure;
};

_Event uFileAccess::uFailure : public uIOFailure1 {
};


_Event R {};
void h( R & ) { _Throw R(); }

void uMain::main() {
	try <R,h> {
		_Resume R();
	} catch( R &e ) {
	}
}


// Local Variables: //
// compile-command: "../../bin/u++ testEvent.cc" //
// End: //
