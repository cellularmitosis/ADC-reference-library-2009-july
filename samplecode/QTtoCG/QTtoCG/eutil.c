#include "eutil.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *program_name = NULL;

void
set_program_name(const char *p)
{
    char *s;

    s = strrchr(p, '/');
    program_name = (s != 0) ? (s + 1) : p;
}

int
int_value(int c, char *arg, void (*usage)(void))
{
    char *end;
    int value;

    value = strtol(arg, &end, 10);
    if (end == arg || end[0] != '\0') {
	if (c > 0) {
	    fprintf(stderr, "%s: invalid argument for `%c' flag: `%s'.\n",
		    program_name, c, arg);
	} else {
	    fprintf(stderr, "%s: invalid argument: `%s'.\n",
		    program_name, arg);
	}
	if (usage != NULL)
	    (*usage)();
	exit(EXIT_FAILURE);
    }
    return value;
}

float
float_value(int c, const char *arg, void (*usage)(void))
{
    float v;
    char *end;

    v = strtod(arg, &end);
    if (end == arg || end[0] != '\0') {
	fprintf(stderr, "%s: invalid argument for `-%c' flag: `%s'.\n",
		program_name, c, arg);
	if (usage != NULL)
	    (*usage)();
	exit(EXIT_FAILURE);
    }
    return v;
}

void
invalid_number_of_arguments(void (*usage)(void))
{
    error("invalid number of arguments.");
    if (usage != NULL)
	(*usage)();
    exit(EXIT_FAILURE);
}

void
eprintf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void
error(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    if (program_name != NULL)
	fprintf(stderr, "%s: ", program_name);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void
fatal(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    if (program_name != NULL)
	fprintf(stderr, "%s: ", program_name);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}
