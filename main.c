#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define STRING_CAPACITY 16
#define STRING_MAX_COUNT 1024
#define WORD_CAPACITY 32
#define WORD_MAX_COUNT 1024

typedef size_t cell_t;

typedef struct {
	char* chars;
} string_t;

typedef enum {
	ERROR_READ_INPUT = 101,
	ERROR_STRING_MAX_COUNT = 102,
	ERROR_STRING_CAPACITY = 103
} failure_t;

char g_string_heap[STRING_CAPACITY * STRING_MAX_COUNT];
int g_string_count;

void fail(failure_t failure) {
	switch (failure) {
		case ERROR_READ_INPUT:			puts("ERROR: failed to read input stream.\n");		break;
		case ERROR_STRING_MAX_COUNT:	puts("ERROR: too many strings.\n");					break;
		case ERROR_STRING_CAPACITY:		puts("ERROR: string is too long.\n");				break;
	}			
	exit(failure);
}

void init_string(string_t* str) {
	if (g_string_count >= STRING_MAX_COUNT)
		fail(ERROR_STRING_MAX_COUNT);
	str->chars = g_string_heap + (g_string_count++ * STRING_CAPACITY);
}

int read_until(string_t* str, char end) {
	int c;
	size_t len = 0;
	while (c = getc(stdin)) {
		if (c == EOF) {
			if (feof(stdin))
				return 1;
			fail(ERROR_READ_INPUT);
		}
		if (end == c)
			return 0;
		if (len >= STRING_CAPACITY)
			fail(ERROR_STRING_CAPACITY);
		str->chars[len++] = (char) c;
	}
}

void write_string(string_t* str) {
	puts(str->chars);
}

int main(int argc, char **argv) 
{
	string_t word;
	while (1) {	
		init_string(&word);
		read_until(&word, '\n');
		write_string(&word);
	}
}
