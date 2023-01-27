//Gabriel Rodriguez
//3823004
//COP3402

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

int base(int *stack, int BP, int L);
void print_instruction(int PC, instruction IR);
void print_stack(int PC, int BP, int SP, int *stack);

void execute(int trace_flag, instruction *code)
{
	if (trace_flag == 1)
	{
		printf("VM Exectution:\n");
		printf("\t\t\t\tPC\tBP\tSP\tstack\n");
		printf("Initial Values:\t\t\t0\t0\t-1\n");
    //Initialize all variables and create the instruct struct IR and int array stack
	int BP = 0;
	int PC = 0;
	int SP = -1;
	int halt = 0;
	instruction IR;
	int stack[ARRAY_SIZE];

	//For loop to initialize each element of the int arary to 0
	for(int i = 0; i  < ARRAY_SIZE; i++)
	{
		stack[i] = 0;
	}
    //While loop that runs until the halt int is set to true or 1
	while(halt == 0)
	{
		IR = code[PC];
		PC++;
        //Call to printing function
		print_instruction(PC, IR);

		//If instruction op equals 1, stack pointer is incremented and IR.m value is stored at position SP in stack array
		if(IR.op == 1)
		{
			SP++;
			stack[SP] = IR.m;
			//Call to printing function
			print_stack(PC, BP, SP, stack);
		}
        //Else if instruction op equals 2 the stack pointer is decremented and stack at position SP equals the sum of stack[SP] + stack[SP + 1]
		else if(IR.op == 2)
		{   //IF IR.m equals 1 then SP gets decremented and the value at stack[SP] is set to the value at stack[SP] + stack[SP + 1]
			if(IR.m == 1)
			{
				SP--;
				stack[SP] = stack[SP] + stack[SP + 1];

			}
            //IF IR.m equals 2 then SP gets decremented and the value at stack[SP] is set to the value at stack[SP] - stack[SP + 1]
			else if(IR.m == 2)
			{
				SP--;
				stack[SP] = stack[SP] - stack[SP + 1];
			}
            //IF IR.m equals 3 then SP gets decremented and the value at stack[SP] is set to the value at stack[SP] = stack[SP] * stack[SP + 1]
			else if(IR.m == 3)
			{
				SP--;
				stack[SP] = stack[SP] * stack[SP + 1];
			}
            //IF IR.m equals 4 then SP gets decremented and the value at stack[SP] is set to the value at stack[SP] = stack[SP] / stack[SP + 1]
			else if(IR.m == 4)
			{
				SP--;
				stack[SP] = stack[SP] / stack[SP + 1];
			}
            //IF IR.m equals 5 then SP gets decremented and the value at stack[SP] is set to 1 or 0 depending on the if condition
			else if(IR.m == 5)
			{
				SP--;
				if(stack[SP] == stack[SP + 1])
				{
				stack[SP] = 1;
				}

				else
				{
					stack[SP] = 0;
				}
			}
            //IF IR.m equals 6 then SP gets decremented and the value at stack[SP] is set to 1 or 0 depending on the if condition
			else if(IR.m == 6)
			{
				SP--;
				if(stack[SP] != stack[SP + 1])
				{
					stack[SP] = 1;
				}

				else
				{
					stack[SP] = 0;
				}
			}
            //IF IR.m equals 7 then SP gets decremented and the value at stack[SP] is set to 1 or 0 depending on the if condition
			else if(IR.m == 7)
			{
				SP--;
				if(stack[SP] < stack[SP + 1])
				{
					stack[SP] = 1;
				}

				else
				{
					stack[SP] = 0;
				}
			}
            //IF IR.m equals 8 then SP gets decremented and the value at stack[SP] is set to 1 or 0 depending on the if condition
			else if(IR.m == 8)
			{
				SP--;
				if(stack[SP] <= stack[SP + 1])
				{
					stack[SP] = 1;
				}

				else
				{
					stack[SP] = 0;
				}
			}
            //IF IR.m equals 9 then SP gets decremented and the value at stack[SP] is set to 1 or 0 depending on the if condition
			else if(IR.m == 9)
			{
				SP--;
				if(stack[SP] > stack[SP + 1])
				{
					stack[SP] = 1;
				}

				else
				{
					stack[SP] = 0;
				}
			}
            //IF IR.m equals 10 then SP gets decremented and the value at stack[SP] is set to 1 or 0 depending on the if condition
			else if(IR.m == 10)
			{
				SP--;
				if(stack[SP] >= stack[SP + 1])
				{
					stack[SP] = 1;
				}

				else
				{
					stack[SP] = 0;
				}
			}
			//Call to printing function
			print_stack(PC, BP, SP, stack);
		}
        //Else if IR op equals 3 then the stack pointer is incremented by 1 and the base function is called + IR m and stack[SP] gets the value at that location
		else if(IR.op == 3)
		{
			SP++;
			stack[SP] = stack[base(stack, BP, IR.l) + IR.m];
			//Call to printing function
			print_stack(PC, BP, SP, stack);
		}
        //Else if IR op equals 4 the stack pointer is decremented and base function is called stack[base(stack, BP, IR.l) + IR.m] = stack[SP];
		else if(IR.op == 4)
		{
			stack[base(stack, BP, IR.l) + IR.m] = stack[SP];
			SP--;
			//Call to printing function
			print_stack(PC, BP, SP, stack);
		}
        //Else if IR op equals 5 then stack[SP +1] gets the value at base(stack, BP, IR.l), stack[SP +2] gets the value of BP and
		//tack[SP + 3] gets the value at PC. BP is set to SP + 1 and PC is set to IR.m
		else if(IR.op == 5)
		{
			stack[SP + 1] = base(stack, BP, IR.l);
			stack[SP + 2] = BP;
			stack[SP + 3] = PC;
			BP = SP + 1;
			PC = IR.m;
			//Call to printing function
			print_stack(PC, BP, SP, stack);
		}
        //Else if IR op equals 6 then SP is set to BP-1 and BP gets the value at stack[SP + 2] and PC gets the value at stack[SP + 3]
		else if(IR.op == 6)
		{
			SP = BP - 1;
			BP = stack[SP + 2];
			PC = stack[SP + 3];
			//Call to printing function
			print_stack(PC, BP, SP, stack);
		}
        //Else if IR op equals 7 then SP is set to SP + IR.m
		else if(IR.op == 7)
		{
			SP = SP + IR.m;
			//Call to printing function
			print_stack(PC, BP, SP, stack);
		}
        //Else if IR op equals 8 then PC is set to IR.m
		else if(IR.op == 8)
		{
			PC = IR.m;
			//Call to printing function
			print_stack(PC, BP, SP, stack);
		}
        //Else if IR op equals 9 then if the value at stack[SP] equals 0, PC is set to IR.m. Stack pointer gets decremented
		else if(IR.op == 9)
		{
			if(stack[SP] == 0)
			{
				PC = IR.m;
			}

			SP--;
			//Call to printing function
			print_stack(PC, BP, SP, stack);
		}
        //Else if IR op equals 10 then if IR.m equals 1 SP gets decremented and some print statements execute
		//If IR.m equals 2 then SP gets incremented and some print statements execute along with a scanner for user input
		//If IR.m equals 3 then the halt value is set to 1 and the program halts
		else if(IR.op == 10)
		{
			if(IR.m == 1)
			{

				printf("\nOutput : %d", stack[SP]);
                printf("\n\t\t\t\t");
				SP--;
			}

			else if(IR.m == 2)
			{
				SP++;
				printf("\nInput : ");
                scanf("%d", &stack[SP]);
       			printf("\t\t\t\t");
			}

			else if(IR.m == 3)
			{
				halt = 1;
			}

			print_stack(PC, BP, SP, stack);
		}

	}
}
}


int base(int *stack, int BP, int L)
{
	while (L > 0)
	{
		BP = stack[BP];
		L--;
	}
	return BP;
}


void print_stack(int PC, int BP, int SP, int *stack)
{
	int i;
	printf("%d\t%d\t%d\t", PC, BP, SP);
	for (i = 0; i <= SP; i++)
		printf("%d ", stack[i]);
	printf("\n");
}

void print_instruction(int PC, instruction IR)
{
	char opname[4];

	switch (IR.op)
	{
		case LIT : strcpy(opname, "LIT"); break;
		case OPR :
			switch (IR.m)
			{
				case ADD : strcpy(opname, "ADD"); break;
				case SUB : strcpy(opname, "SUB"); break;
				case MUL : strcpy(opname, "MUL"); break;
				case DIV : strcpy(opname, "DIV"); break;
				case EQL : strcpy(opname, "EQL"); break;
				case NEQ : strcpy(opname, "NEQ"); break;
				case LSS : strcpy(opname, "LSS"); break;
				case LEQ : strcpy(opname, "LEQ"); break;
				case GTR : strcpy(opname, "GTR"); break;
				case GEQ : strcpy(opname, "GEQ"); break;
				default : strcpy(opname, "err"); break;
			}
			break;
		case LOD : strcpy(opname, "LOD"); break;
		case STO : strcpy(opname, "STO"); break;
		case CAL : strcpy(opname, "CAL"); break;
		case RTN : strcpy(opname, "RTN"); break;
		case INC : strcpy(opname, "INC"); break;
		case JMP : strcpy(opname, "JMP"); break;
		case JPC : strcpy(opname, "JPC"); break;
		case SYS :
			switch (IR.m)
			{
				case WRT : strcpy(opname, "WRT"); break;
				case RED : strcpy(opname, "RED"); break;
				case HLT : strcpy(opname, "HLT"); break;
				default : strcpy(opname, "err"); break;
			}
			break;
		default : strcpy(opname, "err"); break;
	}

	printf("%d\t%s\t%d\t%d\t", PC - 1, opname, IR.l, IR.m);
}
