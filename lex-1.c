//Gabriel Rodriguez
//3823004
//COP3402

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"

#define MAX_IDENT_LENGTH 11
#define MAX_NUM_LENGTH 5
#define ERR_IDENT_LENGTH 1
#define ERR_NUM_LENGTH 2
#define ERR_INVALID_IDENT 3
#define ERR_INVALID_SYMBOL 4
#define ERR_INVALID_IDENT_NAME 5

int keyword_check(char buffer[]);
void print_lexeme_list(lexeme *list, int list_end);

lexeme *lex_analyze(int list_flag, char *input)
{
	//Malloc memory for the lexeme array
	lexeme *lex_array = malloc(sizeof(lexeme) * ARRAY_SIZE);
	char buffer[MAX_IDENT_LENGTH + 1];
	//initialize variables
	int x = 0, y = 0, z = 0, flag = 0;
 
    //While loop that loops through the input char array
	while(input[x] != '\0')
	{	//If condition to check if we are dealing with an identifier/keyword
		if(isalpha(input[x]) > 0)
		{   
			//While loop to loop through the input array based on conditions
			while(input[x] != '\0' && (isalnum(input[x]) > 0) && y < MAX_IDENT_LENGTH)
			{   
				//Store the value at x of the input array in y of the buffer and our lexeme identifier name
				buffer[y] = input[x];
				lex_array[z].identifier_name[y] = buffer[y];
				//incriment our indexing variables
				x++;
				y++;
			}
            //add the null terminators to the buffer and lexeme identifier name
			buffer[y] = '\0';
			lex_array[z].identifier_name[y] = '\0';
            
			//error condition if we have a length error
			if(y == MAX_IDENT_LENGTH && isalnum(input[x]) > 0)
			{
				//Set our flag and assign proper error values
				flag = 1;
				lex_array[z].error_type = 1;
				lex_array[z].type = -1;
				//While loop to move our indexing variable passed the unncessary characters
				while(input[x] != '\0' && isalnum(input[x]) > 0)
				{
					x++;
				}
			}
            
			//else if condition to check if we have main or null in our buffer
			else if(keyword_check(buffer) == -1)
			{
				//set the flag and assign the proper error values
				flag = 1;
				lex_array[z].type = -1;
				lex_array[z].error_type = 5;
			}
            //else if to check if we have a keyword or symbol
			else if(keyword_check(buffer) > 2)
			{
				lex_array[z].type = keyword_check(buffer);

			}
            
			else
			{
				lex_array[z].type = 1;
			}
            
			//reset our indexing variable for the buffer, and increment our indexing variable for the lexeme array
			y = 0;
			z++;
            //Clear the buffer
			for(int i = 0; i < MAX_IDENT_LENGTH; i++)
			{
				buffer[i] = '\0';
			}

		}
        //else if to check if we are dealing with a number
		else if(isdigit(input[x]) > 0)
		{
			//While loop to collect all the characters and fill the buffer
			while(input[x] != '\0' && y < MAX_NUM_LENGTH && isdigit(input[x]) > 0)
			{
				buffer[y] = input[x];
				x++;
				y++;
			}
            //Add the null terminator and assign the number value to the lexeme
			buffer[y + 1] = '\0';
			lex_array[z].number_value = atoi(buffer);
            
			//If condition if we stopped because of an invalid identifier name error
			if(isalpha(input[x]) > 0 && y < MAX_NUM_LENGTH)
			{
				//set the flag and assign the proper error values
				flag = 1;
				lex_array[z].error_type = 3;
				lex_array[z].type = -1;
				//While loop to move our indexing variable passed the remaining characters that do not belong
				while(input[x] != '\0' && isalnum(input[x]) > 0)
				{
					x++;
				}

			}
            //Else if statement if we have an error with the length of the array
			else if(y == MAX_NUM_LENGTH && (isalnum(input[x]) > 0))
			{
				//Set the flag and assign the proper error values 
				flag = 1;
				lex_array[z].error_type = 2;
				lex_array[z].type = -1;
				//While loop to move our indexing variable passed the remaining characters that do not belong
				while(input[x] != '\0' && isalnum(input[x]) > 0)
				{
					x++;
				}
			}
            //Else statement if the number is not an error 
			else
			{
			lex_array[z].type = 2;
			}

			y = 0;
			z++;
            //clear the buffer
			for(int i = 0; i < MAX_IDENT_LENGTH; i++)
			{
				buffer[i] = '\0';
			}

		}
        //If we have a question mark, we move the indexing variable passed the comments
		else if(input[x] == '?')
		{
			while(input[x] != '\n' && input[x] != '\0')
			{
				x++;
			}
		}
        //increment our indexing variable if we have a white space and do nothing else
		else if(isspace(input[x]) > 0)
		{
			x++;
		}
        //Else condition with if else if statement structure to assign the proper values based on symbol
		//From the input array
		else
		{
			//if else if statements will corresponding assignment values and index variable incrementation
			if(input[x] == '.')
			{
				lex_array[z].type = 17;
				x++;
				z++;
			}

			else if(input[x] == '-')
			{
				lex_array[z].type = 19;
				x++;
				z++;
			}

			else if(input[x] == ';')
			{
				lex_array[z].type = 20;
				x++;
				z++;
			}

			else if(input[x] == '{')
			{
				lex_array[z].type = 21;
				x++;
				z++;
			}

			else if(input[x] == '}')
			{
				lex_array[z].type = 22;
				x++;
				z++;
			}

			else if(input[x] == '<')
			{
				if(input[x+1] == '=')
				{
					lex_array[z].type = 26;
				    x++;
					x++;
				    z++;

				}

				else
				{
				lex_array[z].type = 25;
				x++;
				z++;
				}
			}

			else if(input[x] == '>')
			{
				if(input[x+1] == '=')
				{
					lex_array[z].type = 28;
					x++;
					x++;
					z++;
				}

				else
				{
					lex_array[z].type = 27;
					x++;
					z++;
				}

			}

			else if(input[x] == '+')
			{
				lex_array[z].type = 29;
				x++;
				z++;
			}

			else if(input[x] == '*')
			{
				lex_array[z].type = 30;
				x++;
				z++;
			}

			else if(input[x] == '/')
			{
				lex_array[z].type = 31;
				x++;
				z++;
			}

			else if(input[x] == '(')
			{
				lex_array[z].type = 32;
				x++;
				z++;
			}

			else if(input[x] == ')')
			{
				lex_array[z].type = 33;
				x++;
				z++;
			}

			else if(input[x] == '!')
			{
				if(input[x + 1] == '=')
				{
					lex_array[z].type = 24;
					x++;
					x++;
					z++;
				}

				else
				{
					lex_array[z].error_type = 4;
					lex_array[z].type = -1;
					x++;
					z++;
					flag = 1;
				}

			}

			else if(input[x] == ':')
			{
				if(input[x + 1] == '=')
				{
					lex_array[z].type = 18;
					x++;
					x++;
					z++;
				}

				else
				{
					lex_array[z].error_type = 4;
					lex_array[z].type = -1;
					x++;
					z++;
					flag = 1;
				}
			}

			else if(input[x] == '=')
			{
				if(input[x + 1] == '=')
				{
					lex_array[z].type = 23;
					x++;
					x++;
					z++;
				}

				else
				{
					lex_array[z].error_type = 4;
					lex_array[z].type = -1;
					x++;
					z++;
					flag = 1;
				}
			}

			else
			{
				lex_array[z].error_type = 4;
				lex_array[z].type = -1;
				x++;
				z++;
				flag = 1;
			}

		}

	}

	print_lexeme_list(lex_array, z);
        //If our flag was set because of an error, we return null
		if(flag == 1)
		{
			return NULL;
		}
        //Else we return the lexeme array
		else
		{
			return lex_array;
		}

}

int keyword_check(char buffer[])
{
	if (strcmp(buffer, "const") == 0)
		return keyword_const;
	else if (strcmp(buffer, "var") == 0)
		return keyword_var;
	else if (strcmp(buffer, "procedure") == 0)
		return keyword_procedure;
	else if (strcmp(buffer, "call") == 0)
		return keyword_call;
	else if (strcmp(buffer, "begin") == 0)
		return keyword_begin;
	else if (strcmp(buffer, "end") == 0)
		return keyword_end;
	else if (strcmp(buffer, "if") == 0)
		return keyword_if;
	else if (strcmp(buffer, "then") == 0)
		return keyword_then;
	else if (strcmp(buffer, "while") == 0)
		return keyword_while;
	else if (strcmp(buffer, "do") == 0)
		return keyword_do;
	else if (strcmp(buffer, "read") == 0)
		return keyword_read;
	else if (strcmp(buffer, "write") == 0)
		return keyword_write;
	else if (strcmp(buffer, "def") == 0)
		return keyword_def;
	else if (strcmp(buffer, "return") == 0)
		return keyword_return;
	else if (strcmp(buffer, "main") == 0)
		return -1;
	else if (strcmp(buffer, "null") == 0)
		return -1;
	else
		return identifier;
}

void print_lexeme_list(lexeme *list, int list_end)
{
	int i;
	printf("Lexeme List: \n");
	printf("lexeme\t\ttoken type\n");
	for (i = 0; i < list_end; i++)
	{
		// not an error
		if (list[i].type != -1)
		{
			switch (list[i].type)
			{
				case identifier :
					printf("%11s\t%d\n", list[i].identifier_name, identifier);
					break;
				case number :
					printf("%11d\t%d\n", list[i].number_value, number);
					break;
				case keyword_const :
					printf("%11s\t%d\n", "const", keyword_const);
					break;
				case keyword_var :
					printf("%11s\t%d\n", "var", keyword_var);
					break;
				case keyword_procedure :
					printf("%11s\t%d\n", "procedure", keyword_procedure);
					break;
				case keyword_call :
					printf("%11s\t%d\n", "call", keyword_call);
					break;
				case keyword_begin :
					printf("%11s\t%d\n", "begin", keyword_begin);
					break;
				case keyword_end :
					printf("%11s\t%d\n", "end", keyword_end);
					break;
				case keyword_if :
					printf("%11s\t%d\n", "if", keyword_if);
					break;
				case keyword_then :
					printf("%11s\t%d\n", "then", keyword_then);
					break;
				case keyword_while :
					printf("%11s\t%d\n", "while", keyword_while);
					break;
				case keyword_do :
					printf("%11s\t%d\n", "do", keyword_do);
					break;
				case keyword_read :
					printf("%11s\t%d\n", "read", keyword_read);
					break;
				case keyword_write :
					printf("%11s\t%d\n", "write", keyword_write);
					break;
				case keyword_def :
					printf("%11s\t%d\n", "def", keyword_def);
					break;
				case keyword_return :
					printf("%11s\t%d\n", "return", keyword_return);
					break;
				case period :
					printf("%11s\t%d\n", ".", period);
					break;
				case assignment_symbol :
					printf("%11s\t%d\n", ":=", assignment_symbol);
					break;
				case minus :
					printf("%11s\t%d\n", "-", minus);
					break;
				case semicolon :
					printf("%11s\t%d\n", ";", semicolon);
					break;
				case left_curly_brace :
					printf("%11s\t%d\n", "{", left_curly_brace);
					break;
				case right_curly_brace :
					printf("%11s\t%d\n", "}", right_curly_brace);
					break;
				case equal_to :
					printf("%11s\t%d\n", "==", equal_to);
					break;
				case not_equal_to :
					printf("%11s\t%d\n", "!=", not_equal_to);
					break;
				case less_than :
					printf("%11s\t%d\n", "<", less_than);
					break;
				case less_than_or_equal_to :
					printf("%11s\t%d\n", "<=", less_than_or_equal_to);
					break;
				case greater_than :
					printf("%11s\t%d\n", ">", greater_than);
					break;
				case greater_than_or_equal_to :
					printf("%11s\t%d\n", ">=", greater_than_or_equal_to);
					break;
				case plus :
					printf("%11s\t%d\n", "+", plus);
					break;
				case times :
					printf("%11s\t%d\n", "*", times);
					break;
				case division :
					printf("%11s\t%d\n", "/", division);
					break;
				case left_parenthesis :
					printf("%11s\t%d\n", "(", left_parenthesis);
					break;
				case right_parenthesis :
					printf("%11s\t%d\n", ")", right_parenthesis);
					break;
				default :
					printf("Implementation Error: unrecognized token type\n");
					break;
			}
		}
		// errors
		else
		{
			switch (list[i].error_type)
			{
				case ERR_IDENT_LENGTH :
					printf("Lexical Analyzer Error: maximum identifier length is 11\n");
					break;
				case ERR_NUM_LENGTH :
					printf("Lexical Analyzer Error: maximum number length is 5\n");
					break;
				case ERR_INVALID_IDENT :
					printf("Lexical Analyzer Error: identifiers cannot begin with digits\n");
					break;
				case ERR_INVALID_SYMBOL :
					printf("Lexical Analyzer Error: invalid symol\n");
					break;
				case ERR_INVALID_IDENT_NAME :
					printf("Lexical Analyzer Error: identifiers cannot be named 'null' or 'main'\n");
					break;
				default :
					printf("Implementation Error: unrecognized error type\n");
					break;
			}
		}
	}
	printf("\n");
}
