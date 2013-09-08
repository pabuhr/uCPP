//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// DatingTrad.cc -- Exchanging Values Between Tasks using blocking signal
// 
// Author           : Peter A. Buhr
// Created On       : Thu Aug  2 11:47:55 1990
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Mar 25 11:47:42 2010
// Update Count     : 73
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;
#include <iomanip>
using std::setw;

_Monitor DatingService {
	int GirlPhoneNo, BoyPhoneNo;
	uCondition GirlWaiting, BoyWaiting;
  public:
	int Girl( int PhoneNo ) {
		if ( BoyWaiting.empty() ) {
			GirlWaiting.wait();
			GirlPhoneNo = PhoneNo;
		} else {
			GirlPhoneNo = PhoneNo;
			BoyWaiting.signalBlock();
		} // if
		return BoyPhoneNo;
	} // DatingService::Girl

	int Boy( int PhoneNo ) {
		if ( GirlWaiting.empty() ) {
			BoyWaiting.wait();
			BoyPhoneNo = PhoneNo;
		} else {
			BoyPhoneNo = PhoneNo;
			GirlWaiting.signalBlock();
		} // if
		return GirlPhoneNo;
	} // DatingService::Boy
}; // DatingService

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
// compile-command: "u++ DatingTrad.cc" //
// End: //
