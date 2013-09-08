//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// DatingTask.cc -- Exchanging Values Between Tasks
// 
// Author           : Peter A. Buhr
// Created On       : Fri Jul 15 16:25:41 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Mar 25 11:46:39 2010
// Update Count     : 24
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;
#include <iomanip>
using std::setw;

_Task DatingService {
	int GirlPhoneNo, BoyPhoneNo;
	void main();
  public:
	DatingService() {
		GirlPhoneNo = BoyPhoneNo = -1;
	}; // DatingService::DatingService
	int Girl( int PhoneNo );
	int Boy( int PhoneNo );
}; // DatingService

void DatingService::main() {
	for ( ;; ) {
		_Accept( ~DatingService )
			break;
		or _Accept( Girl );
		or _Accept( Boy );
		// do other work
	} // for
} // DatingService::main

int DatingService::Girl( int PhoneNo ) {
	GirlPhoneNo = PhoneNo;
	if ( BoyPhoneNo == -1 ) _Accept( Boy );
	int temp = BoyPhoneNo;
	BoyPhoneNo = -1;
	return temp;
} // DatingService::Girl

int DatingService::Boy( int PhoneNo ) {
	BoyPhoneNo = PhoneNo;
	if ( GirlPhoneNo == -1 ) _Accept( Girl );
	int temp = GirlPhoneNo;
	GirlPhoneNo = -1;
	return temp;
} // DatingService::Boy

_Task Girl {
	DatingService &TheExchange;

	void main() {
		yield( rand() % 100 );							// don't all start at the same time
		int PhoneNo = rand() % 10000000;
		int partner = TheExchange.Girl( PhoneNo );
		osacquire( cout ) << "Girl:" << setw(8) << &uThisTask() << " at " << setw(8) << PhoneNo
			 << " is dating Boy  at " << setw(8) << partner << endl;
	} // main
  public:
	Girl( DatingService &TheExchange ) : TheExchange( TheExchange ) {
	} // Girl
}; // Girl

_Task Boy {
	DatingService &TheExchange;

	void main() {
		yield( rand() % 100 );							// don't all start at the same time
		int PhoneNo = rand() % 10000000;
		int partner = TheExchange.Boy( PhoneNo );
		osacquire( cout ) << " Boy:" << setw(8) << &uThisTask() << " at " << setw(8) << PhoneNo
			<< " is dating Girl at " << setw(8) << partner << endl;
	} // main
  public:
	Boy( DatingService &TheExchange ) : TheExchange( TheExchange ) {
	} // Boy
}; // Boy


void uMain::main() {
	const int NoOfGirls = 20;
	const int NoOfBoys = 20;

	DatingService TheExchange;
	Girl *girls;
	Boy  *boys;

	girls = new Girl[NoOfGirls]( TheExchange );
	boys  = new Boy[NoOfBoys]( TheExchange );

	delete [] girls;
	delete [] boys;

	osacquire( cout ) << "successful completion" << endl;
} // uMain::main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ DatingTask.cc" //
// End: //
