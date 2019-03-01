#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define STRING_SIZE 16
#define STRING_CAPACITY (STRING_SIZE - 1)
#define MAX_CELLS_IN_DEFINITION 16
#define MAX_DEFINITIONS_IN_DICTIONARY 1024
#define MAX_STACK_SIZE 1024 

typedef enum {
	CONSTANT = 0,
	BEGIN_DEFINE = 1,
	END_DEFINE = 2,
	TO_RETURN_STACK = 3,
	FROM_RETURN_STACK = 4,
	PRINT_STACK = 5,
	PEEK_STACK = 6,
	PLUS = 7
} command_t;

typedef enum {
	EXECUTE = 0,
	COMPILE = 1
} run_mode_t;

typedef struct {
	char name[STRING_SIZE];
	int inlined;
	int size;
	int commands[MAX_CELLS_IN_DEFINITION];
} definition_t;

run_mode_t g_mode;
char g_word[STRING_SIZE];
definition_t g_dictionary[MAX_DEFINITIONS_IN_DICTIONARY];
int g_dictionary_i;
int g_stack[MAX_STACK_SIZE];
int g_stack_i;

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

definition_t* find_word(char* str) {
	for (int i = g_dictionary_i; i > 0; --i) {
		char* defname = g_dictionary[i - 1].name;
		if (string_equals(str, defname))
			return &g_dictionary[i - 1];
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
	for (int i = 0; i < g_stack_i; ++i)
		printf("%d\n", g_stack[i]);
}

void append(int data) {
	definition_t def;
	switch (g_mode) {
		case EXECUTE:
			g_stack[g_stack_i++] = data;
			break;
		case COMPILE:
			def = g_dictionary[g_dictionary_i];
			def.commands[def.size++] = CONSTANT; 
			def.commands[def.size++] = data;
			break;
	}
}

void append_definition(definition_t* def) {
	int i = 0;
	while (i < def->size) {
		switch (def->commands[i]) {
			case CONSTANT:
				append(def->commands[i + 1]);
				i += 2;
				break;

			case PLUS:
				switch (g_mode) {
					case EXECUTE: {
						int sum = g_stack[g_stack_i - 1] + g_stack[g_stack_i - 2];
						g_stack_i -= 2;
						append(sum);
						i += 1;
						break;
					}
					case COMPILE:
						printf("NOT IMPLEMENTED");
						exit(1);
						break;	
				}
				break;

			case PEEK_STACK: {
				print_stack();
				i += 1;
				break;
			}
		}
	}
}

void interpret_word(void) {
	definition_t* def = find_word(g_word);
	if (def) {
		append_definition(def);
	} else if (is_number(g_word)) {
		append(number(g_word));
	}
}

void add_builtin_to_dictionary(char* str, command_t command) {
	definition_t* def = &g_dictionary[g_dictionary_i++];
	for (char* c = str; *c != 0; ++c) {
		def->name[c - str] = *c;
	}
	def->inlined = 1;
	def->size = 1;
	def->commands[0] = command;
}

void init_dictionary(void) {
	add_builtin_to_dictionary(":", BEGIN_DEFINE);
	add_builtin_to_dictionary(";", END_DEFINE);
	add_builtin_to_dictionary(">>0", TO_RETURN_STACK);
	add_builtin_to_dictionary("0>>", FROM_RETURN_STACK);
	add_builtin_to_dictionary(".", PRINT_STACK);
	add_builtin_to_dictionary("..", PEEK_STACK);
	add_builtin_to_dictionary("+", PLUS);
}

int main(int argc, char **argv) 
{
	init_dictionary();
	while (1) {
		read_word();
		interpret_word();
	}
}
