/*
COP 3402 Assignment 3  Fall 2022
Jesse Cooley, Evan Naderi, Cody Macek, & Gabriel Rodriguez
Parser and Code generator for PL/0 in C
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

lexeme *tokens;
int token_index = 0;
symbol *table;
int table_index = 0;
instruction *code;
int code_index = 0;

int error = 0;
int level;


void emit(int op, int l, int m);
void add_symbol(int kind, char name[], int value, int level, int address);
void mark();
int multiple_declaration_check(char name[]);
int find_symbol(char name[], int kind);

void print_parser_error(int error_code, int case_code);
void print_assembly_code();
void print_symbol_table();

//helpers
void get_next_token();
void expression();
void term();
void condition();
void factor();
void declaration();
void statement();
void varDeclaration(int numVars);
void constDeclaration();
void procDeclaration();
void ident();
void call();

// Grammar functions
void program();
void block();
void begin();
void ifCondition();
void returnFunc();
int declaration_follow_set(int curr_token);
void whileCondition();
void read();
void write();
void def();
int ifwhile_follow_set(int curr_token);
int left_curly_follow_set(int curr_token);
int expression_follow_set(int curr_token);

// Auxiliary functions
    char* lexStrings[] = {"",
    "identifier","number", "keyword_const", "keyword_var", "keyword_procedure",
	"keyword_call", "keyword_begin", "keyword_end", "keyword_if", "keyword_then", 
	"keyword_while", "keyword_do", "keyword_read", "keyword_write", "keyword_return",
	"keyword_def", "period", "assignment_symbol", "minus", "semicolon",
	"left_curly_brace", "right_curly_brace","equal_to","not_equal_to", "less_than",
	"less_than_or_equal_to", "greater_than", "greater_than_or_equal_to", "plus", "times",
	"division", "left_parenthesis", "right_parenthesis"
};

lexeme currentToken;

int valid_token(int token, int validTokens[], int nParams);

//Main function

instruction *parse(int code_flag, int table_flag, lexeme *list)
{
	table = calloc(ARRAY_SIZE, sizeof(symbol));
	code = calloc(ARRAY_SIZE, sizeof(instruction));
    tokens = list;
	program();

	if (error == 0)
	{
		if (code_flag)
			print_assembly_code();
		if (table_flag)
			print_symbol_table();
		free(table);
		return code;
		
	}

	free(table);
	free(code);
	return NULL;

}


void get_next_token() 
{	
	currentToken = tokens[token_index];
	token_index++;
}

/*
Grammar functions (program, block, declaration, statement, condition, expression, term, factor)
-each is headed with its grammar rule from the assignment file-
Remember that any time a function calls a nested function (i.e. program calls block)
there is a chance that a stopping error was found, therefore we must check the
global error flag after any nested call and abort.
*/
void program()
{

	//Handle Block
	get_next_token();
	level = -1;
	block();

	if (error == -1)
		return;

	if (currentToken.type != period) 
	{
		print_parser_error(1, 0); // missing period
		error = 1;
	}

	//Handle CAL
	/*
	1. Find every CAL instruction in the code array
	2. If the M field is -1, this means there was some error with identifier and we don t/can t correct this, move to the next one
	3. Otherwise the current M field is the index of the desired procedure in the symbol table. Check for error 21.
	    If there is an address set, set the M field of the instruction to it.
	*/
	int i;
	for (i = 0; i < ARRAY_SIZE; i++)
	{
        if (code[i].op == 0 && code[i].l == 0 && code[i].m == 0)
            continue;

        if (code[i].op == CAL) 
			{
				int M = code[i].m;
				
				if (M == -1)
					continue;
				
				if (table[M].address == -1) {
					print_parser_error(21, 0);
					error = 1;
				}	
				else
					code[i].m = table[M].address;
			}
	}
	//HANDLE HLT
	/*the HLT emit is optional here because it s possible that the last statement was 
	an explicit return. We know this has happened if the last instruction in code is a 
	HLT instruction. If this is not the case, the HLT is implicit and so you need to 
	emit it in PROGRAM.*/
	if (code[code_index - 1].m != HLT)
		emit(SYS, 0, HLT);


}
void block() 
{
	/* 
	BLOCK ::= DECLARATIONS STATEMENT
	*/
	level++;
	declaration();
	if (error == -1)
		return;
    
	statement();
	if (error == -1)
		return;
	mark();
	level--;

}
void declaration() 
{
	/*
	DECLARATIONS ::= {CONST|VAR|PROC} INC
	*/
	int vars = 0;
	int validTokens[3];
	validTokens[0] = keyword_const; validTokens[1] = keyword_var; validTokens[2] = keyword_procedure;

	//while currtoken is const,var, or proc declaration
	while (valid_token(currentToken.type, validTokens, 3)) 
	{
	
		if (currentToken.type == keyword_var)
		{
			varDeclaration(vars);			
			vars++;	
		}
		else if (currentToken.type == keyword_procedure)
			procDeclaration();
		else if (currentToken.type == keyword_const)
			constDeclaration();

		if (error == -1)
			return;


	}
  	emit(INC,0,vars+3);
}
void constDeclaration()
{

	/*
	CONST ::= [const (ident [3] | 2-1) (:=|4-1) [-] (number|5) (;|6-1)]
	*/

	get_next_token();
	
	if (multiple_declaration_check(currentToken.identifier_name) != -1) //error 3 (multiple declarations of this const)
	{
		print_parser_error(3, 0);
		error = 1;
	}

	char *currName = malloc(sizeof(char) * 13);
	if (currentToken.type != identifier) //error 2-1 (const not followed by identifier)
	{
		strcpy(currName,"null");
		print_parser_error(2, 1);
		error = 1;
		if (currentToken.type != assignment_symbol) // stopping
		{
			free(currName);
			error = -1;
			return;
		}	
	}
	
    else{
		strcpy(currName,currentToken.identifier_name);
		get_next_token();

	}
	
	int negFlag = 0; //flag for minus sign
	int value = 0;

	
	if (currentToken.type != assignment_symbol) //error 4-1 (missing = in const declaration)
	{
		print_parser_error(4, 1);
		error = 1;
		if (currentToken.type != minus && currentToken.type != number) // stopping
		{
			free(currName);
			error = -1;
			return;
		}
	}
	else
		get_next_token();

	
	if (currentToken.type == minus) //concat minus sign to incoming value
	{
		negFlag = 1; //for storing value later
		get_next_token();
	}
	

	if (currentToken.type != number) //error 5 (missing number in const declaration)
	{
		print_parser_error(5, 0);
		error = 1;
		if (currentToken.type != semicolon) // stopping
		{
			free(currName);
			error = -1;
			return;
		}

	}

	else
	{
		value = currentToken.number_value * (negFlag ? -1 : 1); //no error 5 => store number value with possible minus sign
		get_next_token();
	}

	
	if (currentToken.type != semicolon) //error 6-1 (missing semicolon)
	{

		print_parser_error(6, 1);
		error = 1;
		if (!declaration_follow_set(currentToken.type)) 
		{
			free(currName);
			error = -1;
			return;
		}

	}
	else
		get_next_token();
	//add the assembled data to the symbol table
	add_symbol(1, currName, value, level, 0);	

	free(currName);

}
void varDeclaration(int numVars)
{
	/*
	VAR ::= [var (ident [3] | 2-2) (;|6-2)
	*/

	get_next_token();
	if (multiple_declaration_check(currentToken.identifier_name) != -1) //error 3 (multiple declarations of this var)
	{
		print_parser_error(3, 0);
		error = 1;
	}
	char *currName = malloc(sizeof(char) * 13);
	if (currentToken.type != identifier) //error 2-3 (var not followed by identifier)
	{	
		strcpy(currName,"null");
		print_parser_error(2, 2);
		error = 1;
		
		if (currentToken.type != assignment_symbol) // stopping
		{
			free(currName);
			error = -1;
			return;
		}
	}
	
    else{
		strcpy(currName,currentToken.identifier_name);
		get_next_token();
	}

	
	

	if (currentToken.type != semicolon) //error 6-2 (missing semicolon)
	{
	
		print_parser_error(6, 2);
		error = 1;
		if (!declaration_follow_set(currentToken.type))
		{   
			free(currName);
			error = -1;
			return;
		}

	}
	else
		get_next_token();
	//add the assembled data to the symbol table
	add_symbol(2, currName, 0, level, numVars + 3);

	free(currName);
	
}
void procDeclaration() 
{
	/*
	PROC ::= [procedure (ident [3] | 2-3) (;|6-3)]
	*/
	get_next_token();
	if (multiple_declaration_check(currentToken.identifier_name) != -1) //error 3 (multiple declarations of this proc)
	{
		print_parser_error(3, 0);
		error = 1;
	}

	char *currName = malloc(sizeof(char) * 13);
	if (currentToken.type != identifier) //error 2-3 (proc not followed by identifier)
	{	
		strcpy(currName,"null");
		print_parser_error(2, 3);
		error = 1;	
		
		if (currentToken.type != assignment_symbol) // stopping
		{
			free(currName);
			error = -1;
			return;
		}
	}
	
    else{
		strcpy(currName,currentToken.identifier_name);
		get_next_token();
	}

	if (currentToken.type != semicolon) //error 6-3 (missing semicolon)
	{
	
		print_parser_error(6, 3);
		error = 1;
		if (!declaration_follow_set(currentToken.type))
		{
			free(currName);
			error = -1;
			return;
		}

	}
	else
	    get_next_token();

	//add the assembled data to the symbol table
	add_symbol(3, currName, 0, level, -1);

	free(currName);
}

void statement()
{
	/*
	STATEMENT ::= [ ident [8-1|7] (:=|4-2) EXPRESSION STO
	| call (ident [8-2|9] | 2-4) CAL
	| begin STATEMENT {; STATEMENT} (end|6-4|10)
	| if CONDITION JPC (then|11) STATEMENT
	| while CONDITION JPC (do|12) STATEMENT JMP
	| read (ident [8-3|13] | 2-5) RED STO
	| write EXPRESSION WRT
	| def (ident [8-4|14|22|23] | 2-6) ({|15) JMP BLOCK [RTN] (}|16)
	| return [HLT|RTN] ]
	*/


	if (currentToken.type == identifier)
	{	
		ident();
		if (error == -1)
			return;

	}
	else if (currentToken.type == keyword_call) 
	{
		call();
		if (error == -1)
			return;
	}
	else if (currentToken.type == keyword_begin)
	{
		begin();
     
		if (error == -1)
			return;
	}
	else if (currentToken.type == keyword_if)
	{
		ifCondition();
		if (error == -1)
			return;
	}
	else if (currentToken.type == keyword_while)
	{
		whileCondition();
		if (error == -1)
			return;
	}
	else if (currentToken.type == keyword_read)
	{
		read();
		if (error == -1)
			return;
	}
	else if (currentToken.type == keyword_write)
	{
		write();
		if (error == -1)
			return;
	}
	else if (currentToken.type == keyword_def)
	{
		def();
		if (error == -1)
			return;
	}
	else if (currentToken.type == keyword_return)
	{
		returnFunc();
		if (error == -1)
			return;
	}
	else 
	{    
    
		return;
	}
    
}
void ident()
{
	/*
	[8-1|7] (:=|4-2) EXPRESSION STO
	*/

	int index = find_symbol(currentToken.identifier_name, 2);
	if (index == -1)
	{
		if (find_symbol(currentToken.identifier_name, 3) != -1 || find_symbol(currentToken.identifier_name, 1) != -1)
			print_parser_error(7, 0); // procedures and constants cannot be assigned to (7)
		else
			print_parser_error(8, 1); //undeclared identifier in assignment statement

		error = 1;
	}

	get_next_token();
	if (currentToken.type != assignment_symbol)
	{
		error = 1;
		print_parser_error(4, 2);  //missing = in assignment operator (4-2)
		int followSet[3];
		followSet[0] = identifier; followSet[1] = number; followSet[2] = left_parenthesis; 
		if (!valid_token(currentToken.type, followSet, 3))
		{
			error = -1;
             
			return;
		}
	}else
		get_next_token();
    expression();
	if (error == -1)
		return;
	if (error == 0)
		emit(STO, level - table[index].level, table[index].address);
	else
		emit(STO, -1, -1);
	

}

void call() 
{
	/*
	call (ident [8-2|9] | 2-4) CAL
	*/
	get_next_token();

	int errorFlag = 0;
	int index = -1;
	if (currentToken.type != identifier){
		errorFlag = 1;
		error = 1;
		print_parser_error(2, 4);  //missing identifier after keyword call (2,4)
		int followSet[4];
		followSet[0] = period; followSet[1] = right_curly_brace;
		followSet[2] = semicolon; followSet[3] = keyword_end;
		if (!valid_token(currentToken.type, followSet,4))
		{
			error = -1;
			return;
		}
	}else {
			 index = find_symbol(currentToken.identifier_name, 3);
			if (index == -1)
			{ //possible error 8-2 or 9
				errorFlag = 1;
				if (find_symbol(currentToken.identifier_name, 1) != -1 || find_symbol(currentToken.identifier_name, 2) != -1)
					print_parser_error(9, 0); //variables and constants cannot be called (9)
				else
					print_parser_error(8, 2); //undeclared identifier in call statement (8-2)

				error = 1;

			}
			get_next_token();
	}
	


	if (!errorFlag)
	{

		emit(CAL, level - table[index].level, index);
	}
		
	else
		emit(CAL, -1,-1);
	
	

}

void begin() 
{

	do {
		get_next_token();
		statement();
	} while (currentToken.type == semicolon);

   
	if (currentToken.type != keyword_end) 
	{
	
		//error 6-4 | 10
		int followSet[9];
		followSet[0] = identifier; followSet[1] = keyword_call; followSet[2] = keyword_begin;
		followSet[3] = keyword_if; followSet[4] = keyword_while; followSet[5] = keyword_read;
		followSet[6] = keyword_write; followSet[7] = keyword_def; followSet[8] = keyword_return;
		if (valid_token(currentToken.type, followSet, 9))
		{
			print_parser_error(6, 4); // statement in begin end 
			//ALWAYS STOPPING!
			error = -1;
			return;
		}

		else
		{
			print_parser_error(10, 0); //begin must be followed by end
			error = 1;
			int followSet[3];
			followSet[0] = period; followSet[1] = right_curly_brace; followSet[2] = semicolon;
			if (!valid_token(currentToken.type, followSet, 3))
			{
				error = -1;
				return;
			}
		}

	}else
		get_next_token();

}

void ifCondition() 
{
	get_next_token();
	condition();
	if (error == -1)
		return;
	int storedCodeIndex = code_index;
	emit(JPC,0,0);
	
	if (currentToken.type != keyword_then)
	{
		error = 1;
		print_parser_error(11,0);
		if (!ifwhile_follow_set(currentToken.type)){
			error = -1;
			return;
		}
	}
	else
    	get_next_token();
	statement();
  
	if (error == -1)
		return;
	code[storedCodeIndex].m = code_index;
	
}
void whileCondition() {

	get_next_token();
	int storedIndexForJump = code_index;
	condition();
	int storedCodeIndex = code_index;
	emit(JPC,0,0);

	if (currentToken.type != keyword_do)
	{
		error = 1;
		print_parser_error(12,0);
		if (!ifwhile_follow_set(currentToken.type)){
			error = -1;
			return;
		}
	}else
    	get_next_token();
	statement();
	if (error == -1)
		return;
	emit(JMP,0,storedIndexForJump);
	code[storedCodeIndex].m = code_index;

}
void read() 
{
	get_next_token();

	if (currentToken.type != identifier){
		error = 1;
		print_parser_error(2,5);//missing identifier after keyword read
		int followset[4];
		followset[0] = period; followset[1] = right_curly_brace; followset[2] = semicolon; followset[3] = keyword_end;
		if (!valid_token(currentToken.type,followset,4)){ //check for stopping
			error = 1;
			return;
		}
	}else {

		//check for 8-3 or 13
		int constIndex = find_symbol(currentToken.identifier_name,2);
		int procIndex = find_symbol(currentToken.identifier_name,3);
		int varIndex = find_symbol(currentToken.identifier_name,1);
		if (constIndex == -1){
			error = 1;
			if (procIndex != -1 || varIndex != -1)
				print_parser_error(13,0);
			else 
				print_parser_error(8,3);
		}
		get_next_token();
		int l = (error == 0)? (level - table[constIndex].level) : -1;
		int m = (error == 0)? (table[constIndex].address) : -1;
		emit(SYS,l,RED);
		emit(STO,l,m);
	}

}
void write() {

	get_next_token();
	expression();
	if (error == -1)
		return;
	emit(SYS,0,WRT);
	
}

void def() { 
/*
def (ident [8-4|14|23|22] | 2-6)({|15) JMP BLOCK [RTN] (}|16) 
*/
	get_next_token();
	int errorFlag = 0;
	int procIndex = -1;
	int varIndex = -1;
	int constIndex = -1;

	

	procIndex = find_symbol(currentToken.identifier_name,3);
	varIndex = find_symbol(currentToken.identifier_name,2);
	constIndex = find_symbol(currentToken.identifier_name,1);
	
	if (currentToken.type != identifier)
	{
		error = 1;
		errorFlag = 1;
		print_parser_error(2,6); //missing identifier after keyword def
		if (currentToken.type != left_curly_brace)
		{
			error = -1;
			return;
		}
		
	}else {

		//check for error 8-4/14/22/23 | 2-6

		if (procIndex == -1)
		{
		    errorFlag = 1;
			error = 1;
			if (constIndex == -1 && varIndex == -1)
				print_parser_error(8,4);
			else 
				print_parser_error(14,0);
		}
		else
		{
			 
			//check for error 22
			if (table[procIndex].level != level){
				error = 1;
				//errorFlag = 1;
				print_parser_error(22,0);
			}
			//check for error 23
			else if (table[procIndex].address != -1){
  				error = 1;
				//errorFlag = 1;
				print_parser_error(23,0);
			}
		}
		get_next_token();
	}
	
	if (currentToken.type != left_curly_brace)
	{

		error = 1;
		print_parser_error(15,0);//missing curly brace

		//check for stopping
		if (!left_curly_follow_set(currentToken.type))
		{
			error = -1;
			return;
		}
	}else
	  get_next_token();

	int storedCodeIndex = code_index;
	emit(JMP,0,0);
	
    if (!errorFlag)
	    table[procIndex].address = code_index;
	
  
    block();
	if (error == -1)
		return;

	if (code[code_index-1].op != RTN)
		emit(RTN,0,0);

 
	code[storedCodeIndex].m = code_index; 

	if (currentToken.type != right_curly_brace){
		error = 1;
		print_parser_error(16,0); //{ must be followed by }
		//check for stopping
		int followSet[4];
		followSet[0] = period; followSet[1] = right_curly_brace;
		followSet[2] = semicolon; followSet[3] = keyword_end;
		if (!valid_token(currentToken.type, followSet,4))
		{
			error = -1;
			return;
		}

	}
	else
		get_next_token();


}
void returnFunc() 
{
	if (level == 0)
		emit(SYS,0,HLT);
	else 
		emit(RTN,0,0);
	
  get_next_token();
}


void condition() 
{
	/*
	CONDITION ::= EXPRESSION (==|!=|<|<=|>|>=|17) EXPRESSION
	(EQL|NEQ|LSS|LEQ|GTR|GEQ|OPR)
	*/
	
	expression();
	if (error == -1)
		return;	
	opcode_name storedOp;
	int dontIncrement = 0;
	//check and store relational operator
	if (currentToken.type == equal_to)
		storedOp = EQL;
	else if (currentToken.type == not_equal_to)
		storedOp = NEQ;
	else if (currentToken.type == less_than)
		storedOp = LSS;
	else if (currentToken.type == less_than_or_equal_to)
		storedOp = LEQ;
	else if (currentToken.type == greater_than)
		storedOp = GTR;
	else if (currentToken.type == greater_than_or_equal_to)
		storedOp = GEQ;
	else{
		dontIncrement = 1;
		print_parser_error(17,0); //missing relational operator
		storedOp = -1;
		error = 1;
		int validTokens[3];
		validTokens[0] = identifier; validTokens[1] = number;
		validTokens[2] = left_parenthesis;
		if (!valid_token(currentToken.type,validTokens,3))
		{
		
			error = -1;
			return;
		}
	}	
	if (!dontIncrement)
		get_next_token();	
	expression();
	if (error == -1)
		return;
	emit(OPR,level,storedOp); 
}
void expression() //function to parse an expression 
{
	/*
	EXPRESSION ::= TERM {(+|-) TERM (ADD|SUB)}
	*/
	term();
	if (error == -1)
		return;

	while(currentToken.type == plus || currentToken.type == minus)
	{	

		opcode_name op = (currentToken.type == plus)? ADD : SUB; 
		get_next_token();
		term();
		if (error == -1)
			return;
		emit(OPR,0,op);
	
	}


}
void term()
{
	/*
	TERM ::= FACTOR {(*|/) FACTOR (MUL|DIV)}
	*/
    
	factor();
	if (error == -1)
		return;
	while (currentToken.type == times || currentToken.type == division) 
	{
		opcode_name op = (currentToken.type == times)? MUL : DIV; 
     
		get_next_token();
		factor();
		if (error == -1)
			return;
		emit(OPR,0,op);
		
	}

}
void factor()
{
	/*
	FACTOR ::= ident [8-5|18] (LOD|LIT)
	| number LIT
	| ( EXPRESSION ()|19)
	| 20
	*/
    
	if (currentToken.type == identifier)
	{
		int varIndex = find_symbol(currentToken.identifier_name,2);
		int constIndex = find_symbol(currentToken.identifier_name,1);
		//check for error 8-5 or 18
		if (constIndex == -1 && varIndex == -1)
		{
            error = 1;
			if (find_symbol(currentToken.identifier_name,3) != -1)
				print_parser_error(18,0);
			else 
				print_parser_error(8,5);
		}
		else
		{
			if (varIndex == -1 && constIndex != -1) //const 
				emit(LIT,0,table[constIndex].value);
				
			else if (constIndex == -1 && varIndex != -1 ) //var
				emit(LOD,level - table[varIndex].level,table[varIndex].address);
			else //var or const
			{
				if(table[varIndex].level > table[constIndex].level)
						emit(LOD,level - table[varIndex].level,table[varIndex].address);
				else
						emit(LIT,0,table[constIndex].value);
			}
		}
        get_next_token();
		
	}
	else if (currentToken.type == number)
	{
		emit(LIT,0,currentToken.number_value);
		get_next_token();
	}
	else if (currentToken.type == left_parenthesis)
	{
		get_next_token();
		expression();
		if(error == -1)
			return;

		if (currentToken.type != right_parenthesis)
		{
			print_parser_error(19,0); // left paren must be followed by right paren
			error = 1;
			if (!expression_follow_set(currentToken.type)) //check for stopping
			{
				error = -1;
				return;
			}
		}else
      	 get_next_token();
	}
	else 
	{
		//error 20
		print_parser_error(20,0); //invalid expression
		if (!expression_follow_set(currentToken.type)) //check for stopping
			{
				error = -1;
				return;
			}
	}

}


//little helper func to (hopefully) determine if a token is in a set
int valid_token(int curr_token,  int valid_tokens[], int nParams) 
{
	int i;
	for (i = 0; i < nParams; i++)
	{
		if (curr_token == valid_tokens[i])
			return 1;
	}
	return 0;
}
//the follow set for declarations was huge so i made a helper for 
//the helper to check those
int declaration_follow_set(int curr_token) 
{
	int validTokens[14];
	validTokens[0] = keyword_const; validTokens[1] = keyword_var;
	validTokens[2] = keyword_procedure; validTokens[3] = identifier;
	validTokens[4] = keyword_call; validTokens[5] = keyword_begin;
	validTokens[6] = keyword_if; validTokens[7] = keyword_while;
	validTokens[8] = keyword_read; validTokens[9] = keyword_write;
	validTokens[10] = keyword_def; validTokens[11] = keyword_return;
	validTokens[12] = period; validTokens[13] = right_curly_brace;
	return (valid_token(curr_token, validTokens,14));
}
//more follow set helpers(left parenth and expression both use this)
int expression_follow_set(int curr_token){
	
	int validTokens[16];
	validTokens[0] = times; validTokens[1] = division;
	validTokens[2] = plus; validTokens[3] = minus;
	validTokens[4] = period; validTokens[5] = right_curly_brace;
	validTokens[6] = semicolon; validTokens[7] = keyword_end;
	validTokens[8] = equal_to; validTokens[9] = not_equal_to;
	validTokens[10] = less_than; validTokens[11] = less_than_or_equal_to;
	validTokens[12] = greater_than; validTokens[13] = greater_than_or_equal_to;
	validTokens[14] = keyword_then; validTokens[15] = keyword_do;
	return (valid_token(curr_token,validTokens,16));
}
int left_curly_follow_set(int curr_token){

	int validTokens[13];
	validTokens[0] = keyword_const; validTokens[1] = keyword_var;
	validTokens[2] = keyword_procedure; validTokens[3] = identifier;
	validTokens[4] = keyword_call; validTokens[5] = keyword_begin;
	validTokens[6] = keyword_if; validTokens[7] = keyword_while;
	validTokens[8] = keyword_read; validTokens[9] = keyword_write;
	validTokens[10] = keyword_def; validTokens[11] = keyword_return;
	validTokens[12] = right_curly_brace;
	return (valid_token(curr_token,validTokens,13));
}
int ifwhile_follow_set(int curr_token){

	int validTokens[13];
	validTokens[0] = period; validTokens[1] = right_curly_brace;
	validTokens[2] = semicolon; validTokens[3] = keyword_end;
	validTokens[4] = identifier; validTokens[5] = keyword_call;
	validTokens[6] = keyword_begin; validTokens[7] = keyword_if;
	validTokens[8] = keyword_while; validTokens[9] = keyword_read;
	validTokens[10] = keyword_write; validTokens[11] = keyword_def;
	validTokens[12] = keyword_return;
	return (valid_token(curr_token,validTokens,13));
}


//Included functions (emit,add_symbol,print, etc)

void emit(int op, int l, int m)
{
	code[code_index].op = op;
	code[code_index].l = l;
	code[code_index].m = m;
	code_index++;
}

void add_symbol(int kind, char name[], int value, int level, int address)
{
	table[table_index].kind = kind;
	strcpy(table[table_index].name, name);
	table[table_index].value = value;
	table[table_index].level = level;
	table[table_index].address = address;
	table[table_index].mark = 0;
	table_index++;
}

void mark()
{
	int i;
	for (i = table_index - 1; i >= 0; i--)
	{
		if (table[i].mark == 1)
			continue;
		if (table[i].level < level)
			return;
		table[i].mark = 1;
	}
}

int multiple_declaration_check(char name[])
{
	int i;
	for (i = 0; i < table_index; i++)
		if (table[i].mark == 0 && table[i].level == level && strcmp(name, table[i].name) == 0)
			return i;
	return -1;
}

int find_symbol(char name[], int kind)
{
	int i;
	int max_idx = -1;
	int max_lvl = -1;
	for (i = 0; i < table_index; i++)
	{
		if (table[i].mark == 0 && table[i].kind == kind && strcmp(name, table[i].name) == 0)
		{
			if (max_idx == -1 || table[i].level > max_lvl)
			{
				max_idx = i;
				max_lvl = table[i].level;
			}
		}
	}
	return max_idx;
}


void print_parser_error(int error_code, int case_code)
{
	switch (error_code)
	{
		case 1 :
			printf("Parser Error 1: missing . \n");
			break;
		case 2 :
			switch (case_code)
			{
				case 1 :
					printf("Parser Error 2: missing identifier after keyword const\n");
					break;
				case 2 :
					printf("Parser Error 2: missing identifier after keyword var\n");
					break;
				case 3 :
					printf("Parser Error 2: missing identifier after keyword procedure\n");
					break;
				case 4 :
					printf("Parser Error 2: missing identifier after keyword call\n");
					break;
				case 5 :
					printf("Parser Error 2: missing identifier after keyword read\n");
					break;
				case 6 :
					printf("Parser Error 2: missing identifier after keyword def\n");
					break;
				default :
					printf("Implementation Error: unrecognized error code\n");
			}
			break;
		case 3 :
			printf("Parser Error 3: identifier is declared multiple times by a procedure\n");
			break;
		case 4 :
			switch (case_code)
			{
				case 1 :
					printf("Parser Error 4: missing := in constant declaration\n");
					break;
				case 2 :
					printf("Parser Error 4: missing := in assignment statement\n");
					break;
				default :				
					printf("Implementation Error: unrecognized error code\n");
			}
			break;
		case 5 :
			printf("Parser Error 5: missing number in constant declaration\n");
			break;
		case 6 :
			switch (case_code)
			{
				case 1 :
					printf("Parser Error 6: missing ; after constant declaration\n");
					break;
				case 2 :
					printf("Parser Error 6: missing ; after variable declaration\n");
					break;
				case 3 :
					printf("Parser Error 6: missing ; after procedure declaration\n");
					break;
				case 4 :
					printf("Parser Error 6: missing ; after statement in begin-end\n");
					break;
				default :				
					printf("Implementation Error: unrecognized error code\n");
			}
			break;
		case 7 :
			printf("Parser Error 7: procedures and constants cannot be assigned to\n");
			break;
		case 8 :
			switch (case_code)
			{
				case 1 :
					printf("Parser Error 8: undeclared identifier used in assignment statement\n");
					break;
				case 2 :
					printf("Parser Error 8: undeclared identifier used in call statement\n");
					break;
				case 3 :
					printf("Parser Error 8: undeclared identifier used in read statement\n");
					break;
				case 4 :
					printf("Parser Error 8: undeclared identifier used in define statement\n");
					break;
				case 5 :
					printf("Parser Error 8: undeclared identifier used in arithmetic expression\n");
					break;
				default :				
					printf("Implementation Error: unrecognized error code\n");
			}
			break;
		case 9 :
			printf("Parser Error 9: variables and constants cannot be called\n");
			break;
		case 10 :
			printf("Parser Error 10: begin must be followed by end\n");
			break;
		case 11 :
			printf("Parser Error 11: if must be followed by then\n");
			break;
		case 12 :
			printf("Parser Error 12: while must be followed by do\n");
			break;
		case 13 :
			printf("Parser Error 13: procedures and constants cannot be read\n");
			break;
		case 14 :
			printf("Parser Error 14: variables and constants cannot be defined\n");
			break;
		case 15 :
			printf("Parser Error 15: missing {\n");
			break;
		case 16 :
			printf("Parser Error 16: { must be followed by }\n");
			break;
		case 17 :
			printf("Parser Error 17: missing relational operator\n");
			break;
		case 18 :
			printf("Parser Error 18: procedures cannot be used in arithmetic\n");
			break;
		case 19 :
			printf("Parser Error 19: ( must be followed by )\n");
			break;
		case 20 :
			printf("Parser Error 20: invalid expression\n");
			break;
		case 21 :
			printf("Parser Error 21: procedure being called has not been defined\n");
			break;
		case 22 :
			printf("Parser Error 22: procedures can only be defined within the procedure that declares them\n");
			break;
		case 23 :
			printf("Parser Error 23: procedures cannot be defined multiple times\n");
			break;
		default:
			printf("Implementation Error: unrecognized error code\n");

	}
}

void print_assembly_code()
{
	int i;
	printf("Assembly Code:\n");
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < code_index; i++)
	{
		printf("%d\t%d\t", i, code[i].op);
		switch(code[i].op)
		{
			case LIT :
				printf("LIT\t");
				break;
			case OPR :
				switch (code[i].m)
				{
					case ADD :
						printf("ADD\t");
						break;
					case SUB :
						printf("SUB\t");
						break;
					case MUL :
						printf("MUL\t");
						break;
					case DIV :
						printf("DIV\t");
						break;
					case EQL :
						printf("EQL\t");
						break;
					case NEQ :
						printf("NEQ\t");
						break;
					case LSS :
						printf("LSS\t");
						break;
					case LEQ :
						printf("LEQ\t");
						break;
					case GTR :
						printf("GTR\t");
						break;
					case GEQ :
						printf("GEQ\t");
						break;
					default :
						printf("err\t");
						break;
				}
				break;
			case LOD :
				printf("LOD\t");
				break;
			case STO :
				printf("STO\t");
				break;
			case CAL :
				printf("CAL\t");
				break;
			case RTN :
				printf("RTN\t");
				break;
			case INC :
				printf("INC\t");
				break;
			case JMP :
				printf("JMP\t");
				break;
			case JPC :
				printf("JPC\t");
				break;
			case SYS :
				switch (code[i].m)
				{
					case WRT :
						printf("WRT\t");
						break;
					case RED :
						printf("RED\t");
						break;
					case HLT :
						printf("HLT\t");
						break;
					default :
						printf("err\t");
						break;
				}
				break;
			default :
				printf("err\t");
				break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
	}
	printf("\n");
}

void print_symbol_table()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i < table_index; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].value, table[i].level, table[i].address, table[i].mark); 
	printf("\n");
}
