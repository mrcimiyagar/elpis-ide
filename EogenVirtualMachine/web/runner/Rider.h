
#include <float.h>
#include <sys/time.h>
#include <math.h>
#include <uuid/uuid.h>
#include "../api/IO/ConsolePrinter.h"
#include "../structures/array/array.h"

void* executeIntern(char* c, unsigned long length, Dictionary* entriesDict);
void calculate(int investigateId);
void* ride();
void execute(char* c, long length);
char* stringifyObject(Object* obj);
char* stringifyArray(Array* array);
void* multiply(void* value1, void* value2);