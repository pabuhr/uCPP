//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Salman Ahmed and Peter A. Buhr 1996
// 
// Tree.cc -- Based on the TOPLAS paper 18(1): Iteration Abstraction in Sather
// 
// Author           : Salman Ahmed
// Created On       : Mon Jun 24 13:28:50 1996
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 18 11:14:11 2010
// Update Count     : 319
// 


#include <iostream>
using std::ostream;
using std::cout;
using std::endl;


// Abstract tree node object from which all tree nodes must inherit.
// Concrete type must provide operators:
//     bool operator==( Treeable, Treeable );
//     bool operator<( Treeable op1, Treeable op2 );
//     bool operator>( Treeable op1, Treeable op2 );
//     ostream &operator<<( ostream &os, Treeable &op );

class Treeable {
    friend class TFriend;

    Treeable *leftChild, *rightChild;
  public:
    Treeable() { leftChild = rightChild = NULL; }
}; // Treeable


// TFriend and its descendants have access to Treeable::left and Treeable::right so that direct access to a tree node is
// abstracted and encapsulated

class TFriend {
  protected:
    Treeable *&left( Treeable *tp ) const {
	return tp->leftChild;
    } // TFriend::left

    Treeable *&right( Treeable *tp ) const {
	return tp->rightChild;
    } // TFriend::right
}; // TFriend


// A template tree node object representing a binary search tree of (template) tree nodes.  It has methods to insert and
// remove a new node in the tree.

template <class T> class Tree : protected TFriend {
  protected:
    T *root;

    // Recursively search and insert a new node in a binary search tree object, maintaining binary search tree property.
    // Duplicate nodes are not inserted into the tree, and are just ignored.

    void insertNode( T *&rootNode, T *&newNode ) {
	if ( ! rootNode ) {
	    rootNode = newNode;
	} else if ( *newNode < *rootNode ) {
	    insertNode( (T *&)left( rootNode ), newNode );
	} else if ( *newNode > *rootNode ) {
	    insertNode( (T *&)right( rootNode ), newNode );
	} else if ( *newNode == *rootNode ) {
	    // do nothing!
	} else {
	    uAbort();
	} // if
    } // insertNode
  public:
    Tree() { root = NULL; }
    ~Tree();

    T *top() const {					// root node of the tree
	return root;
    } // Tree::top

    void insert( T *newNode ) {				// insert a node into the tree
	insertNode( root, newNode );
    } // Tree::insert

    void remove( T *newNode ) {				// remove a node from the tree
	// balanced removal: needs writing
    } // Tree::remove
}; // Tree


// Abstract template tree iterator coroutine. It uses coroutine properties to generate a sequence of the type T over a
// tree of such a type.

template <class T> _Coroutine uTreeIter : protected TFriend {
  protected:
    const Tree<T> *tree;
    T *curr;
    
    virtual void findNextNode( T *node ) = 0;

    void main () {
	for ( ;; ) {
	    findNextNode( curr );
	    curr = NULL;				// traversal finished so set curr to null
	    suspend();
	} // for
    } // uTreeIter::main

    void init( const Tree<T> &t ) {
	tree = &t;
	curr = t.top();
    } // uTreeIter::init
  public:
    uTreeIter() {
	tree = NULL;
	curr = NULL;
    } // uTreeIter::uTreeIter

    uTreeIter( const Tree<T> &t ) {
	init( t );
    } // uTreeIter::uTreeIter

    virtual void over( const Tree<T> &t ) {
	init( t );
    } // uTreeIter::over

    virtual bool operator>>( T *&tp ) {
	if ( curr != NULL ) resume();			// protect against calls after traversal ends
	tp = curr;
	return curr != NULL;
    } // uTreeIter::>>
}; // uTreeIter


// A preorder template tree iterator coroutine. It uses coroutine properties to generate a sequence of the type T in a
// preorder order manner over a tree of such a type.

template <class T> _Coroutine uPreorderTreeGen : public uTreeIter<T> {
    void findNextNode( T *node ) {			// preorder tree traversal
	if ( node ) {
	    curr = node;
	    suspend();
	    findNextNode( (T *)left( node ) );
	    findNextNode( (T *)right( node ) );
	} // if
    } // uPreorderTreeGen::findNextNode()
  public:
    uPreorderTreeGen() {}
    uPreorderTreeGen( const Tree<T> &t ) : uTreeIter<T>( t ) {}
}; // uPreorderTreeGen


// An inorder template tree iterator coroutine. It uses coroutine properties to generate a sequence of the type T in an
// inorder manner over a tree of such a type.

template <class T> _Coroutine uInorderTreeGen : public uTreeIter<T> {
    void findNextNode( T *node ) {			// inorder tree traversal
	if ( node ) {
	    findNextNode( (T *)left( node ) );
	    curr = node;
	    suspend();
	    findNextNode( (T *)right( node ) );
	} // if
    } // uInorderTreeGen::findNextNode
  public:
    uInorderTreeGen() {}
    uInorderTreeGen( const Tree<T> &t ) : uTreeIter<T>( t ) {}
}; // uInorderTreeGen


// A postorder template tree iterator coroutine. It uses coroutine properties to generate a sequence of the type T in a
// postorder order manner over a tree of such a type.

template <class T> _Coroutine uPostorderTreeGen : public uTreeIter<T> {
    void findNextNode( T *node ) {			// postorder tree traversal
	if ( node ) {
	    findNextNode( (T *)left( node ) );
	    findNextNode( (T *)right( node ) );
	    curr = node;
	    suspend();
	} // if
    } // uPostorderTreeGen::findNextNode
  public:        
    uPostorderTreeGen() {}
    uPostorderTreeGen( const Tree<T> &t ) : uTreeIter<T>( t ) {}
}; // uPostorderTreeGen


// deleteTree()
//
// Non-member method to recursively delete all the nodes in a binary tree object.

template <class T> static void deleteTree( const Tree<T> &tree ) {
    T *ptr;

    for ( uPostorderTreeGen<T> iter( tree ); iter >> ptr ; ) {
        delete ptr;
    } // for
} // deleteTree


// printTree()
//
// Non-member method to recursively print out the contents of the binary search tree.

template <class T> static void printTree( const Tree<T> &tree ) {
    T *ptr;

    for ( uInorderTreeGen<T> iter( tree ); iter >> ptr ; ) {
        cout << *ptr << endl;
    } // for
} // printTree


// Tree::~Tree()
//
// Tree object destructor

template <class T> Tree<T>::~Tree() {
    deleteTree( *this );					// delete the tree
} // Tree::~Tree


// SameFringe ()
//
// Function to determine whether the fringe of two tree objects is the same. Uses inorder iterators for the two trees to
// generate sequences and compares them.

template <class T> bool SameFringe( const Tree<T> &tree1, const Tree<T> &tree2 ) {
    uInorderTreeGen<T> tree1Gen( tree1 );
    uInorderTreeGen<T> tree2Gen( tree2 );
    T *t1Ptr, *t2Ptr;

    for ( ; tree1Gen >> t1Ptr & tree2Gen >> t2Ptr; ) {	// always evalute both operands
      if ( !(*t1Ptr == *t2Ptr) ) return false;		// elements must be equal (construct != from ==)
    } // for
    return t1Ptr == NULL && t2Ptr == NULL;		// and both traversals must have completed
} // SameFringe


class mynode : public Treeable {
    friend bool operator==( mynode, mynode );
    friend bool operator<( mynode op1, mynode op2 );
    friend bool operator>( mynode op1, mynode op2 );
    friend ostream &operator<<( ostream &os, mynode &op );

    int i;
  public:
    mynode( int i ) : i( i ) {}
}; // mynode

bool operator==( mynode op1, mynode op2 ) {
    return op1.i == op2.i;
}
bool operator<( mynode op1, mynode op2 ) {
    return op1.i < op2.i;
}
bool operator>( mynode op1, mynode op2 ) {
    return op1.i > op2.i;
}
ostream &operator<<( ostream &os, mynode &op ) {
    os << op.i;
    return os;
}


void uMain::main() {
    Tree<mynode> tree1;
    Tree<mynode> tree2;
    mynode *p1, *p2;

    uInorderTreeGen<mynode> in1, in2;
    uPreorderTreeGen<mynode> pre1, pre2;
    uPostorderTreeGen<mynode> post1, post2;

    for ( int i = 0; i < 20; i += 1 ) {			// create trees
	tree1.insert( new mynode( i ) );
	tree2.insert( new mynode( i + 1 ) );
    } // for

    cout << "preorder traversal" << endl;
    for ( pre1.over( tree1 ), pre2.over( tree2 ); pre1 >> p1 & pre2 >> p2; ) {
	cout << "\t" << *p1 << "\t" << *p2 << endl;
    } // for

    cout << "inorder traversal" << endl;
    for ( in1.over( tree1 ), in2.over( tree2 ); in1 >> p1 & in2 >> p2; ) {
	cout << "\t" << *p1 << "\t" << *p2 << endl;
    } // for

    cout << "portorder traversal" << endl;
    for ( post1.over( tree1 ), post2.over( tree2 ); post1 >> p1 & post2 >> p2; ) {
	cout << "\t" << *p1 << "\t" << *p2 << endl;
    } // for
    cout << "portorder traversal, again" << endl;
    for ( post1.over( tree1 ), post2.over( tree2 ); post1 >> p1 & post2 >> p2; ) {
	cout << "\t" << *p1 << "\t" << *p2 << endl;
    } // for

    if ( SameFringe( tree1, tree2 ) ) {
        cout << "Same fringe"        << endl;
    } else {
	cout << "Different fringes!" << endl;
    } // if
} // uMain::main
