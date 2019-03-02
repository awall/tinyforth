#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define STRING_SIZE 16
#define STRING_CAPACITY (STRING_SIZE - 1)
#define MAX_DEFINITIONS_IN_DICTIONARY 1024
#define MAX_STACK_SIZE 1024
#define MAX_RETURN_STACK_SIZE 64
#define MAX_INSTRUCTIONS_IN_PROGRAM 4096

typedef enum {
	I_NOOP, I_LIT, I_PLUS, I_TO_0, I_FROM_0, I_JMP
} instruction_t;

typedef struct {
	char name[STRING_SIZE];
	instruction_t* instructions;
} definition_t;

typedef enum {
	M_EXECUTE, M_COMPILE
} run_mode_t;

definition_t g_definitions[MAX_DEFINITIONS_IN_DICTIONARY];
definition_t* g_definitions_end = g_definitions;

instruction_t g_program[MAX_INSTRUCTIONS_IN_PROGRAM * 2];
instruction_t* g_program_end = g_program;
instruction_t* g_program_counter = 0;

run_mode_t g_mode;
char g_word[STRING_SIZE];

int g_stack[MAX_STACK_SIZE];
int* g_stack_end = g_stack;

int g_return_stack[MAX_RETURN_STACK_SIZE];
int* g_return_stack_end = g_return_stack;

int whitespace(int c) {
	return c == EOF || c == ' ' || c == '\n';
}

int string_equals(char* s1, char* s2) {
	for (int i = 0; 1; ++i) {
		if (!s1[i] && !s2[i])
			return 1;
		if (s1[i] != s2[i])
			return 0;
	}
}

void read_word(void) {
	size_t i;
	for (i = 0; i < STRING_CAPACITY; ++i) {
		int c = getchar();
		if (whitespace(c))
			break;
		g_word[i] = (char) c;
	}
	g_word[i] = 0;
}

definition_t* find_definition(char* str) {
	for (definition_t* d = g_definitions_end - 1; d >= g_definitions; --d) {
		if (string_equals(str, d->name))
			return d;
	}
	return 0;
}

int is_number(char *str) {
	for (char *c = str; *c != 0; ++c) {
		if (*c < '0' || *c > '9')
			return 0;
	}
	return 1;
}

int number(char* str) {
	int x = 0;
	for (char *c = str; *c != 0; ++c) {
		x *= 10;
		x += *c - '0';
	}
	return x;
}

void print_stack(void) {
	for (int* s = g_stack; s < g_stack_end; ++s)
		printf("%d\n", *s);
}

void fail(void) {
	printf("FAILURE! stupid you!");
	exit(1);
}

void compile_instruction(instruction_t inst, int data) {
	*g_program_end++ = inst;
	*g_program_end++ = data;
}

void execute_plus(void) {
	int a = *(g_stack_end - 1);
	int b = *(g_stack_end - 2);
	*(g_stack_end - 2)= a + b;
	--g_stack_end;
}

void execute_instruction(void) {
	instruction_t inst = *g_program_counter++;
	int data = *g_program_counter++;

	switch (inst) {
		case I_NOOP:
			break;
		case I_LIT:
			*g_stack_end++ = data;
			break;
		case I_PLUS:
			execute_plus();
			break;
		case I_TO_0:
			*g_return_stack_end++ = *--g_stack_end;
			break;
		case I_FROM_0:
			*g_stack_end++ = *--g_return_stack_end;
			break;
		case I_JMP:
			g_program_counter = (instruction_t*) *--g_stack_end;
			break;
	}	
}

void interpret_word(void) {
	definition_t* def = find_definition(g_word);
	if (def) {
		if (g_mode == M_EXECUTE) {
			*g_return_stack_end++ = 0;
			g_program_counter = def->instructions;
			while (1) {
				execute_instruction();
				if (!g_program_counter)
					break;
			}
		} else {
			compile_instruction(I_LIT, (int) (g_program_end + 8));
			compile_instruction(I_TO_0, 0);
			compile_instruction(I_LIT, (int) def->instructions);
			compile_instruction(I_JMP, 0);
		}
	}
	else if (is_number(g_word)) {
		int num = number(g_word);
		if (g_mode == M_EXECUTE) {
			*g_stack_end++ = num;
		} else {
			compile_instruction(I_LIT, num);
		}
	}
	else if (string_equals(":", g_word)) {
		if (g_mode != M_EXECUTE)
			fail();
		read_word();
		definition_t* next = g_definitions_end++;
		next->instructions = g_program_end;
		for (char* t = next->name, *c = g_word; *c != 0; ++t, ++c)
			*t = *c;
		g_mode = M_COMPILE;
	}
	else if (string_equals(";", g_word)) {
		if (g_mode != M_COMPILE)
			fail();
		compile_instruction(I_FROM_0, 0);
		compile_instruction(I_JMP, 0);
		g_mode = M_EXECUTE;
	}
	else if (string_equals("+", g_word)) {
		if (g_mode == M_EXECUTE) {
			execute_plus();
		} else {
			compile_instruction(I_PLUS, 0);
		}
	}
	else if (string_equals("..", g_word)) {
		print_stack();
	}
}

int main(int argc, char **argv) 
{
	while (1) {
		read_word();
		interpret_word();
	}
}