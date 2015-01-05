//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// CardGame.cc -- Play a game of cards. The game consists of each player taking a number of cards from a deck and
//     passing the deck to the player on the left.  A player must take at least one card and no more then a certain
//     maximum. The player that takes the last cards is the winner.
// 
// Author           : Peter A. Buhr
// Created On       : Wed Jun 23 14:29:26 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 18 11:03:05 2010
// Update Count     : 43
// 

#include <iostream>
using std::cout;
using std::endl;

int RandBetween( int low, int high ) {
    return rand() % ( high - low + 1 ) + low;
} // RandBetween

_Task player {
    const int MinCardsTaken = 1, MaxCardsTaken = 5;
    player *partner;
    int deck;

    int MakePlay( int RemainingCards ) {
	int passing, took;

	if ( RemainingCards <= MaxCardsTaken ) {
	    took = RemainingCards;
	    passing = 0;
	} else {
	    took = RandBetween( MinCardsTaken, MaxCardsTaken ); // random no. between MinCardsTaken & MaxCardsTaken
	    passing = RemainingCards - took;
	} // if
	cout << "task:" << &uThisTask() << " took " << took << " cards from " << RemainingCards
	      << " passing " << passing << " to the left" << endl;
	return passing;
    } // player::MakePlay
  public:
    player() {
    } // player::player

    void start( player *partner ) {
	player::partner = partner;
    } // player::start

    void play( int deck ) {
	player::deck = deck;
    } // player::play
  private:
    void main() {
	_Accept( start );				// obtain partner

	for ( ;; ) {
	    _Accept( play );				// obtain deck of cards

	    if ( deck == 0 ) {				// end of game ?
		partner->play( 0 );			// tell parnter about the end of the game
		break;
	    } // exit
	    deck = MakePlay( deck );			// make a play
	    if ( deck == 0 ) {				// did I win ?
		cout << "task:" << &uThisTask() << " WON" << endl;
		partner->play( deck );			// tell parnter about the end of the game
		_Accept( play );			// make sure all players heard
		cout << "all players ended" << endl;
		break;
	    } // exit
	    partner->play( deck );			// pass remaining cards to player on the left
	} // for
    } // player::main
}; // player

void uMain::main() {
    const int MaxNoGames = 5, MaxNoPlayers = 8, MinNoCards = 20, MaxNoCards = 40;
    int NoOfGames, NoOfPlayers, NoOfCards;
    int i;
    
    for ( NoOfGames = 1; NoOfGames <= MaxNoGames; NoOfGames += 1 ) {
	NoOfPlayers = RandBetween( 2, MaxNoPlayers );	// random no. between 2 & MaxNoPlayers
	cout << "The number of players is:" << NoOfPlayers << endl;
	
	NoOfCards = RandBetween( MinNoCards, MaxNoCards ); // random no. between MinNoCards & MaxNoCards
	cout << "The number of cards is:" << NoOfCards << endl;
	
	{
	    player players[NoOfPlayers];		// start players

	    for ( i = 0; i < NoOfPlayers - 1; i += 1 ) { // tell each player who its partner is
		players[i].start( &players[i + 1] );
	    } // for
	    players[i].start( &players[0] );

	    players[0].play( NoOfCards );		// the dealer starts the game
	}
	cout << endl << endl;				// whitespace between games
    } // for
    cout << "successful completion" << endl;
} // uMain::main

// Local Variables: //
// compile-command: "u++ BinaryInsertionSort.cc" //
// End: //
