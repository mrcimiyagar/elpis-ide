
#include <float.h>
#include <sys/time.h>
#include <math.h>
#include <uuid/uuid.h>
#include "../api/IO/ConsolePrinter.h"
#include "../api/IO/HttpServer.h"

void* equal(void* value1, void* value2);
void* not(void* value1);
void* executeIntern(bool addNewLayer, char* c, unsigned long length, struct Dictionary* entriesDict);
void calculate(int investigateId);
void* ride();
void execute(char* fileTreePath);
void prepareDataStructs();
void executeFile(char* file);
void prepareFunctions();
void execFunc(char* funcName, struct Dictionary* args);
char* stringifyObject(void* data);
char* stringifyArray(struct Array* array);
void* multiply(void* value1, void* value2);
void onExit();