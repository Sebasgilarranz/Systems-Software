#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
 ////////////
int isVarCreated(char* var);
void finishTemplate();
void evalStatement(char* statement);
void handleElse(char* statement);
char* doFunctionShit(char* funcProto);
int isFunction(char* expression);
int evalFunction(char *function, int a);

struct token
{
	char grammar;
	int number;
	int last;
	int tempVar;
};

struct intToken
{
	char var[1000];
	int tempNum;
};

struct token* eq;
struct token* postEq;

struct intToken* allInt;
int allIntCount = 0;

int lexCount;
int funcLexCount;

FILE* file;
char charStack[1000], intStack[1000][1000], array[1000];

int top = -1;
int temp = 1;
int a, c;


int labelNum = 1;
int condNum = 1;
int bodyNum = 1;
int headNum = 1;

/////////////////Function shit //////////////////////////

char functions[1000][1000];
int functionsCount = 0;
struct token* funcEq;
struct token* funcPostEq;
int funcLexCount;
char funcArray[1000];
int funcA;
////////////////////////////////////////////////////////


//***************************************************************
// The following functions evaluate simplec 
// if statement read was PRINT.
// some functions may be resused for future 
// functions that involve stack, etc.

void push(char x){ charStack[++top] = x; }
 
char pop()
{
	if(top == -1)
        return -1;
    else
        return charStack[top--];
}

void pushInt(int x){ sprintf(intStack[++top],"%d", x); }

void pushTemp(char* x){ strcpy(intStack[++top], x); }

char* popInt()
{
    if(top == -1)
        return "Error";
    else
    	return intStack[top--];
 }

int priority(char x)
{
    if(x == '(')
        return 0;
    if(x == '+' || x == '-')
        return 5;
    if(x == '*' || (x == '/' || x == '%'))
        return 6;
    if(x == '>' || x == '<')
        return 4;
    if(x == '=' || x == '@')
        return 3;
    if(x== '&')
    	return 2;
    if(x == '|')
    	return 1;
    if(x == '!')
		return 7;
	if(x == ',')
		return 8;
}

struct token* inToPost()
{
	postEq = malloc(sizeof(struct token) * 1000);
    char x;
    int q = 0, z = 0; 
    
    while(eq[q].last != 'E')
    {
        if(eq[q].grammar == '#')
        {
        	postEq[z].number = eq[q].number;
          	postEq[z++].grammar = '#';
        }
        
        else if(eq[q].grammar == 't')
        {
        	postEq[z].tempVar = eq[q].tempVar;
          	postEq[z++].grammar = 't';
        }

		else if(eq[q].grammar == '(')
            push(eq[q].grammar);
        else if(eq[q].grammar == ')')
        {
            while((x = pop()) != '(')
            {
                postEq[z++].grammar = x;
            }
        }
        else
        {
            while(priority(charStack[top]) >= priority(eq[q].grammar))
            {
                postEq[z++].grammar = pop();
            }
            push(eq[q].grammar);
        }
        q++;
    }
    while(top != -1)
    {
        postEq[z++].grammar = pop();
    }

    postEq[z].last = 'E';
    return postEq;
}

char* getOpString(char op)
{
	if (op == '+')
		return "add nsw ";
	if (op == '-')
		return "sub nsw ";
	if (op == '*')
		return "mul nsw ";
	if (op == '/')
		return "sdiv    ";
	if (op == '%')
		return "srem    ";
	if (op == '@')
		return "icmp ne ";
	if (op == '=')
		return "icmp eq ";
	if (op == '>')
		return "icmp sgt ";
	if (op == '<')
		return "icmp slt ";



	return 0;
}

char* consume(char* one, char* two, char operation)
{	
	char* retTemp = malloc(sizeof(char) * 1000);
	char temptemp[1000];
	retTemp[0] = '%';
	retTemp[1] = 't';
	retTemp[2] = '\0';

	if(operation == '!')
	{
		printf("  %%t%d = xor i1 %s, 1\n", temp, two);
		sprintf(temptemp, "%d", temp++);
		strcat(retTemp, temptemp);
		return retTemp;
	}
	if(operation == '&')
	{
		printf("  %%t%d = and i1 %s, %s\n", temp,one, two);
		sprintf(temptemp, "%d", temp++);
		strcat(retTemp, temptemp);
		return retTemp;
	}
	if(operation == '|')
	{
		printf("  %%t%d = or i1 %s, %s\n", temp, one, two);
		sprintf(temptemp, "%d", temp++);
		strcat(retTemp, temptemp);
		return retTemp;
	}

	if(operation == ',')
	{
		printf("doSomething");
	}

	printf("  %%t%d = %s i32 %s, %s\n", temp, getOpString(operation), one, two);
	sprintf(temptemp, "%d", temp++);
	strcat(retTemp, temptemp);
	
	return retTemp;
}

int getVarTempNum(char* var)
{
	for (int i = 0; i < allIntCount; i++)
	{
		if (strcmp(allInt[i].var, var) == 0)
			return allInt[i].tempNum;
	}
}

int evalPost(struct token* postExp, char* statement)
{
	int i = 0;
	char temp1[1000], temp2[1000];

	while (postExp[i].last != 'E')
	{
		if(postExp[i].grammar == '#')
		{
			pushInt(postExp[i].number);
		}
		else if (postExp[i].grammar == 't')
		{
			temp2[0] = '%';
			temp2[1] = 't';
			temp2[2] = '\0';
			sprintf(temp1, "%d", postExp[i].tempVar);
			pushTemp(strcat(temp2, temp1));
		}
		else 
		{
			if(postExp[i].grammar == ',')
			{
				return 0;
			}
			strcpy(temp2, popInt());
			strcpy(temp1, popInt());
			pushTemp(consume(temp1, temp2, postExp[i].grammar));
		}
		i++;
	}
	if (strcmp(statement, "print") == 0)
		printf("  call void @print_integer(i32 %s)\n", popInt());
	
	/////////////////proj3////////////////////
	else if (strcmp(statement, "if") == 0)
	{
		popInt();
		return 1;
	}
	else if (strcmp(statement, "function") == 0)
	{
		popInt();
		return 1;
	}
	/////////////////proj2////////////////////
	else 
	{
		popInt();
		
		if(isVarCreated(statement) == 0){
			if(postExp[0].grammar == '#')
			{	
				printf("  store i32 %d, i32* %%t%d\n", postExp[0].number, getVarTempNum(statement));
				return 1;
			}
			
			else 
			{
				printf("  store i32 %%t%d, i32* %%t%d\n", temp-1, getVarTempNum(statement));
				return 1;
			}

		}
		else 
		{
			fprintf(stderr, "error: use of undeclared variable %s\n", statement);
			finishTemplate();
			exit(0);
	
		}
	}
	/////////////////////////////////////
}

char* getStatement()
{
	char* statement = malloc(sizeof(char) * 100);
	int cnt = 0;

	while (c == 10 || c == 32 || (c == '	' || c == ';' || c == '{'))
	{
		c = fgetc(file);

	}

	while(isalpha(c) != 0)
	{	
		statement[cnt++] = c;
		c = fgetc(file);
	}

	statement[cnt] = '\0';
	
	return statement;
}

void getNum()
{
	char* buffer = malloc(sizeof(char) * 1000);
	int i = 0;
	int neg = 0;

	if (array[a]  == '-')
	{
		neg = 1;
		a++;
	}

	while(isdigit(array[a]) != 0)
	{
		buffer[i++] = array[a];
		a++;
	}

	if (neg == 1)
	{
		int hold = atoi(buffer);
		hold *= -1;
		sprintf(buffer, "%d", hold);
	}
	
	eq[lexCount].grammar = '#';
	eq[lexCount++].number = atoi(buffer);
}

///////////
void loadTempVar(char* var)
{
	for(int i = 0; i < allIntCount; i ++)
	{
		if(strcmp(var, allInt[i].var) == 0)
		{
			printf("  %%t%d = load i32, i32* %%t%d\n",temp, allInt[i].tempNum);
			return;
		}
	}

	for(int i = 0; i < functionsCount; i ++)
	{
		if(strcmp(var, functions[i]) == 0)
		{
			printf("  %%t%d = call i32 @%s(i32 %%t%d)\n",temp,var, temp - 1);
			return;
		}
	}



	fprintf(stderr, "error: use of undeclared variable %s\n", var);
	finishTemplate();
	exit(0);

}
///////////

void lexLine()
{
	char* var = malloc(sizeof(char) * 1000);
	eq = malloc(sizeof(struct token) * 1000);
	lexCount = 0;
	a = 0;
	int opParen = 0;
	int closParen = 0;
	while(array[a] != ';')
	{	
		if(array[a] == '-' && (lexCount == 0 || eq[lexCount - 1].grammar != '#'))
		{
			if (eq[lexCount - 1].grammar == ')')
			{
				eq[lexCount++].grammar = array[a];
				a++;
				continue;
			}
			else if (eq[lexCount - 1].grammar == 't')
			{
				eq[lexCount++].grammar = array[a];
				a++;
				continue;
			}
			getNum();
			continue;
		}
		////////////
		else if (isalpha(array[a]))
		{
			
			for(int i = 0; isalpha(array[a]) != 0; i++)
			{
				var[i] = array[a++];
			}

			if(isFunction(var)== 0)
			{
				// if(evalFunction(var,a) == 0)
				// {
				// 	printf("  %%t%d = call void @%s(",temp, var);

				// 	while(top != -1)
				// 	{
				// 		printf("i32 %s", popInt());
				// 		if(top == -1)
				// 			break;
				// 		printf(", ");

				// 	}
				// 	printf(")\n");
				// 	loadTempVar(var);
				
				// }
				// else 
					loadTempVar(var);
				eq[lexCount].grammar = 't';
				eq[lexCount++].tempVar = temp++;

			var = malloc(sizeof(char) * 1000);
				
				while ((opParen != closParen) || opParen == 0)
   				{	
   					if (array[a] == '(')
   					{
   						opParen++;
   					}
   					else if (array[a] == ')')
   					{
   						closParen++;
   					}

   					a++;
				}
				continue;
			}
			if(array[a] == '(' && isFunction(var) == 0)
			{
				fprintf(stderr, "error: use of undeclared function %s\n", var);
				finishTemplate();
				exit(0);
			}
			eq[lexCount].grammar = 't';
			loadTempVar(var);
			eq[lexCount++].tempVar = temp++;
			var = malloc(sizeof(char) * 1000);
			continue;
		}
		///////////
		else if(isdigit(array[a]) != 0)
		{
			getNum();
			continue;
		}
		else 
		{
			eq[lexCount++].grammar = array[a];
			a++;
			continue;
		}
	}
	eq[lexCount].last = 'E';

}
//***************************************************************

// proj 2 *******************************************************
int storeVarNum(char* var)
{
	for (int i = 0; i < allIntCount; i++)
	{
		if (strcmp(allInt[i].var, var) == 0)
		{
			printf("  %%t%d = call i32 @read_integer()\n", temp);
			printf("  store i32 %%t%d, i32* %%t%d\n", temp ++, allInt[i].tempNum);
		}
	}
}

int isVarCreated(char* var)
{
	for (int i = 0; i < allIntCount; i++)
	{
		if (strcmp(allInt[i].var, var) == 0)
		{
			return 0;
		}
	}

	return 1;
}

char readVar()
{
	char* var = malloc(sizeof(char) * 1000);
	while (c == ' ' || c == '\n')
		c = fgetc(file);

	for(int i = 0; isalpha(c) != 0; i++)
	{
		var[i] = c;
		c = fgetc(file);
	}

	if (isVarCreated(var) == 0)
		storeVarNum(var);

	else 
	{
		fprintf(stderr, "error: use of undeclared variable %s\n", var);
		finishTemplate();
		exit(0);
	}
}

char* getVar()
{
	char* var = malloc(sizeof(char) * 1000);
	int i = 0;
	while (c == 10 || c == 32)
		c = fgetc(file);
	
	for(i = 0;c != ';'; i++)
	{
		if (c == '{')
		{
			var[i] = '\0';
			doFunctionShit(var);
			return "function";
		}

		var[i] = c;
		c = fgetc(file);
	} 
		
		return var;
}
/////////////////check....////////////////
void createVar()
{
	char* currVar = getVar();

	if(strcmp("function", currVar) == 0)
	{
		return;
	}

	if(isVarCreated(currVar) == 0)
	{
		fprintf(stderr, "error: multiple definitions of %s\n", currVar);
		finishTemplate();
		exit(0);
	}

	c = fgetc(file);

	for (int i = 0; i < allIntCount; i++)
	{
		if (strcmp(allInt[i].var, currVar) == 0)
			return;
	}

	strcpy(allInt[allIntCount].var, currVar);
	allInt[allIntCount].tempNum = temp++;
	printf("  %%t%d = alloca i32\n", allInt[allIntCount++].tempNum);
}

void getExpr()
{
	int i = 0;
	
	while (c != ';' && c != EOF)
   	{
   		if(c == 10 || (c == 32 || c == '	'))
   		{
   			c = fgetc(file);
   			continue;
   		}
   		else if(c == '=')
   		{
   			c = fgetc(file);
   			continue;
   		}
   		if (isalpha(c))
   		{
   			array[i++] = c;
   			c = fgetc(file);
   			continue;

   		}
   		array[i++] = c;
   		c = fgetc(file);
	}
	array[i] = ';';

	// printf("(%s)", array);
}

void clearArray(char* array2)
{
	for (int i = 0; i < sizeof(array2); i++)
	{
		array2[i] = 0;
	}
}
//Proj3***************************************

void getCondExp()
{
	int i = 0;
	int opParen = 0;
	int closParen = 0;
	
	while (c == ' ')
		c = fgetc(file);
	
	while ((opParen != closParen) || opParen == 0)
   	{
   		if(c == 10 || c == 32)
   		{
   			c = fgetc(file);
   			continue;
   		}
   		else if (c == '(')
   			opParen++;
   		else if (c == ')')
   			closParen++;

   		else if (c == '|' || (c == '=' || c == '&'))
   		{
   			array[i++] = c;
   			c = fgetc(file);
   			c = fgetc(file);
   			continue;
   		}
   		else if (c == '!')
   		{
   			c = fgetc(file);
   			if (c == '=')
   			{
   				array[i++] = '@';
   				c = fgetc(file);
   				continue;
   			}
   			else 
   			{
   				array[i++] = '!';
   				continue;
   			}
   		}
   		
   		else if (isalpha(c))
   		{
   			array[i++] = c;
   			c = fgetc(file);
   			continue;
   		}

   		array[i++] = c;
   		c = fgetc(file);
	}
	array[i] = ';';
}

void evalWhile(char* statement)
{
	int thisLabel = headNum++;
	int endLabel = labelNum++;
	int body = bodyNum++;


	 printf("   br label %%head%d", thisLabel);
	 printf("\nhead%d:\n", thisLabel);
	 getCondExp();
	 lexLine();
	 evalPost(inToPost(), "if");
	 clearArray(array);

	 printf("  br i1 %%t%d, label %%body%d, label %%label%d\n", temp - 1, body, endLabel);

	 printf ("\nbody%d:\n", body);

	 while(c != '}')
	 {
		statement = getStatement();
		evalStatement(statement);
	 }
	 printf("  br label %%head%d", thisLabel);

	 printf ("\nlabel%d:\n", endLabel);

}

void evalIf(char* statement)
{
	int thisLabel = labelNum++;
	int endLabel = labelNum++;
	getCondExp();
	lexLine();
	evalPost(inToPost(), statement);
	clearArray(array);
	FILE* tempFile = NULL;
	int a;
	while(c == ' ' || c == '\n' || c == '	')
	{
		c = fgetc(file);
	}

	printf("  br i1 %%t%d, label %%label%d, label %%label%d\n", temp - 1, thisLabel, endLabel);
	printf("\nlabel%d:\n", thisLabel);

	if(c == '{')
	{
		while(c != '}')
		{
			statement = getStatement();
			evalStatement(statement);
		}

		c = fgetc(file);
		statement = getStatement();

		if(strcmp("else", statement) == 0)
		{

			printf("  br label %%label%d\n", labelNum);
			printf("\nlabel%d:\n", endLabel);
			while(c != '}')
			{
				statement = getStatement();
				evalStatement(statement);
			}

			printf("  br label %%label%d\n", labelNum);
			printf("\nlabel%d:\n", labelNum++);
		}
		else 
		{

			printf("  br label %%label%d\n", endLabel);
			printf("\nlabel%d:\n", endLabel);
			evalStatement(statement);
		}

	}
	
	else 
	{

		statement = getStatement();
		evalStatement(statement);
		statement = getStatement();
		if(strcmp("else", statement) == 0)
		{

			printf("  br label %%label%d\n", labelNum);
			printf("\nlabel%d:\n", endLabel);
			while(c ==  ' ' || c == '\n' || c == '	')
				c = fgetc(file);
	
			statement = getStatement();
			evalStatement(statement);

			printf("  br label %%label%d\n", labelNum);
			printf("\nlabel%d:\n", labelNum++);
		}
		else 
		{

			printf("  br label %%label%d\n", endLabel);
			printf("\nlabel%d:\n", endLabel);
			evalStatement(statement);
		}


	}

}

////////////proj 4 stupid bitch///////////////////////////////////////


struct token* funcInToPost()
{
	funcPostEq = malloc(sizeof(struct token) * 1000);
    char x;
    int q = 0, z = 0; 
    while(funcEq[q].last != 'E')
    {
        if(funcEq[q].grammar == '#')
        {
        	funcPostEq[z].number = funcEq[q].number;
          	funcPostEq[z++].grammar = '#';
        }
        
        else if(funcEq[q].grammar == 't')
        {
        	funcPostEq[z].tempVar = funcEq[q].tempVar;
          	funcPostEq[z++].grammar = 't';
        }

		else if(funcEq[q].grammar == '(')
            push(funcEq[q].grammar);
        else if(funcEq[q].grammar == ')')
        {
            while((x = pop()) != '(')
            {
                funcPostEq[z++].grammar = x;
            }
        }
        else
        {
            while(priority(charStack[top]) >= priority(funcEq[q].grammar))
            {
                funcPostEq[z++].grammar = pop();
            }
            push(funcEq[q].grammar);
        }
        q++;
    }
    while(top != -1)
    {
        funcPostEq[z++].grammar = pop();
    }

    funcPostEq[z].last = 'E';
 //    for(int i = 0; funcPostEq[i].last != 'E'; i++)
	// {
	// 	if(funcPostEq[i].grammar == '#')
	// 	{
	// 		printf ("(%d)\n",funcPostEq[i].number);
	// 	}

	// 	printf("(%c)\n", funcPostEq[i].grammar);
	// }
    return funcPostEq;
}

void getFuncNum()
{
	char* buffer = malloc(sizeof(char) * 1000);
	int i = 0;
	int neg = 0;

	if (funcArray[funcA]  == '-')
	{
		neg = 1;
		funcA++;
	}

	while(isdigit(funcArray[funcA]) != 0)
	{
		buffer[i++] = funcArray[funcA];
		funcA++;
	}

	if (neg == 1)
	{
		int hold = atoi(buffer);
		hold *= -1;
		sprintf(buffer, "%d", hold);
	}
	
	funcEq[funcLexCount].grammar = '#';
	funcEq[funcLexCount++].number = atoi(buffer);
}

void lexFuncLine()
{
	char* var = malloc(sizeof(char) * 1000);
	funcEq = malloc(sizeof(struct token) * 1000);
	funcLexCount = 0;
	funcA = 0;

	while(funcArray[funcA] != ';')
	{	
		if(funcArray[funcA] == '-' && (funcLexCount == 0 || funcEq[funcLexCount - 1].grammar != '#'))
		{
			if (funcEq[funcLexCount - 1].grammar == ')')
			{
				funcEq[funcLexCount++].grammar = funcArray[funcA];
				funcA++;
				continue;
			}
			else if (funcEq[funcLexCount - 1].grammar == 't')
			{
				funcEq[funcLexCount++].grammar = funcArray[funcA];
				funcA++;
				continue;
			}
			getFuncNum();
			continue;
		}
		////////////
		else if (isalpha(funcArray[funcA]))
		{
			for(int i = 0; isalpha(funcArray[funcA]) != 0; i++)
			{

				var[i] = funcArray[funcA++];
			}

			
			funcEq[funcLexCount].grammar = 't';
			loadTempVar(var);
			funcEq[funcLexCount++].tempVar = temp++;
			var = malloc(sizeof(char) * 1000);
			continue;
		}
		///////////
		else if(isdigit(funcArray[funcA]) != 0)
		{
			getFuncNum(); ////change
			continue;
		}
		else 
		{
			funcEq[funcLexCount++].grammar = funcArray[funcA];
			funcA++;
			continue;
		}
	}
	funcEq[funcLexCount].last = 'E';

	// for(int i = 0; funcEq[i].last != 'E'; i++)
	// {
	// 	if(funcEq[i].grammar == '#')
	// 	{
	// 		printf ("(%d)\n",funcEq[i].number);
	// 	}

	// 	printf("(%c)\n", funcEq[i].grammar);
	// }

}
int evalFunction(char *function, int a)
{
	int funcA = 0;

	int i = 0;
	int opParen = 0;
	int closParen = 0;
	
	while (array[a] == ' ')
		a++;

	funcArray[funcA] = array[a];
	
	while ((opParen != closParen) || opParen == 0)
   	{
   		if(array[a] == 10 || array[a] == 32)
   		{
   			a++;
   			continue;
   		}
   		else if (array[a] == '(')
   			opParen++;
   		else if (array[a] == ')')
   			closParen++;

   		else if (array[a] == '|' || (array[a] == '=' || array[a] == '&'))
   		{
   			funcArray[funcA++] = array[a];
   			a++;
   			a++;
   			continue;
   		}
   		else if (array[a] == '!')
   		{
   			a++;
   			if (array[a] == '=')
   			{
   				funcArray[funcA++] = '@';
   				a++;
   				continue;
   			}
   			else 
   			{
   				funcArray[funcA] = '!';
   				continue;
   			}
   		}
   		
   		else if (isalpha(array[a]))
   		{
   			funcArray[funcA++] = array[a];
   			a++;
   			continue;
   		}

   		funcArray[funcA++] = array[a];
   		a++;
	}
	funcArray[funcA] = ';';

	lexFuncLine();
	return evalPost(funcInToPost(), "function");
}

int isFunction(char* expression)
{
	for(int i = 0 ; i < functionsCount; i++)
	{
		if(strcmp(functions[i], expression) == 0)
		{
			return 0 ;
		}

		else 
		{
			return 1;

		}
	}
}
char* doFunctionShit(char* funcProto)
{
	char* function = malloc(sizeof(char) * 1000);
	char* statement = malloc(sizeof(char) * 100);
	char* theVar;
	
	allInt = malloc(sizeof(struct intToken) * 100);
	allIntCount = 0;
	
	int q = 0;
	int i = 0;
	
	for(i = 0; funcProto[i] != '('; i++)
	{
		function[i] = funcProto[i];
	}

	strcpy(functions[functionsCount++], function);

	while (funcProto[i] == ' ' || funcProto[i] == '(')
		i++;

	while(funcProto[i] != ')')
	{
		while (funcProto[i] == ' ' || funcProto[i] == ',')
			i++;

		while(isalpha(funcProto[i]))
		{
			i++;
		}

		while (funcProto[i] == ' ')
			i++;


		theVar = malloc(sizeof(char) * 100);
		
		while(isalpha(funcProto[i]))
		{
			theVar[q++] = funcProto[i++];
		}
		q = 0;

		allInt[allIntCount].tempNum = temp++;
		strcpy(allInt[allIntCount++].var, theVar);

		while (funcProto[i] == ' ')
			i++;

		if(funcProto[i] == ')')
			break;
		else 
			i++;

		
	}

	printf("\ndefine i32 @%s(", function);
	for(int g = 0; g < allIntCount; g++)
	{
		if(g>0)
		{
			printf(", ");
		}
		printf("i32");
	}
	printf(") {\n\n");


	while(c == ' ' ||c == '\n' ||c == '	' ||c == '{')
		c = fgetc(file);

	for(int i = 0; i < allIntCount; i++)
	{
		printf("  %%t%d = alloca i32\n", allInt[i].tempNum);
		printf("  store i32 %%%d, i32* %%t%d\n", i, allInt[i].tempNum);
	}

	while (c != '}')
	{
		if (c == 32 || c == 10)
		{
			c = fgetc(file);
			continue;
		}

		statement = getStatement();
		evalStatement(statement);
		c = fgetc(file);
	}


	printf("\n}\n");

	// printf ("%s",function);

	//print header
	// print/store inputs
	// do body
	// close 
	//
}

void doReturn()
{
	char *retVar = malloc(sizeof(char) * 1000);

	while(c == ' ' || c == '\n')
	{
		c = fgetc(file);
	}

	if(isdigit(c))
	{
		printf("\nret i32");
		while(c != ';')
		c = fgetc(file);
		return;
	}

	getExpr();
	lexLine();
	evalPost(inToPost(), "if");

	printf("\nret i32");
	printf(" %%t%d\n", temp - 1);

	// eq = malloc(sizeof(struct token) * 1000);
	// postEq = malloc(sizeof(struct token) * 1000);

	// allInt = malloc (sizeof(struct intToken) * 1000);
	// allIntCount = 0;

	// funcEq= malloc(sizeof(struct token) * 1000);
	// funcPostEq = malloc(sizeof(struct token) * 1000);
}

// Evaluates the statement
void evalStatement(char* statement)
{
	if (strcmp("print", statement) == 0)
	{
		getExpr();
		lexLine();
		evalPost(inToPost(), statement);
		clearArray(array);
		return;
	}

	else if (strcmp("int", statement) == 0)
	{
		createVar();
		clearArray(array);
		return;
	}
	
	else if (strcmp("read", statement) == 0)
	{
		readVar();
		return;
	}
	else if (strcmp("if", statement) == 0)
	{
		evalIf(statement);
		clearArray(array);
		return;
	}
	else if (strcmp("while", statement) == 0)
	{
		evalWhile(statement);
		clearArray(array);
		return;
	}
	else if (strcmp("return", statement) == 0)
	{
		doReturn();
		return;
	}
	
	else if (isalpha(statement[0]))
	{
		getExpr();
		lexLine();
		evalPost(inToPost(), statement);
		clearArray(array);
		return;
	}
}

// Kicks off evaluation of simpleC.
void beginEval()
{
	char* statement = malloc(sizeof(char) * 100);
	
	c = fgetc(file);
	allInt = malloc (sizeof(struct intToken) * 1000);
	while (c != EOF)
	{
		if (c == 32 || c == 10)
		{
			c = fgetc(file);
			continue;
		}

		statement = getStatement();
		evalStatement(statement);
		c = fgetc(file);
	}
}

// the following two functions print template.
void startTemplate()
{
	FILE* template = fopen("startTemplate.ll", "r");
	int ch;

	if (template == NULL)
		 printf ("Template could not be opened.\n");
	else 
	{
		while((ch = fgetc(template)) != EOF)
		{
			printf("%c", ch);
		}
	}
}

void finishTemplate(){ printf("\n  ret i32 0 \n}\n"); }

void openSimpleC(char* fileName)
{
	file = fopen(fileName, "r");

	if (file == NULL)
		printf ("File could not be opened.\n");
	else 
		beginEval();
}

int main(int argc, char ** argv)
{
	startTemplate();
	openSimpleC(argv[1]);
	finishTemplate();

	return (0);
}