//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Jun Shih 1995
// 
// uBConditionEval.cc -- 
// 
// Author           : Jun Shih
// Created On       : Sat Nov 11 14:44:08 EST 1995
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Apr 30 21:53:17 2009
// Update Count     : 46
//
// This  library is free  software; you  can redistribute  it and/or  modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software  Foundation; either  version 2.1 of  the License, or  (at your
// option) any later version.
// 
// This library is distributed in the  hope that it will be useful, but WITHOUT
// ANY  WARRANTY;  without even  the  implied  warranty  of MERCHANTABILITY  or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
// 
// You should  have received a  copy of the  GNU Lesser General  Public License
// along  with this library.
// 


#define __U_KERNEL__
#include <uBConditionEval.h>
//#include <uDebug.h>


/******************************* uBConditionEval *****************************/


int uBConditionEval::eval_int( int which ) {
    assert( bp_cond.var[which].vtype == BreakpointCondition::INT || bp_cond.var[which].vtype == BreakpointCondition::PTR);

    switch ( bp_cond.var[which].atype ) {
      case BreakpointCondition::LOCAL:
	return *(int*) eval_address_local( which );
	
      case BreakpointCondition::STATIC:
	return *(int*) eval_address_static( which );

      case BreakpointCondition::REGISTER:
	return *(int*) eval_address_register( which );

      case BreakpointCondition::CONST:
	return (size_t) eval_address_const( which );

      default:
	assert( 0);
	return 0;
    }
} // uBConditionEval::eval_int


CodeAddress uBConditionEval::eval_address_local( int which ) {
    assert( bp_cond.var[which].atype == BreakpointCondition::LOCAL);
    //local address is the real fp + offset (usually negative)

    unsigned int prev_2fp;
#if defined( __sparc__ )
    prev_2fp = *(unsigned long*) ( *(unsigned long*)(bp_cond.fp + 4 * 14) + 4 * 14 );
#elif defined( __i386__ )
    prev_2fp = *(unsigned long*) ( *(unsigned int*) ((int) bp_cond.fp ));
#elif defined( __ia64__ )
    prev_2fp = 0;
#elif defined( __x86_64__ )
    prev_2fp = 0;
#else
    #error uC++ internal error : unsupported architecture
#endif

    // local address is the real fp + offset (usually negative)
    unsigned long address = ((unsigned long) prev_2fp + (int)bp_cond.var[which].offset );

#ifdef __U_DEBUG_H__
    uDebugPrt( " EVAL_ADDRESS_LOCAL: fp = 0x%x %d 0x%x\n", bp_cond.fp, bp_cond.var[which].offset, (unsigned int)bp_cond.fp + (int)bp_cond.var[which].offset );
    uDebugPrt( " EVAL_ADDRESS_LOCAL: 0x%x\n", address );
#endif // __U_DEBUG_H__

    if ( bp_cond.var[which].field_off ) {
	if ( bp_cond.var[which].field_off == -1) { // *p
	    bp_cond.var[which].field_off = 0;
	} // if
#ifdef __U_DEBUG_H__
	uDebugPrt( "address = 0x%x \n", *(long*)address );
#endif // __U_DEBUG_H__
	return (CodeAddress)(*(long*) address + bp_cond.var[which].field_off);
    } // if
    return (CodeAddress) address;
} // uBConditionEval::eval_address_local


CodeAddress uBConditionEval::eval_address_static( int which ) {
    assert( bp_cond.var[which].atype == BreakpointCondition::STATIC);

    if ( bp_cond.var[which].field_off == -1 ) { // *p
#ifdef __U_DEBUG_H__
	uDebugPrt( "address = 0x%x \n", bp_cond.var[which].offset );
	uDebugPrt( "address = %d \n", *(long*)bp_cond.var[which].offset );
	return (CodeAddress) *(long*) bp_cond.var[which].offset;
#endif // __U_DEBUG_H__
    } // if

    unsigned long address = bp_cond.var[which].offset + bp_cond.var[which].field_off;

    if ( bp_cond.var[which].field_off ) {
#ifdef __U_DEBUG_H__
	uDebugPrt( "address = 0x%x \n", *(long*)address );
#endif // __U_DEBUG_H__
	return (CodeAddress) address + bp_cond.var[which].field_off;
    } // if
    // offset is the address
    return (CodeAddress) address;
} // uBConditionEval::eval_address_static

CodeAddress uBConditionEval::eval_address_const( int which ) {
    assert( bp_cond.var[which].atype == BreakpointCondition::CONST);
    
    // offset is the value of the const
    return (CodeAddress) bp_cond.var[which].offset;
} // uBConditionEval::eval_address_const


CodeAddress uBConditionEval::eval_address_register( int which ) {
    assert( bp_cond.var[which].atype == BreakpointCondition::REGISTER);

    unsigned long address;

#if defined( __sparc__ )
    // registers are assumed to be on the stack, the layout of which can be found on 
    // page 195 "The SPARC Architecture Manual" version 8.

    unsigned long prev_fp = *(unsigned long*) (bp_cond.fp + 4 * 14);
#ifdef __U_DEBUG_H__
    uDebugPrt( "prev_fp:%p\n",prev_fp );
    for ( int i = 0; i < 25; i++) {
	int *addr = (int*) (prev_fp + (i*4));
	uDebugPrt( "addr:%9p i:%3d off:%4d val:%11d val:0x%9p\n",addr,i,bp_cond.var[which].offset,*addr, *addr );
    } // for
    for ( int i = 0; i < 25; i++) {
	int *addr = (int*) ( bp_cond.fp + ( i*4));
	uDebugPrt( "addr:%9p i:%3d off:%4d val:%11d val:0x%9p\n",addr,i,bp_cond.var[which].offset,*addr, *addr );
    } // for
#endif // __U_DEBUG_H__
    if ( (int)bp_cond.var[which].offset >= 16 && (int) bp_cond.var[which].offset < 32 ) { // local and in registers
	address = ((unsigned long) prev_fp + (long) (sizeof(long)  *((int)bp_cond.var[which].offset - 16)));
    } else if ( (int)bp_cond.var[which].offset >= 8 && (int) bp_cond.var[which].offset < 15) { // out register (r8 at 17th word)
	address = ((unsigned long) bp_cond.fp + (long) (sizeof(long)  *(bp_cond.var[which].offset)));
    } else {
	uAbort( "uBConditionEval::eval_address_register : internal error, failed to find global registers" );
    } // if
#elif defined( __i386__ )
    unsigned long prev_fp = *(unsigned int*) ((int) bp_cond.fp);
    address =  (unsigned long) prev_fp - (sizeof(long)  *(bp_cond.var[which].offset+1));
#ifdef __U_DEBUG_H__
    for ( int i = -10; i < 10; i++){
	unsigned long addr = (unsigned long) prev_fp + (long) (i * sizeof(long));
	uDebugPrt( "i:%d addr:%p val:%d %p\n",i,addr, *(int*)addr,*(int*)addr );
    } // for
#endif // __U_DEBUG_H__
#elif defined( __ia64__ )
    address = 0;
#elif defined( __x86_64__ )
    address = 0;
#else
    #error uC++ internal error : unsupported architecture
#endif

    if ( bp_cond.var[which].field_off ) {
	if ( bp_cond.var[which].field_off == -1) { // *p
	    bp_cond.var[which].field_off = 0;
	} // if
#ifdef __U_DEBUG_H__
	uDebugPrt( "address = 0x%x \n", *(long*)address);
#endif // __U_DEBUG_H__
	return (CodeAddress)(*(long*) address + bp_cond.var[which].field_off);
    } // if
    return (CodeAddress) address;
} // uBConditionEval::eval_address_register


uBConditionEval::uBConditionEval( ULThreadId ul_thread_id ) : ul_thread_id( ul_thread_id ) {
    bp_cond.Operator = BreakpointCondition::NOT_SET;
} // uBConditionEval::uBConditionEval

uBConditionEval::~uBConditionEval() {}


void uBConditionEval::setId( ULThreadId Id ) {
    ul_thread_id = Id;
} // uBConditionEval::setId


ULThreadId uBConditionEval::getId() {
    return ul_thread_id;
} // uBConditionEval::getId


void uBConditionEval::setFp( long fp_val ) {
    bp_cond.fp = fp_val;
} // uBConditionEval::setFp


void uBConditionEval::setSp( long sp_val ) {
    bp_cond.sp = sp_val;
} // uBConditionEval::setSp


long uBConditionEval::getFp() {
    return bp_cond.fp;
} // uBConditionEval::getFp


long uBConditionEval::getSp() {
    return bp_cond.sp;
} // uBConditionEval::getSp


BreakpointCondition::OperationType uBConditionEval::getOperator() {
    return bp_cond.Operator;
} // uBConditionEval::getOperator


BreakpointCondition &uBConditionEval::getBp_cond() {
    return bp_cond;
} // uBConditionEval::getBp_cond


int uBConditionEval::evaluate() {
#ifdef __U_DEBUG_H__
    uDebugPrt( "bp_cond.var[0].vtype is %d and bp_cond.var[1].vtype is %d\n",bp_cond.var[0].vtype,bp_cond.var[1].vtype );
#endif // __U_DEBUG_H__
    if ( bp_cond.var[0].vtype == BreakpointCondition::INVALID || bp_cond.var[1].vtype == BreakpointCondition::INVALID) {
	return 0;					// can not find address of one of them
    } // if

    assert( ( bp_cond.var[0].vtype == BreakpointCondition::INT && bp_cond.var[1].vtype == BreakpointCondition::INT ) ||
	    ( bp_cond.var[0].vtype == BreakpointCondition::PTR && bp_cond.var[1].vtype == BreakpointCondition::PTR ) );

    switch (bp_cond.Operator ) {
      case BreakpointCondition::EQUAL:
	return eval_int(0) == eval_int(1);
      case BreakpointCondition::NOT_EQUAL:
	return eval_int(0) != eval_int(1);
      case BreakpointCondition::GREATER_EQUAL:
	return eval_int(0) >= eval_int(1);
      case BreakpointCondition::GREATER:
	return eval_int(0) >  eval_int(1);
      case BreakpointCondition::LESS_EQUAL:
	return eval_int(0) <= eval_int(1);
      case BreakpointCondition::LESS:
	return eval_int(0) <  eval_int(1);
      default:
	assert(0);
	return 0;
    } // switch
} // uBConditionEval::evaluate


/****************************** uBConditionList ******************************/


uBConditionList::uBConditionList() {
} // uBConditionList::uBConditionList


uBConditionList::~uBConditionList() {
    uSeqIter<uBConditionEval> iter;
    uBConditionEval *bc_eval;

    for ( iter.over(bp_list ); iter >> bc_eval; ) {
	bp_list.remove(bc_eval );
	delete bc_eval;
    } // for
} // uBConditionList::uBConditionList


uBConditionEval *uBConditionList::search( ULThreadId ul_thread_id ) {
    uSeqIter<uBConditionEval> iter;
    uBConditionEval *bc_eval;

  if ( bp_list.empty() ) return NULL;

    for ( iter.over(bp_list ); iter >> bc_eval; ) {
	if ( bc_eval->getId() == ul_thread_id ) return bc_eval;
    } // for
    return NULL;
} //  uBConditionList::search


void uBConditionList::add( uBConditionEval *bc_eval ) {
    bp_list.add( bc_eval );
} // uBConditionList::add


bool uBConditionList::del( ULThreadId ul_thread_id ) {
    uBConditionEval *bc_eval = search(ul_thread_id );

  if ( ! bc_eval ) return false;
    bp_list.remove( bc_eval );
    delete bc_eval;
    return true;
} // uBConditionList::del


// Local Variables: //
// compile-command: "make install" //
// End: //
