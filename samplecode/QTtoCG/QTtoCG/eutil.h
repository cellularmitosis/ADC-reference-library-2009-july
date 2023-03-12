#ifndef EUTIL_H_
#define EUTIL_H_

//??#include "getopt.h"

extern const char *program_name;

void set_program_name(const char *p);

int int_value(int c, char *arg, void (*usage)(void));

float float_value(int c, const char *arg, void (*usage)(void));

void invalid_number_of_arguments(void (*usage)(void))
    __attribute__((noreturn));

void eprintf(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

void error(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

void fatal(const char *format, ...)
    __attribute__((format(printf, 1, 2), noreturn));


#endif	/* EUTIL_H_ */
