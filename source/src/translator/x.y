%token IDENTIFIER
%start statement
%%
expression:
	IDENTIFIER
	;

dname:
	IDENTIFIER
	;

dname_list:
	dname "," dname_list
	| dname
	;

statement:
	";"
	| expression ";"
	| "return" ";"
	| "return" expression ";"
	| "uWait" expression ";"
	| "uWait" expression "uWith" expression ";"
	| "uSignal" expression ";"
	| "uSignalBlock" expression ";"
	| accept_statement ";"
	| enable_statement ";"
	;

accept_statement:
	when_clause "uAccept" "(" dname_list ")" statement
	| when_clause "uAccept" "(" dname_list ")" statement "uElse" statement
	| timeout_clause
	| when_clause "uAccept" "(" dname_list ")" statement "uOr" accept_statement
	;

enable_statement:
	when_clause "uEnable" "(" dname_list ")" "return"
	| "uOr" when_clause "uEnable" "(" dname_list ")" "return"
	| when_clause "uEnable" "(" dname_list ")" "return" expression
	| "uOr" when_clause "uEnable" "(" dname_list ")" "return" expression
	| when_clause "uEnable" "(" dname_list ")" "uWait" expression
	| "uOr" when_clause "uEnable" "(" dname_list ")" "uWait" expression
	| when_clause "uEnable" "(" dname_list ")" "uWait" expression "uWith" expression
	| "uOr" when_clause "uEnable" "(" dname_list ")" "uWait" expression "uWith" expression
	| when_clause "uEnable" "(" dname_list ")" "uOr" remaining_enable_statement
	| "uOr" when_clause "uEnable" "(" dname_list ")" "uOr" remaining_enable_statement
	;

remaining_enable_statement:
	when_clause "uEnable" "(" dname_list ")" "return"
	| when_clause "uEnable" "(" dname_list ")" "return" expression
	| when_clause "uEnable" "(" dname_list ")" "uWait" expression
	| when_clause "uEnable" "(" dname_list ")" "uWait" expression "uWith" expression
	| when_clause "uEnable" "(" dname_list ")" "uOr" remaining_enable_statement
	;

when_clause:
	/* empty */
	| "uWhen" "(" expression ")"
	;

timeout_clause:
	when_clause "uTimeout" "(" expression ")" statement
	;
%%
