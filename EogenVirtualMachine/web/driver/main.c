
#include "../runner/Kasper.h"
#include <emscripten.h>

void EMSCRIPTEN_KEEPALIVE executeFunction(char* funcName, char* args) {
    json_value *jsonValue = json_parse(args, strlen(args));
    struct Object *body = process_value(jsonValue, 0);
    execFunc(funcName, body->value);
}

int main(int argc, char **argv) {
    chdir("root");
    printf("started loading...\n");
    setStringifyObj(stringifyObject);
    prepareDataStructs();
    executeFile("@ClientSideFunctions.elp");
    prepareFunctions();
    executeFunction("main", "{}");
    printf("loaded.\n");
}