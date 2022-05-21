
#include <stdbool.h>
#include <glob.h>

struct Code {
    char* type;
    void* owner;
    short alive;
};

struct Array {
    struct Code base;
    int used;
    int size;
    void** array;
};

struct String {
    struct Code base;
    char* value;
};
 
struct CodeBlock {
    struct Code base;
    char* data;
};

struct Pair {
    struct Code base;
    void* first;
    void* second;
};

struct ListDataItem {
    void* data;
    struct ListDataItem* prev;
    struct ListDataItem* next;
};

struct List {
    struct Code base;
    int size;
    struct ListDataItem* endPointer;
    struct ListDataItem* startPointer;
    void  (*append)(struct List*, void*);
    void  (*delete)(struct List*, void*);
};

struct elt {
    struct elt *next;
    struct String* key;
    void *value;
};

struct StackDataItem {
    int level;
    void* data;
    struct StackDataItem* prev;
};

struct Stack {
    struct Code base;
    char* stackName;
    int stackSize;
    struct StackDataItem* item;
    void (*push)(struct Stack*, void*);
    void* (*top)(struct Stack*);
    void* (*pop)(struct Stack*);
    int (*size)(struct Stack*);
    bool (*isEmpty)(struct Stack*);
    struct StackDataItem* (*iterator)(struct Stack*);
};

struct Dictionary {
    struct Code base;
    char* id;
    int size;
    int n;
    struct elt **table;
};

struct Empty {
    struct Code base;
};

struct Value {
    struct Code base;
    int valueType;
};

struct StringValue {
    struct Value base;
    char* value;
};

struct DoubleValue {
    struct Value base;
    double value;
};

struct FloatValue {
    struct Value base;
    float value;
};

struct LongValue {
    struct Value base;
    long value;
};

struct IntValue {
    struct Value base;
    int value;
};

struct ShortValue {
    struct Value base;
    short value;
};

struct BoolValue {
    struct Value base;
    bool value;
};

struct WrapperValue {
    struct Value base;
    void* value;
};

struct Object {
    struct Code base;
    struct Dictionary* value;
    struct Dictionary* funcs;
};

struct Identifier {
    struct Code base;
    struct String* id;
};

struct Reference {
    struct Code base;
    struct Identifier* currentChain;
    void* restOfTheChain;
};

struct Index {
    struct Code base;
    struct List* index;
    void* var;
    void* restOfTheChain;
};

struct Period {
    struct Code base;
    void* start;
    void* end;
};

struct Prop {
    struct Code base;
    struct Identifier* id;
    unsigned long loc;
    struct CodeBlock* value;
};

struct Function {
    struct Code base;
    struct String* funcName;
    struct List* params;
    unsigned long loc;
    struct CodeBlock* codes;
};

struct Constructor {
    struct Code base;
    struct List* params;
    struct CodeBlock* body;
    unsigned long loc;
};

struct Class {
    struct Code base;
    struct String* className;
    struct List* inheritance;
    struct List* behavior;
    struct List* properties;
    struct Dictionary* functions;
    struct Constructor* constructor;
};

struct CodePack {
    struct Code base;
    struct CodeBlock* code;
    unsigned long loc;
    unsigned long pointer;
};

struct HttpServerArg {
    struct Stack* ds;
    void* (*serve)(bool addNewLayer, char* c, unsigned long length, struct Dictionary* entries);
    struct Object* server;
    char* (*stringifyObj)(void*);
};

typedef enum Method {UNSUPPORTED, GET, HEAD, POST} Method;

typedef struct Header {
    char *name;
    char *value;
    struct Header *next;
} Header;

typedef struct Request {
    struct Code base;
    enum Method method;
    char *url;
    char *version;
    struct Header *headers;
    char *body;
} Request;