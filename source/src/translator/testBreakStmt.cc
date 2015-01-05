_Mutex class fred {
  public:
    void mem() {
	int i;

      A: B: for ( ;; ) {
	  _Accept ( mem ) break A;
	  _Accept ( mem ) continue A;
      }

	Z : i += 1;
	goto Z;

      F: {
	    while (true) {
		i =+ 1;
	      if ( i < 5 ) break F;
	        goto F;
	    }
	    i += 1;
	}
      X: Y: while ( true ) {
	  i += 1;
	  if ( i > 5 ) continue X;
	  if ( i < 5 ) break X;
	  if ( i < 5 ) break Y;
	  break;
      }
      X1: Y1: do {
	  i += 1;
	  if ( i > 5 ) continue X1;
	  if ( i < 5 ) break X1;
	  if ( i < 5 ) break Y1;
	  break;
      } while ( true );
      XX: for ( ;; ) {
	YY: for ( ;; ) {
	  ZZ: for ( ;; ) {
	      i += 1;
	      if ( i > 5 ) continue XX;
	      if ( i > 5 ) break XX;
	      if ( i < 5 ) continue YY;
	      if ( i < 5 ) break YY;
	      if ( i < 5 ) continue ZZ;
	      if ( i < 5 ) break ZZ;
	      break;
	  }
	}
      }

      P: switch ( i ) {
	case 0:
	  i += 1;
	  break P;
	case 1:
	  switch ( i ) {
	    case 0:
	      break P;
	    default:
	      i += 1;
	  }
	default:
	  i += 1;
      }


#if 0
      W: W: for ( ;; ) { // g++ deals with this
	  break W;
      }
      L0:  L1:  L2:  L3:  L4:  L5:  L6:  L7:  L8:  L9:
      L10: L11: L12: L13: L14: L15: L16: L17: L18: L19:
      L20: L21: L22: L23: L24: L25: L26: L27: L28: L29:
      L31: L32: L33: L34:
	for ( ;; ) {
	}
	break Z;
      Q: if ( i > 5 ) {
	  i += 1;
	  break Q;
      } else
	  i += 1;

      PP: switch ( i ) {
	case 0:
	  i += 1;
	  continue PP;
	default:
	  i += 1;
      }
#endif
    }
};


// Local Variables: //
// compile-command: "../../bin/u++ testBreakStmt.cc" //
// End: //
