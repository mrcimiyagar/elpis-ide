#include "Rider.h"
#include "../api/Cipher/Sha256.h"
#include "../api/String/String.h"

struct Stack codeLengthStack;
struct Stack bufferStack;
struct Stack expStack;
struct Stack dataStack;

unsigned long codeLength = 0;
char* code;
int machineState = 0x00;
unsigned long pointer = 0;

long measureLog10l(long input) {
    if (input == 0) return 0;
    else if (input < 0) input *= -1;
    return log10l(input);
}

float measureLog10f(float input) {
    if (input == 0) return 0;
    else if (input < 0) input *= -1;
    return log10f(input);
}

double measureLog10(double input) {
    if (input == 0) return 0;
    else if (input < 0) input *= -1;
    return log10(input);
}

char* concat(const char *s1, char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char* stringifyObject(Object* obj) {
    void* stringifyRaw = dict_get(obj->funcs, "stringify");
    Dictionary* entriesDict = dict_new();
    dict_add(entriesDict, "this", obj);
    if (stringifyRaw == NULL) {
        struct ListDataItem* iterator = toList(obj->value)->listPointer;
        char* result = "{";
        while (iterator != NULL) {
            struct Pair* pair = iterator->data;
            result = concat(result, pair->first);
            result = concat(result, ": ");
            if (strcmp(((Code*)pair->second)->type, "Object") == 0) {
                result = concat(result, stringifyObject((Object*)pair->second));
            } else if (strcmp(((Code*)pair->second)->type, "Array") == 0) {
                result = concat(result, stringifyArray(((Array*)pair->second)));
            }  else if ((*(Value*)pair->second).valueType == 0x01) {
                result = concat(result, (*(StringValue*)pair->second).value);
            } else if ((*(Value*)pair->second).valueType == 0x02) {
                ShortValue val1 = *(ShortValue*)pair->second;
                bool isNegative = false;
                if (val1.value < 0) {
                    val1.value *= -1;
                    isNegative = true;
                }
                char str[(int)(ceilf(measureLog10f(val1.value)+1)*sizeof(char))];
                sprintf(str, "%hd", val1.value);
                char* str2 = &str[0];
                if (isNegative) str2 = concat("-", str2);
                result = concat(result, str2);
            }
            else if ((*(Value*)pair->second).valueType == 0x06) {
                IntValue val1 = *(IntValue*)pair->second;
                bool isNegative = false;
                if (val1.value < 0) {
                    val1.value *= -1;
                    isNegative = true;
                }
                char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                sprintf(str, "%d", val1.value);
                char* str2 = &str[0];
                if (isNegative) str2 = concat("-", str2);
                result = concat(result, str2);
            }
            else if ((*(Value*)pair->second).valueType == 0x03) {
                LongValue val1 = *(LongValue*)pair->second;
                bool isNegative = false;
                if (val1.value < 0) {
                    val1.value *= -1;
                    isNegative = true;
                }
                char str[(int)(ceill(measureLog10l(val1.value)+1)*sizeof(char))];
                sprintf(str, "%ld", val1.value);
                char* str2 = &str[0];
                if (isNegative) str2 = concat("-", str2);
                result = concat(result, str2);
            }
            else if ((*(Value*)pair->second).valueType == 0x04) {
                FloatValue val1 = *(FloatValue*)pair->second;
                char str[(int)(ceilf(measureLog10f(val1.value)+1)*sizeof(char))];
                sprintf(str, "%f", val1.value);
                char* str2 = &str[0];
                str2 = concat("-", str2);
                result = concat(result, str2);
            }
            else if ((*(Value*)pair->second).valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue*)pair->second;
                bool isNegative = false;
                if (val1.value < 0) {
                    val1.value *= -1;
                    isNegative = true;
                }
                char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                sprintf(str, "%f", val1.value);
                char* str2 = &str[0];
                if (isNegative) str2 = concat("-", str2);
                result = concat(result, str2);
            }
            if (iterator->prev != NULL)
                result = concat(result, ", ");
            iterator = iterator->prev;
        }
        result = concat(result, "}");
        return result;
    } else {
        Function* stringify = (Function*) stringifyRaw;
        StringValue *stringified = executeIntern(stringify->codes->data, stringify->loc, entriesDict);
        return stringified->value;
    }
}

char* stringifyArray(Array* array) {
    char* textToPrint = "[";
    for (unsigned long i = 0; i < array->used; i++) {
        void* arrayItem = array->array[i];
        if (strcmp(((Code*)arrayItem)->type, "Array") == 0) {
            char* stringified = stringifyArray((Array*)arrayItem);
            textToPrint = concat(textToPrint, stringified);
        }
        else if (strcmp(((Code*)arrayItem)->type, "Object") == 0) {
            Object* obj = (Object*) arrayItem;
            Function* stringify = (Function*) dict_get(obj->funcs, "stringify");
            Dictionary* entriesDict = dict_new();
            dict_add(entriesDict, "this", obj);
            if (stringify == NULL) {
                struct ListDataItem* iterator = toList(obj->value)->listPointer;
                char* result = "{";
                while (iterator != NULL) {
                    struct Pair* pair = iterator->data;
                    result = concat(result, pair->first);
                    result = concat(result, ": ");
                    if (strcmp(((Code*)pair->second)->type, "Object") == 0) {
                        result = concat(result, stringifyObject((Object*)pair->second));
                    } else if (strcmp(((Code*)pair->second)->type, "Array") == 0) {
                        result = concat(result, stringifyArray(((Array*)pair->second)));
                    }  else if ((*(Value*)pair->second).valueType == 0x01) {
                        result = concat(result, (*(StringValue*)pair->second).value);
                    } else if ((*(Value*)pair->second).valueType == 0x02) {
                        ShortValue val1 = *(ShortValue*)pair->second;
                        bool isNegative = false;
                        if (val1.value < 0) {
                            val1.value *= -1;
                            isNegative = true;
                        }
                        char str[(int)(ceilf(measureLog10f(val1.value)+1)*sizeof(char))];
                        sprintf(str, "%hd", val1.value);
                        char* str2 = &str[0];
                        if (isNegative) str2 = concat("-", str2);
                        result = concat(result, str2);
                    }
                    else if ((*(Value*)pair->second).valueType == 0x06) {
                        IntValue val1 = *(IntValue*)pair->second;
                        bool isNegative = false;
                        if (val1.value < 0) {
                            val1.value *= -1;
                            isNegative = true;
                        }
                        char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                        sprintf(str, "%d", val1.value);
                        char* str2 = &str[0];
                        if (isNegative) str2 = concat("-", str2);
                        result = concat(result, str2);
                    }
                    else if ((*(Value*)pair->second).valueType == 0x03) {
                        LongValue val1 = *(LongValue*)pair->second;
                        bool isNegative = false;
                        if (val1.value < 0) {
                            val1.value *= -1;
                            isNegative = true;
                        }
                        char str[(int)(ceill(measureLog10l(val1.value)+1)*sizeof(char))];
                        sprintf(str, "%ld", val1.value);
                        char* str2 = &str[0];
                        if (isNegative) str2 = concat("-", str2);
                        result = concat(result, str2);
                    }
                    else if ((*(Value*)pair->second).valueType == 0x04) {
                        FloatValue val1 = *(FloatValue*)pair->second;
                        bool isNegative = false;
                        if (val1.value < 0) {
                            val1.value *= -1;
                            isNegative = true;
                        }
                        char str[(int)(ceilf(measureLog10f(val1.value)+1)*sizeof(char))];
                        sprintf(str, "%f", val1.value);
                        char* str2 = &str[0];
                        if (isNegative) str2 = concat("-", str2);
                        result = concat(result, str2);
                    }
                    else if ((*(Value*)pair->second).valueType == 0x05) {
                        DoubleValue val1 = *(DoubleValue*)pair->second;
                        bool isNegative = false;
                        if (val1.value < 0) {
                            val1.value *= -1;
                            isNegative = true;
                        }
                        char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                        sprintf(str, "%f", val1.value);
                        char* str2 = &str[0];
                        if (isNegative) str2 = concat("-", str2);
                        result = concat(result, str2);
                    }
                    if (iterator->prev != NULL)
                        result = concat(result, ", ");
                    iterator = iterator->prev;
                }
                result = concat(result, "}");
                return result;
            } else {
                void* stringified = executeIntern(stringify->codes->data, stringify->loc, entriesDict);
                StringValue* stringValue = (StringValue*) stringified;
                textToPrint = concat(textToPrint, stringValue->value);
            }
        }
        if (i < array->used - 1)
            textToPrint = concat(textToPrint, ", ");
    }
    textToPrint = concat(textToPrint, "]");
    return textToPrint;
}

void* routeAndResolve(char* funcRef, Dictionary* entries) {
    if (strcmp(funcRef, "print") == 0) {
        ConsolePrinter printer = createConsolePrinter();
        void* v = dict_get(entries, "text");
        if (strcmp((*(Code*)v).type, "Object") == 0) {
            Object* obj = (Object*) v;
            printer.print(stringifyObject(obj));
        }
        else if (strcmp((*(Code*)v).type, "Array") == 0) {
            printer.print(stringifyArray((Array*) v));
        } else if ((*(Value*)v).valueType == 0x01) {
            printer.print((*(StringValue*)v).value);
        } else if ((*(Value*)v).valueType == 0x02) {
            ShortValue val1 = *(ShortValue*)v;
            char str[(int)(ceilf(measureLog10f(val1.value)+1)*sizeof(char))];
            sprintf(str, "%hd", val1.value);
            char* str2 = &str[0];
            printer.print(str2);
        }
        else if ((*(Value*)v).valueType == 0x06) {
            IntValue val1 = *(IntValue*)v;
            char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
            sprintf(str, "%d", val1.value);
            char* str2 = &str[0];
            printer.print(str2);
        }
        else if ((*(Value*)v).valueType == 0x03) {
            LongValue val1 = *(LongValue*)v;
            char str[(int)(ceill(measureLog10l(val1.value)+1)*sizeof(char))];
            sprintf(str, "%ld", val1.value);
            char* str2 = &str[0];
            printer.print(str2);
        }
        else if ((*(Value*)v).valueType == 0x04) {
            FloatValue val1 = *(FloatValue*)v;
            char str[(int)(ceilf(measureLog10f(val1.value)+1)*sizeof(char))];
            sprintf(str, "%f", val1.value);
            char* str2 = &str[0];
            printer.print(str2);
        }
        else if ((*(Value*)v).valueType == 0x05) {
            DoubleValue val1 = *(DoubleValue*)v;
            char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
            sprintf(str, "%f", val1.value);
            char* str2 = &str[0];
            printer.print(str2);
        }
        StringValue* value = custom_malloc(sizeof(StringValue));
        value->base.base.type = "Value";
        value->base.valueType = 0x01;
        value->value = "done.";
        return value;
    }
    else if (strcmp(funcRef, "sha256") == 0) {
        StringValue* result = custom_malloc(sizeof(StringValue));
        result->base.base.type = "Value";
        result->base.valueType = 0x01;
        char* value = sha256_hex((char*)((StringValue*)dict_get(entries, "content"))->value);
        result->value = value;
        return result;
    } else if (strcmp(funcRef, "len") == 0) {
        void* rawArr = dict_get(entries, "arr");
        if (strcmp(((Code*)rawArr)->type, "Array") == 0) {
            IntValue* result = custom_malloc(sizeof(IntValue));
            result->base.valueType = 0x06;
            result->base.base.type = "Value";
            result->value = ((Array*)rawArr)->used;
            return result;
        }
    } else if (strcmp(funcRef, "time.now") == 0) {
        LongValue* result = custom_malloc(sizeof(LongValue));
        result->base.valueType = 0x03;
        result->base.base.type = "Value";
        struct timeval time;
        gettimeofday(&time, NULL);
        int64_t s1 = (int64_t)(time.tv_sec) * 1000;
        int64_t s2 = (time.tv_usec / 1000);
        result->value = s1 + s2;
        return result;
    } else if (strcmp(funcRef, "append") == 0) {
        Array* array = (Array*) dict_get(entries, "list");
        insertArray(array, dict_get(entries, "listItem"));
        StringValue* value = custom_malloc(sizeof(StringValue));
        value->base.base.type = "Value";
        
        value->base.valueType = 0x01;
        value->value = "done.";
        return value;
    } else if (strcmp(funcRef, "last") == 0) {
        Array* array = (Array*) dict_get(entries, "arr");
        printf("\nvay %zu\n", array->used);
        return array->array[array->used - 1];
    } else if (strcmp(funcRef, "stringify") == 0) {
        void* obj = dict_get(entries, "obj");
        Code* temp = (Code*)obj;
        if (strcmp(temp->type, "Array") == 0) {
            StringValue* result = custom_malloc(sizeof(StringValue));
            result->base.base.type = "Value";
            result->base.valueType = 0x01;
            char* resValue = stringifyArray((Array*)obj);
            result->value = resValue;
            return result;
        }
        else if (strcmp(temp->type, "Object") == 0) {
            StringValue* result = custom_malloc(sizeof(StringValue));
            result->base.base.type = "Value";
            result->base.valueType = 0x01;
            result->value = stringifyObject((Object*)obj);
            return result;
        } else if (strcmp(temp->type, "Value")) {
            if ((*(Value *) obj).valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) obj;
                char str[(int) (ceilf(measureLog10f(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%hd", val1.value);
                char *str2 = &str[0];
                StringValue *result = custom_malloc(sizeof(StringValue));
                result->base.base.type = "Value";
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if ((*(Value *) obj).valueType == 0x06) {
                IntValue val1 = *(IntValue *) obj;
                char str[(int) (ceil(measureLog10(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%d", val1.value);
                char *str2 = &str[0];
                StringValue *result = custom_malloc(sizeof(StringValue));
                result->base.base.type = "Value";
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if ((*(Value *) obj).valueType == 0x03) {
                LongValue val1 = *(LongValue *) obj;
                char str[(int) (ceill(measureLog10l(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%ld", val1.value);
                char *str2 = &str[0];
                StringValue *result = custom_malloc(sizeof(StringValue));
                result->base.base.type = "Value";
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if ((*(Value *) obj).valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) obj;
                char str[(int) (ceilf(measureLog10f(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%f", val1.value);
                char *str2 = &str[0];
                StringValue *result = custom_malloc(sizeof(StringValue));
                result->base.base.type = "Value";
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if ((*(Value *) obj).valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) obj;
                char str[(int) (ceil(measureLog10(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%f", val1.value);
                char *str2 = &str[0];
                StringValue *result = custom_malloc(sizeof(StringValue));
                result->base.base.type = "Value";
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if ((*(Value *) obj).valueType == 0x01) {
                return obj;
            }
        }
    } else if (strcmp(funcRef, "uuid") == 0) {
        uuid_t binuuid;
        uuid_generate_random(binuuid);
        char *uuid = malloc(37);
#ifdef capitaluuid
        uuid_unparse_upper(binuuid, uuid);
#elif lowercaseuuid
        uuid_unparse_lower(binuuid, uuid);
#else
        uuid_unparse(binuuid, uuid);
#endif
        StringValue* result = custom_malloc(sizeof(StringValue));
        result->base.base.type = "Value";
        
        result->base.valueType = 0x01;
        result->value = uuid;
        return result;
    } else if (strcmp(funcRef, "replace") == 0) {
        StringValue* str = (StringValue*)dict_get(entries, "str");
        StringValue* from = (StringValue*)dict_get(entries, "from");
        StringValue* to = (StringValue*)dict_get(entries, "to");
        char* resultStr = malloc(strlen(str->value));
        strcpy(resultStr, str->value);
        replaceWord(resultStr, from->value, to->value);
        StringValue* result = custom_malloc(sizeof(StringValue));
        result->base.base.type = "Value";
        
        result->base.valueType = 0x01;
        result->value = resultStr;
        return result;
    } else if (strcmp(funcRef, "match_object_structure") == 0) {
        Object* obj = (Object*) dict_get(entries, "obj");
        Array* schema = (Array*) dict_get(entries, "structure");
        for (unsigned long i = 0; i < schema->used; i++) {
            StringValue* fieldName = (StringValue*) schema->array[i];
            if (dict_get(obj->value, fieldName->value) == NULL) {
                BoolValue* result = custom_malloc(sizeof(BoolValue));
                result->base.base.type = "Value";
                
                result->base.valueType = 0x07;
                result->value = false;
                return result;
            }
        }
        BoolValue* result = custom_malloc(sizeof(BoolValue));
        result->base.base.type = "Value";
        result->base.valueType = 0x07;
        result->value = true;
        return result;
    }
}

void* sum(void* value1, void* value2) {
    Value val1Raw = *(Value*)value1;
    Value val2Raw = *(Value*)value2;
    if (val1Raw.valueType == 0x06 ||
        val1Raw.valueType == 0x02 ||
        val1Raw.valueType == 0x03 ||
        val1Raw.valueType == 0x04 ||
        val1Raw.valueType == 0x05) {
        if (val2Raw.valueType == 0x06 ||
            val2Raw.valueType == 0x02 ||
            val2Raw.valueType == 0x03 ||
            val2Raw.valueType == 0x04 ||
            val2Raw.valueType == 0x05) {
            long double result = 0;
            if (val1Raw.valueType == 0x02) {
                ShortValue val1 = *(ShortValue*)value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue*)value2;
                    result = (long double)val1.value + (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue*)value2;
                    result = (long double)val1.value + (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                }
            } else if (val1Raw.valueType == 0x06) {
                IntValue val1 = *(IntValue*)value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue*)value2;
                    result = (long double)val1.value + (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                }
            } else if (val1Raw.valueType == 0x03) {
                LongValue val1 = *(LongValue*)value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                }
            } else if (val1Raw.valueType == 0x04) {
                FloatValue val1 = *(FloatValue*)value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                }
            } else if (val1Raw.valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue*)value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    ShortValue* resValue = custom_malloc(sizeof(ShortValue));
                    short r = (short)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
                else if (result < INT32_MAX) {
                    IntValue* resValue = custom_malloc(sizeof(IntValue));
                    int r = (int)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
                else if (result < INT64_MAX) {
                    LongValue* resValue = custom_malloc(sizeof(LongValue));
                    long r = (long)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
            else {
                if (result < FLT_MAX) {
                    FloatValue* resValue = custom_malloc(sizeof(FloatValue));
                    float r = (float)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
                else if (result < DBL_MAX) {
                    DoubleValue* resValue = custom_malloc(sizeof(DoubleValue));
                    double r = (double)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        }
        else if (val2Raw.valueType == 0x01) {
            StringValue val2 = *(StringValue*)value2;
            char* str2 = "";
            if (val1Raw.valueType == 0x02) {
                ShortValue val1 = *(ShortValue*)value1;
                char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                sprintf(str, "%hd", val1.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            } else if (val1Raw.valueType == 0x06) {
                IntValue val1 = *(IntValue*)value1;
                char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                sprintf(str, "%d", val1.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            } else if (val1Raw.valueType == 0x03) {
                LongValue val1 = *(LongValue*)value1;
                char str[(int)(ceill(measureLog10l(val1.value)+1)*sizeof(char))];
                sprintf(str, "%ld", val1.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            } else if (val1Raw.valueType == 0x04) {
                FloatValue val1 = *(FloatValue*)value1;
                char str[(int)(ceilf(measureLog10f(val1.value)+1)*sizeof(char))];
                sprintf(str, "%f", val1.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            } else if (val1Raw.valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue*)value1;
                char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                sprintf(str, "%f", val1.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            }
            StringValue* resValue = custom_malloc(sizeof(StringValue));
            char* result = concat(str2, val2.value);
            resValue->value = result;
            resValue->base.valueType = 0x01;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
        else if (val2Raw.valueType == 0x07) {
            StringValue val2 = *(StringValue*)value2;
            double result = 0;
            if (val1Raw.valueType == 0x02) {
                ShortValue val1 = *(ShortValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            } else if (val1Raw.valueType == 0x06) {
                IntValue val1 = *(IntValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            } else if (val1Raw.valueType == 0x03) {
                LongValue val1 = *(LongValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            } else if (val1Raw.valueType == 0x04) {
                FloatValue val1 = *(FloatValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            } else if (val1Raw.valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            }
            if (floor(result) == result) {
                if (result < INT16_MAX) {
                    ShortValue* resValue = custom_malloc(sizeof(ShortValue));
                    short r = (short)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
                else if (result < INT32_MAX) {
                    IntValue* resValue = custom_malloc(sizeof(IntValue));
                    int r = (int)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
                else if (result < INT64_MAX) {
                    LongValue* resValue = custom_malloc(sizeof(LongValue));
                    long r = (long)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
            else {
                if (result < FLT_MAX) {
                    FloatValue* resValue = custom_malloc(sizeof(FloatValue));
                    float r = (float)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
                else if (result < DBL_MAX) {
                    DoubleValue* resValue = custom_malloc(sizeof(DoubleValue));
                    double r = (double)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        }
    }
    else if (val1Raw.valueType == 0x01) {
        StringValue val1 = *(StringValue*)value1;
        if (val2Raw.valueType == 0x06 ||
            val2Raw.valueType == 0x02 ||
            val2Raw.valueType == 0x03 ||
            val2Raw.valueType == 0x04 ||
            val2Raw.valueType == 0x05) {
            char* str2 = "";
            if (val2Raw.valueType == 0x02) {
                ShortValue val2 = *(ShortValue*)value2;
                char str[(int)(ceil(measureLog10(val2.value)+1)*sizeof(char))];
                sprintf(str, "%hd", val2.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            } else if (val2Raw.valueType == 0x06) {
                IntValue val2 = *(IntValue*)value2;
                char str[(int)(ceil(measureLog10(val2.value)+1)*sizeof(char))];
                sprintf(str, "%d", val2.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            } else if (val2Raw.valueType == 0x03) {
                LongValue val2 = *(LongValue*)value2;
                char str[(int)(ceill(measureLog10l(val2.value)+1)*sizeof(char))];
                sprintf(str, "%ld", val2.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            } else if (val2Raw.valueType == 0x04) {
                FloatValue val2 = *(FloatValue*)value2;
                char str[(int)(ceilf(measureLog10f(val2.value)+1)*sizeof(char))];
                sprintf(str, "%f", val2.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            } else if (val2Raw.valueType == 0x05) {
                DoubleValue val2 = *(DoubleValue*)value2;
                char str[(int)(ceil(measureLog10(val2.value)+1)*sizeof(char))];
                sprintf(str, "%f", val2.value);
                str2 = malloc(strlen(str));
                strcpy(str2, str);
            }
            StringValue* resValue = custom_malloc(sizeof(StringValue));
            char* dest = concat(val1.value, str2);
            resValue->value = dest;
            resValue->base.valueType = 0x01;
            resValue->base.base.type = "Value";
            
            return resValue;
        } else if (val2Raw.valueType == 0x01) {
            StringValue val2 = *(StringValue*)value2;
            char* result = concat(val1.value, val2.value);
            StringValue* resValue = custom_malloc(sizeof(StringValue));
            resValue->value = result;
            resValue->base.valueType = 0x01;
            resValue->base.base.type = "Value";
            
            return resValue;
        } else if (val2Raw.valueType == 0x07) {
            BoolValue val2 = *(BoolValue*)value2;
            char* boolStr = val2.value ? "true" : "false";
            char* result = concat(val1.value, boolStr);
            StringValue* resValue = custom_malloc(sizeof(StringValue));
            resValue->value = result;
            resValue->base.valueType = 0x01;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw.valueType == 0x07) {
        BoolValue val1 = *(BoolValue*)value1;
        if (val2Raw.valueType == 0x06 ||
            val2Raw.valueType == 0x02 ||
            val2Raw.valueType == 0x03 ||
            val2Raw.valueType == 0x04 ||
            val2Raw.valueType == 0x05) {
            double result = 0;
            if (val2Raw.valueType == 0x02) {
                ShortValue val2 = *(ShortValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            } else if (val2Raw.valueType == 0x06) {
                IntValue val2 = *(IntValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            } else if (val2Raw.valueType == 0x03) {
                LongValue val2 = *(LongValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            } else if (val2Raw.valueType == 0x04) {
                FloatValue val2 = *(FloatValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            } else if (val2Raw.valueType == 0x05) {
                DoubleValue val2 = *(DoubleValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            }
            if (floor(result) == result) {
                if (result < INT16_MAX) {
                    short r = (short)result;
                    ShortValue* resValue = custom_malloc(sizeof(ShortValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
                else if (result < INT32_MAX) {
                    int r = (int)result;
                    IntValue* resValue = custom_malloc(sizeof(IntValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
                else if (result < INT64_MAX) {
                    long r = (long)result;
                    LongValue* resValue = custom_malloc(sizeof(LongValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
            else {
                if (result < FLT_MAX) {
                    float r = (float)result;
                    FloatValue* resValue = custom_malloc(sizeof(FloatValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
                else if (result < DBL_MAX) {
                    double r = (double)result;
                    DoubleValue* resValue = custom_malloc(sizeof(DoubleValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        } else if (val2Raw.valueType == 0x01) {
            StringValue val2 = *(StringValue*)value2;
            char* result = concat(val1.value ? "true" : "false", val2.value);
            StringValue* resValue = custom_malloc(sizeof(StringValue));
            resValue->value = result;
            resValue->base.valueType = 0x01;
            resValue->base.base.type = "Value";
            
            return resValue;
        } else if (val2Raw.valueType == 0x07) {
            BoolValue val2 = *(BoolValue*)value2;
            bool result = val1.value || val2.value;
            BoolValue* resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
}

void* subtract(void* value1, void* value2) {
    Value val1Raw = *(Value*)value1;
    Value val2Raw = *(Value*)value2;
    if (val1Raw.valueType == 0x06 ||
        val1Raw.valueType == 0x02 ||
        val1Raw.valueType == 0x03 ||
        val1Raw.valueType == 0x04 ||
        val1Raw.valueType == 0x05) {
        if (val2Raw.valueType == 0x06 ||
            val2Raw.valueType == 0x02 ||
            val2Raw.valueType == 0x03 ||
            val2Raw.valueType == 0x04 ||
            val2Raw.valueType == 0x05) {
            long double result = 0;
            if (val1Raw.valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    ShortValue *resValue = custom_malloc(sizeof(ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT32_MAX) {
                    IntValue *resValue = custom_malloc(sizeof(IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT64_MAX) {
                    LongValue *resValue = custom_malloc(sizeof(LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    FloatValue *resValue = custom_malloc(sizeof(FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < DBL_MAX) {
                    DoubleValue *resValue = custom_malloc(sizeof(DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        }
    }
}

void* multiply(void* value1, void* value2) {
    Value val1Raw = *(Value*)value1;
    Value val2Raw = *(Value*)value2;
    if (val1Raw.valueType == 0x06 ||
        val1Raw.valueType == 0x02 ||
        val1Raw.valueType == 0x03 ||
        val1Raw.valueType == 0x04 ||
        val1Raw.valueType == 0x05) {
        if (val2Raw.valueType == 0x06 ||
            val2Raw.valueType == 0x02 ||
            val2Raw.valueType == 0x03 ||
            val2Raw.valueType == 0x04 ||
            val2Raw.valueType == 0x05) {
            long double result = 0;
            if (val1Raw.valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    ShortValue *resValue = custom_malloc(sizeof(ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT32_MAX) {
                    IntValue *resValue = custom_malloc(sizeof(IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT64_MAX) {
                    LongValue *resValue = custom_malloc(sizeof(LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    FloatValue *resValue = custom_malloc(sizeof(FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < DBL_MAX) {
                    DoubleValue *resValue = custom_malloc(sizeof(DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        }
    }
}

void* divide(void* value1, void* value2) {
    Value val1Raw = *(Value*)value1;
    Value val2Raw = *(Value*)value2;
    if (val1Raw.valueType == 0x06 ||
        val1Raw.valueType == 0x02 ||
        val1Raw.valueType == 0x03 ||
        val1Raw.valueType == 0x04 ||
        val1Raw.valueType == 0x05) {
        if (val2Raw.valueType == 0x06 ||
            val2Raw.valueType == 0x02 ||
            val2Raw.valueType == 0x03 ||
            val2Raw.valueType == 0x04 ||
            val2Raw.valueType == 0x05) {
            long double result = 0;
            if (val1Raw.valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            } else if (val1Raw.valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (val2Raw.valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    ShortValue *resValue = custom_malloc(sizeof(ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT32_MAX) {
                    IntValue *resValue = custom_malloc(sizeof(IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT64_MAX) {
                    LongValue *resValue = custom_malloc(sizeof(LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    FloatValue *resValue = custom_malloc(sizeof(FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < DBL_MAX) {
                    DoubleValue *resValue = custom_malloc(sizeof(DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        }
    }
}

void* mod(void* value1, void* value2) {
    Value val1Raw = *(Value*)value1;
    Value val2Raw = *(Value*)value2;
    if (val1Raw.valueType == 0x06 ||
        val1Raw.valueType == 0x02 ||
        val1Raw.valueType == 0x03 ||
        val1Raw.valueType == 0x04 ||
        val1Raw.valueType == 0x05) {
        if (val2Raw.valueType == 0x06 ||
            val2Raw.valueType == 0x02 ||
            val2Raw.valueType == 0x03 ||
            val2Raw.valueType == 0x04 ||
            val2Raw.valueType == 0x05) {
            long double result = 0;
            if (val1Raw.valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                }
            } else if (val1Raw.valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                }
            } else if (val1Raw.valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    ShortValue *resValue = custom_malloc(sizeof(ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT32_MAX) {
                    IntValue *resValue = custom_malloc(sizeof(IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT64_MAX) {
                    LongValue *resValue = custom_malloc(sizeof(LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        }
    }
}

void* power(void* value1, void* value2) {
    Value* val1Raw = (Value*)value1;
    Value* val2Raw = (Value*)value2;
    if (val1Raw->valueType == 0x06 ||
        val1Raw->valueType == 0x02 ||
        val1Raw->valueType == 0x03 ||
        val1Raw->valueType == 0x04 ||
        val1Raw->valueType == 0x05) {
        if (val2Raw->valueType == 0x06 ||
            val2Raw->valueType == 0x02 ||
            val2Raw->valueType == 0x03 ||
            val2Raw->valueType == 0x04 ||
            val2Raw->valueType == 0x05) {
            long double result = false;
            if (val1Raw->valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                }
            } else if (val1Raw->valueType == 0x06) {
                IntValue* val1 = (IntValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue* val2 = (ShortValue *) value2;
                    result = powl((long double) val1->value, (long double) val2->value);
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = powl((long double) val1->value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = powl((long double) val1->value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = powl((long double) val1->value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = powl((long double) val1->value, (long double) val2.value);
                }
            } else if (val1Raw->valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                }
            } else if (val1Raw->valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                }
            } else if (val1Raw->valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    ShortValue *resValue = custom_malloc(sizeof(ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT32_MAX) {
                    IntValue *resValue = custom_malloc(sizeof(IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT64_MAX) {
                    LongValue *resValue = custom_malloc(sizeof(LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    FloatValue *resValue = custom_malloc(sizeof(FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < DBL_MAX) {
                    DoubleValue *resValue = custom_malloc(sizeof(DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        }
    }
}

void* and(void* value1, void* value2) {
    Value val1Raw = *(Value*)value1;
    Value val2Raw = *(Value*)value2;
    if (value1 == NULL || value2 == NULL) {
        LongValue* result = custom_malloc(sizeof(LongValue));
        result->base.valueType = 0x03;
        
        result->base.base.type = "Value";
        result->value = 0;
        return value2;
    }
    if (val1Raw.valueType == 0x06 ||
        val1Raw.valueType == 0x02 ||
        val1Raw.valueType == 0x03 ||
        val1Raw.valueType == 0x04 ||
        val1Raw.valueType == 0x05) {
        if (val2Raw.valueType == 0x06 ||
            val2Raw.valueType == 0x02 ||
            val2Raw.valueType == 0x03 ||
            val2Raw.valueType == 0x04 ||
            val2Raw.valueType == 0x05) {
            long double result = 0;
            if (val1Raw.valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = val1.value & val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = val1.value & val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = val1.value & val2.value;
                }
            } else if (val1Raw.valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = val1.value & val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = val1.value & val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = val1.value & val2.value;
                }
            } else if (val1Raw.valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = val1.value & val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = val1.value & val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = val1.value & val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    ShortValue *resValue = custom_malloc(sizeof(ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT32_MAX) {
                    IntValue *resValue = custom_malloc(sizeof(IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT64_MAX) {
                    LongValue *resValue = custom_malloc(sizeof(LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    FloatValue *resValue = custom_malloc(sizeof(FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < DBL_MAX) {
                    DoubleValue *resValue = custom_malloc(sizeof(DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        }
    }
    else if (val1Raw.valueType == 0x07) {
        BoolValue* val1 = (BoolValue*) value1;
        if (val2Raw.valueType == 0x07) {
            BoolValue* val2 = (BoolValue*) value2;
            bool result = val1->value & val2->value;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            return resValue;
        }
    }
}

void* or(void* value1, void* value2) {
    if (value1 == NULL || strcmp(((Code*)value1)->type, "Empty") == 0) {
        return value2;
    }
    else if (value2 == NULL || strcmp(((Code*)value2)->type, "Empty") == 0) {
        return value1;
    }
    Value val1Raw = *(Value*)value1;
    Value val2Raw = *(Value*)value2;
    if (val1Raw.valueType == 0x06 ||
        val1Raw.valueType == 0x02 ||
        val1Raw.valueType == 0x03 ||
        val1Raw.valueType == 0x04 ||
        val1Raw.valueType == 0x05) {
        if (val2Raw.valueType == 0x06 ||
            val2Raw.valueType == 0x02 ||
            val2Raw.valueType == 0x03 ||
            val2Raw.valueType == 0x04 ||
            val2Raw.valueType == 0x05) {
            long double result = 0;
            if (val1Raw.valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = val1.value | val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = val1.value | val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = val1.value | val2.value;
                }
            } else if (val1Raw.valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = val1.value | val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = val1.value | val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = val1.value | val2.value;
                }
            } else if (val1Raw.valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw.valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = val1.value | val2.value;
                } else if (val2Raw.valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = val1.value | val2.value;
                } else if (val2Raw.valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = val1.value | val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    ShortValue *resValue = custom_malloc(sizeof(ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT32_MAX) {
                    IntValue *resValue = custom_malloc(sizeof(IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < INT64_MAX) {
                    LongValue *resValue = custom_malloc(sizeof(LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    FloatValue *resValue = custom_malloc(sizeof(FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                } else if (result < DBL_MAX) {
                    DoubleValue *resValue = custom_malloc(sizeof(DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = "Value";
                    
                    return resValue;
                }
            }
        }
    }
    else if (val1Raw.valueType == 0x07) {
        BoolValue* val1 = (BoolValue*) value1;
        if (val2Raw.valueType == 0x07) {
            BoolValue* val2 = (BoolValue*) value2;
            bool result = val1->value | val2->value;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            return resValue;
        }
    }
}

void* equal(void* value1, void* value2) {
    Value* val1Raw = (Value*)value1;
    Value* val2Raw = (Value*)value2;
    if (val1Raw->valueType == 0x06 ||
        val1Raw->valueType == 0x02 ||
        val1Raw->valueType == 0x03 ||
        val1Raw->valueType == 0x04 ||
        val1Raw->valueType == 0x05) {
        if (val2Raw->valueType == 0x06 ||
            val2Raw->valueType == 0x02 ||
            val2Raw->valueType == 0x03 ||
            val2Raw->valueType == 0x04 ||
            val2Raw->valueType == 0x05) {
            bool result = false;
            if (val1Raw->valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) *(long *) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) *(long *) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) *(long *) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) *(long *) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) *(long *) val1.value == (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                }
            }
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x01) {
        StringValue* val1 = (StringValue*) value1;
        if (val2Raw->valueType == 0x01) {
            StringValue* val2 = (StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) == 0;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x07) {
        BoolValue* val1 = (BoolValue*) value1;
        if (val2Raw->valueType == 0x07) {
            BoolValue* val2 = (BoolValue*) value2;
            bool result = val1->value == val2->value;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    BoolValue *resValue = custom_malloc(sizeof(BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = "Value";
    
    return resValue;
}

void* ne(void* value1, void* value2) {
    Value* val1Raw = (Value*)value1;
    Value* val2Raw = (Value*)value2;
    if (val1Raw->valueType == 0x06 ||
        val1Raw->valueType == 0x02 ||
        val1Raw->valueType == 0x03 ||
        val1Raw->valueType == 0x04 ||
        val1Raw->valueType == 0x05) {
        if (val2Raw->valueType == 0x06 ||
            val2Raw->valueType == 0x02 ||
            val2Raw->valueType == 0x03 ||
            val2Raw->valueType == 0x04 ||
            val2Raw->valueType == 0x05) {
            bool result = false;
            if (val1Raw->valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) *(long *) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) *(long *) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) *(long *) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) *(long *) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) *(long *) val1.value != (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value != (long double) val2.value;
                }
            }
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x01) {
        StringValue* val1 = (StringValue*) value1;
        if (val2Raw->valueType == 0x01) {
            StringValue* val2 = (StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) != 0;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x07) {
        BoolValue* val1 = (BoolValue*) value1;
        if (val2Raw->valueType == 0x07) {
            BoolValue* val2 = (BoolValue*) value2;
            bool result = val1->value != val2->value;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    BoolValue *resValue = custom_malloc(sizeof(BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = "Value";
    
    return resValue;
}

void* lt(void* value1, void* value2) {
    Value* val1Raw = (Value*)value1;
    Value* val2Raw = (Value*)value2;
    if (val1Raw->valueType == 0x06 ||
        val1Raw->valueType == 0x02 ||
        val1Raw->valueType == 0x03 ||
        val1Raw->valueType == 0x04 ||
        val1Raw->valueType == 0x05) {
        if (val2Raw->valueType == 0x06 ||
            val2Raw->valueType == 0x02 ||
            val2Raw->valueType == 0x03 ||
            val2Raw->valueType == 0x04 ||
            val2Raw->valueType == 0x05) {
            bool result = false;
            if (val1Raw->valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) *(long *) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) *(long *) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) *(long *) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) *(long *) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) *(long *) val1.value < (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                }
            }
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x01) {
        StringValue* val1 = (StringValue*) value1;
        if (val2Raw->valueType == 0x01) {
            StringValue* val2 = (StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) < 0;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x07) {
        BoolValue* val1 = (BoolValue*) value1;
        if (val2Raw->valueType == 0x07) {
            BoolValue* val2 = (BoolValue*) value2;
            bool result = val1->value < val2->value;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    BoolValue *resValue = custom_malloc(sizeof(BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = "Value";
    
    return resValue;
}

void* le(void* value1, void* value2) {
    Value* val1Raw = (Value*)value1;
    Value* val2Raw = (Value*)value2;
    if (val1Raw->valueType == 0x06 ||
        val1Raw->valueType == 0x02 ||
        val1Raw->valueType == 0x03 ||
        val1Raw->valueType == 0x04 ||
        val1Raw->valueType == 0x05) {
        if (val2Raw->valueType == 0x06 ||
            val2Raw->valueType == 0x02 ||
            val2Raw->valueType == 0x03 ||
            val2Raw->valueType == 0x04 ||
            val2Raw->valueType == 0x05) {
            bool result = false;
            if (val1Raw->valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) *(long *) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) *(long *) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) *(long *) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) *(long *) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) *(long *) val1.value <= (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                }
            }
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x01) {
        StringValue* val1 = (StringValue*) value1;
        if (val2Raw->valueType == 0x01) {
            StringValue* val2 = (StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) <= 0;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x07) {
        BoolValue* val1 = (BoolValue*) value1;
        if (val2Raw->valueType == 0x07) {
            BoolValue* val2 = (BoolValue*) value2;
            bool result = val1->value <= val2->value;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    BoolValue *resValue = custom_malloc(sizeof(BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = "Value";
    
    return resValue;
}

void* ge(void* value1, void* value2) {
    Value* val1Raw = (Value*)value1;
    Value* val2Raw = (Value*)value2;
    if (val1Raw->valueType == 0x06 ||
        val1Raw->valueType == 0x02 ||
        val1Raw->valueType == 0x03 ||
        val1Raw->valueType == 0x04 ||
        val1Raw->valueType == 0x05) {
        if (val2Raw->valueType == 0x06 ||
            val2Raw->valueType == 0x02 ||
            val2Raw->valueType == 0x03 ||
            val2Raw->valueType == 0x04 ||
            val2Raw->valueType == 0x05) {
            bool result = false;
            if (val1Raw->valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) *(long *) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) *(long *) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) *(long *) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) *(long *) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) *(long *) val1.value >= (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                }
            }
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x01) {
        StringValue* val1 = (StringValue*) value1;
        if (val2Raw->valueType == 0x01) {
            StringValue* val2 = (StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) >= 0;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x07) {
        BoolValue* val1 = (BoolValue*) value1;
        if (val2Raw->valueType == 0x07) {
            BoolValue* val2 = (BoolValue*) value2;
            bool result = val1->value >= val2->value;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    BoolValue *resValue = custom_malloc(sizeof(BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = "Value";
    
    return resValue;
}

void* gt(void* value1, void* value2) {
    Value* val1Raw = (Value*)value1;
    Value* val2Raw = (Value*)value2;
    if (val1Raw->valueType == 0x06 ||
        val1Raw->valueType == 0x02 ||
        val1Raw->valueType == 0x03 ||
        val1Raw->valueType == 0x04 ||
        val1Raw->valueType == 0x05) {
        if (val2Raw->valueType == 0x06 ||
            val2Raw->valueType == 0x02 ||
            val2Raw->valueType == 0x03 ||
            val2Raw->valueType == 0x04 ||
            val2Raw->valueType == 0x05) {
            bool result = false;
            if (val1Raw->valueType == 0x02) {
                ShortValue val1 = *(ShortValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x06) {
                IntValue val1 = *(IntValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x03) {
                LongValue val1 = *(LongValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) *(long *) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) *(long *) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) *(long *) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) *(long *) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) *(long *) val1.value > (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x04) {
                FloatValue val1 = *(FloatValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                }
            } else if (val1Raw->valueType == 0x05) {
                DoubleValue val1 = *(DoubleValue *) value1;
                if (val2Raw->valueType == 0x02) {
                    ShortValue val2 = *(ShortValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x06) {
                    IntValue val2 = *(IntValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x03) {
                    LongValue val2 = *(LongValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x04) {
                    FloatValue val2 = *(FloatValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (val2Raw->valueType == 0x05) {
                    DoubleValue val2 = *(DoubleValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                }
            }
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x01) {
        StringValue* val1 = (StringValue*) value1;
        if (val2Raw->valueType == 0x01) {
            StringValue* val2 = (StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) > 0;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    else if (val1Raw->valueType == 0x07) {
        BoolValue* val1 = (BoolValue*) value1;
        if (val2Raw->valueType == 0x07) {
            BoolValue* val2 = (BoolValue*) value2;
            bool result = val1->value > val2->value;
            BoolValue *resValue = custom_malloc(sizeof(BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = "Value";
            
            return resValue;
        }
    }
    BoolValue *resValue = custom_malloc(sizeof(BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = "Value";
    
    return resValue;
}

void* not(void* value) {
    Value* valueRaw = (Value*) value;
    if (valueRaw->valueType == 0x07) {
        BoolValue* result = custom_malloc(sizeof(BoolValue));
        result->value = !((BoolValue*)value)->value;
        result->base.base.type = "Value";
        
        result->base.valueType = 0x07;
        return result;
    }
}

void* resolveIndex(struct Index* index) {
    void* target = index->var;
    if (strcmp(((Code *) target)->type, "Array") == 0) {
        Array* array = (Array*)target;
        if (index->index->size == 1 && strcmp(((Code*)index->index->listPointer->data)->type, "Value") == 0) {
            IntValue* indexValue = (IntValue*)index->index->listPointer->data;
            return array->array[indexValue->value];
        }
        else if (index->index->size == 1 && strcmp(((Code*)index->index->listPointer->data)->type, "Period") == 0) {
            struct Period* indexValue = (struct Period*)index->index->listPointer->data;
            int start = ((IntValue*)indexValue->start)->value, end = ((IntValue*)indexValue->end)->value;
            Array* result = custom_malloc(sizeof(Array));
            initArray(result, end - start);
            memcpy(result->array, array->array[start], end - start);
            return result;
        }
    }
    else if (strcmp(((Code*)target)->type, "Value") == 0) {
        Value* rawValue = (Value*)target;
        if (rawValue->valueType == 0x01) {
            StringValue* stringValue = (StringValue*)target;
            StringValue *result = custom_malloc(sizeof(StringValue));
            result->base.base.type = "Value";
            result->base.valueType = 0x01;
            if (index->index->size == 1 && strcmp(((Code*)index->index->listPointer->data)->type, "Value") == 0) {
                IntValue* indexValue = (IntValue*)index->index->listPointer->data;
                result->value = malloc(1 + 1);
                memcpy(result->value, &stringValue->value[indexValue->value], 1);
                return result;
            }
            else if (index->index->size == 1 && strcmp(((Code*)index->index->listPointer->data)->type, "Period") == 0) {
                struct Period* indexValue = (struct Period*)index->index->listPointer->data;
                int start = ((IntValue*)indexValue->start)->value, end = ((IntValue*)indexValue->end)->value;
                result->value = malloc(end - start);
                memcpy(result->value, &(stringValue->value)[start], end - start);
                return result;
            }
        }
    }
}

void* resolveRef(void* refRaw) {
    void* objectChain = NULL;
    void* target = NULL;
    char* refStr = "";
    char* chainName;
    struct StackDataItem *iterator = dataStack.item;
    if (strcmp(((Code*)refRaw)->type, "Reference") == 0) {
        struct Reference* ref = (struct Reference*) refRaw;
        chainName = malloc(strlen(ref->currentChain->id->value) + 1);
        strcpy(chainName, ref->currentChain->id->value);
        target = dict_get((Dictionary *) iterator->data, chainName);
        while (target == NULL && iterator->prev != NULL) {
            iterator = iterator->prev;
            target = dict_get((Dictionary *) iterator->data, ref->currentChain->id->value);
        }
        if (target != NULL && strcmp(((Code *) target)->type, "Object") == 0)
            objectChain = target;
        refStr = chainName;
        refRaw = ref->restOfTheChain;
    }
    else if (strcmp(((Code*)refRaw)->type, "Identifier") == 0) {
        Identifier* ref = (Identifier*) refRaw;
        chainName = malloc(strlen(ref->id->value) + 1);
        strcpy(chainName, ref->id->value);
        target = dict_get((Dictionary *) iterator->data, chainName);
        while (target == NULL && iterator->prev != NULL) {
            iterator = iterator->prev;
            target = dict_get((Dictionary *) iterator->data, ref->id->value);
        }
        if (target != NULL && strcmp(((Code *) target)->type, "Object") == 0)
            objectChain = target;
        refStr = chainName;
        refRaw = NULL;
    }
    else if (strcmp(((Code*)refRaw)->type, "Index") == 0) {
        struct Index* index = (struct Index*) refRaw;
        struct Pair* tempResult = resolveRef(index->var);
        refStr = ((struct Pair*)tempResult->second)->first;
        index->var = tempResult->first;
        target = resolveIndex(index);
        refRaw = index->restOfTheChain;
    }
    while (target != NULL && refRaw != NULL) {
        if (strcmp(((Code*)refRaw)->type, "Reference") == 0) {
            struct Reference* ref = (struct Reference*) refRaw;
            if (strcmp(((Code*)target)->type, "Object") == 0) {
                chainName = malloc(strlen(ref->currentChain->id->value) + 1);
                strcpy(chainName, ref->currentChain->id->value);
                refStr = concat(refStr, ".");
                refStr = concat(refStr, chainName);
                void* target2 = dict_get(((Object*)target)->value, chainName);
                if (target2 != NULL)
                    target = target2;
                else {
                    target2 = dict_get(((Object*)target)->funcs, chainName);
                    if (target2 != NULL)
                        target = target2;
                }
                refRaw = ref->restOfTheChain;
            }
            else if (strcmp(((Code*)target)->type, "Class") == 0) {
                chainName = malloc(strlen(ref->currentChain->id->value) + 1);
                strcpy(chainName, ref->currentChain->id->value);
                refStr = concat(refStr, ".");
                refStr = concat(refStr, chainName);
                target = dict_get(((Class*)target)->functions, chainName);
                refRaw = ref->restOfTheChain;
            }
            else if (strcmp(((Code*)target)->type, "Function") == 0) {
                chainName = malloc(strlen(ref->currentChain->id->value) + 1);
                strcpy(chainName, ref->currentChain->id->value);
                refStr = concat(refStr, ".");
                refStr = concat(refStr, chainName);
                dict_add((Dictionary*) expStack.top(&expStack), "value", target);
                dict_add((Dictionary*) expStack.top(&expStack), "value2", refStr);
                dict_add((Dictionary*) expStack.top(&expStack), "value3", objectChain);
                break;
            }
        }
        else if (strcmp(((Code*)refRaw)->type, "Index") == 0) {
            struct Index* index = (struct Index*) refRaw;
            struct Pair* tempResult = resolveRef(index->var);
            refStr = ((struct Pair*)tempResult->second)->first;
            index->var = tempResult->first;
            target = resolveIndex(index);
            refRaw = index->restOfTheChain;
        }
    }
    struct Pair* result;
    result = custom_malloc(sizeof(*result));
    result->base.type = "Pair";
    result->first = target;
    struct Pair* result2;
    result2 = custom_malloc(sizeof(*result2));
    result2->base.type = "Pair";
    result2->first = refStr;
    result2->second = objectChain;
    result->second = result2;
    return result;
}

int convertBytesToInt(const char bytes[]) {
    return (bytes[3] & 0xff) | ((bytes[2] & 0xff) << 8) | ((bytes[1] & 0xff) << 16) | ((bytes[0] & 0xff) << 24);
}

short convertBytesToShort(const char bytes[]) {
    return (bytes[1] & 0xff) | ((bytes[0] & 0xff) << 8);
}

unsigned long calculateBytes(int investigateId, char* c, unsigned long p) {
    while (!expStack.isEmpty(&expStack)) {
        if (c[p] == 0x4d) {
            p++;
            Empty* empty = custom_malloc(sizeof(Empty));
            dict_add(expStack.top(&expStack), "value", empty);
            return p;
        }
        else if (c[p] == 0x4e) {
            p++;
            char countBytes[4];
            for (int i = 0; i < (int)sizeof(countBytes); i++)
                countBytes[i] = c[p + i];
            p += (int) sizeof(countBytes);
            int count = convertBytesToInt(countBytes);
            Object* obj = custom_malloc(sizeof(Object));
            obj->base.type = "Object";
            obj->funcs = dict_new();
            obj->value = dict_new();
            for (int counter = 0; counter < count; counter++) {
                if (c[p] == 0x01) {
                    p++;
                    char keyLengthBytes[4];
                    for (int i = 0; i < (int)sizeof(keyLengthBytes); i++)
                        keyLengthBytes[i] = c[p + i];
                    p += (int) sizeof(keyLengthBytes);
                    int keyLength = convertBytesToInt(keyLengthBytes);
                    char keyBytes[keyLength];
                    for (int i = 0; i < keyLength; i++)
                        keyBytes[i] = c[p + i];
                    p += keyLength;
                    char* key = malloc(strlen(keyBytes) + 1);
                    strcpy(key, keyBytes);
                    expStack.push(&expStack, dict_new());
                    p = calculateBytes(investigateId, c, p);
                    void* exp = dict_get(expStack.pop(&expStack), "value");
                    dict_add(obj->value, key, exp);
                }
            }
            dict_add(expStack.top(&expStack), "value", obj);
            return p;
        }
        else if (c[p] == 0x4f) {
            p++;
            if (c[p] == 0x01) {
                p++;
                expStack.push(&expStack, dict_new());
                p = calculateBytes(investigateId, c, p);
                dict_add(expStack.top(&expStack), "value", not(dict_get(expStack.pop(&expStack), "value")));
                return p;
            }
        }
        else if (c[p] == 0x68) {
            p++;
            Array* array = custom_malloc(sizeof(Array));
            if (c[p] == 0x01) {
                p++;
                char itemsCountBytes[4];
                for (int i = 0; i < (int) sizeof(itemsCountBytes); i++)
                    itemsCountBytes[i] = c[p + i];
                p += (int) sizeof(itemsCountBytes);
                int itemsCount = convertBytesToInt(itemsCountBytes);
                initArray(array, itemsCount);
                for (int i = 0; i < itemsCount; i++) {
                    if (c[p] == 0x02) {
                        p++;
                        expStack.push(&expStack, dict_new());
                        p = calculateBytes(investigateId, c, p);
                        void* value = dict_get((Dictionary*)expStack.pop(&expStack), "value");
                        insertArray(array, value);
                    }
                }
                dict_add((Dictionary*)expStack.top(&expStack), "value", array);
                return p;
            }
        }
        else if (c[p] == 0x7f) {
            p++;
            struct Reference *ref = custom_malloc(sizeof(struct Reference));
            ref->base.type = "Reference";
            if (c[p] == 0x01) {
                p++;
                expStack.push(&expStack, dict_new());
                p = calculateBytes(0, c, p);
                Identifier *id = (Identifier *) dict_get((Dictionary *) expStack.pop(&expStack), "value");
                ref->currentChain = id;
                if (c[p] == 0x02) {
                    p++;
                    expStack.push(&expStack, dict_new());
                    p = calculateBytes(0, c, p);
                    ref->restOfTheChain = dict_get((Dictionary *) expStack.pop(&expStack), "value");
                }
                else {
                    ref->restOfTheChain = NULL;
                }
                if (c[p] == 0x6d) {
                    p++;
                    if (investigateId == 1) {
                        struct Pair* result = resolveRef(ref);
                        dict_add((Dictionary*) expStack.top(&expStack), "value", result->first);
                        dict_add((Dictionary*) expStack.top(&expStack), "value2", ((struct Pair*)result->second)->first);
                        dict_add((Dictionary*) expStack.top(&expStack), "value3", ((struct Pair*)result->second)->second);
                    } else {
                        dict_add((Dictionary*) expStack.top(&expStack), "value", ref);
                    }
                    return p;
                }
            }
        }
        else if (c[p] == 0x6c) {
            p++;
            struct Index* index = custom_malloc(sizeof(struct Index));
            index->base.type = "Index";
            if (c[p] == 0x01) {
                p++;
                expStack.push(&expStack, dict_new());
                p = calculateBytes(0, c, p);
                void* var = dict_get((Dictionary*)expStack.pop(&expStack), "value");
                index->var = var;
                if (c[p] == 0x02) {
                    p++;
                    char indicesCountBytes[4];
                    for (int i = 0; i < (int) sizeof(indicesCountBytes); i++)
                        indicesCountBytes[i] = c[p + i];
                    p += (int) sizeof(indicesCountBytes);
                    int indicesCount = convertBytesToInt(indicesCountBytes);
                    struct List* indicesList = custom_malloc(sizeof(struct List));
                    initList(indicesList);
                    for (int i = 0; i < indicesCount; i++) {
                        if (c[p] == 0x03) {
                            p++;
                            expStack.push(&expStack, dict_new());
                            p = calculateBytes(investigateId, c, p);
                            void *indexItem = dict_get((Dictionary *) expStack.pop(&expStack), "value");
                            indicesList->append(indicesList, indexItem);
                        }
                    }
                    index->index = indicesList;
                    if (c[p] == 0x04) {
                        p++;
                        expStack.push(&expStack, dict_new());
                        p = calculateBytes(investigateId, c, p);
                        void* restOfChains = dict_get((Dictionary *) expStack.pop(&expStack), "value");
                        index->restOfTheChain = restOfChains;
                    }
                    else {
                        index->restOfTheChain = NULL;
                    }
                    if (c[p] == 0x6b) {
                        p++;
                        if (investigateId == 1) {
                            struct Pair* result = resolveRef(index);
                            dict_add((Dictionary*) expStack.top(&expStack), "value", result->first);
                            dict_add((Dictionary*) expStack.top(&expStack), "value2", ((struct Pair*)result->second)->first);
                            dict_add((Dictionary*) expStack.top(&expStack), "value3", ((struct Pair*)result->second)->second);
                        } else {
                            dict_add((Dictionary*) expStack.top(&expStack), "value", index);
                        }
                        return p;
                    }
                }
            }
        }
        else if (c[p] == 0x6a) {
            p++;
            struct Period* period = custom_malloc(sizeof(struct Period));
            period->base.type = "Period";
            if (c[p] == 0x01) {
                p++;
                expStack.push(&expStack, dict_new());
                p = calculateBytes(investigateId, c, p);
                period->start = dict_get((Dictionary*)expStack.pop(&expStack), "value");
                if (c[p] == 0x02) {
                    p++;
                    expStack.push(&expStack, dict_new());
                    p = calculateBytes(investigateId, c, p);
                    period->end = dict_get((Dictionary *) expStack.pop(&expStack), "value");
                    if (c[p] == 0x69) {
                        p++;
                        dict_add((Dictionary*)expStack.top(&expStack), "value", period);
                        return p;
                    }
                }
            }
        }
        else if (c[p] == 0x55) {
            p++;
            if (c[p] == 0x01) {
                p++;
                Dictionary* expDict = dict_new();
                expStack.push(&expStack, expDict);
                p = calculateBytes(1, c, p);
                void* target = dict_get((Dictionary*)expStack.top(&expStack), "value");
                char* refStr = dict_get((Dictionary*)expStack.top(&expStack), "value2");
                Object* thisObj = dict_get((Dictionary*)expStack.pop(&expStack), "value3");
                if (c[p] == 0x02) {
                    p++;
                    char entriesCountBytes[4];
                    for (int index = 0; index < 4; index++)
                        entriesCountBytes[index] = c[p + index];
                    p += 4;
                    int entriesCount = convertBytesToInt(entriesCountBytes);
                    Dictionary* entriesDict = dict_new();
                    for (int counter = 0; counter < entriesCount; counter++) {
                        if (c[p] == 0x03) {
                            p++;
                            char keyLengthBytes[4];
                            for (int index = 0; index < 4; index++)
                                keyLengthBytes[index] = c[p + index];
                            p += 4;
                            int keyLength = convertBytesToInt(keyLengthBytes);
                            char keyBytes[keyLength];
                            for (int index = 0; index < keyLength; index++)
                                keyBytes[index] = c[p + index];
                            p += keyLength;
                            char *key = malloc(strlen(keyBytes) + 1);
                            strcpy(key, keyBytes);
                            char valueLengthBytes[4];
                            for (int index = 0; index < 4; index++)
                                valueLengthBytes[index] = c[p + index];
                            p += 4;
                            Dictionary *expD = dict_new();
                            expStack.push(&expStack, expD);
                            p = calculateBytes(1, c, p);
                            dict_add(entriesDict, key, dict_get((Dictionary *) expStack.pop(&expStack), "value"));
                        }
                    }
                    if (target == NULL) {
                        dict_add((Dictionary*)expStack.top(&expStack), "value", routeAndResolve(refStr, entriesDict));
                    } else {
                        Function* func = (Function*) target;
                        if (thisObj != NULL)
                            dict_add(entriesDict, "this", thisObj);
                        dict_add((Dictionary*)expStack.top(&expStack), "value", executeIntern(func->codes->data, func->loc, entriesDict));
                    }
                    return p;
                }
            }
        } else if (c[p] == 0x57) {
            p++;
            if (c[p] == 0x01) {
                p++;
                Dictionary* expDict = dict_new();
                expStack.push(&expStack, expDict);
                p = calculateBytes(1, c, p);
                void* target = dict_get((Dictionary*)expStack.pop(&expStack), "value");
                if (c[p] == 0x02) {
                    p++;
                    char entriesCountBytes[4];
                    for (int index = 0; index < 4; index++)
                        entriesCountBytes[index] = c[p + index];
                    p += 4;
                    int entriesCount = convertBytesToInt(entriesCountBytes);
                    Dictionary* entriesDict = dict_new();
                    for (int counter = 0; counter < entriesCount; counter++) {
                        if (c[p] == 0x03) {
                            p++;
                            char keyLengthBytes[4];
                            for (int index = 0; index < 4; index++)
                                keyLengthBytes[index] = c[p + index];
                            p += 4;
                            int keyLength = convertBytesToInt(keyLengthBytes);
                            char keyBytes[keyLength];
                            for (int index = 0; index < keyLength; index++)
                                keyBytes[index] = c[p + index];
                            p += keyLength;
                            char *key = malloc(strlen(keyBytes) + 1);
                            strcpy(key, keyBytes);
                            char valueLengthBytes[4];
                            for (int index = 0; index < 4; index++)
                                valueLengthBytes[index] = c[p + index];
                            p += 4;
                            expStack.push(&expStack, dict_new());
                            p = calculateBytes(1, c, p);
                            void* data = dict_get((Dictionary *) expStack.pop(&expStack), "value");
                            dict_add(entriesDict, key, data);
                        }
                    }
                    if (target != NULL) {
                        Class* classObj = (Class*) target;
                        Object* object = custom_malloc(sizeof(Object));
                        object->base.type = "Object";
                        object->value = dict_new();
                        while (classObj->properties->iteratorHasNext(classObj->properties)) {
                            Prop* prop = (Prop*) classObj->properties->iteratorForward(classObj->properties);
                            char* propId = malloc(strlen(prop->id->id->value) + 1);
                            strcpy(propId, prop->id->id->value);
                            expStack.push(&expStack, dict_new());
                            calculateBytes(1, prop->value->data, 0);
                            dict_add(object->value, propId, dict_get((Dictionary*)expStack.pop(&expStack), "value"));
                        }
                        struct ListDataItem* iterator = classObj->constructor->params->listPointer;
                        while (iterator != NULL) {
                            if (dict_get(entriesDict, ((Identifier*)iterator->data)->id->value) == NULL) {
                                Empty* empty = custom_malloc(sizeof(Empty));
                                empty->base.type = "Empty";
                                dict_add(entriesDict, ((Identifier*)iterator->data)->id->value, empty);
                            }
                            iterator = iterator->prev;
                        }
                        object->funcs = classObj->functions;
                        if (classObj->constructor != NULL) {
                            dict_add(entriesDict, "this", object);
                            executeIntern(classObj->constructor->body->data, classObj->constructor->loc, entriesDict);
                        }
                        dict_add((Dictionary*)expStack.top(&expStack), "value", object);
                    }
                    return p;
                }
            }
        } else if (c[p] == 0x71) {
            p++;
            if (c[p] == 0x01) {
                p++;
                expStack.push(&expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    expStack.push(&expStack, dict_new());
                    p = calculateBytes(1, c, p);
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = sum(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x72) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();
                expStack.push(&expStack, dict);
                p = calculateBytes(1, c, p);
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    expStack.push(&expStack, dict_new());
                    p = calculateBytes(1, c, p);
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = subtract(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x73) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                int tempMachineState = machineState;
                machineState = 0x00;
                p = calculateBytes(1, c, p);
                machineState = tempMachineState;
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    tempMachineState = machineState;
                    machineState = 0x00;
                    p = calculateBytes(1, c, p);
                    machineState = tempMachineState;
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = multiply(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x74) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                int tempMachineState = machineState;
                machineState = 0x00;
                p = calculateBytes(1, c, p);
                machineState = tempMachineState;
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    tempMachineState = machineState;
                    machineState = 0x00;
                    p = calculateBytes(1, c, p);
                    machineState = tempMachineState;
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = divide(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x75) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                int tempMachineState = machineState;
                machineState = 0x00;
                p = calculateBytes(1, c, p);
                machineState = tempMachineState;
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    tempMachineState = machineState;
                    machineState = 0x00;
                    p = calculateBytes(1, c, p);
                    machineState = tempMachineState;
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = mod(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x76) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                int tempMachineState = machineState;
                machineState = 0x00;
                p = calculateBytes(1, c, p);
                machineState = tempMachineState;
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    tempMachineState = machineState;
                    machineState = 0x00;
                    p = calculateBytes(1, c, p);
                    machineState = tempMachineState;
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = power(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x77) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                int tempMachineState = machineState;
                machineState = 0x00;
                p = calculateBytes(1, c, p);
                machineState = tempMachineState;
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    tempMachineState = machineState;
                    machineState = 0x00;
                    p = calculateBytes(1, c, p);
                    machineState = tempMachineState;
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = and(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x78) {
            p++;
            if (c[p] == 0x01) {
                p++;
                expStack.push(&expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    expStack.push(&expStack, dict_new());
                    p = calculateBytes(1, c, p);
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = or(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x79) {
            p++;
            if (c[p] == 0x01) {
                p++;
                Dictionary* dict = dict_new();
                expStack.push(&expStack, dict);
                p = calculateBytes(1, c, p);
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    p = calculateBytes(1, c, p);
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = equal(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x7a) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                int tempMachineState = machineState;
                machineState = 0x00;
                p = calculateBytes(1, c, p);
                machineState = tempMachineState;
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    tempMachineState = machineState;
                    machineState = 0x00;
                    p = calculateBytes(1, c, p);
                    machineState = tempMachineState;
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = gt(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x7b) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                int tempMachineState = machineState;
                machineState = 0x00;
                p = calculateBytes(1, c, p);
                machineState = tempMachineState;
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    tempMachineState = machineState;
                    machineState = 0x00;
                    p = calculateBytes(1, c, p);
                    machineState = tempMachineState;
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = ge(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x7c) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                int tempMachineState = machineState;
                machineState = 0x00;
                p = calculateBytes(1, c, p);
                machineState = tempMachineState;
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    tempMachineState = machineState;
                    machineState = 0x00;
                    p = calculateBytes(1, c, p);
                    machineState = tempMachineState;
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = ne(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x7d) {
            machineState = c[p];
            p++;
            if (c[p] == 0x01) {
                p++;
                machineState = 0x711;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                int tempMachineState = machineState;
                machineState = 0x00;
                p = calculateBytes(1, c, p);
                machineState = tempMachineState;
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    tempMachineState = machineState;
                    machineState = 0x00;
                    p = calculateBytes(1, c, p);
                    machineState = tempMachineState;
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = le(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        } else if (c[p] == 0x7e) {
            p++;
            if (c[p] == 0x01) {
                p++;
                Dictionary* dict = dict_new();

                expStack.push(&expStack, dict);
                p = calculateBytes(1, c, p);
                void* value1 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                if (c[p] == 0x02) {
                    p++;
                    Dictionary* dict2 = dict_new();

                    expStack.push(&expStack, dict2);
                    p = calculateBytes(1, c, p);
                    void* value2 = dict_get((Dictionary*) expStack.pop(&expStack),"value");
                    void* result = lt(value1, value2);
                    dict_add((Dictionary*) expStack.top(&expStack),"value", result);
                    return p;
                }
            }
        }
        else if (c[p] == 0x61) {
            p++;
            char idNameLengthArr[4];
            for (int index = 0; index < (int)sizeof(idNameLengthArr); index++)
                idNameLengthArr[index] = c[p + index];
            p += 4;
            int idNameLength = convertBytesToInt(idNameLengthArr);
            char idNameArr[idNameLength];
            for (int index = 0; index < idNameLength; index++)
                idNameArr[index] = c[p + index];
            p += idNameLength;
            Identifier* id = custom_malloc(sizeof(Identifier));
            id->base.type = "Identifier";
            String* str = custom_malloc(sizeof(String));
            initString(str, strlen(idNameArr) + 1);
            strcpy(str->value, idNameArr);
            id->id = str;
            if (investigateId == 1) {
                struct StackDataItem *iterator = dataStack.item;
                char* idName = malloc(strlen(id->id->value) + 1);
                strcpy(idName, id->id->value);
                void *value = dict_get((Dictionary *) iterator->data, idName);
                while (value == NULL && iterator->prev != NULL) {
                    iterator = iterator->prev;
                    value = dict_get((Dictionary *) iterator->data, idName);
                }
                if (value == NULL)
                    dict_add((Dictionary *) expStack.top(&expStack),"value", id);
                else
                    dict_add((Dictionary *) expStack.top(&expStack),"value", value);
            }
            else
                dict_add((Dictionary *) expStack.top(&expStack),"value", id);
            return p;
        } else if (c[p] == 0x62) {
            p++;
            char valueLengthArr[4];
            for (int index = 0; index < (int)sizeof(valueLengthArr); index++)
                valueLengthArr[index] = c[p + index];
            p += (int)sizeof(valueLengthArr);
            int valueLength = convertBytesToInt(valueLengthArr);
            char valueArr[valueLength];
            for (int index = 0; index < valueLength; index++)
                valueArr[index] = c[p + index];
            p += valueLength;
            StringValue* val = custom_malloc(sizeof(StringValue));
            val->value = malloc(strlen(valueArr)+1);
            strcpy(val->value, valueArr);
            val->base.valueType = 0x01;
            val->base.base.type = "Value";
            dict_add((Dictionary*)expStack.top(&expStack), "value", val);
            return p;
        } else if (c[p] == 0x63) {
            p++;
            char valueArr[8];
            for (int index = 0; index < (int)sizeof(valueArr); index++)
                valueArr[index] = c[p + index];
            p += (int)sizeof(valueArr);
            char* valueRaw = valueArr;
            double value;
            memcpy(&value, valueRaw, sizeof(double));
            DoubleValue* val = custom_malloc(sizeof(DoubleValue));
            val->value = value;
            val->base.valueType = 0x05;
            
            val->base.base.type = "Value";
            dict_add((Dictionary*)expStack.top(&expStack), "value", val);
            return p;
        } else if (c[p] == 0x64) {
            p++;
            char valueArr[4];
            for (int index = 0; index < (int)sizeof(valueArr); index++)
                valueArr[index] = c[p + index];
            p += (int)sizeof(valueArr);
            char* valueRaw = valueArr;
            float value;
            memcpy(&value, valueRaw, sizeof(float));
            FloatValue* val = custom_malloc(sizeof(FloatValue));
            val->value = value;
            val->base.valueType = 0x04;
            
            val->base.base.type = "Value";
            dict_add((Dictionary*)expStack.top(&expStack), "value", val);
            return p;
        } else if (c[p] == 0x65) {
            machineState = c[p];
            p++;
            char valueArr[2];
            for (int index = 0; index < (int)sizeof(valueArr); index++)
                valueArr[index] = c[p + index];
            p += (int)sizeof(valueArr);
            ShortValue* val = custom_malloc(sizeof(ShortValue));
            val->value = convertBytesToShort(valueArr);
            val->base.valueType = 0x02;
            
            val->base.base.type = "Value";
            dict_add((Dictionary*)expStack.top(&expStack), "value", val);
            return p;
        } else if (c[p] == 0x66) {
            p++;
            char valueArr[4];
            for (int index = 0; index < (int)sizeof(valueArr); index++)
                valueArr[index] = c[p + index];
            p += (int)sizeof(valueArr);
            char* valueRaw = valueArr;
            int value;
            memcpy(&value, valueRaw, sizeof(int));
            IntValue* val = custom_malloc(sizeof(IntValue));
            val->value = value;
            val->base.valueType = 0x06;
            
            val->base.base.type = "Value";
            dict_add((Dictionary*)expStack.top(&expStack), "value", val);
            return p;
        } else if (c[p] == 0x67) {
            p++;
            char valueArr[8];
            for (int index = 0; index < (int)sizeof(valueArr); index++)
                valueArr[index] = c[p + index];
            p += (int)sizeof(valueArr);
            char* valueRaw = valueArr;
            long value;
            memcpy(&value, valueRaw, sizeof(long));
            LongValue* val = custom_malloc(sizeof(LongValue));
            val->value = value;
            val->base.valueType = 0x03;
            
            val->base.base.type = "Value";
            dict_add((Dictionary*) expStack.top(&expStack),"value", val);
            return p;
        } else if (c[p] == 0x68) {
            p++;
            char value = c[p];
            p++;
            bool finalValue = false;
            if ((int)value == 1) finalValue = true;
            BoolValue* val = custom_malloc(sizeof(BoolValue));
            val->value = finalValue;
            val->base.valueType = 0x07;
            
            val->base.base.type = "Value";
            dict_add((Dictionary*) expStack.top(&expStack),"value", val);
            return p;
        }
    }
}

void calculate(int investigateId) {
    pointer = calculateBytes(investigateId, code, pointer);
}

void* ride() {
    while (codeLength > pointer) {
        if (code[pointer] == 0x6e) {
            break;
        }
        if (code[pointer] == 0x4c) {
            return "break";
        }
        else if (code[pointer] == 0x51) {
            pointer++;
            Function* function = custom_malloc(sizeof(Function));
            function->base.type = "Function";
            if (code[pointer] == 0x01) {
                pointer++;
                char funcNameLengthBytes[4];
                for (int index = 0; index < (int)sizeof(funcNameLengthBytes); index++)
                    funcNameLengthBytes[index] = code[pointer + index];
                pointer += (int)sizeof(funcNameLengthBytes);
                int funcNameLength = convertBytesToInt(funcNameLengthBytes);
                char funcName[funcNameLength];
                for (int index = 0; index < funcNameLength; index++)
                    funcName[index] = code[pointer + index];
                pointer += funcNameLength;
                char* funcNameStr = malloc(sizeof(funcName));
                strcpy(funcNameStr, funcName);
                if (code[pointer] == 0x02) {
                    pointer++;
                    char funcLevelLengthBytes[4];
                    for (int index = 0; index < (int)sizeof(funcLevelLengthBytes); index++)
                        funcLevelLengthBytes[index] = code[pointer + index];
                    pointer += (int)sizeof(funcLevelLengthBytes);
                    int funcLevelLength = convertBytesToInt(funcLevelLengthBytes);
                    char funcLevelStr[funcLevelLength];
                    for (int index = 0; index < funcLevelLength; index++)
                        funcLevelStr[index] = code[pointer + index];
                    pointer += funcLevelLength;
                    if (code[pointer] == 0x03) {
                        pointer++;
                        struct List* identifiers = custom_malloc(sizeof(struct List));
                        initList(identifiers);
                        char paramsCountBytes[4];
                        for (int index = 0; index < 4; index++)
                            paramsCountBytes[index] = code[pointer + index];
                        pointer += 4;
                        int paramsCount = convertBytesToInt(paramsCountBytes);
                        for (int counter = 0; counter < paramsCount; counter++) {
                            char paramLengthBytes[4];
                            for (int index = 0; index < (int)sizeof(paramLengthBytes); index++)
                                paramLengthBytes[index] = code[pointer + index];
                            pointer += (int)sizeof(paramLengthBytes);
                            int paramLength = convertBytesToInt(paramLengthBytes);
                            char idStr[paramLength];
                            for (int index = 0; index < paramLength; index++)
                                idStr[index] = code[pointer + index];
                            pointer += paramLength;
                            Identifier* id = custom_malloc(sizeof(Identifier));
                            id->base.type = "Identifier";
                            id->id = custom_malloc(sizeof(String));
                            initString(id->id, strlen(idStr) + 1);
                            strcpy(id->id->value, idStr);
                            identifiers->append(identifiers, id);
                        }
                        if (code[pointer] == 0x04) {
                            pointer++;
                            if (code[pointer] == 0x6f) {
                                pointer++;
                                char jumpBytes[4];
                                for (int index = 0; index < (int)sizeof(jumpBytes); index++)
                                    jumpBytes[index] = code[pointer + index];
                                pointer += (int)sizeof(jumpBytes);
                                int jump = convertBytesToInt(jumpBytes);
                                char body[jump];
                                for(unsigned long i = pointer; i < pointer + jump; i++)
                                    body[i - pointer] = code[i];
                                pointer += jump;
                                if (code[pointer] == 0x6e) {
                                    pointer++;
                                    function->base.type = "Function";
                                    String* str = custom_malloc(sizeof(String));
                                    initString(str, strlen(funcNameStr) + 1);
                                    strcpy(str->value, funcNameStr);
                                    function->funcName = str;
                                    function->params = identifiers;
                                    CodeBlock* cb = custom_malloc(sizeof(CodeBlock));
                                    cb->base.type = "CodeBlock";
                                    cb->data = malloc(jump);
                                    memcpy(cb->data, &body[0], jump);
                                    function->codes = cb;
                                    function->loc = jump;
                                    dict_add((Dictionary *) dataStack.top(&dataStack), funcNameStr, function);
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (code[pointer] == 0x57) {
            pointer++;
            if (code[pointer] == 0x01) {
                pointer++;
                Dictionary* expDict = dict_new();
                expStack.push(&expStack, expDict);
                calculate(1);
                void* target = dict_get((Dictionary*)expStack.pop(&expStack), "value");
                if (code[pointer] == 0x02) {
                    pointer++;
                    char entriesCountBytes[4];
                    for (int index = 0; index < 4; index++)
                        entriesCountBytes[index] = code[pointer + index];
                    pointer += 4;
                    int entriesCount = convertBytesToInt(entriesCountBytes);
                    Dictionary* entriesDict = dict_new();
                    for (int counter = 0; counter < entriesCount; counter++) {
                        if (code[pointer] == 0x03) {
                            pointer++;
                            char keyLengthBytes[4];
                            for (int index = 0; index < 4; index++)
                                keyLengthBytes[index] = code[pointer + index];
                            pointer += 4;
                            int keyLength = convertBytesToInt(keyLengthBytes);
                            char keyBytes[keyLength];
                            for (int index = 0; index < keyLength; index++)
                                keyBytes[index] = code[pointer + index];
                            pointer += keyLength;
                            char *key = &keyBytes[0];
                            char valueLengthBytes[4];
                            for (int index = 0; index < 4; index++)
                                valueLengthBytes[index] = code[pointer + index];
                            pointer += 4;
                            Dictionary *expD = dict_new();
                            expStack.push(&expStack, expD);
                            calculate(1);
                            dict_add(entriesDict, key, dict_get((Dictionary *) expStack.pop(&expStack), "value"));
                        }
                    }
                    if (target != NULL) {
                        Class* classObj = (Class*) target;
                        Object *object = custom_malloc(sizeof(Object));
                        object->base.type = "Object";
                        object->value = dict_new();
                        while (classObj->properties->iteratorHasNext(classObj->properties)) {
                            Prop *prop = (Prop *) classObj->properties->iteratorForward(classObj->properties);
                            char *propId = malloc(strlen(prop->id->id->value) + 1);
                            strcpy(propId, prop->id->id->value);
                            expStack.push(&expStack, dict_new());
                            calculateBytes(1, prop->value->data, 0);
                            dict_add(object->value, propId, dict_get((Dictionary *) expStack.pop(&expStack), "value"));
                        }
                        struct ListDataItem* iterator = classObj->constructor->params->listPointer;
                        while (iterator != NULL) {
                            if (dict_get(entriesDict, ((Identifier*)iterator->data)->id->value) == NULL) {
                                Empty* empty = custom_malloc(sizeof(Empty));
                                dict_add(entriesDict, ((Identifier*)iterator->data)->id->value, empty);
                            }
                            iterator = iterator->prev;
                        }
                        object->funcs = classObj->functions;
                        if (classObj->constructor != NULL) {
                            dict_add(entriesDict, "this", object);
                            executeIntern(classObj->constructor->body->data, classObj->constructor->loc, entriesDict);
                        }
                    }
                }
            }
        }
        else if (code[pointer] == 0x58) {
            pointer++;
            Class* classObj = custom_malloc(sizeof(Class));
            classObj->base.type = "Class";
            if (code[pointer] == 0x01) {
                pointer++;
                char classNameLengthBytes[4];
                for (int index = 0; index < (int)sizeof(classNameLengthBytes); index++)
                    classNameLengthBytes[index] = code[pointer + index];
                pointer += (int)sizeof(classNameLengthBytes);
                int classNameLength = convertBytesToInt(classNameLengthBytes);
                char className[classNameLength];
                for (int index = 0; index < classNameLength; index++)
                    className[index] = code[pointer + index];
                pointer += classNameLength;
                char* classNameStr = malloc(strlen(className) + 1);
                strcpy(classNameStr, className);
                if (code[pointer] == 0x02) {
                    pointer++;
                    char inheritanceCountBytes[4];
                    for (int index = 0; index < (int)sizeof(inheritanceCountBytes); index++)
                        inheritanceCountBytes[index] = code[pointer + index];
                    pointer += (int)sizeof(inheritanceCountBytes);
                    int inheritanceCount = convertBytesToInt(inheritanceCountBytes);
                    struct List* inheritance = custom_malloc(sizeof(struct List));
                    initList(inheritance);
                    for (int i = 0; i < inheritanceCount; i++) {
                        char identifierNameLengthBytes[4];
                        for (int index = 0; index < (int)sizeof(identifierNameLengthBytes); index++)
                            identifierNameLengthBytes[index] = code[pointer + index];
                        pointer += (int)sizeof(identifierNameLengthBytes);
                        int idNameLength = convertBytesToInt(identifierNameLengthBytes);
                        char idNameBytes[idNameLength];
                        for (int index = 0; index < idNameLength; index++)
                            idNameBytes[index] = code[pointer + index];
                        pointer += idNameLength;
                        char* idName = malloc(strlen(idNameBytes) + 1);
                        strcpy(idName, idNameBytes);
                        Identifier* id = custom_malloc(sizeof(Identifier));
                        String* str = custom_malloc(sizeof(String));
                        initString(str, strlen(idName) + 1);
                        strcpy(str->value, idName);
                        id->id = str;
                        inheritance->append(inheritance, id);
                    }
                    if (code[pointer] == 0x03) {
                        pointer++;
                        char behaviorCountBytes[4];
                        for (int index = 0; index < (int)sizeof(behaviorCountBytes); index++)
                            behaviorCountBytes[index] = code[pointer + index];
                        pointer += (int)sizeof(behaviorCountBytes);
                        int behaviorCount = convertBytesToInt(behaviorCountBytes);
                        struct List* behavior = custom_malloc(sizeof(struct List));
                        initList(behavior);
                        for (int i = 0; i < behaviorCount; i++) {
                            char identifierNameLengthBytes[4];
                            for (int index = 0; index < (int)sizeof(identifierNameLengthBytes); index++)
                                identifierNameLengthBytes[index] = code[pointer + index];
                            pointer += (int)sizeof(identifierNameLengthBytes);
                            int idNameLength = convertBytesToInt(identifierNameLengthBytes);
                            char idNameBytes[idNameLength];
                            for (int index = 0; index < idNameLength; index++)
                                idNameBytes[index] = code[pointer + index];
                            pointer += idNameLength;
                            char* idName = &idNameBytes[0];
                            Identifier* id = custom_malloc(sizeof(Identifier));
                            String* str = custom_malloc(sizeof(String));
                            initString(str, strlen(idName) + 1);
                            strcpy(str->value, idName);
                            id->id = str;
                            behavior->append(behavior, id);
                        }
                        if (code[pointer] == 0x04) {
                            pointer++;
                            char propCountBytes[4];
                            for (int index = 0; index < (int)sizeof(propCountBytes); index++)
                                propCountBytes[index] = code[pointer + index];
                            pointer += (int)sizeof(propCountBytes);
                            int propCount = convertBytesToInt(propCountBytes);
                            struct List* props = custom_malloc(sizeof(struct List));
                            initList(props);
                            for (int i = 0; i < propCount; i++) {
                                char identifierNameLengthBytes[4];
                                for (int index = 0; index < (int)sizeof(identifierNameLengthBytes); index++)
                                    identifierNameLengthBytes[index] = code[pointer + index];
                                pointer += (int)sizeof(identifierNameLengthBytes);
                                int idNameLength = convertBytesToInt(identifierNameLengthBytes);
                                char idNameBytes[idNameLength];
                                for (int index = 0; index < idNameLength; index++)
                                    idNameBytes[index] = code[pointer + index];
                                pointer += idNameLength;
                                char* idName = malloc(strlen(idNameBytes) + 1);
                                strcpy(idName, idNameBytes);
                                Identifier* id = custom_malloc(sizeof(Identifier));
                                String* str = custom_malloc(sizeof(String));
                                initString(str, strlen(idName) + 1);
                                strcpy(str->value, idName);
                                id->id = str;
                                id->base.type = "Identifier";
                                char valueLengthBytes[4];
                                for (int index = 0; index < (int)sizeof(valueLengthBytes); index++)
                                    valueLengthBytes[index] = code[pointer + index];
                                pointer += (int)sizeof(valueLengthBytes);
                                unsigned long valueLength = convertBytesToInt(valueLengthBytes);
                                char value[valueLength];
                                for (unsigned long i2 = pointer; i2 < pointer + valueLength; i2++)
                                    value[i2 - pointer] = code[i2];
                                pointer += valueLength;
                                Prop* prop = custom_malloc(sizeof(Prop));
                                prop->id = id;
                                CodeBlock* cb = custom_malloc(sizeof(CodeBlock));
                                cb->base.type = "CodeBlock";
                                cb->data = malloc(valueLength);
                                memcpy(cb->data, &value[0], valueLength);
                                prop->value = cb;
                                prop->loc = valueLength;
                                props->append(props, prop);
                            }
                            if (code[pointer] == 0x05) {
                                pointer++;
                                char funcCountBytes[4];
                                for (int index = 0; index < (int)sizeof(funcCountBytes); index++)
                                    funcCountBytes[index] = code[pointer + index];
                                pointer += (int)sizeof(funcCountBytes);
                                int funcCount = convertBytesToInt(funcCountBytes);
                                Dictionary* funcs = dict_new();
                                for (int i = 0; i < funcCount; i++) {
                                    if (code[pointer] == 0x51) {
                                        pointer++;
                                        Function* function = custom_malloc(sizeof(Function));
                                        function->base.type = "Function";
                                        if (code[pointer] == 0x01) {
                                            pointer++;
                                            char funcNameLengthBytes[4];
                                            for (int index = 0; index < (int)sizeof(funcNameLengthBytes); index++)
                                                funcNameLengthBytes[index] = code[pointer + index];
                                            pointer += (int)sizeof(funcNameLengthBytes);
                                            int funcNameLength = convertBytesToInt(funcNameLengthBytes);
                                            char funcName[funcNameLength];
                                            for (int index = 0; index < funcNameLength; index++)
                                                funcName[index] = code[pointer + index];
                                            pointer += funcNameLength;
                                            char* funcNameStr = malloc(sizeof(funcName));
                                            strcpy(funcNameStr, funcName);
                                            if (code[pointer] == 0x02) {
                                                pointer++;
                                                char funcLevelLengthBytes[4];
                                                for (int index = 0; index < (int)sizeof(funcLevelLengthBytes); index++)
                                                    funcLevelLengthBytes[index] = code[pointer + index];
                                                pointer += (int)sizeof(funcLevelLengthBytes);
                                                int funcLevelLength = convertBytesToInt(funcLevelLengthBytes);
                                                char funcLevelStr[funcLevelLength];
                                                for (int index = 0; index < funcLevelLength; index++)
                                                    funcLevelStr[index] = code[pointer + index];
                                                pointer += funcLevelLength;
                                                if (code[pointer] == 0x03) {
                                                    pointer++;
                                                    struct List* identifiers = custom_malloc(sizeof(struct List));
                                                    initList(identifiers);
                                                    char paramsCountBytes[4];
                                                    for (int index = 0; index < 4; index++)
                                                        paramsCountBytes[index] = code[pointer + index];
                                                    pointer += 4;
                                                    int paramsCount = convertBytesToInt(paramsCountBytes);
                                                    for (int counter = 0; counter < paramsCount; counter++) {
                                                        char paramLengthBytes[4];
                                                        for (int counter2 = 0; counter2 < (int)sizeof(paramLengthBytes); counter2++)
                                                            paramLengthBytes[counter2] = code[pointer + counter2];
                                                        pointer += (int)sizeof(paramLengthBytes);
                                                        int paramLength = convertBytesToInt(paramLengthBytes);
                                                        char* idStr = malloc(paramLength);
                                                        memcpy(idStr, &code[pointer], paramLength);
                                                        pointer += paramLength;
                                                        Identifier* id = custom_malloc(sizeof(Identifier));
                                                        id->base.type = "Identifier";
                                                        id->id = custom_malloc(sizeof(String));
                                                        initString(id->id, strlen(idStr) + 1);
                                                        strcpy(id->id->value, idStr);
                                                        identifiers->append(identifiers, id);
                                                    }
                                                    if (code[pointer] == 0x04) {
                                                        pointer++;
                                                        if (code[pointer] == 0x6f) {
                                                            pointer++;
                                                            char jumpBytes[4];
                                                            for (int index = 0; index < (int)sizeof(jumpBytes); index++)
                                                                jumpBytes[index] = code[pointer + index];
                                                            pointer += (int)sizeof(jumpBytes);
                                                            int jump = convertBytesToInt(jumpBytes);
                                                            char* body = malloc(jump);
                                                            memcpy(body, &code[pointer], jump);
                                                            pointer += jump;
                                                            if (code[pointer] == 0x6e) {
                                                                pointer++;
                                                                function->base.type = "Function";
                                                                String* str = custom_malloc(sizeof(String));
                                                                initString(str, strlen(funcNameStr) + 1);
                                                                strcpy(str->value, funcNameStr);
                                                                function->funcName = str;
                                                                function->params = identifiers;
                                                                function->codes = custom_malloc(sizeof(CodeBlock));
                                                                function->codes->base.type = "CodeBlock";
                                                                function->codes->data = malloc(jump);
                                                                memcpy(function->codes->data, body, jump);
                                                                function->loc = jump;
                                                                dict_add(funcs, funcNameStr, function);
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                if (code[pointer] == 0x05) {
                                    Constructor* constructor = custom_malloc(sizeof(Constructor));
                                    pointer++;
                                    char paramsCountBytes[4];
                                    for (int i = 0; i < (int) sizeof(paramsCountBytes); i++)
                                        paramsCountBytes[i] = code[pointer + i];
                                    pointer += (int) sizeof(paramsCountBytes);
                                    int paramsCount = convertBytesToInt(paramsCountBytes);
                                    struct List* ids = custom_malloc(sizeof(struct List));
                                    initList(ids);
                                    for (int counter = 0; counter < paramsCount; counter++) {
                                        char idNameLengthBytes[4];
                                        for (int i = 0; i < (int) sizeof(idNameLengthBytes); i++)
                                            idNameLengthBytes[i] = code[pointer + i];
                                        pointer += (int) sizeof(idNameLengthBytes);
                                        int idNameLength = convertBytesToInt(idNameLengthBytes);
                                        char idNameBytes[idNameLength];
                                        for (int i = 0; i < idNameLength; i++)
                                            idNameBytes[i] = code[pointer + i];
                                        pointer += idNameLength;
                                        char* idName = malloc(strlen(idNameBytes) + 1);
                                        strcpy(idName, idNameBytes);
                                        Identifier* id = custom_malloc(sizeof(Identifier));
                                        id->base.type = "Identifier";
                                        String* str = custom_malloc(sizeof(String));
                                        initString(str, strlen(idName) + 1);
                                        strcpy(str->value, idName);
                                        id->id = str;
                                        ids->append(ids, id);
                                    }
                                    constructor->params = ids;
                                    if (code[pointer] == 0x06) {
                                        pointer++;
                                        if (code[pointer] == 0x6f) {
                                            pointer++;
                                            char bodyLengthBytes[4];
                                            for (int i = 0; i < (int) sizeof(bodyLengthBytes); i++)
                                                bodyLengthBytes[i] = code[pointer + i];
                                            pointer += (int) sizeof(bodyLengthBytes);
                                            int bodyLength = convertBytesToInt(bodyLengthBytes);
                                            char* body = malloc(bodyLength);
                                            memcpy(body, &code[pointer], bodyLength);
                                            pointer += bodyLength;
                                            constructor->loc = bodyLength;
                                            if (code[pointer] == 0x6e) {
                                                pointer++;
                                                CodeBlock* cb = custom_malloc(sizeof(CodeBlock));
                                                cb->base.type = "CodeBlock";
                                                cb->data = malloc(bodyLength);
                                                memcpy(cb->data, &body[0], bodyLength);
                                                constructor->body = cb;
                                            }
                                        }
                                    }
                                    classObj->inheritance = inheritance;
                                    classObj->behavior = behavior;
                                    classObj->properties = props;
                                    classObj->functions = funcs;
                                    String* str = custom_malloc(sizeof(String));
                                    initString(str, strlen(classNameStr) + 1);
                                    strcpy(str->value, classNameStr);
                                    classObj->className = str;
                                    classObj->constructor = constructor;
                                    dict_add((Dictionary *) dataStack.top(&dataStack), className, classObj);
                                    classObj->constructor = constructor;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (code[pointer] == 0x52) {
            pointer++;
            bool matched = false;
            if (code[pointer] == 0x01) {
                pointer++;
                Dictionary* expDict = dict_new();
                expStack.push(&expStack, expDict);
                int tempMachineState = machineState;
                calculate(1);
                machineState = tempMachineState;
                BoolValue* condition = (BoolValue*)dict_get((Dictionary*)expStack.pop(&expStack), "value");
                if (code[pointer] == 0x02) {
                    pointer++;
                    if (code[pointer] == 0x6f) {
                        pointer++;
                        char jumpBytes[4];
                        for (int index = 0; index < (int)sizeof(jumpBytes); index++)
                            jumpBytes[index] = code[pointer + index];
                        pointer += (int)sizeof(jumpBytes);
                        int jump = convertBytesToInt(jumpBytes);
                        char* body = malloc(jump);
                        memcpy(body, &code[pointer], jump);
                        pointer += jump;
                        if (condition->value) {
                            matched = true;
                            void* retVal = executeIntern(body, jump, NULL);
                            if (retVal != NULL && strcmp(((char*)retVal), "break") == 0) {
                                break;
                            } else if (retVal != NULL && strcmp(((char*)retVal), "break") != 0) {
                                return retVal;
                            }
                        }
                        if (code[pointer] == 0x6e) {
                            pointer++;
                            if (code[pointer] == 0x03) {
                                pointer++;
                                char elseCountLength[4];
                                for (int index = 0; index < 4; index++)
                                    elseCountLength[index] = code[pointer + index];
                                pointer += 4;
                                int elseCount = convertBytesToInt(elseCountLength);
                                for (int elseCounter = 0; elseCounter < elseCount; elseCounter++) {
                                    if (code[pointer] == 0x53) {
                                        pointer++;
                                        if (code[pointer] == 0x01) {
                                            pointer++;
                                            Dictionary* expDict2 = dict_new();
                                            expStack.push(&expStack, expDict2);
                                            calculate(1);
                                            BoolValue* elseCondition = (BoolValue*)dict_get((Dictionary*)expStack.pop(&expStack), "value");
                                            if (code[pointer] == 0x02) {
                                                pointer++;
                                                if (code[pointer] == 0x6f) {
                                                    pointer++;
                                                    char jumpBytes2[4];
                                                    for (int index = 0; index < (int)sizeof(jumpBytes2); index++)
                                                        jumpBytes2[index] = code[pointer + index];
                                                    pointer += (int)sizeof(jumpBytes2);
                                                    int jump2 = convertBytesToInt(jumpBytes2);
                                                    char* body2 = malloc(jump2);
                                                    memcpy(body2, &code[pointer], jump2);
                                                    pointer += jump2;
                                                    if (!matched && elseCondition->value) {
                                                        matched = true;
                                                        executeIntern(body2, jump2, NULL);
                                                    }
                                                    if (code[pointer] == 0x6e) {
                                                        pointer++;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else if (code[pointer] == 0x54) {
                                        pointer++;
                                        if (code[pointer] == 0x01) {
                                            pointer++;
                                            if (code[pointer] == 0x6f) {
                                                pointer++;
                                                char jumpBytes2[4];
                                                for (int index = 0; index < (int)sizeof(jumpBytes2); index++)
                                                    jumpBytes2[index] = code[pointer + index];
                                                pointer += (int)sizeof(jumpBytes2);
                                                int jump2 = convertBytesToInt(jumpBytes2);
                                                char* body2 = malloc(jump2);
                                                memcpy(body2, &code[pointer], jump2);
                                                pointer += jump2;
                                                if (!matched)
                                                    executeIntern(body2, jump2, NULL);
                                                if (code[pointer] == 0x6e) {
                                                    pointer++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (code[pointer] == 0x53) {
            pointer++;
            unsigned long bodyStartPos = pointer;
            void* counter = NULL;
            char* body = NULL;
            while (true) {
                pointer = bodyStartPos;
                if (code[pointer] == 0x01) {
                    pointer++;
                    expStack.push(&expStack, dict_new());
                    calculate(1);
                    void* limitRaw = dict_get((Dictionary*)expStack.pop(&expStack), "value");
                    if (code[pointer] == 0x02) {
                        pointer++;
                        expStack.push(&expStack, dict_new());
                        calculate(1);
                        void* stepRaw = dict_get((Dictionary*)expStack.pop(&expStack), "value");
                        if (code[pointer] == 0x03) {
                            pointer++;
                            if (code[pointer] == 0x6f) {
                                pointer++;
                                char jumpBytes[4];
                                for (int index = 0; index < (int)sizeof(jumpBytes); index++)
                                    jumpBytes[index] = code[pointer + index];
                                pointer += (int)sizeof(jumpBytes);
                                int jump = convertBytesToInt(jumpBytes);
                                if (body == NULL) {
                                    body = malloc(jump);
                                    memcpy(body, &code[pointer], jump);
                                }
                                pointer += jump;
                                if (counter == NULL) {
                                    if (((Value *) limitRaw)->valueType == 0x02) {
                                        counter = custom_malloc(sizeof(ShortValue));
                                        ((ShortValue *) counter)->value = 0;
                                        ((ShortValue *) counter)->base.valueType = 0x02;
                                    } else if (((Value *) limitRaw)->valueType == 0x06) {
                                        counter = custom_malloc(sizeof(IntValue));
                                        ((IntValue *) counter)->value = 0;
                                        ((ShortValue *) counter)->base.valueType = 0x06;
                                    } else if (((Value *) limitRaw)->valueType == 0x03) {
                                        counter = custom_malloc(sizeof(LongValue));
                                        ((LongValue *) counter)->value = 0;
                                        ((ShortValue *) counter)->base.valueType = 0x03;
                                    } else if (((Value *) limitRaw)->valueType == 0x04) {
                                        counter = custom_malloc(sizeof(FloatValue));
                                        ((FloatValue *) counter)->value = 0;
                                        ((ShortValue *) counter)->base.valueType = 0x04;
                                    } else if (((Value *) limitRaw)->valueType == 0x05) {
                                        counter = custom_malloc(sizeof(DoubleValue));
                                        ((DoubleValue *) counter)->value = 0;
                                        ((ShortValue *) counter)->base.valueType = 0x05;
                                    }
                                }
                                if (((BoolValue*)lt(counter, limitRaw))->value) {
                                    void* retVal = executeIntern(body, jump, NULL);
                                    if (retVal != NULL && strcmp(((char*)retVal), "break") == 0) {
                                        break;
                                    } else if (retVal != NULL && strcmp(((char*)retVal), "break") != 0) {
                                        return retVal;
                                    }
                                    counter = sum(counter, stepRaw);
                                }
                                else {
                                    if (code[pointer] == 0x6e) {
                                        pointer++;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (code[pointer] == 0x54) {
            pointer++;
            unsigned long bodyStartPos = pointer;
            char* body = NULL;
            while (true) {
                pointer = bodyStartPos;
                if (code[pointer] == 0x01) {
                    pointer++;
                    expStack.push(&expStack, dict_new());
                    calculate(1);
                    void *condition = dict_get((Dictionary *) expStack.pop(&expStack), "value");
                    if (code[pointer == 0x02]) {
                        pointer++;
                        if (code[pointer] == 0x6f) {
                            pointer++;
                            char jumpBytes[4];
                            for (int index = 0; index < (int)sizeof(jumpBytes); index++)
                                jumpBytes[index] = code[pointer + index];
                            pointer += (int)sizeof(jumpBytes);
                            int jump = convertBytesToInt(jumpBytes);
                            if (body == NULL) {
                                body = malloc(jump);
                                memcpy(body, &code[pointer], jump);
                            }
                            pointer += jump;
                            if (((BoolValue *) condition)->value) {
                                void* retVal = executeIntern(body, jump, NULL);
                                if (retVal != NULL && strcmp(((char*)retVal), "break") == 0) {
                                    break;
                                } else if (retVal != NULL && strcmp(((char*)retVal), "break") != 0) {
                                    return retVal;
                                }
                            } else {
                                if (code[pointer] == 0x6e) {
                                    pointer++;
                                }
                                break;
                            }
                            if (code[pointer] == 0x6e) {
                                pointer++;
                            }
                        }
                    }
                }
            }
        }
        else if (code[pointer] == 0x55) {
            pointer++;
            if (code[pointer] == 0x01) {
                pointer++;
                expStack.push(&expStack, dict_new());
                calculate(1);
                void* target = dict_get((Dictionary*)expStack.top(&expStack), "value");
                char* refStr = dict_get((Dictionary*)expStack.top(&expStack), "value2");
                Object* thisObj = dict_get((Dictionary*)expStack.pop(&expStack), "value3");
                if (code[pointer] == 0x02) {
                    pointer++;
                    char entriesCountBytes[4];
                    for (int index = 0; index < 4; index++)
                        entriesCountBytes[index] = code[pointer + index];
                    pointer += 4;
                    int entriesCount = convertBytesToInt(entriesCountBytes);
                    Dictionary* entriesDict = dict_new();
                    for (int counter = 0; counter < entriesCount; counter++) {
                        if (code[pointer] == 0x03) {
                            pointer++;
                            char keyLengthBytes[4];
                            for (int index = 0; index < 4; index++)
                                keyLengthBytes[index] = code[pointer + index];
                            pointer += 4;
                            int keyLength = convertBytesToInt(keyLengthBytes);
                            char keyBytes[keyLength];
                            for (int index = 0; index < keyLength; index++)
                                keyBytes[index] = code[pointer + index];
                            pointer += keyLength;
                            char *key = malloc(strlen(keyBytes) + 1);
                            strcpy(key, keyBytes);
                            char valueLengthBytes[4];
                            for (int index = 0; index < 4; index++)
                                valueLengthBytes[index] = code[pointer + index];
                            pointer += 4;
                            expStack.push(&expStack, dict_new());
                            calculate(1);
                            void* value =dict_get((Dictionary*) expStack.pop(&expStack), "value");
                            dict_add(entriesDict, key, value);
                        }
                    }
                    if (target == NULL) {
                        routeAndResolve(refStr, entriesDict);
                    } else {
                        Function* func = (Function*) target;
                        if (thisObj != NULL)
                            dict_add(entriesDict, "this", thisObj);
                        executeIntern(func->codes->data, func->loc, entriesDict);
                    }
                }
            }
        }
        else if (code[pointer] == 0x56) {
            pointer++;
            if (code[pointer] == 0x01) {
                pointer++;
                expStack.push(&expStack, dict_new());
                calculate(0);
                struct Reference* ref = (struct Reference*)dict_get((Dictionary*)expStack.pop(&expStack), "value");
                if (code[pointer] == 0x02) {
                    pointer++;
                    expStack.push(&expStack, dict_new());
                    calculate(1);
                    void* exp = dict_get((Dictionary*)expStack.pop(&expStack), "value");
                    Object* objectChain = NULL;
                    char* varNameStr = malloc(strlen(ref->currentChain->id->value) + 1);
                    strcpy(varNameStr, ref->currentChain->id->value);
                    struct StackDataItem* iterator = dataStack.item;
                    void* variable = dict_get(((Dictionary*)iterator->data), varNameStr);
                    if (variable != NULL && strcmp(((Code*)variable)->type, "Object") == 0)
                        objectChain = variable;
                    while (variable == NULL && iterator->prev != NULL) {
                        iterator = iterator->prev;
                        variable = dict_get((Dictionary*)iterator->data, varNameStr);
                    }
                    Identifier* finalChain = ref->currentChain;
                    ref = ref->restOfTheChain;
                    char* refStr = varNameStr;
                    while (variable != NULL && ref != NULL) {
                        if (strcmp(((Code*)variable)->type, "Object") == 0) {
                            void* variable2 = dict_get(((Object*)variable)->value, ref->currentChain->id->value);
                            if (variable == NULL) {
                                variable2 = dict_get(((Object *) variable)->funcs, ref->currentChain->id->value);
                                if (variable2 != NULL)
                                    variable = variable2;
                            }
                            else
                                variable = variable2;
                        }
                        else if (strcmp(((Code*)variable)->type, "Class") == 0) {
                            variable = dict_get(((Class*)variable)->functions, ref->currentChain->id->value);
                        }
                        strcat(refStr, ".");
                        strcat(refStr, ref->currentChain->id->value);
                        finalChain = ref->currentChain;
                        ref = ref->restOfTheChain;
                        if (variable != NULL && strcmp(((Code*)variable)->type, "Object") == 0)
                            objectChain = variable;
                    }
                    if (objectChain != NULL) {
                        char *varNameStr2 = malloc(strlen(finalChain->id->value) + 1);
                        strcpy(varNameStr2, finalChain->id->value);
                        dict_delete(objectChain->value, varNameStr2);
                        dict_add(objectChain->value, varNameStr2, exp);
                    } else {
                        if (variable == NULL) {
                            char *varNameStr2 = malloc(strlen(finalChain->id->value) + 1);
                            strcpy(varNameStr2, finalChain->id->value);
                            dict_add(((Dictionary *) dataStack.top(&dataStack)), varNameStr2, exp);
                        } else {
                            char *varNameStr2 = malloc(strlen(finalChain->id->value) + 1);
                            strcpy(varNameStr2, finalChain->id->value);
                            dict_delete((Dictionary *) iterator->data, varNameStr2);
                            dict_add((Dictionary *) iterator->data, varNameStr2, exp);
                        }
                    }
                }
            }
        }
        else if (code[pointer] == 0x59) {
            pointer++;
            if (code[pointer] == 0x01) {
                pointer++;
                expStack.push(&expStack, dict_new());
                calculate(1);
                return dict_get((Dictionary*)expStack.pop(&expStack), "value");
            }
        }
        else {
            break;
        }
    }
    return NULL;
}

void* executeIntern(char* c, unsigned long length, Dictionary* entriesDict) {
    if (entriesDict == NULL)
        dataStack.push(&dataStack, dict_new());
    else
        dataStack.push(&dataStack, entriesDict);
    bufferStack.push(&bufferStack, dict_new());
    CodePack* cp = custom_malloc(sizeof(CodePack));
    CodeBlock* cb = custom_malloc(sizeof(CodeBlock));
    cb->base.type = "CodeBlock";
    cb->data = malloc(codeLength);
    memcpy(cb->data, code, codeLength);
    cp->code = cb;
    cp->loc = codeLength;
    cp->pointer = pointer;
    codeLengthStack.push(&codeLengthStack, cp);
    code = c;
    codeLength = length;
    pointer = 0;
    void* returnValue = ride();
    cp = codeLengthStack.pop(&codeLengthStack);
    code = cp->code->data;
    codeLength = cp->loc;
    pointer = cp->pointer;
    notifyUsageEnded(bufferStack.pop(&bufferStack));
    notifyUsageEnded(dataStack.pop(&dataStack));
    return returnValue;
}

void execute(char* c, long length) {
    initStack(&codeLengthStack);
    initStack(&bufferStack);
    initStack(&expStack);
    initStack(&dataStack);
    executeIntern(c, length, NULL);
}