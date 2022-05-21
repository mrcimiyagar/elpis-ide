
#include "Kasper.h"
#include "../api/Cipher/Sha256.h"
#include "../api/String/String.h"
#include <uuid/uuid.h>
#include <pthread.h>
#include <string.h>
#include <emscripten.h>
//#include <curl/curl.h>

static struct Dictionary* aliveThreads;
static struct Stack* codeLengthStack;
static struct Stack* expStack;
static struct Stack* dataStack;

unsigned long codeLength = 0;
char* code;
unsigned long pointer = 0; 

long measureLog10l(long input) {
    if (input == 0) return 0;
    else if (input < 0) input *= -1;
    return (long) log10l(input);
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

char* concat(const char *s1, const char *s2)
{
    if (strlen(s1) == 0 && strlen(s2) == 0) {
        char *result = malloc(1);
        strcpy(result, "");
        return result;
    }
    else if (strlen(s1) == 0) {
        char *result = malloc(strlen(s2) + 1);
        strcpy(result, s2);
        result[strlen(s2)] = '\0';
        return result;
    }
    else if (strlen(s2) == 0) {
        char *result = malloc(strlen(s1) + 1);
        strcpy(result, s1);
        result[strlen(s1)] = '\0';
        return result;
    }
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char* stringifyObject(void* data) {
    if (strcmp(((struct Code*)data)->type, "Value") == 0) {
        if ((*(struct Value *) data).valueType == 0x01) {
            char* result = malloc((strlen(((struct StringValue *) data)->value) + 1) * sizeof(char));
            strcpy(result, ((struct StringValue *) data)->value);
            return result;
        } else if ((*(struct Value *) data).valueType == 0x02) {
            struct ShortValue val1 = *(struct ShortValue *) data;
            char str[(int) (ceilf(measureLog10f(val1.value) + 1) * sizeof(char))];
            sprintf(str, "%hd", val1.value);
            char *str2 = &str[0];
            return str2;
        } else if ((*(struct Value *) data).valueType == 0x06) {
            struct IntValue val1 = *(struct IntValue *) data;
            char str[(int) (ceil(measureLog10(val1.value) + 1) * sizeof(char))];
            sprintf(str, "%d", val1.value);
            char *str2 = &str[0];
            return str2;
        } else if ((*(struct Value *) data).valueType == 0x03) {
            struct LongValue val1 = *(struct LongValue *) data;
            char str[(int) (ceill(measureLog10l(val1.value) + 1) * sizeof(char))];
            sprintf(str, "%ld", val1.value);
            char *str2 = &str[0];
            return str2;
        } else if ((*(struct Value *) data).valueType == 0x04) {
            struct FloatValue val1 = *(struct FloatValue *) data;
            char str[(int) (ceilf(measureLog10f(val1.value) + 1) * sizeof(char))];
            sprintf(str, "%f", val1.value);
            char *str2 = &str[0];
            return str2;
        } else if ((*(struct Value *) data).valueType == 0x05) {
            struct DoubleValue val1 = *(struct DoubleValue *) data;
            char str[(int) (ceil(measureLog10(val1.value) + 1) * sizeof(char))];
            sprintf(str, "%f", val1.value);
            char *str2 = &str[0];
            return str2;
        }
        else if ((*(struct Value *) data).valueType == 0x07) {
            struct BoolValue val1 = *(struct BoolValue *) data;
            char *str2 = malloc((strlen(val1.value ? "true" : "false") + 1) * sizeof(char));
            strcpy(str2, val1.value ? "true" : "false");
            return str2;
        }
    }
    else if (strcmp(((struct Code*)data)->type, "Object") == 0) {
        struct Object *obj = (struct Object *) data;
        void *stringifyRaw = dict_get(obj->funcs, "stringify");
        if (stringifyRaw == NULL) {
            struct List *iteratorList = toList(obj->value);
            struct ListDataItem *iterator = iteratorList->endPointer;
            char *result = malloc((strlen("{ ") + 1) * sizeof(char));
            strcpy(result, "{ ");
            while (iterator != NULL) {
                struct Pair *pair = iterator->data;
                char *tempResult = concat(result, "\"");
                free(result);
                result = tempResult;
                tempResult = concat(result, ((struct String *) pair->first)->value);
                free(result);
                result = tempResult;
                tempResult = concat(result, "\" : ");
                free(result);
                result = tempResult;
                if (strcmp(((struct Code *) pair->second)->type, "Object") == 0) {
                    char *part2 = stringifyObject((struct Object *) pair->second);
                    tempResult = concat(result, part2);
                    free(result);
                    free(part2);
                    result = tempResult;
                } else if (strcmp(((struct Code *) pair->second)->type, "Array") == 0) {
                    char *part2 = stringifyArray(((struct Array *) pair->second));
                    tempResult = concat(result, part2);
                    free(result);
                    free(part2);
                    result = tempResult;
                } else if (strcmp(((struct Code*)pair->second)->type, "Empty") == 0) {
                    result = malloc((strlen("{}") + 1) * sizeof(char));
                    strcpy(result, "{}");
                    result[strlen("{}")] = '\0';
                } else if ((*(struct Value *) pair->second).valueType == 0x01) {
                    tempResult = concat(result, "\"");
                    free(result);
                    result = tempResult;
                    tempResult = concat(result, ((struct StringValue *) pair->second)->value);
                    free(result);
                    result = tempResult;
                    tempResult = concat(result, "\"");
                    free(result);
                    result = tempResult;
                } else if ((*(struct Value *) pair->second).valueType == 0x02) {
                    struct ShortValue val1 = *(struct ShortValue *) pair->second;
                    char str[(int) (ceilf(measureLog10f(val1.value) + 1) * sizeof(char))];
                    sprintf(str, "%hd", val1.value);
                    char *str2 = &str[0];
                    tempResult = concat(result, str2);
                    free(result);
                    result = tempResult;
                } else if ((*(struct Value *) pair->second).valueType == 0x06) {
                    struct IntValue val1 = *(struct IntValue *) pair->second;
                    char str[(int) (ceil(measureLog10(val1.value) + 1) * sizeof(char))];
                    sprintf(str, "%d", val1.value);
                    char *str2 = &str[0];
                    tempResult = concat(result, str2);
                    free(result);
                    result = tempResult;
                } else if ((*(struct Value *) pair->second).valueType == 0x03) {
                    struct LongValue val1 = *(struct LongValue *) pair->second;
                    char str[(int) (ceill(measureLog10l(val1.value) + 1) * sizeof(char))];
                    sprintf(str, "%ld", val1.value);
                    char *str2 = &str[0];
                    tempResult = concat(result, str2);
                    free(result);
                    result = tempResult;
                } else if ((*(struct Value *) pair->second).valueType == 0x04) {
                    struct FloatValue val1 = *(struct FloatValue *) pair->second;
                    char str[(int) (ceilf(measureLog10f(val1.value) + 1) * sizeof(char))];
                    sprintf(str, "%f", val1.value);
                    char *str2 = &str[0];
                    tempResult = concat(result, str2);
                    free(result);
                    result = tempResult;
                } else if ((*(struct Value *) pair->second).valueType == 0x05) {
                    struct DoubleValue val1 = *(struct DoubleValue *) pair->second;
                    char str[(int) (ceil(measureLog10(val1.value) + 1) * sizeof(char))];
                    sprintf(str, "%f", val1.value);
                    char *str2 = &str[0];
                    tempResult = concat(result, str2);
                    free(result);
                    result = tempResult;
                }
                else if ((*(struct Value *) pair->second).valueType == 0x07) {
                    struct BoolValue val1 = *(struct BoolValue *) pair->second;
                    char *str2 = malloc((strlen(val1.value ? "true" : "false") + 1) * sizeof(char));
                    strcpy(str2, val1.value ? "true" : "false");
                    tempResult = concat(result, str2);
                    free(result);
                    result = tempResult;
                }
                if (iterator->prev != NULL) {
                    tempResult = concat(result, ", ");
                    free(result);
                    result = tempResult;
                }
                iterator = iterator->prev;
            }
            notifyUsageEnded(iteratorList);
            char *tempResult = concat(result, " }");
            free(result);
            result = tempResult;
            return result;
        } else {
            struct Dictionary *entriesDict = dict_new();
            dict_add_str(entriesDict, "this", obj);
            struct Function *stringify = (struct Function *) stringifyRaw;
            struct StringValue *stringified = executeIntern(true, stringify->codes->data, stringify->loc, entriesDict);
            char *result = malloc(strlen(stringified->value) + 1);
            strcpy(result, stringified->value);
            notifyUsageEnded(stringified);
            return result;
        }
    }
    else if (strcmp(((struct Code*)data)->type, "Empty") == 0) {
        char* result = malloc((strlen("{}") + 1) * sizeof(char));
        strcpy(result, "{}");
        result[strlen("{}")] = '\0';
        return result;
    }
}

char* stringifyArray(struct Array* array) {
    char* textToPrint = malloc((strlen("[") + 1) * sizeof(char));
    strcpy(textToPrint, "[");
    for (int i = 0; i < array->used; i++) {
        void* arrayItem = array->array[i];
        if (arrayItem != NULL) {
            if (strcmp(((struct Code *) arrayItem)->type, "Array") == 0) {
                char *stringified = stringifyArray((struct Array *) arrayItem);
                char* tempTextToPrint = concat(textToPrint, stringified);
                free(textToPrint);
                free(stringified);
                textToPrint = tempTextToPrint;
            } else if (strcmp(((struct Code *) arrayItem)->type, "Object") == 0) {
                struct Object *obj = (struct Object *) arrayItem;
                char* result = stringifyObject(obj);
                char* tempTextToPrint = concat(textToPrint, result);
                free(result);
                free(textToPrint);
                textToPrint = tempTextToPrint;
            } else if (strcmp(((struct Code*)arrayItem)->type, "Value") == 0) {
                struct Value *obj = (struct Value *) arrayItem;
                char* result = stringifyObject(obj);
                char* tempTextToPrint = concat(textToPrint, result);
                //free(result);
                free(textToPrint);
                textToPrint = tempTextToPrint;
            }
            if (i < array->used - 1) {
                char* tempTextToPrint = concat(textToPrint, ", ");
                free(textToPrint);
                textToPrint = tempTextToPrint;
            }
        }
    }
    char* textToPrintTemp = concat(textToPrint, "]");
    free(textToPrint);
    textToPrint = textToPrintTemp;
    return textToPrint;
}

char* httpResult = NULL;
char* httpResultHeaders = NULL;
bool firstLine = true;

static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    if (firstLine) {
        firstLine = false;
        return nitems * size;
    }
    char* httpResultHeadersTemp = concat(httpResultHeaders, buffer);
    free(httpResultHeaders);
    httpResultHeaders = httpResultHeadersTemp;
    return nitems * size;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    char* httpResultTemp = concat(httpResult, ptr);
    free(httpResult);
    httpResult = httpResultTemp;
    return nmemb * size;
}

Header* processHeaders(char* raw) {
    struct Header *header = NULL, *last = NULL;
    while (raw[0]!='\r' || raw[1]!='\n') {
        last = header;
        header = malloc(sizeof(Header));
        size_t name_len = strcspn(raw, ":");
        header->name = malloc(name_len + 1);
        memcpy(header->name, raw, name_len);
        header->name[name_len] = '\0';
        raw += name_len + 1;
        while (*raw == ' ') {
            raw++;
        }
        size_t value_len = strcspn(raw, "\r\n");
        header->value = malloc(value_len + 1);
        memcpy(header->value, raw, value_len);
        header->value[value_len] = '\0';
        raw += value_len + 2;
        header->next = last;
    }
    return header;
}

struct Object* prepareResponse() {
    httpResultHeaders = concat(httpResultHeaders, "\r\n");
    Header* headerPtr = processHeaders(httpResultHeaders);
    struct Object *resObj = custom_malloc(sizeof(struct Object));
    resObj->base.type = malloc((strlen("Object") + 1) * sizeof(char));
    strcpy(resObj->base.type, "Object");
    resObj->value = dict_new();
    notifyNewUsage(resObj, resObj->value);
    resObj->funcs = NULL;
    struct Dictionary *headers = dict_new();
    while (headerPtr != NULL) {
        struct StringValue *headerValue = custom_malloc(sizeof(struct StringValue));
        headerValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(headerValue->base.base.type, "Value");
        headerValue->base.valueType = 0x01;
        headerValue->value = malloc((strlen(headerPtr->value) + 1) * sizeof(char));
        strcpy(headerValue->value, headerPtr->value);
        dict_add_str(headers, headerPtr->name, headerValue);
        headerPtr = headerPtr->next;
    }
    struct Object *headersObj = custom_malloc(sizeof(struct Object));
    headersObj->base.type = malloc((strlen("Object") + 1) * sizeof(char));
    strcpy(headersObj->base.type, "Object");
    headersObj->value = headers;
    notifyNewUsage(headersObj, headersObj->value);
    headersObj->funcs = NULL;
    dict_add_str(resObj->value, "headers", headersObj);
    struct StringValue* contentType = (struct StringValue*) dict_get(headersObj->value, "content-type");
    if (contentType == NULL)
        contentType = dict_get(headersObj->value, "Content-Type");
    if (contentType != NULL && strcmp(contentType->value, "application/json") == 0) {
        json_value *jsonValue = json_parse(httpResult, strlen(httpResult));
        struct Object *body = process_value(jsonValue, 0);
        dict_add_str(resObj->value, "body", body);
    }
    else {
        struct StringValue *bodyStr = custom_malloc(sizeof(struct StringValue));
        bodyStr->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(bodyStr->base.base.type, "Value");
        bodyStr->base.valueType = 0x01;
        bodyStr->value = malloc((strlen(httpResult) + 1) * sizeof(char));
        strcpy(bodyStr->value, httpResult);
        dict_add_str(resObj->value, "body", bodyStr);
    }
    httpResult = NULL;
    httpResultHeaders = NULL;
    return resObj;
}

EM_JS(void, call_set_state, (char* key, char* value), {
    window.parent.setState({...window.parent.state, [UTF8ToString(key)]: UTF8ToString(value)});
});

EM_JS(void, attach_event_listener, (char* eventName, char* funcName), {
    const fName = UTF8ToString(funcName);
    window.parent.addEventListener(UTF8ToString(eventName), () => {
        window[fName]({})
    });
});

void* routeAndResolve(struct String* funcRef, struct Dictionary* entries) {
    /*if (strcmp(funcRef->value, "httpGet") == 0) {
        struct StringValue* address = (struct StringValue*)dict_get(entries, "address");
        CURL *curl = curl_easy_init();
        httpResult = malloc(strlen("") + 1);
        strcpy(httpResult, "");
        httpResultHeaders = malloc(strlen("") + 1);
        strcpy(httpResultHeaders, "");
        if(curl) {
            CURLcode res;
            curl_easy_setopt(curl, CURLOPT_URL, address->value);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
        }
        return prepareResponse();
    }
    else if (strcmp(funcRef->value, "httpPost") == 0) {
        struct StringValue* address = (struct StringValue*)dict_get(entries, "address");
        struct StringValue* body = (struct StringValue*)dict_get(entries, "body");
        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        httpResult = malloc(strlen("") + 1);
        strcpy(httpResult, "");
        httpResultHeaders = malloc(strlen("") + 1);
        strcpy(httpResultHeaders, "");
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, address->value);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body->value);
            struct curl_slist *chunk = NULL;
            chunk = curl_slist_append(chunk, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
        }
        return prepareResponse();
    }*/
    if (strcmp(funcRef->value, "runJS") == 0) {
        struct StringValue* jsCode = (struct StringValue*)dict_get(entries, "jsCode");
        emscripten_run_script((char*)jsCode->value);
    }
    else if (strcmp(funcRef->value, "setState") == 0) {
        struct StringValue* key = (struct StringValue*)dict_get(entries, "stateKey");
        struct StringValue* value = (struct StringValue*)dict_get(entries, "stateValue");
        call_set_state(key->value, value->value);
    }
    else if (strcmp(funcRef->value, "attachEventListener") == 0) {
        struct StringValue* eventName = (struct StringValue*)dict_get(entries, "eventName");
        struct StringValue* funcName = (struct StringValue*)dict_get(entries, "funcName");
        attach_event_listener(eventName->value, funcName->value);
    }
    else if (strcmp(funcRef->value, "intify") == 0) {
        void* inputParam = dict_get(entries, "input");
        if (strcmp(((struct Code*)inputParam)->type, "Value") == 0) {
            if (((struct Value*)inputParam)->valueType == 0x01) {
                struct StringValue* param = (struct StringValue*) inputParam;
                struct IntValue* result = custom_malloc(sizeof(struct IntValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x06;
                char *garbage = NULL;
                result->value = (int)strtol(param->value, &garbage, 0);
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x02) {
                struct ShortValue *param = (struct ShortValue *) inputParam;
                struct IntValue *result = custom_malloc(sizeof(struct IntValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x06;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x03) {
                struct LongValue *param = (struct LongValue *) inputParam;
                struct IntValue *result = custom_malloc(sizeof(struct IntValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x06;
                result->value = (int)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x04) {
                struct FloatValue *param = (struct FloatValue *) inputParam;
                struct IntValue *result = custom_malloc(sizeof(struct IntValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x06;
                result->value = (int)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x05) {
                struct DoubleValue *param = (struct DoubleValue *) inputParam;
                struct IntValue *result = custom_malloc(sizeof(struct IntValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x06;
                result->value = (int)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x06) {
                struct IntValue *param = (struct IntValue *) inputParam;
                struct IntValue *result = custom_malloc(sizeof(struct IntValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x06;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x07) {
                struct BoolValue *param = (struct BoolValue *) inputParam;
                struct IntValue *result = custom_malloc(sizeof(struct IntValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x06;
                result->value = param->value ? 1 : 0;
                return result;
            }
        }
    }
    else if (strcmp(funcRef->value, "shortify") == 0) {
        void* inputParam = dict_get(entries, "input");
        if (strcmp(((struct Code*)inputParam)->type, "Value") == 0) {
            if (((struct Value*)inputParam)->valueType == 0x01) {
                struct StringValue* param = (struct StringValue*) inputParam;
                struct ShortValue* result = custom_malloc(sizeof(struct ShortValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x02;
                char *garbage = NULL;
                result->value = (short)strtol(param->value, &garbage, 0);
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x02) {
                struct ShortValue *param = (struct ShortValue *) inputParam;
                struct ShortValue *result = custom_malloc(sizeof(struct ShortValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x02;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x03) {
                struct LongValue *param = (struct LongValue *) inputParam;
                struct ShortValue *result = custom_malloc(sizeof(struct ShortValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x02;
                result->value = (short)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x04) {
                struct FloatValue *param = (struct FloatValue *) inputParam;
                struct ShortValue *result = custom_malloc(sizeof(struct ShortValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x02;
                result->value = (short)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x05) {
                struct DoubleValue *param = (struct DoubleValue *) inputParam;
                struct ShortValue *result = custom_malloc(sizeof(struct ShortValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x02;
                result->value = (short)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x06) {
                struct IntValue *param = (struct IntValue *) inputParam;
                struct ShortValue *result = custom_malloc(sizeof(struct ShortValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x02;
                result->value = (short)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x07) {
                struct BoolValue *param = (struct BoolValue *) inputParam;
                struct ShortValue *result = custom_malloc(sizeof(struct ShortValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x02;
                result->value = param->value ? 1 : 0;
                return result;
            }
        }
    }
    else if (strcmp(funcRef->value, "longify") == 0) {
        void* inputParam = dict_get(entries, "input");
        if (strcmp(((struct Code*)inputParam)->type, "Value") == 0) {
            if (((struct Value*)inputParam)->valueType == 0x01) {
                struct StringValue* param = (struct StringValue*) inputParam;
                struct LongValue* result = custom_malloc(sizeof(struct LongValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x03;
                char *garbage = NULL;
                result->value = strtol(param->value, &garbage, 0);
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x02) {
                struct ShortValue *param = (struct ShortValue *) inputParam;
                struct LongValue *result = custom_malloc(sizeof(struct LongValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x03;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x03) {
                struct LongValue *param = (struct LongValue *) inputParam;
                struct LongValue *result = custom_malloc(sizeof(struct LongValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x03;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x04) {
                struct FloatValue *param = (struct FloatValue *) inputParam;
                struct LongValue *result = custom_malloc(sizeof(struct LongValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x03;
                result->value = (long)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x05) {
                struct DoubleValue *param = (struct DoubleValue *) inputParam;
                struct LongValue *result = custom_malloc(sizeof(struct LongValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x03;
                result->value = (long)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x06) {
                struct IntValue *param = (struct IntValue *) inputParam;
                struct LongValue *result = custom_malloc(sizeof(struct LongValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x03;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x07) {
                struct BoolValue *param = (struct BoolValue *) inputParam;
                struct LongValue *result = custom_malloc(sizeof(struct LongValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x03;
                result->value = param->value ? 1 : 0;
                return result;
            }
        }
    }
    else if (strcmp(funcRef->value, "floatify") == 0) {
        void* inputParam = dict_get(entries, "input");
        if (strcmp(((struct Code*)inputParam)->type, "Value") == 0) {
            if (((struct Value*)inputParam)->valueType == 0x01) {
                struct StringValue* param = (struct StringValue*) inputParam;
                struct FloatValue* result = custom_malloc(sizeof(struct FloatValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x04;
                char *garbage = NULL;
                result->value = strtof(param->value, &garbage);
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x02) {
                struct ShortValue *param = (struct ShortValue *) inputParam;
                struct FloatValue *result = custom_malloc(sizeof(struct FloatValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x04;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x03) {
                struct LongValue *param = (struct LongValue *) inputParam;
                struct FloatValue *result = custom_malloc(sizeof(struct FloatValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x04;
                result->value = (float)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x04) {
                struct FloatValue *param = (struct FloatValue *) inputParam;
                struct FloatValue *result = custom_malloc(sizeof(struct FloatValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x04;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x05) {
                struct DoubleValue *param = (struct DoubleValue *) inputParam;
                struct FloatValue *result = custom_malloc(sizeof(struct FloatValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x04;
                result->value = (float)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x06) {
                struct IntValue *param = (struct IntValue *) inputParam;
                struct FloatValue *result = custom_malloc(sizeof(struct FloatValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x04;
                result->value = (float)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x07) {
                struct BoolValue *param = (struct BoolValue *) inputParam;
                struct FloatValue *result = custom_malloc(sizeof(struct FloatValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x04;
                result->value = param->value ? 1 : 0;
                return result;
            }
        }
    }
    else if (strcmp(funcRef->value, "doublify") == 0) {
        void* inputParam = dict_get(entries, "input");
        if (strcmp(((struct Code*)inputParam)->type, "Value") == 0) {
            if (((struct Value*)inputParam)->valueType == 0x01) {
                struct StringValue* param = (struct StringValue*) inputParam;
                struct DoubleValue* result = custom_malloc(sizeof(struct DoubleValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x05;
                char *garbage = NULL;
                result->value = strtod(param->value, &garbage);
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x02) {
                struct ShortValue *param = (struct ShortValue *) inputParam;
                struct DoubleValue *result = custom_malloc(sizeof(struct DoubleValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x05;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x03) {
                struct LongValue *param = (struct LongValue *) inputParam;
                struct DoubleValue *result = custom_malloc(sizeof(struct DoubleValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x05;
                result->value = (double)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x04) {
                struct FloatValue *param = (struct FloatValue *) inputParam;
                struct DoubleValue *result = custom_malloc(sizeof(struct DoubleValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x05;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x05) {
                struct DoubleValue *param = (struct DoubleValue *) inputParam;
                struct DoubleValue *result = custom_malloc(sizeof(struct DoubleValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x05;
                result->value = param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x06) {
                struct IntValue *param = (struct IntValue *) inputParam;
                struct DoubleValue *result = custom_malloc(sizeof(struct DoubleValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x05;
                result->value = (double)param->value;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x07) {
                struct BoolValue *param = (struct BoolValue *) inputParam;
                struct DoubleValue *result = custom_malloc(sizeof(struct DoubleValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x05;
                result->value = param->value ? 1 : 0;
                return result;
            }
        }
    }
    else if (strcmp(funcRef->value, "boolify") == 0) {
        void* inputParam = dict_get(entries, "input");
        if (strcmp(((struct Code*)inputParam)->type, "Value") == 0) {
            if (((struct Value*)inputParam)->valueType == 0x01) {
                struct StringValue* param = (struct StringValue*) inputParam;
                struct BoolValue* result = custom_malloc(sizeof(struct BoolValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x07;
                result->value = param->value > 0;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x02) {
                struct ShortValue *param = (struct ShortValue *) inputParam;
                struct BoolValue *result = custom_malloc(sizeof(struct BoolValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x07;
                result->value = param->value > 0;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x03) {
                struct LongValue *param = (struct LongValue *) inputParam;
                struct BoolValue *result = custom_malloc(sizeof(struct BoolValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x07;
                result->value = param->value > 0;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x04) {
                struct FloatValue *param = (struct FloatValue *) inputParam;
                struct BoolValue *result = custom_malloc(sizeof(struct BoolValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x07;
                result->value = param->value > 0;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x05) {
                struct DoubleValue *param = (struct DoubleValue *) inputParam;
                struct BoolValue *result = custom_malloc(sizeof(struct BoolValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x07;
                result->value = param->value > 0;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x06) {
                struct IntValue *param = (struct IntValue *) inputParam;
                struct BoolValue *result = custom_malloc(sizeof(struct BoolValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x07;
                result->value = param->value > 0;
                return result;
            }
            else if (((struct Value*)inputParam)->valueType == 0x07) {
                struct BoolValue *param = (struct BoolValue *) inputParam;
                struct BoolValue *result = custom_malloc(sizeof(struct BoolValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x07;
                result->value = param->value;
                return result;
            }
        }
    }
    else if (strcmp(funcRef->value, "random") == 0) {
        struct IntValue* max = (struct IntValue*) dict_get(entries, "max");
        struct IntValue* result = custom_malloc(sizeof(struct IntValue));
        result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(result->base.base.type, "Value");
        result->base.valueType = 0x06;
        result->value = (int)floor(max->value / 2);
        return result;
    }
    if (strcmp(funcRef->value, "HttpServer.createServer") == 0) {
        struct StringValue* filesPathVal = (struct StringValue*) dict_get(entries, "filesPath");
        struct StringValue* rootVal = (struct StringValue*) dict_get(entries, "root");
        struct Object* result = custom_malloc(sizeof(struct Object));
        result->base.type = malloc((strlen("Object") + 1) * sizeof(char));
        strcpy(result->base.type, "Object");
        result->funcs = NULL;
        result->value = dict_new();
        notifyNewUsage(result, result->value);
        dict_add_str(result->value, "filesPath", filesPathVal);
        dict_add_str(result->value, "root", rootVal);
        dict_add_str(result->value, "routes", dict_new());
        return result;
    }
    else if (strcmp(funcRef->value, "HttpServer.registerRoute") == 0) {
        struct Object* server = (struct Object*) dict_get(entries, "server");
        struct StringValue* path = (struct StringValue*) dict_get(entries, "path");
        struct Function* pathAction = (struct Function*) dict_get(entries, "pathAction");
        dict_add_str((struct Dictionary *) dict_get(server->value, "routes"), path->value, pathAction);
    }
    else if (strcmp(funcRef->value, "HttpServer.startServer") == 0) {
        struct Object* server = (struct Object*) dict_get(entries, "server");
        struct ShortValue* port = (struct ShortValue*) dict_get(entries, "port");
        char str[(int) (ceilf(measureLog10f(port->value) + 1) * sizeof(char))];
        sprintf(str, "%hd", port->value);
        char *str2 = malloc(sizeof(str));
        strcpy(str2, str);
        struct StringValue* portStr = custom_malloc(sizeof(struct StringValue));
        portStr->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(portStr->base.base.type, "Value");
        portStr->base.valueType = 0x01;
        portStr->value = str2;
        dict_add_str(server->value, "port", portStr);
        pthread_t thread;
        struct HttpServerArg* httpServerArg = malloc(sizeof(struct HttpServerArg));
        httpServerArg->ds = dataStack;
        httpServerArg->serve = executeIntern;
        httpServerArg->server = server;
        httpServerArg->stringifyObj = stringifyObject;
        http_server_start(httpServerArg);
    }
    else if (strcmp(funcRef->value, "print") == 0) {
        void* v = dict_get(entries, "text");
        if (strcmp(((struct Code*)v)->type, "Object") == 0) {
            char* temp = stringifyObject((struct Object*) v);
            printf("%s\n", temp);
            free(temp);
        } else if (strcmp(((struct Code*)v)->type, "Array") == 0) {
            char* temp = stringifyArray((struct Array*) v);
            printf("%s\n", temp);
            free(temp);
        }
        else if (strcmp(((struct Code*)v)->type, "Empty") == 0) {
            printf("{ }\n");
        }
        else if (((struct Value*)v)->valueType == 0x01) {
            printf("%s\n", ((struct StringValue*)v)->value);
        }
        else if (((struct Value*)v)->valueType == 0x02) {
            struct ShortValue* val1 = (struct ShortValue*)v;
            printf("%hd\n", val1->value);
        }
        else if (((struct Value*)v)->valueType == 0x06) {
            struct IntValue* val1 = (struct IntValue*)v;
            printf("%d\n", val1->value);
        }
        else if (((struct Value*)v)->valueType == 0x03) {
            struct LongValue* val1 = (struct LongValue*)v;
            printf("%ld\n", val1->value);
        }
        else if (((struct Value*)v)->valueType == 0x04) {
            struct FloatValue* val1 = (struct FloatValue*)v;
            printf("%f\n", val1->value);
        }
        else if (((struct Value*)v)->valueType == 0x05) {
            struct DoubleValue* val1 = (struct DoubleValue*)v;
            printf("%f\n", val1->value);
        }
        else if (((struct Value*)v)->valueType == 0x07) {
            struct BoolValue* val1 = (struct BoolValue*)v;
            printf("%s\n", val1->value ? "true" : "false");
        }
        else {
            printf("Error line: %d , file: %s\n", __LINE__, __FILE__);
            exit(15);
        }
    }
    else if (strcmp(funcRef->value, "sha256") == 0) {
//        struct StringValue* result = custom_malloc(sizeof(struct StringValue));
//        result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
//        strcpy(result->base.base.type, "Value");
//        result->base.valueType = 0x01;
//        char* value = sha256_hex((char*)((struct StringValue*)dict_get(entries, "content"))->value);
//        result->value = value;
//        return result;
    } else if (strcmp(funcRef->value, "len") == 0) {
        void* rawArr = dict_get(entries, "obj");
        if (rawArr != NULL) {
            if (strcmp(((struct Code *) rawArr)->type, "Array") == 0) {
                struct LongValue *result = custom_malloc(sizeof(struct LongValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x03;
                result->value = (long) ((struct Array *) rawArr)->used;
                return result;
            }
            else if (strcmp(((struct Code*)rawArr)->type, "Value") == 0 && ((struct Value*)rawArr)->valueType == 0x01) {
                struct StringValue* str = (struct StringValue*) rawArr;
                struct LongValue *result = custom_malloc(sizeof(struct LongValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x03;
                result->value = (long)strlen(str->value);
                return result;
            }
            else {
                printf("Error line: %d , file: %s\n", __LINE__, __FILE__);
                exit(15);
            }
        }
        else {
            printf("Error line: %d , file: %s\n", __LINE__, __FILE__);
            exit(15);
        }
    } else if (strcmp(funcRef->value, "time.now") == 0) {
        struct LongValue* result = custom_malloc(sizeof(struct LongValue));
        result->base.valueType = 0x03;
        result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(result->base.base.type, "Value");
        struct timeval time;
        gettimeofday(&time, NULL);
        int64_t s1 = (int64_t)(time.tv_sec) * 1000;
        int64_t s2 = (time.tv_usec / 1000);
        result->value = s1 + s2;
        return result;
    } else if (strcmp(funcRef->value, "append") == 0) {
        void* rawArr = (struct Array*) dict_get(entries, "list");
        if (rawArr != NULL) {
            if (strcmp(((struct Code*)rawArr)->type, "Array") == 0) {
                void *obj = dict_get(entries, "listItem");
                insertArray((struct Array *) rawArr, obj);
            }
            else {
                printf("Error line: %d , file: %s\n", __LINE__, __FILE__);
                exit(15);
            }
        }
        else {
            printf("Error line: %d , file: %s\n", __LINE__, __FILE__);
            exit(15);
        }
    } else if (strcmp(funcRef->value, "last") == 0) {
        void* rawArr = (struct Array*) dict_get(entries, "arr");
        if (rawArr != NULL) {
            if (strcmp(((struct Code*)rawArr)->type, "Array") == 0) {
                struct Array* array = (struct Array*) rawArr;
                for (int counter = array->used - 1; counter >= 0; counter--) {
                    if (array->array[counter] != NULL) {
                        return array->array[counter];
                    } else {
                        printf("Error line: %d , file: %s\n", __LINE__, __FILE__);
                        exit(15);
                    }
                }
            }
            else {
                printf("Error line: %d , file: %s\n", __LINE__, __FILE__);
                exit(15);
            }
        }
        else {
            printf("Error line: %d , file: %s\n", __LINE__, __FILE__);
            exit(15);
        }
    } else if (strcmp(funcRef->value, "stringify") == 0) {
        void* obj = dict_get(entries, "obj");
        if (strcmp(((struct Code*)obj)->type, "Array") == 0) {
            struct StringValue* result = custom_malloc(sizeof(struct StringValue));
            result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(result->base.base.type, "Value");
            result->base.valueType = 0x01;
            char* resValue = stringifyArray((struct Array*)obj);
            result->value = resValue;
            return result;
        }
        else if (strcmp(((struct Code*)obj)->type, "Object") == 0) {
            struct StringValue* result = custom_malloc(sizeof(struct StringValue));
            result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(result->base.base.type, "Value");
            result->base.valueType = 0x01;
            result->value = stringifyObject((struct Object*)obj);
            return result;
        } else if (strcmp(((struct Code*)obj)->type, "Value") == 0) {
            if ((*(struct Value *) obj).valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) obj;
                char str[(int) (ceilf(measureLog10f(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%hd", val1.value);
                char *str2 = &str[0];
                struct StringValue *result = custom_malloc(sizeof(struct StringValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if ((*(struct Value *) obj).valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) obj;
                char str[(int) (ceil(measureLog10(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%d", val1.value);
                char *str2 = &str[0];
                struct StringValue *result = custom_malloc(sizeof(struct StringValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if ((*(struct Value *) obj).valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) obj;
                char str[(int) (ceill(measureLog10l(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%ld", val1.value);
                char *str2 = &str[0];
                struct StringValue *result = custom_malloc(sizeof(struct StringValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if ((*(struct Value *) obj).valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) obj;
                char str[(int) (ceilf(measureLog10f(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%f", val1.value);
                char *str2 = &str[0];
                struct StringValue *result = custom_malloc(sizeof(struct StringValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if ((*(struct Value *) obj).valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) obj;
                char str[(int) (ceil(measureLog10(val1.value) + 1) * sizeof(char))];
                sprintf(str, "%f", val1.value);
                char *str2 = &str[0];
                struct StringValue *result = custom_malloc(sizeof(struct StringValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x01;
                result->value = str2;
                return result;
            } else if (((struct Value *) obj)->valueType == 0x01) {
                return obj;
            }
        }
    } else if (strcmp(funcRef->value, "uuid") == 0) {
        uuid_t binuuid;
        uuid_generate(binuuid);
        char *uuid = malloc(37);
        #ifdef capitaluuid
        uuid_unparse_upper(binuuid, uuid);
        #elif lowercaseuuid
        uuid_unparse_lower(binuuid, uuid);
        #else
        uuid_unparse(binuuid, uuid);
        #endif
        struct StringValue* result = custom_malloc(sizeof(struct StringValue));
        result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(result->base.base.type, "Value");
        result->base.valueType = 0x01;
        result->value = uuid;
        return result;
    } else if (strcmp(funcRef->value, "replace") == 0) {
        struct StringValue* str = (struct StringValue*)dict_get(entries, "str");
        struct StringValue* from = (struct StringValue*)dict_get(entries, "from");
        struct StringValue* to = (struct StringValue*)dict_get(entries, "to");
        char* resultStr = strdup(str->value);
        char* tempResultStr = replaceWord(resultStr, from->value, to->value);
        free(resultStr);
        resultStr = tempResultStr;
        struct StringValue* result = custom_malloc(sizeof(struct StringValue));
        result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(result->base.base.type, "Value");
        result->base.valueType = 0x01;
        result->value = resultStr;
        return result;
    } else if (strcmp(funcRef->value, "match_object_structure") == 0) {
        struct Object* obj = (struct Object*) dict_get(entries, "obj");
        struct Array* schema = (struct Array*) dict_get(entries, "structure");
        for (int i = 0; i < schema->used; i++) {
            struct StringValue* fieldName = (struct StringValue*) schema->array[i];
            if (dict_get(obj->value, fieldName->value) == NULL) {
                struct BoolValue* result = custom_malloc(sizeof(struct BoolValue));
                result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                strcpy(result->base.base.type, "Value");
                result->base.valueType = 0x07;
                result->value = false;
                return result;
            }
        }
        struct BoolValue* result = custom_malloc(sizeof(struct BoolValue));
        result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(result->base.base.type, "Value");
        result->base.valueType = 0x07;
        result->value = true;
        return result;
    }
    return NULL;
}

void* sum(void* value1, void* value2) {
    void* res = NULL;
    if (strcmp(((struct Code*)value1)->type, "Empty") == 0) {
        if (strcmp(((struct Code*)value2)->type, "Empty") == 0) {
            res = custom_malloc(sizeof(struct ShortValue));
            ((struct ShortValue*)res)->value = 0;
            ((struct Value*)res)->base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(((struct Value*)res)->base.type, "Value");
            ((struct Value*)res)->valueType = ((struct Value*)value1)->valueType;
            return res;
        }
        else {
            if (((struct Value*)value2)->valueType == 0x01) {
                struct StringValue* strValue = custom_malloc(sizeof(struct StringValue));
                strValue->value = malloc((strlen(((struct StringValue*)value2)->value) + 1) * sizeof(char));
                strcpy(strValue->value, ((struct StringValue*)value2)->value);
                res = strValue;
            }
            else if (((struct Value*)value2)->valueType == 0x02) {
                struct ShortValue* strValue = custom_malloc(sizeof(struct ShortValue));
                strValue->value = ((struct ShortValue*)value2)->value;
                res = strValue;
            }
            else if (((struct Value*)value2)->valueType == 0x03) {
                struct LongValue* strValue = custom_malloc(sizeof(struct LongValue));
                strValue->value = ((struct LongValue*)value2)->value;
                res = strValue;
            }
            else if (((struct Value*)value2)->valueType == 0x04) {
                struct FloatValue* strValue = custom_malloc(sizeof(struct FloatValue));
                strValue->value = ((struct FloatValue*)value2)->value;
                res = strValue;
            }
            else if (((struct Value*)value2)->valueType == 0x05) {
                struct DoubleValue* strValue = custom_malloc(sizeof(struct DoubleValue));
                strValue->value = ((struct DoubleValue*)value2)->value;
                res = strValue;
            }
            else if (((struct Value*)value2)->valueType == 0x06) {
                struct IntValue* strValue = custom_malloc(sizeof(struct IntValue));
                strValue->value = ((struct IntValue*)value2)->value;
                res = strValue;
            }
            else if (((struct Value*)value2)->valueType == 0x07) {
                struct BoolValue* strValue = custom_malloc(sizeof(struct BoolValue));
                strValue->value = ((struct BoolValue*)value2)->value;
                res = strValue;
            }
            ((struct Value*)res)->base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(((struct Value*)res)->base.type, "Value");
            ((struct Value*)res)->valueType = ((struct Value*)value2)->valueType;
            return res;
        }
    }
    else {
        if (strcmp(((struct Code*)value2)->type, "Empty") == 0) {
            if (((struct Value*)value1)->valueType == 0x01) {
                struct StringValue* strValue = custom_malloc(sizeof(struct StringValue));
                strValue->value = malloc((strlen(((struct StringValue*)value1)->value) + 1) * sizeof(char));
                strcpy(strValue->value, ((struct StringValue*)value1)->value);
                res = strValue;
            }
            else if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue* strValue = custom_malloc(sizeof(struct ShortValue));
                strValue->value = ((struct ShortValue*)value1)->value;
                res = strValue;
            }
            else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue* strValue = custom_malloc(sizeof(struct LongValue));
                strValue->value = ((struct LongValue*)value1)->value;
                res = strValue;
            }
            else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue* strValue = custom_malloc(sizeof(struct FloatValue));
                strValue->value = ((struct FloatValue*)value1)->value;
                res = strValue;
            }
            else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue* strValue = custom_malloc(sizeof(struct DoubleValue));
                strValue->value = ((struct DoubleValue*)value1)->value;
                res = strValue;
            }
            else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue* strValue = custom_malloc(sizeof(struct IntValue));
                strValue->value = ((struct IntValue*)value1)->value;
                res = strValue;
            }
            else if (((struct Value*)value1)->valueType == 0x07) {
                struct BoolValue* strValue = custom_malloc(sizeof(struct BoolValue));
                strValue->value = ((struct BoolValue*)value1)->value;
                res = strValue;
            }
            ((struct Value*)res)->base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(((struct Value*)res)->base.type, "Value");
            ((struct Value*)res)->valueType = ((struct Value*)value1)->valueType;
            return res;
        }
    }
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            long double result = 0;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue*)value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue*)value2;
                    result = (long double)val1.value + (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue*)value2;
                    result = (long double)val1.value + (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue*)value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue*)value2;
                    result = (long double)val1.value + (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue*)value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue*)value2;
                    result = (long double) val1.value + (long double)val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue*)value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue*)value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue*)value2;
                    result = (long double)val1.value + (long double)val2.value;
                }
            }

            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    struct ShortValue* resValue = custom_malloc(sizeof(struct ShortValue));
                    resValue->value = (short)result;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
                else if (result < INT32_MAX) {
                    struct IntValue* resValue = custom_malloc(sizeof(struct IntValue));
                    resValue->value = (int)result;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
                else if (result < INT64_MAX) {
                    struct LongValue* resValue = custom_malloc(sizeof(struct LongValue));
                    resValue->value = (long)result;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
            }
            else {
                if (result < FLT_MAX) {
                    struct FloatValue* resValue = custom_malloc(sizeof(struct FloatValue));
                    resValue->value = (float)result;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
                else if (result < DBL_MAX) {
                    struct DoubleValue* resValue = custom_malloc(sizeof(struct DoubleValue));
                    resValue->value = (double)result;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
            }
        }
        else if (((struct Value*)value2)->valueType == 0x01) {
            struct StringValue val2 = *(struct StringValue*)value2;
            char* str2 = NULL;
            if (((struct Value*)value1)->valueType == 0x01) {
                struct StringValue val1 = *(struct StringValue*)value1;
                str2 = malloc(strlen(val1.value) + 1);
                strcpy(str2, val1.value);
            }
            else if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue*)value1;
                char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                sprintf(str, "%hd", val1.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue*)value1;
                char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                sprintf(str, "%d", val1.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue*)value1;
                char str[(int)(ceill(measureLog10l(val1.value)+1)*sizeof(char))];
                sprintf(str, "%ld", val1.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue*)value1;
                char str[(int)(ceilf(measureLog10f(val1.value)+1)*sizeof(char))];
                sprintf(str, "%f", val1.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue*)value1;
                char str[(int)(ceil(measureLog10(val1.value)+1)*sizeof(char))];
                sprintf(str, "%f", val1.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            }
            struct StringValue* resValue = custom_malloc(sizeof(struct StringValue));
            char* result = concat(str2, val2.value);
            free(str2);
            resValue->value = result;
            resValue->base.valueType = 0x01;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            res = resValue;
        }
        else if (((struct Value*)value2)->valueType == 0x07) {
            struct StringValue val2 = *(struct StringValue*)value2;
            double result = 0;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue*)value1;
                result = (double)val1.value + (val2.value ? 1 : 0);
            }

            if (floor(result) == result) {
                if (result < INT16_MAX) {
                    struct ShortValue* resValue = custom_malloc(sizeof(struct ShortValue));
                    short r = (short)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
                else if (result < INT32_MAX) {
                    struct IntValue* resValue = custom_malloc(sizeof(struct IntValue));
                    int r = (int)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
                else if (result < INT64_MAX) {
                    struct LongValue* resValue = custom_malloc(sizeof(struct LongValue));
                    long r = (long)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
            }
            else {
                if (result < FLT_MAX) {
                    struct FloatValue* resValue = custom_malloc(sizeof(struct FloatValue));
                    float r = (float)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
                else if (result < DBL_MAX) {
                    struct DoubleValue* resValue = custom_malloc(sizeof(struct DoubleValue));
                    double r = (double)result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
            }
        }
    }
    else if (((struct Value*)value1)->valueType == 0x01) {
        struct StringValue* val1 = (struct StringValue*)value1;
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            char* str2 = "";
            if (((struct Value*)value2)->valueType == 0x02) {
                struct ShortValue val2 = *(struct ShortValue*)value2;
                char str[(int)(ceil(measureLog10(val2.value)+1)*sizeof(char))];
                sprintf(str, "%hd", val2.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            } else if (((struct Value*)value2)->valueType == 0x06) {
                struct IntValue val2 = *(struct IntValue*)value2;
                char str[(int)(ceil(measureLog10(val2.value)+1)*sizeof(char))];
                sprintf(str, "%d", val2.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            } else if (((struct Value*)value2)->valueType == 0x03) {
                struct LongValue val2 = *(struct LongValue*)value2;
                char str[(int)(ceill(measureLog10l(val2.value)+1)*sizeof(char))];
                sprintf(str, "%ld", val2.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            } else if (((struct Value*)value2)->valueType == 0x04) {
                struct FloatValue val2 = *(struct FloatValue*)value2;
                char str[(int)(ceilf(measureLog10f(val2.value)+1)*sizeof(char))];
                sprintf(str, "%f", val2.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            } else if (((struct Value*)value2)->valueType == 0x05) {
                struct DoubleValue val2 = *(struct DoubleValue*)value2;
                char str[(int)(ceil(measureLog10(val2.value)+1)*sizeof(char))];
                sprintf(str, "%f", val2.value);
                str2 = malloc(strlen(str) + 1);
                strcpy(str2, str);
            }
            struct StringValue* resValue = custom_malloc(sizeof(struct StringValue));
            char* dest = concat(val1->value, str2);
            free(str2);
            resValue->value = dest;
            resValue->base.valueType = 0x01;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            res = resValue;
        } else if (((struct Value*)value2)->valueType == 0x01) {
            struct StringValue* val2 = (struct StringValue*)value2;
            struct StringValue* resValue = custom_malloc(sizeof(struct StringValue));
            resValue->base.valueType = 0x01;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            resValue->value = concat(val1->value, val2->value);
            res = resValue;
        } else if (((struct Value*)value2)->valueType == 0x07) {
            struct BoolValue val2 = *(struct BoolValue*)value2;
            char* boolStr = val2.value ? "true" : "false";
            char* result = concat(val1->value, boolStr);
            struct StringValue* resValue = custom_malloc(sizeof(struct StringValue));
            resValue->value = result;
            resValue->base.valueType = 0x01;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            res = resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x07) {
        struct BoolValue val1 = *(struct BoolValue*)value1;
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            double result = 0;
            if (((struct Value*)value2)->valueType == 0x02) {
                struct ShortValue val2 = *(struct ShortValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            } else if (((struct Value*)value2)->valueType == 0x06) {
                struct IntValue val2 = *(struct IntValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            } else if (((struct Value*)value2)->valueType == 0x03) {
                struct LongValue val2 = *(struct LongValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            } else if (((struct Value*)value2)->valueType == 0x04) {
                struct FloatValue val2 = *(struct FloatValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            } else if (((struct Value*)value2)->valueType == 0x05) {
                struct DoubleValue val2 = *(struct DoubleValue*)value2;
                result = (double)(val1.value ? 1 : 0) + (double)val2.value;
            }

            if (floor(result) == result) {
                if (result < INT16_MAX) {
                    short r = (short)result;
                    struct ShortValue* resValue = custom_malloc(sizeof(struct ShortValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
                else if (result < INT32_MAX) {
                    int r = (int)result;
                    struct IntValue* resValue = custom_malloc(sizeof(struct IntValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
                else if (result < INT64_MAX) {
                    long r = (long)result;
                    struct LongValue* resValue = custom_malloc(sizeof(struct LongValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
            }
            else {
                if (result < FLT_MAX) {
                    float r = (float)result;
                    struct FloatValue* resValue = custom_malloc(sizeof(struct FloatValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
                else if (result < DBL_MAX) {
                    double r = (double)result;
                    struct DoubleValue* resValue = custom_malloc(sizeof(struct DoubleValue));
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    res = resValue;
                }
            }
        } else if (((struct Value*)value2)->valueType == 0x01) {
            struct StringValue val2 = *(struct StringValue*)value2;
            char* result = concat(val1.value ? "true" : "false", val2.value);
            struct StringValue* resValue = custom_malloc(sizeof(struct StringValue));
            resValue->value = result;
            resValue->base.valueType = 0x01;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            res = resValue;
        } else if (((struct Value*)value2)->valueType == 0x07) {
            struct BoolValue val2 = *(struct BoolValue*)value2;
            bool result = val1.value || val2.value;
            struct BoolValue* resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            res = resValue;
        }
    }

    return res;
}

void* subtract(void* value1, void* value2) {
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            long double result = 0;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value - (long double) val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    struct ShortValue *resValue = custom_malloc(sizeof(struct ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT32_MAX) {
                    struct IntValue *resValue = custom_malloc(sizeof(struct IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT64_MAX) {
                    struct LongValue *resValue = custom_malloc(sizeof(struct LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    struct FloatValue *resValue = custom_malloc(sizeof(struct FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < DBL_MAX) {
                    struct DoubleValue *resValue = custom_malloc(sizeof(struct DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            }
        }
    }
}

void* multiply(void* value1, void* value2) {
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            long double result = 0;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value * (long double) val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    struct ShortValue *resValue = custom_malloc(sizeof(struct ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT32_MAX) {
                    struct IntValue *resValue = custom_malloc(sizeof(struct IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT64_MAX) {
                    struct LongValue *resValue = custom_malloc(sizeof(struct LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    struct FloatValue *resValue = custom_malloc(sizeof(struct FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < DBL_MAX) {
                    struct DoubleValue *resValue = custom_malloc(sizeof(struct DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            }
        }
    }
}

void* divide(void* value1, void* value2) {
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            long double result = 0;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value / (long double) val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    struct ShortValue *resValue = custom_malloc(sizeof(struct ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT32_MAX) {
                    struct IntValue *resValue = custom_malloc(sizeof(struct IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT64_MAX) {
                    struct LongValue *resValue = custom_malloc(sizeof(struct LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    struct FloatValue *resValue = custom_malloc(sizeof(struct FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < DBL_MAX) {
                    struct DoubleValue *resValue = custom_malloc(sizeof(struct DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            }
        }
    }
}

void* mod(void* value1, void* value2) {
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            long double result = 0;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long) val1.value % (long) val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    struct ShortValue *resValue = custom_malloc(sizeof(struct ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT32_MAX) {
                    struct IntValue *resValue = custom_malloc(sizeof(struct IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT64_MAX) {
                    struct LongValue *resValue = custom_malloc(sizeof(struct LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            }
        }
    }
}

void* power(void* value1, void* value2) {
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            long double result = false;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue* val1 = (struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue* val2 = (struct ShortValue *) value2;
                    result = powl((long double) val1->value, (long double) val2->value);
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = powl((long double) val1->value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = powl((long double) val1->value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = powl((long double) val1->value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = powl((long double) val1->value, (long double) val2.value);
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = powl((long double) val1.value, (long double) val2.value);
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    struct ShortValue *resValue = custom_malloc(sizeof(struct ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT32_MAX) {
                    struct IntValue *resValue = custom_malloc(sizeof(struct IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT64_MAX) {
                    struct LongValue *resValue = custom_malloc(sizeof(struct LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    struct FloatValue *resValue = custom_malloc(sizeof(struct FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < DBL_MAX) {
                    struct DoubleValue *resValue = custom_malloc(sizeof(struct DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            }
        }
    }
}

void* and(void* value1, void* value2) {
    if (strcmp(((struct Code*)value1)->type, "Empty") == 0 || strcmp(((struct Code*)value2)->type, "Empty") == 0) {
        struct LongValue* result = custom_malloc(sizeof(struct LongValue));
        result->base.valueType = 0x03;
        result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(result->base.base.type, "Value");
        result->value = 0;
        return value2;
    }
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            long double result = 0;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = val1.value & val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = val1.value & val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = val1.value & val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = val1.value & val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = val1.value & val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = val1.value & val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = val1.value & val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = val1.value & val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = val1.value & val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    struct ShortValue *resValue = custom_malloc(sizeof(struct ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT32_MAX) {
                    struct IntValue *resValue = custom_malloc(sizeof(struct IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT64_MAX) {
                    struct LongValue *resValue = custom_malloc(sizeof(struct LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    struct FloatValue *resValue = custom_malloc(sizeof(struct FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < DBL_MAX) {
                    struct DoubleValue *resValue = custom_malloc(sizeof(struct DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            }
        }
    }
    else if (((struct Value*)value1)->valueType == 0x07) {
        struct BoolValue* val1 = (struct BoolValue*) value1;
        if (((struct Value*)value2)->valueType == 0x07) {
            struct BoolValue* val2 = (struct BoolValue*) value2;
            bool result = val1->value & val2->value;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            return resValue;
        }
    }
}

void* or(void* value1, void* value2) {
    if (value1 == NULL || strcmp(((struct Code*)value1)->type, "Empty") == 0) {
        return value2;
    }
    else if (value2 == NULL || strcmp(((struct Code*)value2)->type, "Empty") == 0) {
        return value1;
    }
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            long double result = 0;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = val1.value | val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = val1.value | val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = val1.value | val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = val1.value | val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = val1.value | val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = val1.value | val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = val1.value | val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = val1.value | val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = val1.value | val2.value;
                }
            }
            if (floorl(result) == result) {
                if (result < INT16_MAX) {
                    struct ShortValue *resValue = custom_malloc(sizeof(struct ShortValue));
                    short r = (short) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x02;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT32_MAX) {
                    struct IntValue *resValue = custom_malloc(sizeof(struct IntValue));
                    int r = (int) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x06;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < INT64_MAX) {
                    struct LongValue *resValue = custom_malloc(sizeof(struct LongValue));
                    long r = (long) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x03;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            } else {
                if (result < FLT_MAX) {
                    struct FloatValue *resValue = custom_malloc(sizeof(struct FloatValue));
                    float r = (float) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x04;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                } else if (result < DBL_MAX) {
                    struct DoubleValue *resValue = custom_malloc(sizeof(struct DoubleValue));
                    double r = (double) result;
                    resValue->value = r;
                    resValue->base.valueType = 0x05;
                    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
                    strcpy(resValue->base.base.type, "Value");
                    return resValue;
                }
            }
        }
    }
    else if (((struct Value*)value1)->valueType == 0x07) {
        struct BoolValue* val1 = (struct BoolValue*) value1;
        if (((struct Value*)value2)->valueType == 0x07) {
            struct BoolValue* val2 = (struct BoolValue*) value2;
            bool result = val1->value | val2->value;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            resValue->value = result;
            resValue->base.valueType = 0x07;
            return resValue;
        }
    }
}

void* equal(void* value1, void* value2) {
    if (strcmp(((struct Code*)value1)->type, "Empty") == 0) {
        if (strcmp(((struct Code*)value2)->type, "Empty") == 0) {
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = true;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
        else {
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = false;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else {
        if (strcmp(((struct Code*)value2)->type, "Empty") == 0) {
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = false;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            bool result = false;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value == (long double) val2.value;
                }
            }
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x01) {
        struct StringValue* val1 = (struct StringValue*) value1;
        if (((struct Value*)value2)->valueType == 0x01) {
            struct StringValue* val2 = (struct StringValue*) value2;
            bool result = (strcmp(val1->value, val2->value) == 0);
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x07) {
        struct BoolValue* val1 = (struct BoolValue*) value1;
        if (((struct Value*)value2)->valueType == 0x07) {
            struct BoolValue* val2 = (struct BoolValue*) value2;
            bool result = val1->value == val2->value;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
    strcpy(resValue->base.base.type, "Value");
    return resValue;
}

void* ne(void* value1, void* value2) {
    return not(equal(value1, value2));
}

void* lt(void* value1, void* value2) {
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            bool result = false;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value < (long double) val2.value;
                }
            }
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x01) {
        struct StringValue* val1 = (struct StringValue*) value1;
        if (((struct Value*)value2)->valueType == 0x01) {
            struct StringValue* val2 = (struct StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) < 0;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x07) {
        struct BoolValue* val1 = (struct BoolValue*) value1;
        if (((struct Value*)value2)->valueType == 0x07) {
            struct BoolValue* val2 = (struct BoolValue*) value2;
            bool result = val1->value < val2->value;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
    strcpy(resValue->base.base.type, "Value");
    return resValue;
}

void* le(void* value1, void* value2) {
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            bool result = false;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value <= (long double) val2.value;
                }
            }
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x01) {
        struct StringValue* val1 = (struct StringValue*) value1;
        if (((struct Value*)value2)->valueType == 0x01) {
            struct StringValue* val2 = (struct StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) <= 0;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x07) {
        struct BoolValue* val1 = (struct BoolValue*) value1;
        if (((struct Value*)value2)->valueType == 0x07) {
            struct BoolValue* val2 = (struct BoolValue*) value2;
            bool result = val1->value <= val2->value;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
    strcpy(resValue->base.base.type, "Value");
    return resValue;
}

void* ge(void* value1, void* value2) {
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            bool result = false;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value >= (long double) val2.value;
                }
            }
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x01) {
        struct StringValue* val1 = (struct StringValue*) value1;
        if (((struct Value*)value2)->valueType == 0x01) {
            struct StringValue* val2 = (struct StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) >= 0;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x07) {
        struct BoolValue* val1 = (struct BoolValue*) value1;
        if (((struct Value*)value2)->valueType == 0x07) {
            struct BoolValue* val2 = (struct BoolValue*) value2;
            bool result = val1->value >= val2->value;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
    strcpy(resValue->base.base.type, "Value");
    return resValue;
}

void* gt(void* value1, void* value2) {
    if (strcmp(((struct Code*)value1)->type, "Empty") == 0 || strcmp(((struct Code*)value2)->type, "Empty") == 0) {
        printf("Error line: %d , file: %s\n", __LINE__, __FILE__);
        exit(15);
    }
    if (((struct Value*)value1)->valueType == 0x06 ||
        ((struct Value*)value1)->valueType == 0x02 ||
        ((struct Value*)value1)->valueType == 0x03 ||
        ((struct Value*)value1)->valueType == 0x04 ||
        ((struct Value*)value1)->valueType == 0x05) {
        if (((struct Value*)value2)->valueType == 0x06 ||
            ((struct Value*)value2)->valueType == 0x02 ||
            ((struct Value*)value2)->valueType == 0x03 ||
            ((struct Value*)value2)->valueType == 0x04 ||
            ((struct Value*)value2)->valueType == 0x05) {
            bool result = false;
            if (((struct Value*)value1)->valueType == 0x02) {
                struct ShortValue val1 = *(struct ShortValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x06) {
                struct IntValue val1 = *(struct IntValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x03) {
                struct LongValue val1 = *(struct LongValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x04) {
                struct FloatValue val1 = *(struct FloatValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                }
            } else if (((struct Value*)value1)->valueType == 0x05) {
                struct DoubleValue val1 = *(struct DoubleValue *) value1;
                if (((struct Value*)value2)->valueType == 0x02) {
                    struct ShortValue val2 = *(struct ShortValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x06) {
                    struct IntValue val2 = *(struct IntValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x03) {
                    struct LongValue val2 = *(struct LongValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x04) {
                    struct FloatValue val2 = *(struct FloatValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                } else if (((struct Value*)value2)->valueType == 0x05) {
                    struct DoubleValue val2 = *(struct DoubleValue *) value2;
                    result = (long double) val1.value > (long double) val2.value;
                }
            }
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x01) {
        struct StringValue* val1 = (struct StringValue*) value1;
        if (((struct Value*)value2)->valueType == 0x01) {
            struct StringValue* val2 = (struct StringValue*) value2;
            bool result = strcmp(val1->value, val2->value) > 0;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");
            return resValue;
        }
    }
    else if (((struct Value*)value1)->valueType == 0x07) {
        struct BoolValue* val1 = (struct BoolValue*) value1;
        if (((struct Value*)value2)->valueType == 0x07) {
            struct BoolValue* val2 = (struct BoolValue*) value2;
            bool result = val1->value > val2->value;
            struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
            resValue->value = result;
            resValue->base.valueType = 0x07;
            resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(resValue->base.base.type, "Value");

            return resValue;
        }
    }
    struct BoolValue *resValue = custom_malloc(sizeof(struct BoolValue));
    resValue->value = false;
    resValue->base.valueType = 0x07;
    resValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
    strcpy(resValue->base.base.type, "Value");
    return resValue;
}

void* not(void* value) {
    struct Value* valueRaw = (struct Value*) value;
    if (valueRaw->valueType == 0x07) {
        struct BoolValue* result = custom_malloc(sizeof(struct BoolValue));
        result->value = !((struct BoolValue*)value)->value;
        result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(result->base.base.type, "Value");
        result->base.valueType = 0x07;
        return result;
    }
}

void* resolveIndex(struct Dictionary* buffer, struct Index* index) {
    void* target = index->var;
    if (strcmp(((struct Code *) target)->type, "Object") == 0) {
        struct Object* obj = (struct Object*)target;
        if (index->index->size == 1) {
            void* indexNum = index->index->startPointer->data;
            if (strcmp(((struct Code*)indexNum)->type, "Value") == 0) {
                if (((struct Value*)indexNum)->valueType == 0x01) {
                    struct StringValue* indexStr = (struct StringValue*) indexNum;
                    return dict_get(obj->value, indexStr->value);
                }
            }
        }
        struct Empty *empty = custom_malloc(sizeof(struct Empty));
        empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
        strcpy(empty->base.type, "Empty");
        return empty;
    }
    else if (strcmp(((struct Code *) target)->type, "Array") == 0) {
        struct Array* array = (struct Array*)target;
        if (index->index->size == 1 && strcmp(((struct Code*)index->index->endPointer->data)->type, "Value") == 0) {
            void* indexNum = index->index->endPointer->data;
            if (((struct Value*)indexNum)->valueType == 0x02) {
                struct ShortValue* indexValue = (struct ShortValue*) indexNum;
                dict_add_str(buffer, "result", array->array[indexValue->value]);
                return array->array[indexValue->value];
            }
            else if (((struct Value*)indexNum)->valueType == 0x06) {
                struct IntValue* indexValue = (struct IntValue*) indexNum;
                dict_add_str(buffer, "result", array->array[indexValue->value]);
                return array->array[indexValue->value];
            }
            else if (((struct Value*)indexNum)->valueType == 0x03) {
                struct LongValue* indexValue = (struct LongValue*) indexNum;
                dict_add_str(buffer, "result", array->array[indexValue->value]);
                return array->array[indexValue->value];
            }
        }
        else if (index->index->size == 1 && strcmp(((struct Code*)index->index->endPointer->data)->type, "Period") == 0) {
            struct Period* indexValue = (struct Period*)index->index->endPointer->data;
            int start = ((struct IntValue*)indexValue->start)->value, end = ((struct IntValue*)indexValue->end)->value;
            struct Array* result = custom_malloc(sizeof(struct Array));
            initArray(result, (end - start));
            memcpy(result->array, array->array[start], (end - start));
            dict_add_str(buffer, "result", result);
            return result;
        }
    }
    else if (strcmp(((struct Code*)target)->type, "Value") == 0) {
        struct Value* rawValue = (struct Value*)target;
        if (rawValue->valueType == 0x01) {
            struct StringValue* stringValue = (struct StringValue*)target;
            struct StringValue *result = custom_malloc(sizeof(struct StringValue));
            result->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(result->base.base.type, "Value");
            result->base.valueType = 0x01;
            if (index->index->size == 1 && strcmp(((struct Code*)index->index->endPointer->data)->type, "Value") == 0) {
                struct IntValue* indexValue = (struct IntValue*)index->index->endPointer->data;
                long indexNum = 0;
                if (((struct Value*)indexValue)->valueType == 0x02)
                    indexNum = ((struct ShortValue*)indexValue)->value;
                else if (((struct Value*)indexValue)->valueType == 0x06)
                    indexNum = ((struct IntValue*)indexValue)->value;
                else if (((struct Value*)indexValue)->valueType == 0x03)
                    indexNum = ((struct LongValue*)indexValue)->value;
                result->value = malloc(1 + 1);
                memcpy(result->value, &stringValue->value[indexNum], 1);
                result->value[1] = '\0';
                dict_add_str(buffer, "result", result);
                return result;
            }
            else if (index->index->size == 1 && strcmp(((struct Code*)index->index->endPointer->data)->type, "Period") == 0) {
                struct Period* indexValue = (struct Period*)index->index->endPointer->data;
                long start = 0, end = 0;
                if (((struct Value*)indexValue->start)->valueType == 0x02)
                    start = ((struct ShortValue*)indexValue->start)->value;
                else if (((struct Value*)indexValue->start)->valueType == 0x06)
                    start = ((struct IntValue*)indexValue->start)->value;
                else if (((struct Value*)indexValue->start)->valueType == 0x03)
                    start = ((struct LongValue*)indexValue->start)->value;
                if (((struct Value*)indexValue->end)->valueType == 0x02)
                    end = ((struct ShortValue*)indexValue->end)->value;
                else if (((struct Value*)indexValue->end)->valueType == 0x06)
                    end = ((struct IntValue*)indexValue->end)->value;
                else if (((struct Value*)indexValue->end)->valueType == 0x03)
                    end = ((struct LongValue*)indexValue->end)->value;
                char* resultValue = malloc((end - start + 1) * sizeof(char));
                memcpy(resultValue, &(stringValue->value)[start], (end - start) * sizeof(char));
                resultValue[end - start] = '\0';
                result->value = resultValue;
                dict_add_str(buffer, "result", result);
                return result;
            }
        }
    }
    return NULL;
}

void* resolveRef(struct Dictionary* buffer, void* refRaw) {
    void* objectChain = NULL;
    void* target = NULL;
    struct String* refStr = NULL;
    struct StackDataItem *iterator = dataStack->item;
    if (strcmp(((struct Code*)refRaw)->type, "Reference") == 0) {
        struct Reference* ref = (struct Reference*) refRaw;
        target = dict_get((struct Dictionary *) iterator->data, ref->currentChain->id->value);
        while (target == NULL && iterator->prev != NULL) {
            iterator = iterator->prev;
            target = dict_get((struct Dictionary *) iterator->data, ref->currentChain->id->value);
        }
        if (target != NULL && strcmp(((struct Code *) target)->type, "Object") == 0)
            objectChain = target;
        refStr = custom_malloc(sizeof(struct String));
        initString(refStr, strlen(ref->currentChain->id->value) + 1);
        strcpy(refStr->value, ref->currentChain->id->value);
        dict_delete(buffer, "refStr", true);
        dict_add_str(buffer, "refStr", refStr);
        refRaw = ref->restOfTheChain;
        if (target == NULL) {
            struct Empty* empty = custom_malloc(sizeof(struct Empty));
            empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
            strcpy(empty->base.type, "Empty");
            target = empty;
        }
    }
    else if (strcmp(((struct Code*)refRaw)->type, "Identifier") == 0) {
        struct Identifier* ref = (struct Identifier*) refRaw;
        target = dict_get((struct Dictionary *) iterator->data, ref->id->value);
        while (target == NULL && iterator->prev != NULL) {
            iterator = iterator->prev;
            target = dict_get((struct Dictionary *) iterator->data, ref->id->value);
        }
        if (target != NULL && strcmp(((struct Code *) target)->type, "Object") == 0)
            objectChain = target;
        refStr = custom_malloc(sizeof(struct String));
        initString(refStr, strlen(ref->id->value) + 1);
        strcpy(refStr->value, ref->id->value);
        dict_delete(buffer, "refStr", true);
        dict_add_str(buffer, "refStr", refStr);
        refRaw = NULL;
        if (target == NULL) {
            struct Empty* empty = custom_malloc(sizeof(struct Empty));
            empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
            strcpy(empty->base.type, "Empty");
            target = empty;
        }
    }
    else if (strcmp(((struct Code*)refRaw)->type, "Index") == 0) {
        struct Index* index = (struct Index*) refRaw;
        struct Pair* tempResult = resolveRef(buffer, index->var);
        refStr = ((struct String*)((struct Pair*)tempResult->second)->first);
        dict_add_str(buffer, "refStr", refStr);
        index->var = tempResult->first;
        notifyNewUsage(index, tempResult->first);
        target = resolveIndex(buffer, index);
        refRaw = index->restOfTheChain;
        notifyUsageEnded(tempResult);
        if (target == NULL) {
            struct Empty* empty = custom_malloc(sizeof(struct Empty));
            empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
            strcpy(empty->base.type, "Empty");
            target = empty;
        }
    }
    while (target != NULL && strcmp(((struct Code*)target)->type, "Empty") != 0 && refRaw != NULL) {
        if (strcmp(((struct Code*)refRaw)->type, "Reference") == 0) {
            struct Reference* ref = (struct Reference*) refRaw;
            if (strcmp(((struct Code*)target)->type, "Object") == 0) {
                char* refStrTemp = concat(refStr->value, ".");
                free(refStr->value);
                refStr->value = malloc(strlen(refStrTemp) + 1);
                strcpy(refStr->value, refStrTemp);
                free(refStrTemp);
                refStrTemp = concat(refStr->value, ref->currentChain->id->value);
                free(refStr->value);
                refStr->value = malloc(strlen(refStrTemp) + 1);
                strcpy(refStr->value, refStrTemp);
                free(refStrTemp);
                void* target2 = dict_get(((struct Object*)target)->value, ref->currentChain->id->value);
                if (target2 != NULL)
                    target = target2;
                else {
                    target2 = dict_get(((struct Object*)target)->funcs, ref->currentChain->id->value);
                    if (target2 != NULL)
                        target = target2;
                }
                if (target == NULL) {
                    struct Empty* empty = custom_malloc(sizeof(struct Empty));
                    empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
                    strcpy(empty->base.type, "Empty");
                    target = empty;
                }
                refRaw = ref->restOfTheChain;
            }
            else if (strcmp(((struct Code*)target)->type, "Class") == 0) {
                char* refStrTemp = concat(refStr->value, ".");
                free(refStr->value);
                refStr->value = malloc(strlen(refStrTemp) + 1);
                strcpy(refStr->value, refStrTemp);
                free(refStrTemp);
                refStrTemp = concat(refStr->value, ref->currentChain->id->value);
                free(refStr->value);
                refStr->value = malloc(strlen(refStrTemp) + 1);
                strcpy(refStr->value, refStrTemp);
                free(refStrTemp);
                target = dict_get(((struct Class*)target)->functions, ref->currentChain->id->value);
                refRaw = ref->restOfTheChain;
                if (target == NULL) {
                    struct Empty* empty = custom_malloc(sizeof(struct Empty));
                    empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
                    strcpy(empty->base.type, "Empty");
                    target = empty;
                }
            }
            else if (strcmp(((struct Code*)target)->type, "Function") == 0) {
                char* chainName = malloc(strlen(ref->currentChain->id->value) + 1);
                strcpy(chainName, ref->currentChain->id->value);
                char* refStrTemp = concat(refStr->value, ".");
                free(refStr->value);
                refStr->value = malloc(strlen(refStrTemp) + 1);
                strcpy(refStr->value, refStrTemp);
                refStrTemp = concat(refStr->value, chainName);
                free(refStr->value);
                refStr->value = malloc(strlen(refStrTemp) + 1);
                strcpy(refStr->value, refStrTemp);
                free(chainName);
                break;
            }
        }
        else if (strcmp(((struct Code*)refRaw)->type, "Index") == 0) {
            struct Index* index = (struct Index*) refRaw;
            struct Pair* tempResult = resolveRef(buffer, index->var);
            refStr = ((struct Pair*)tempResult->second)->first;
            dict_delete(buffer, "refStr", true);
            dict_add_str(buffer, "refStr", refStr);
            index->var = tempResult->first;
            notifyNewUsage(index, index->var);
            target = resolveIndex(buffer, index);
            refRaw = index->restOfTheChain;
            if (target == NULL) {
                struct Empty* empty = custom_malloc(sizeof(struct Empty));
                empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
                strcpy(empty->base.type, "Empty");
                target = empty;
            }
        }
    }
    struct Pair* result = custom_malloc(sizeof(struct Pair));
    result->base.type = malloc((strlen("Pair") + 1) * sizeof(char));
    strcpy(result->base.type, "Pair");
    result->first = target;
    struct Pair* result2 = custom_malloc(sizeof(struct Pair));
    result2->base.type = malloc((strlen("Pair") + 1) * sizeof(char));
    strcpy(result2->base.type, "Pair");
    result2->first = refStr;
    result2->second = objectChain;
    result->second = result2;
    return result;
}

int convertBytesToInt(const char bytes[]) {
    return (bytes[3] & 0xff) | ((bytes[2] & 0xff) << 8) | ((bytes[1] & 0xff) << 16) | ((bytes[0] & 0xff) << 24);
}

short convertBytesToShort(const char bytes[]) {
    return (short) ((bytes[1] & 0xff) | ((bytes[0] & 0xff) << 8));
}

unsigned long calculateBytes(int investigateId, char* c, unsigned long p) {
    if (c[p] == 0x4d) {
        p++;
        struct Empty* empty = custom_malloc(sizeof(struct Empty));
        empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
        strcpy(empty->base.type, "Empty");
        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", empty);
        return p;
    }
    else if (c[p] == 0x4e) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        char countBytes[4];
        for (int i = 0; i < (int)sizeof(countBytes); i++)
            countBytes[i] = c[p + i];
        p += (int) sizeof(countBytes);
        int count = convertBytesToInt(countBytes);
        struct Object* obj = custom_malloc(sizeof(struct Object));
        dict_add_str(buffer, "obj", obj);
        obj->base.type = malloc((strlen("Object") + 1) * sizeof(char));
        strcpy(obj->base.type, "Object");
        obj->funcs = NULL;
        obj->value = dict_new();
        notifyNewUsage(obj, obj->value);
        for (int counter = 0; counter < count; counter++) {
            if (c[p] == 0x01) {
                p++;
                char keyLengthBytes[4];
                for (int i = 0; i < (int)sizeof(keyLengthBytes); i++)
                    keyLengthBytes[i] = c[p + i];
                p += (int) sizeof(keyLengthBytes);
                int keyLength = convertBytesToInt(keyLengthBytes);
                char* key = malloc(keyLength);
                memcpy(key, &c[p], keyLength);
                p += keyLength;
                expStack->push(expStack, dict_new());
                p = calculateBytes(investigateId, c, p);
                void* exp = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                dict_add_str(obj->value, key, exp);
                notifyUsageEnded(expStack->pop(expStack));
                free(key);
            }
        }
        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", obj);
        buffer_free(dataStack, buffer);
        return p;
    }
    else if (c[p] == 0x4f) {
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(investigateId, c, p);
            void* dataToBeNot = dict_get((struct Dictionary *) expStack->top(expStack), "value");
            void* oldDict = expStack->pop(expStack);
            dict_add_str((struct Dictionary *) expStack->top(expStack), "value", not(dataToBeNot));
            notifyUsageEnded(oldDict);
            return p;
        }
    }
    else if (c[p] == 0x4b) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        struct Array* array = custom_malloc(sizeof(struct Array));
        if (c[p] == 0x01) {
            p++;
            char itemsCountBytes[4];
            for (int i = 0; i < (int) sizeof(itemsCountBytes); i++)
                itemsCountBytes[i] = c[p + i];
            p += (int) sizeof(itemsCountBytes);
            int itemsCount = convertBytesToInt(itemsCountBytes);
            initArray(array, itemsCount);
            dict_add_str(buffer, "array", array);
            for (int i = 0; i < itemsCount; i++) {
                if (c[p] == 0x02) {
                    p++;
                    expStack->push(expStack, dict_new());
                    p = calculateBytes(investigateId, c, p);
                    void* value = dict_get((struct Dictionary*)expStack->top(expStack), "value");
                    insertArray(array, value);
                    notifyUsageEnded(expStack->pop(expStack));
                }
            }
            dict_add_str((struct Dictionary *) expStack->top(expStack), "value", array);
            buffer_free(dataStack, buffer);
            return p;
        }
    }
    else if (c[p] == 0x7f) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        struct Reference *ref = custom_malloc(sizeof(struct Reference));
        dict_add_str(buffer, "ref", ref);
        ref->base.type = malloc((strlen("Reference") + 1) * sizeof(char));
        strcpy(ref->base.type, "Reference");
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(0, c, p);
            struct Identifier *id = (struct Identifier *) dict_get((struct Dictionary *) expStack->top(expStack), "value");
            ref->currentChain = id;
            notifyNewUsage(ref, id);
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(0, c, p);
                void* restOfTheChain = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                ref->restOfTheChain = restOfTheChain;
                notifyNewUsage(ref, restOfTheChain);
                notifyUsageEnded(expStack->pop(expStack));
            }
            else {
                ref->restOfTheChain = NULL;
            }
            if (c[p] == 0x6d) {
                p++;
                if (investigateId == 1) {
                    struct Pair* result = resolveRef(buffer, ref);
                    dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result->first);
                    dict_add_str((struct Dictionary *) expStack->top(expStack), "value2",
                                 ((struct Pair *) result->second)->first);
                    dict_add_str((struct Dictionary *) expStack->top(expStack), "value3",
                                 ((struct Pair *) result->second)->second);
                    notifyUsageEnded(result);
                }
                else {
                    dict_add_str((struct Dictionary *) expStack->top(expStack), "value", ref);
                }
                buffer_free(dataStack, buffer);
                return p;
            }
        }
    }
    else if (c[p] == 0x6c) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        struct Index* index = custom_malloc(sizeof(struct Index));
        dict_add_str(buffer, "index", index);
        index->base.type = malloc((strlen("Index") + 1) * sizeof(char));
        strcpy(index->base.type, "Index");
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(0, c, p);
            void* var = dict_get((struct Dictionary*)expStack->top(expStack), "value");
            index->var = var;
            notifyNewUsage(index, var);
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                char indicesCountBytes[4];
                for (int i = 0; i < (int) sizeof(indicesCountBytes); i++)
                    indicesCountBytes[i] = c[p + i];
                p += (int) sizeof(indicesCountBytes);
                int indicesCount = convertBytesToInt(indicesCountBytes);
                struct List* indicesList = custom_malloc(sizeof(struct List));
                initList(indicesList);
                index->index = indicesList;
                notifyNewUsage(index, indicesList);
                for (int i = 0; i < indicesCount; i++) {
                    if (c[p] == 0x03) {
                        p++;
                        expStack->push(expStack, dict_new());
                        p = calculateBytes(investigateId, c, p);
                        void *indexItem = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                        indicesList->append(indicesList, indexItem);
                        notifyUsageEnded(expStack->pop(expStack));
                    }
                }
                if (c[p] == 0x04) {
                    p++;
                    expStack->push(expStack, dict_new());
                    p = calculateBytes(investigateId, c, p);
                    void* restOfChains = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                    index->restOfTheChain = restOfChains;
                    notifyNewUsage(index, restOfChains);
                    notifyUsageEnded(expStack->pop(expStack));
                }
                else {
                    index->restOfTheChain = NULL;
                }
                if (c[p] == 0x6b) {
                    p++;
                    if (investigateId == 1) {
                        struct Pair* result = resolveRef(buffer, index);
                        dict_add_str((struct Dictionary *) expStack->top(expStack), "value",
                                     result->first);
                        dict_add_str((struct Dictionary *) expStack->top(expStack), "value2",
                                     ((struct Pair *) result->second)->first);
                        dict_add_str((struct Dictionary *) expStack->top(expStack), "value3",
                                     ((struct Pair *) result->second)->second);
                        notifyUsageEnded(result);
                        notifyParentDestroyed(index, var);
                        notifyUsageEnded(var);
                    } else {
                        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", index);
                    }
                    buffer_free(dataStack, buffer);
                    return p;
                }
            }
        }
    }
    else if (c[p] == 0x6a) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        struct Period* period = custom_malloc(sizeof(struct Period));
        dict_add_str(buffer, "period", period);
        period->base.type = malloc((strlen("Period") + 1) * sizeof(char));
        strcpy(period->base.type, "Period");
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(investigateId, c, p);
            period->start = dict_get((struct Dictionary*)expStack->top(expStack), "value");
            notifyNewUsage(period, period->start);
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(investigateId, c, p);
                period->end = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                notifyNewUsage(period, period->end);
                notifyUsageEnded(expStack->pop(expStack));
                if (c[p] == 0x69) {
                    p++;
                    dict_add_str((struct Dictionary *) expStack->top(expStack), "value", period);
                    buffer_free(dataStack, buffer);
                    return p;
                }
            }
        }
    }
    else if (c[p] == 0x55) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* target = dict_get((struct Dictionary*)expStack->top(expStack), "value");
            struct String* refStr = (struct String*)dict_get((struct Dictionary*)expStack->top(expStack), "value2");
            struct Object* thisObj = dict_get((struct Dictionary*)expStack->top(expStack), "value3");
            dict_add_str(buffer, "target", target);
            dict_add_str(buffer, "refStr", refStr);
            dict_add_str(buffer, "thisObj", thisObj);
            notifyUsageEnded((struct Dictionary*)expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                char entriesCountBytes[4];
                for (int index = 0; index < 4; index++)
                    entriesCountBytes[index] = c[p + index];
                p += 4;
                int entriesCount = convertBytesToInt(entriesCountBytes);
                struct Dictionary* entriesDict = dict_new();
                dict_add_str(buffer, "entries", entriesDict);
                for (int counter = 0; counter < entriesCount; counter++) {
                    if (c[p] == 0x03) {
                        p++;
                        char keyLengthBytes[4];
                        for (int index = 0; index < 4; index++)
                            keyLengthBytes[index] = c[p + index];
                        p += 4;
                        int keyLength = convertBytesToInt(keyLengthBytes);
                        struct String* keyStr = custom_malloc(sizeof(struct String));
                        initString(keyStr, keyLength);
                        memcpy(keyStr->value, &c[p], keyLength);
                        p += keyLength;
                        p += 4;
                        expStack->push(expStack, dict_new());
                        p = calculateBytes(1, c, p);
                        void* data = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                        dict_add_str(entriesDict, keyStr->value, data);
                        notifyUsageEnded(expStack->pop(expStack));
                        notifyUsageEnded(keyStr);
                    }
                }
                if (target != NULL && strcmp(((struct Code*)target)->type, "Empty") != 0) {
                    struct Function* func = (struct Function*) target;
                    if (thisObj != NULL) dict_add_str(entriesDict, "this", thisObj);
                    dict_add_str((struct Dictionary *) expStack->top(expStack), "value",
                                 executeIntern(true, func->codes->data, func->loc, entriesDict));
                } else {
                    dict_add_str((struct Dictionary *) expStack->top(expStack), "value",
                                 routeAndResolve(refStr, entriesDict));
                }
                buffer_free(dataStack, buffer);
                return p;
            }
        }
    } else if (c[p] == 0x57) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* target = dict_get((struct Dictionary*)expStack->top(expStack), "value");
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                char entriesCountBytes[4];
                for (int index = 0; index < 4; index++)
                    entriesCountBytes[index] = c[p + index];
                p += 4;
                int entriesCount = convertBytesToInt(entriesCountBytes);
                struct Dictionary* entriesDict = dict_new();
                dict_add_str(buffer, "entries", entriesDict);
                for (int counter = 0; counter < entriesCount; counter++) {
                    if (c[p] == 0x03) {
                        p++;
                        char keyLengthBytes[4];
                        for (int index = 0; index < 4; index++)
                            keyLengthBytes[index] = c[p + index];
                        p += 4;
                        int keyLength = convertBytesToInt(keyLengthBytes);
                        char *key = malloc(keyLength);
                        memcpy(key, &c[p], keyLength);
                        p += keyLength;
                        p += 4;
                        expStack->push(expStack, dict_new());
                        p = calculateBytes(1, c, p);
                        void* data = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                        dict_add_str(entriesDict, key, data);
                        notifyUsageEnded(expStack->pop(expStack));
                        free(key);
                    }
                }
                if (target != NULL) {
                    struct Class* classObj = (struct Class*) target;
                    struct Object* object = custom_malloc(sizeof(struct Object));
                    dict_add_str(buffer, "obj", object);
                    object->base.type = malloc((strlen("Object") + 1) * sizeof(char));
                    strcpy(object->base.type, "Object");
                    object->value = dict_new();
                    notifyNewUsage(object, object->value);
                    struct ListDataItem* classPropsIterator = classObj->properties->startPointer;
                    while (classPropsIterator != NULL) {
                        struct Prop* prop = (struct Prop*) classPropsIterator->data;
                        expStack->push(expStack, dict_new());
                        calculateBytes(1, prop->value->data, 0);
                        void* propValue = dict_get((struct Dictionary*)expStack->top(expStack), "value");
                        dict_add_str(object->value, prop->id->id->value, propValue);
                        notifyUsageEnded(expStack->pop(expStack));
                        classPropsIterator = classPropsIterator->next;
                    }
                    struct ListDataItem* iterator = classObj->constructor->params->endPointer;
                    while (iterator != NULL) {
                        if (dict_get(entriesDict, ((struct Identifier*)iterator->data)->id->value) == NULL) {
                            struct Empty* empty = custom_malloc(sizeof(struct Empty));
                            empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
                            strcpy(empty->base.type, "Empty");
                            dict_add_str(entriesDict, ((struct Identifier *) iterator->data)->id->value, empty);
                        }
                        iterator = iterator->prev;
                    }
                    object->funcs = classObj->functions;
                    dict_add_str((struct Dictionary *) expStack->top(expStack), "value", object);
                    if (classObj->constructor != NULL) {
                        dict_add_str(entriesDict, "this", object);
                        executeIntern(true, classObj->constructor->body->data, classObj->constructor->loc, entriesDict);
                    }
                }
                buffer_free(dataStack, buffer);
                return p;
            }
        }
    } else if (c[p] == 0x71) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            dict_add_str(buffer, "value1", value1);
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                dict_add_str(buffer, "value2", value2);
                notifyUsageEnded(expStack->pop(expStack));
                void* result = sum(value1, value2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                buffer_free(dataStack, buffer);
                return p;
            }
        }
    } else if (c[p] == 0x72) {
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            
            void* trash1 = expStack->pop(expStack);
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                
                
                void* trash2 = expStack->pop(expStack);
                void* result = subtract(value1, value2);
                notifyUsageEnded(trash1);
                notifyUsageEnded(trash2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                return p;
            }
        }
    } else if (c[p] == 0x73) {
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            
            void* trash1 = expStack->pop(expStack);
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                
                
                void* trash2 = expStack->pop(expStack);
                void* result = multiply(value1, value2);
                notifyUsageEnded(trash1);
                notifyUsageEnded(trash2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                return p;
            }
        }
    } else if (c[p] == 0x74) {
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            
            void* trash1 = expStack->pop(expStack);
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                
                
                void* trash2 = expStack->pop(expStack);
                void* result = divide(value1, value2);
                notifyUsageEnded(trash1);
                notifyUsageEnded(trash2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                return p;
            }
        }
    } else if (c[p] == 0x75) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            dict_add_str(buffer, "value1", value1);
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                dict_add_str(buffer, "value2", value2);
                notifyUsageEnded(expStack->pop(expStack));
                void* result = mod(value1, value2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                buffer_free(dataStack, buffer);
                return p;
            }
        }
    } else if (c[p] == 0x76) {
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            
            void* trash1 = expStack->pop(expStack);
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                
                void* trash2 = expStack->pop(expStack);
                void* result = power(value1, value2);
                notifyUsageEnded(trash1);
                notifyUsageEnded(trash2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                return p;
            }
        }
    } else if (c[p] == 0x77) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            dict_add_str(buffer, "value1", value1);
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                dict_add_str(buffer, "value2", value2);
                notifyUsageEnded(expStack->pop(expStack));
                void* result = and(value1, value2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                buffer_free(dataStack, buffer);
                return p;
            }
        }
    } else if (c[p] == 0x78) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            dict_add_str(buffer, "value1", value1);
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                dict_add_str(buffer, "value2", value2);
                notifyUsageEnded(expStack->pop(expStack));
                void* result = or(value1, value2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                buffer_free(dataStack, buffer);
                return p;
            }
        }
    } else if (c[p] == 0x79) {
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            
            void* trash1 = expStack->pop(expStack);
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                
                
                void* trash2 = expStack->pop(expStack);
                void* result = equal(value1, value2);
                notifyUsageEnded(trash1);
                notifyUsageEnded(trash2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                return p;
            }
        }
    } else if (c[p] == 0x7a) {
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            
            void* trash1 = expStack->pop(expStack);
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                
                
                void* trash2 = expStack->pop(expStack);
                void* result = gt(value1, value2);
                notifyUsageEnded(trash1);
                notifyUsageEnded(trash2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                return p;
            }
        }
    } else if (c[p] == 0x7b) {
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            
            void* trash1 = expStack->pop(expStack);
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                
                
                void* trash2 = expStack->pop(expStack);
                void* result = ge(value1, value2);
                notifyUsageEnded(trash1);
                notifyUsageEnded(trash2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                return p;
            }
        }
    } else if (c[p] == 0x7c) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            dict_add_str(buffer, "value1", value1);
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                dict_add_str(buffer, "value2", value2);
                notifyUsageEnded(expStack->pop(expStack));
                void* result = ne(value1, value2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                buffer_free(dataStack, buffer);
                return p;
            }
        }
    } else if (c[p] == 0x7d) {
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            
            void* trash1 = expStack->pop(expStack);
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                
                
                void* trash2 = expStack->pop(expStack);
                void* result = le(value1, value2);
                notifyUsageEnded(trash1);
                notifyUsageEnded(trash2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                return p;
            }
        }
    } else if (c[p] == 0x7e) {
        struct Dictionary* buffer = buffer_new(dataStack);
        p++;
        if (c[p] == 0x01) {
            p++;
            expStack->push(expStack, dict_new());
            p = calculateBytes(1, c, p);
            void* value1 = dict_get((struct Dictionary*) expStack->top(expStack), "value");
            dict_add_str(buffer, "value1", value1);
            notifyUsageEnded(expStack->pop(expStack));
            if (c[p] == 0x02) {
                p++;
                expStack->push(expStack, dict_new());
                p = calculateBytes(1, c, p);
                void* value2 = dict_get((struct Dictionary*) expStack->top(expStack),"value");
                dict_add_str(buffer, "value2", value2);
                notifyUsageEnded(expStack->pop(expStack));
                void* result = lt(value1, value2);
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", result);
                buffer_free(dataStack, buffer);
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
        struct String* str = custom_malloc(sizeof(struct String));
        initString(str, idNameLength);
        memcpy(str->value, &c[p], idNameLength);
        p += idNameLength;
        struct Identifier* id = custom_malloc(sizeof(struct Identifier));
        id->base.type = malloc((strlen("Identifier") + 1) * sizeof(char));
        strcpy(id->base.type, "Identifier");
        id->id = str;
        notifyNewUsage(id, str);
        if (investigateId == 1) {
            struct StackDataItem *iterator = dataStack->item;
            char* idName = malloc(strlen(id->id->value) + 1);
            strcpy(idName, id->id->value);
            void *value = dict_get((struct Dictionary *) iterator->data, idName);
            while (value == NULL && iterator->prev != NULL) {
                iterator = iterator->prev;
                value = dict_get((struct Dictionary *) iterator->data, idName);
            }
            if (value == NULL)
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", id);
            else
                dict_add_str((struct Dictionary *) expStack->top(expStack), "value", value);
        }
        else
            dict_add_str((struct Dictionary *) expStack->top(expStack), "value", id);
        return p;
    } else if (c[p] == 0x62) {
        p++;
        char valueLengthArr[4];
        for (int index = 0; index < (int)sizeof(valueLengthArr); index++)
            valueLengthArr[index] = c[p + index];
        p += (int)sizeof(valueLengthArr);
        int valueLength = convertBytesToInt(valueLengthArr);
        struct StringValue* val = custom_malloc(sizeof(struct StringValue));
        val->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(val->base.base.type, "Value");
        val->base.valueType = 0x01;
        val->value = malloc(valueLength);
        memcpy(val->value, &c[p], valueLength);
        p += valueLength;
        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", val);
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
        struct DoubleValue* val = custom_malloc(sizeof(struct DoubleValue));
        val->value = value;
        val->base.valueType = 0x05;
        val->base.base.type = malloc(sizeof(char) * (strlen("Value") + 1));
        strcpy(val->base.base.type, "Value");
        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", val);
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
        struct FloatValue* val = custom_malloc(sizeof(struct FloatValue));
        val->value = value;
        val->base.valueType = 0x04;
        val->base.base.type = malloc(sizeof(char) * (strlen("Value") + 1));
        strcpy(val->base.base.type, "Value");
        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", val);
        return p;
    } else if (c[p] == 0x65) {
        p++;
        char valueArr[2];
        for (int index = 0; index < (int)sizeof(valueArr); index++)
            valueArr[index] = c[p + index];
        p += (int)sizeof(valueArr);
        struct ShortValue* val = custom_malloc(sizeof(struct ShortValue));
        val->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(val->base.base.type, "Value");
        val->base.valueType = 0x02;
        val->value = convertBytesToShort(valueArr);
        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", val);
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
        struct IntValue* val = custom_malloc(sizeof(struct IntValue));
        val->value = value;
        val->base.valueType = 0x06;
        val->base.base.type = malloc(sizeof(char) * (strlen("Value") + 1));
        strcpy(val->base.base.type, "Value");
        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", val);
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
        struct LongValue* val = custom_malloc(sizeof(struct LongValue));
        val->value = value;
        val->base.valueType = 0x03;
        val->base.base.type = malloc(sizeof(char) * (strlen("Value") + 1));
        strcpy(val->base.base.type, "Value");
        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", val);
        return p;
    } else if (c[p] == 0x68) {
        p++;
        char value = c[p];
        p++;
        bool finalValue = false;
        if ((int)value == 1) finalValue = true;
        struct BoolValue* val = custom_malloc(sizeof(struct BoolValue));
        val->value = finalValue;
        val->base.valueType = 0x07;
        val->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
        strcpy(val->base.base.type, "Value");
        dict_add_str((struct Dictionary *) expStack->top(expStack), "value", val);
        return p;
    }
}

void calculate(int investigateId) {
    pointer = calculateBytes(investigateId, code, pointer);
}

void* ride() {

    while (codeLength > pointer) {

        if (code[pointer] == 0x6e) break;
        if (code[pointer] == 0x4c) {
            pointer++;
            return "break";
        }
        else if (code[pointer] == 0x51) {
            pointer++;
            struct Function* function = custom_malloc(sizeof(struct Function));
            function->base.type = malloc((strlen("Function") + 1) * sizeof(char));
            strcpy(function->base.type, "Function");
            if (code[pointer] == 0x01) {
                pointer++;
                char funcNameLengthBytes[4];
                for (int index = 0; index < (int)sizeof(funcNameLengthBytes); index++)
                    funcNameLengthBytes[index] = code[pointer + index];
                pointer += (int)sizeof(funcNameLengthBytes);
                int funcNameLength = convertBytesToInt(funcNameLengthBytes);
                char* funcNameStr = malloc(funcNameLength);
                memcpy(funcNameStr, &code[pointer], funcNameLength);
                pointer += funcNameLength;
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
                            struct Identifier* id = custom_malloc(sizeof(struct Identifier));
                            id->base.type = malloc((strlen("Identifier") + 1) * sizeof(char));
                            strcpy(id->base.type, "Identifier");
                            id->id = custom_malloc(sizeof(struct String));
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
                                    struct String* str = custom_malloc(sizeof(struct String));
                                    ((struct String*)str)->base.type = malloc((strlen("String") + 1) * sizeof(char));
                                    strcpy(((struct String*)str)->base.type, "String");
                                    str->value = funcNameStr;
                                    function->funcName = str;
                                    notifyNewUsage(function, str);
                                    function->params = identifiers;
                                    notifyNewUsage(function, identifiers);
                                    struct CodeBlock* cb = custom_malloc(sizeof(struct CodeBlock));
                                    cb->base.type = malloc((strlen("CodeBlock") + 1) * sizeof(char));
                                    strcpy(cb->base.type, "CodeBlock");
                                    cb->data = malloc(jump);
                                    memcpy(cb->data, &body[0], jump);
                                    function->codes = cb;
                                    notifyNewUsage(function, cb);
                                    function->loc = jump;
                                    dict_add_str((struct Dictionary *) dataStack->top(dataStack), funcNameStr,
                                                 function);
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (code[pointer] == 0x57) {
            struct Dictionary* buffer = buffer_new(dataStack);
            pointer++;
            if (code[pointer] == 0x01) {
                pointer++;
                expStack->push(expStack, dict_new());
                calculate(1);
                void* target = dict_get((struct Dictionary*)expStack->top(expStack), "value");
                dict_add_str(buffer, "target", target);
                notifyUsageEnded(expStack->pop(expStack));
                if (code[pointer] == 0x02) {
                    pointer++;
                    char entriesCountBytes[4];
                    for (int index = 0; index < 4; index++)
                        entriesCountBytes[index] = code[pointer + index];
                    pointer += 4;
                    int entriesCount = convertBytesToInt(entriesCountBytes);
                    struct Dictionary* entriesDict = dict_new();
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
                            pointer += 4;
                            expStack->push(expStack, dict_new());
                            calculate(1);
                            void* data = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                            dict_add_str(entriesDict, key, data);
                            notifyUsageEnded(expStack->pop(expStack));
                            free(key);
                        }
                    }
                    if (target != NULL) {
                        struct Class* classObj = (struct Class*) target;
                        struct Object* object = custom_malloc(sizeof(struct Object));
                        dict_add_str(buffer, "obj", object);
                        object->base.type = malloc((strlen("Object") + 1) * sizeof(char));
                        strcpy(object->base.type, "Object");
                        object->value = dict_new();
                        notifyNewUsage(object, object->value);
                        struct ListDataItem* classPropsIterator = classObj->properties->startPointer;
                        while (classPropsIterator != NULL) {
                            struct Prop* prop = (struct Prop*) classPropsIterator->data;
                            expStack->push(expStack, dict_new());
                            calculateBytes(1, prop->value->data, 0);
                            void* propValue = dict_get((struct Dictionary*)expStack->top(expStack), "value");
                            dict_add_str(object->value, prop->id->id->value, propValue);
                            notifyUsageEnded(expStack->pop(expStack));
                            classPropsIterator = classPropsIterator->next;
                        }
                        struct ListDataItem* iterator = classObj->constructor->params->endPointer;
                        while (iterator != NULL) {
                            if (dict_get(entriesDict, ((struct Identifier*)iterator->data)->id->value) == NULL) {
                                struct Empty* empty = custom_malloc(sizeof(struct Empty));
                                empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
                                strcpy(empty->base.type, "Empty");
                                dict_add_str(entriesDict, ((struct Identifier *) iterator->data)->id->value, empty);
                            }
                            iterator = iterator->prev;
                        }
                        object->funcs = classObj->functions;
                        if (classObj->constructor != NULL) {
                            dict_add_str(entriesDict, "this", object);
                            executeIntern(true, classObj->constructor->body->data, classObj->constructor->loc, entriesDict);
                        }
                    }
                    notifyUsageEnded(entriesDict);
                }
            }
            buffer_free(dataStack, buffer);
        }
        else if (code[pointer] == 0x58) {
            struct Dictionary* buffer = buffer_new(dataStack);
            pointer++;
            struct Class* classObj = custom_malloc(sizeof(struct Class));
            dict_add_str(buffer, "class", classObj);
            classObj->base.type = malloc((strlen("Class") + 1) * sizeof(char));
            strcpy(classObj->base.type, "Class");
            if (code[pointer] == 0x01) {
                pointer++;
                char classNameLengthBytes[4];
                for (int index = 0; index < (int)sizeof(classNameLengthBytes); index++)
                    classNameLengthBytes[index] = code[pointer + index];
                pointer += (int)sizeof(classNameLengthBytes);
                int classNameLength = convertBytesToInt(classNameLengthBytes);
                struct String* classNameStr = custom_malloc(sizeof(struct String));
                notifyNewUsage(classObj, classNameStr);
                initString(classNameStr, classNameLength);
                memcpy(classNameStr->value, &code[pointer], classNameLength);
                pointer += classNameLength;
                if (code[pointer] == 0x02) {
                    pointer++;
                    char inheritanceCountBytes[4];
                    for (int index = 0; index < (int)sizeof(inheritanceCountBytes); index++)
                        inheritanceCountBytes[index] = code[pointer + index];
                    pointer += (int)sizeof(inheritanceCountBytes);
                    int inheritanceCount = convertBytesToInt(inheritanceCountBytes);
                    struct List* inheritance = custom_malloc(sizeof(struct List));
                    notifyNewUsage(classObj, inheritance);
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
                        struct Identifier* id = custom_malloc(sizeof(struct Identifier));
                        struct String* str = custom_malloc(sizeof(struct String));
                        initString(str, strlen(idName) + 1);
                        strcpy(str->value, idName);
                        id->id = str;
                        notifyNewUsage(id, str);
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
                        notifyNewUsage(classObj, behavior);
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
                            struct Identifier* id = custom_malloc(sizeof(struct Identifier));
                            struct String* str = custom_malloc(sizeof(struct String));
                            initString(str, strlen(idName) + 1);
                            strcpy(str->value, idName);
                            id->id = str;
                            notifyNewUsage(id, str);
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
                            notifyNewUsage(classObj, props);
                            initList(props);
                            for (int i = 0; i < propCount; i++) {
                                char identifierNameLengthBytes[4];
                                for (int index = 0; index < (int)sizeof(identifierNameLengthBytes); index++)
                                    identifierNameLengthBytes[index] = code[pointer + index];
                                pointer += (int)sizeof(identifierNameLengthBytes);
                                int idNameLength = convertBytesToInt(identifierNameLengthBytes);
                                struct Identifier* id = custom_malloc(sizeof(struct Identifier));
                                id->base.type = malloc((strlen("Identifier") + 1) * sizeof(char));
                                strcpy(id->base.type, "Identifier");
                                struct String* str = custom_malloc(sizeof(struct String));
                                str->base.type = malloc((strlen("String") + 1) * sizeof(char));
                                strcpy(str->base.type, "String");
                                str->value = malloc((idNameLength) * sizeof(char));
                                memcpy(str->value, &code[pointer], idNameLength);
                                pointer += idNameLength;
                                id->id = str;
                                notifyNewUsage(id, str);
                                char valueLengthBytes[4];
                                for (int index = 0; index < (int)sizeof(valueLengthBytes); index++)
                                    valueLengthBytes[index] = code[pointer + index];
                                pointer += (int)sizeof(valueLengthBytes);
                                unsigned long valueLength = convertBytesToInt(valueLengthBytes);
                                struct CodeBlock* cb = custom_malloc(sizeof(struct CodeBlock));
                                cb->base.type = malloc((strlen("CodeBlock") + 1) * sizeof(char));
                                strcpy(cb->base.type, "CodeBlock");
                                char* value = malloc(valueLength);
                                memcpy(value, &code[pointer], valueLength);
                                cb->data = value;
                                pointer += valueLength;
                                struct Prop* prop = custom_malloc(sizeof(struct Prop));
                                prop->base.type = malloc((strlen("Prop") + 1) * sizeof(char));
                                strcpy(prop->base.type, "Prop");
                                prop->id = id;
                                notifyNewUsage(prop, id);
                                prop->value = cb;
                                notifyNewUsage(prop, cb);
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
                                struct Dictionary* funcs = dict_new();
                                notifyNewUsage(classObj, funcs);
                                for (int i = 0; i < funcCount; i++) {
                                    if (code[pointer] == 0x51) {
                                        pointer++;
                                        struct Function* function = custom_malloc(sizeof(struct Function));
                                        function->base.type = malloc((strlen("Function") + 1) * sizeof(char));
                                        strcpy(function->base.type, "Function");
                                        if (code[pointer] == 0x01) {
                                            pointer++;
                                            char funcNameLengthBytes[4];
                                            for (int index = 0; index < (int)sizeof(funcNameLengthBytes); index++)
                                                funcNameLengthBytes[index] = code[pointer + index];
                                            pointer += (int)sizeof(funcNameLengthBytes);
                                            int funcNameLength = convertBytesToInt(funcNameLengthBytes);
                                            struct String* str = custom_malloc(sizeof(struct String));
                                            str->base.type = malloc((strlen("String") + 1) * sizeof(char));
                                            strcpy(str->base.type, "String");
                                            str->value = malloc(funcNameLength * sizeof(char));
                                            memcpy(str->value, &code[pointer], funcNameLength);
                                            function->funcName = str;
                                            notifyNewUsage(function, str);
                                            pointer += funcNameLength;
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
                                                        struct Identifier* id = custom_malloc(sizeof(struct Identifier));
                                                        id->base.type = malloc((strlen("Identifier") + 1) * sizeof(char));
                                                        strcpy(id->base.type, "Identifier");
                                                        id->id = custom_malloc(sizeof(struct String));
                                                        initString(id->id, paramLength);
                                                        memcpy(id->id->value, &code[pointer], paramLength);
                                                        pointer += paramLength;
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
                                                            struct CodeBlock* cb = custom_malloc(sizeof(struct CodeBlock));
                                                            cb->base.type = malloc((strlen("CodeBlock") + 1) * sizeof(char));
                                                            strcpy(cb->base.type, "CodeBlock");
                                                            cb->data = body;
                                                            pointer += jump;
                                                            if (code[pointer] == 0x6e) {
                                                                pointer++;
                                                                function->params = identifiers;
                                                                notifyNewUsage(function, identifiers);
                                                                function->codes = cb;
                                                                notifyNewUsage(function, cb);
                                                                function->loc = jump;
                                                                dict_add_str(funcs, str->value, function);
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                if (code[pointer] == 0x05) {
                                    struct Constructor* constructor = custom_malloc(sizeof(struct Constructor));
                                    constructor->base.type = malloc((strlen("Constructor") + 1) * sizeof(char));
                                    strcpy(constructor->base.type, "Constructor");
                                    notifyNewUsage(classObj, constructor);
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
                                        struct Identifier* id = custom_malloc(sizeof(struct Identifier));
                                        id->base.type = malloc((strlen("Identifier") + 1) * sizeof(char));
                                        strcpy(id->base.type, "Identifier");
                                        struct String* str = custom_malloc(sizeof(struct String));
                                        initString(str, idNameLength);
                                        strcpy(str->value, idNameBytes);
                                        id->id = str;
                                        notifyNewUsage(id, str);
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
                                            struct CodeBlock* cb = custom_malloc(sizeof(struct CodeBlock));
                                            cb->base.type = malloc((strlen("CodeBlock") + 1) * sizeof(char));
                                            strcpy(cb->base.type, "CodeBlock");
                                            cb->data = malloc(bodyLength);
                                            memcpy(cb->data, &code[pointer], bodyLength);
                                            pointer += bodyLength;
                                            constructor->loc = bodyLength;
                                            if (code[pointer] == 0x6e) {
                                                pointer++;
                                                constructor->body = cb;
                                                notifyNewUsage(constructor, cb);
                                            }
                                        }
                                    }
                                    classObj->inheritance = inheritance;
                                    classObj->behavior = behavior;
                                    classObj->properties = props;
                                    classObj->functions = funcs;
                                    classObj->className = classNameStr;
                                    classObj->constructor = constructor;
                                    dict_add_str((struct Dictionary *) dataStack->top(dataStack), classNameStr->value,
                                                 classObj);
                                }
                            }
                        }
                    }
                }
            }
            buffer_free(dataStack, buffer);
        }
        else if (code[pointer] == 0x52) {
            struct Dictionary* buffer = buffer_new(dataStack);
            pointer++;
            bool matched = false;
            if (code[pointer] == 0x01) {
                pointer++;
                expStack->push(expStack, dict_new());
                calculate(1);
                struct BoolValue* condition = (struct BoolValue*)dict_get((struct Dictionary*)expStack->top(expStack), "value");
                dict_add_str(buffer, "condition", condition);
                notifyUsageEnded(expStack->pop(expStack));
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
                            void* retVal = executeIntern(true, body, jump, NULL);
                            if (retVal != NULL && strcmp(((char*)retVal), "break") == 0) {
                                notifyUsageEnded(condition);
                                free(body);
                                return retVal;
                            } else if (retVal != NULL && strcmp(((char*)retVal), "break") != 0) {
                                notifyUsageEnded(condition);
                                free(body);
                                return retVal;
                            }
                            notifyUsageEnded(retVal);
                        }
                        free(body);
                        notifyUsageEnded(condition);
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
                                            expStack->push(expStack, dict_new());
                                            calculate(1);
                                            struct BoolValue* elseCondition = (struct BoolValue*)dict_get((struct Dictionary*)expStack->top(expStack), "value");
                                            char str[(int)((ceil(log10(elseCount))+1)*sizeof(char))];
                                            sprintf(str, "%d", elseCounter);
                                            dict_add_str(buffer, concat("elseCondition", str), elseCondition);
                                            notifyUsageEnded(expStack->pop(expStack));
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
                                                        void* retVal = executeIntern(true, body2, jump2, NULL);
                                                        if (retVal != NULL && strcmp(((char*)retVal), "break") == 0) {
                                                            notifyUsageEnded(condition);
                                                            notifyUsageEnded(retVal);
                                                            free(body2);
                                                            break;
                                                        } else if (retVal != NULL && strcmp(((char*)retVal), "break") != 0) {
                                                            notifyUsageEnded(condition);
                                                            free(body2);
                                                            return retVal;
                                                        }
                                                        notifyUsageEnded(retVal);
                                                    }
                                                    notifyUsageEnded(elseCondition);
                                                    free(body2);
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
                                                if (!matched) {
                                                    void* retVal = executeIntern(true, body2, jump2, NULL);
                                                    if (retVal != NULL && strcmp(((char*)retVal), "break") == 0) {
                                                        notifyUsageEnded(condition);
                                                        notifyUsageEnded(retVal);
                                                        free(body2);
                                                        break;
                                                    } else if (retVal != NULL && strcmp(((char*)retVal), "break") != 0) {
                                                        notifyUsageEnded(condition);
                                                        free(body2);
                                                        return retVal;
                                                    }
                                                    notifyUsageEnded(retVal);
                                                }
                                                free(body2);
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
            buffer_free(dataStack, buffer);
        }
        else if (code[pointer] == 0x53) {
            pointer++;
            unsigned long bodyStartPos = pointer;
            void* counter = NULL;
            char* body = NULL;
            while (true) {
                struct Dictionary* buffer = buffer_new(dataStack);
                pointer = bodyStartPos;
                if (code[pointer] == 0x01) {
                    pointer++;
                    expStack->push(expStack, dict_new());
                    calculate(1);
                    void* limitRaw = dict_get((struct Dictionary*)expStack->top(expStack), "value");
                    dict_add_str(buffer, "limit", limitRaw);
                    notifyUsageEnded(expStack->pop(expStack));
                    if (code[pointer] == 0x02) {
                        pointer++;
                        expStack->push(expStack, dict_new());
                        calculate(1);
                        void* stepRaw = dict_get((struct Dictionary*)expStack->top(expStack), "value");
                        dict_add_str(buffer, "step", stepRaw);
                        notifyUsageEnded(expStack->pop(expStack));
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
                                    if (((struct Value *) limitRaw)->valueType == 0x02) {
                                        counter = custom_malloc(sizeof(struct ShortValue));
                                        ((struct ShortValue *) counter)->value = 0;
                                        ((struct ShortValue *) counter)->base.valueType = 0x02;
                                    } else if (((struct Value *) limitRaw)->valueType == 0x06) {
                                        counter = custom_malloc(sizeof(struct IntValue));
                                        ((struct IntValue *) counter)->value = 0;
                                        ((struct IntValue *) counter)->base.valueType = 0x06;
                                    } else if (((struct Value *) limitRaw)->valueType == 0x03) {
                                        counter = custom_malloc(sizeof(struct LongValue));
                                        ((struct LongValue *) counter)->value = 0;
                                        ((struct LongValue *) counter)->base.valueType = 0x03;
                                    } else if (((struct Value *) limitRaw)->valueType == 0x04) {
                                        counter = custom_malloc(sizeof(struct FloatValue));
                                        ((struct FloatValue *) counter)->value = 0;
                                        ((struct FloatValue *) counter)->base.valueType = 0x04;
                                    } else if (((struct Value *) limitRaw)->valueType == 0x05) {
                                        counter = custom_malloc(sizeof(struct DoubleValue));
                                        ((struct DoubleValue *) counter)->value = 0;
                                        ((struct DoubleValue *) counter)->base.valueType = 0x05;
                                    }
                                    else {
                                        counter = custom_malloc(sizeof(struct LongValue));
                                        ((struct LongValue *) counter)->value = 0;
                                        ((struct LongValue *) counter)->base.valueType = 0x03;
                                    }
                                    ((struct Value*)counter)->base.type = malloc((strlen("Value") + 1) * sizeof(char));
                                    strcpy(((struct Value*)counter)->base.type, "Value");
                                }
                                struct BoolValue* notReachedLimit = lt(counter, limitRaw);
                                if (notReachedLimit->value) {
                                    void* retVal = executeIntern(true, body, jump, NULL);
                                    if (retVal != NULL && strcmp(((char*)retVal), "break") == 0) {
                                        free(body);
                                        pointer++;
                                        notifyUsageEnded(counter);
                                        notifyUsageEnded(notReachedLimit);
                                        notifyUsageEnded(stepRaw);
                                        notifyUsageEnded(limitRaw);
                                        break;
                                    } else if (retVal != NULL && strcmp(((char*)retVal), "break") != 0) {
                                        free(body);
                                        pointer++;
                                        notifyUsageEnded(counter);
                                        notifyUsageEnded(notReachedLimit);
                                        notifyUsageEnded(stepRaw);
                                        notifyUsageEnded(limitRaw);
                                        return retVal;
                                    }
                                    void* tempCounter = sum(counter, stepRaw);
                                    notifyUsageEnded(counter);
                                    counter = tempCounter;
                                }
                                else {
                                    if (code[pointer] == 0x6e) {
                                        pointer++;
                                        free(body);
                                        notifyUsageEnded(counter);
                                        notifyUsageEnded(notReachedLimit);
                                        notifyUsageEnded(stepRaw);
                                        notifyUsageEnded(limitRaw);
                                        break;
                                    }
                                }
                                notifyUsageEnded(notReachedLimit);
                                notifyUsageEnded(stepRaw);
                                notifyUsageEnded(limitRaw);
                            }
                        }
                    }
                }
                buffer_free(dataStack, buffer);
            }
        }
        else if (code[pointer] == 0x54) {
            pointer++;
            unsigned long bodyStartPos = pointer;
            char* body = NULL;
            while (true) {
                struct Dictionary* buffer = buffer_new(dataStack);
                pointer = bodyStartPos;
                if (code[pointer] == 0x01) {
                    pointer++;
                    expStack->push(expStack, dict_new());
                    calculate(1);
                    void* condition = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                    dict_add_str(buffer, "condition", condition);
                    notifyUsageEnded(expStack->pop(expStack));
                    if (code[pointer] == 0x02) {
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
                            if (((struct BoolValue *) condition)->value) {
                                void* retVal = executeIntern(true, body, jump, NULL);
                                if (retVal != NULL) {
                                    if (strcmp(((char*)retVal), "break") == 0) {
                                        notifyUsageEnded(condition);
                                        free(body);
                                        buffer_free(dataStack, buffer);
                                        pointer++;
                                        break;
                                    } else if (strcmp(((char*)retVal), "break") != 0) {
                                        notifyUsageEnded(condition);
                                        free(body);
                                        buffer_free(dataStack, buffer);
                                        pointer++;
                                        return retVal;
                                    }
                                    notifyUsageEnded(retVal);
                                }
                            } else {
                                if (code[pointer] == 0x6e) {
                                    notifyUsageEnded(condition);
                                    free(body);
                                    pointer++;
                                }
                                break;
                            }
                            if (code[pointer] == 0x6e) {
                                notifyUsageEnded(condition);
                                pointer++;
                            }
                        }
                    }
                }
                buffer_free(dataStack, buffer);
            }
        }
        else if (code[pointer] == 0x55) {
            struct Dictionary* buffer = buffer_new(dataStack);
            pointer++;
            if (code[pointer] == 0x01) {
                pointer++;
                expStack->push(expStack, dict_new());
                calculate(1);
                void          *target = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                struct String *refStr = dict_get((struct Dictionary *) expStack->top(expStack), "value2");
                struct Object *thisObj = dict_get((struct Dictionary *) expStack->top(expStack), "value3");
                dict_add_str(buffer, "target", target);
                dict_add_str(buffer, "refStr", refStr);
                dict_add_str(buffer, "thisObj", thisObj);
                notifyUsageEnded(expStack->pop(expStack));
                if (code[pointer] == 0x02) {
                    pointer++;
                    char entriesCountBytes[4];
                    for (int index = 0; index < 4; index++)
                        entriesCountBytes[index] = code[pointer + index];
                    pointer += 4;
                    int entriesCount = convertBytesToInt(entriesCountBytes);
                    struct Dictionary *entriesDict = dict_new();
                    dict_add_str(buffer, "entries", entriesDict);
                    for (int counter = 0; counter < entriesCount; counter++) {
                        if (code[pointer] == 0x03) {
                            pointer++;
                            char keyLengthBytes[4];
                            for (int index = 0; index < 4; index++)
                                keyLengthBytes[index] = code[pointer + index];
                            pointer += 4;
                            int keyLength = convertBytesToInt(keyLengthBytes);
                            char *key = malloc(keyLength);
                            memcpy(key, &code[pointer], keyLength);
                            pointer += keyLength;
                            pointer += 4;
                            expStack->push(expStack, dict_new());
                            calculate(1);
                            void *value = dict_get((struct Dictionary *) expStack->top(expStack), "value");
                            dict_add_str(entriesDict, key, value);
                            notifyUsageEnded(expStack->pop(expStack));
                            free(key);
                        }
                    }
                    if (target != NULL && strcmp(((struct Code*)target)->type, "Empty") != 0) {
                        struct Function *func = (struct Function *) target;
                        if (thisObj != NULL) dict_add_str(entriesDict, "this", thisObj);
                        notifyUsageEnded(executeIntern(true, func->codes->data, func->loc, entriesDict));
                    } else {
                        routeAndResolve(refStr, entriesDict);
                    }
                }
            }
            buffer_free(dataStack, buffer);
        }
        else if (code[pointer] == 0x56) {
            struct Dictionary* buffer = buffer_new(dataStack);
            pointer++;
            if (code[pointer] == 0x01) {
                pointer++;
                expStack->push(expStack, dict_new());
                calculate(0);
                struct Reference* ref = (struct Reference*)dict_get((struct Dictionary*)expStack->top(expStack), "value");
                dict_add_str(buffer, "ref", ref);
                notifyUsageEnded(expStack->pop(expStack));
                if (code[pointer] == 0x02) {
                    pointer++;
                    expStack->push(expStack, dict_new());
                    calculate(1);
                    void* exp = dict_get((struct Dictionary*)expStack->top(expStack), "value");
                    dict_add_str(buffer, "exp", exp);
                    notifyUsageEnded(expStack->pop(expStack));
                    struct Object* objectChain = NULL;
                    struct StackDataItem* iterator = dataStack->iterator(dataStack);
                    void* variable = dict_get(((struct Dictionary*)iterator->data), ref->currentChain->id->value);
                    if (variable != NULL && strcmp(((struct Code*)variable)->type, "Object") == 0)
                        objectChain = variable;
                    while (variable == NULL && iterator->prev != NULL) {
                        iterator = iterator->prev;
                        variable = dict_get((struct Dictionary*)iterator->data, ref->currentChain->id->value);
                    }
                    struct Identifier* finalChain = ref->currentChain;
                    char* refStr = strdup(ref->currentChain->id->value);
                    ref = ref->restOfTheChain;
                    while (variable != NULL && ref != NULL) {
                        if (strcmp(((struct Code*)variable)->type, "Object") == 0) {
                            void* variable2 = dict_get(((struct Object*)variable)->value, ref->currentChain->id->value);
                            if (variable == NULL) {
                                variable2 = dict_get(((struct Object *) variable)->funcs, ref->currentChain->id->value);
                                if (variable2 != NULL)
                                    variable = variable2;
                            }
                            else
                                variable = variable2;
                        }
                        else if (strcmp(((struct Code*)variable)->type, "Class") == 0) {
                            variable = dict_get(((struct Class*)variable)->functions, ref->currentChain->id->value);
                        }
                        char* tempRefStr = concat(refStr, ".");
                        free(refStr);
                        refStr = tempRefStr;
                        tempRefStr = concat(refStr, ref->currentChain->id->value);
                        free(refStr);
                        refStr = tempRefStr;
                        finalChain = ref->currentChain;
                        ref = ref->restOfTheChain;
                        if (variable != NULL && strcmp(((struct Code*)variable)->type, "Object") == 0)
                            objectChain = variable;
                    }
                    free(refStr);
                    if (objectChain != NULL) {
                        dict_delete(objectChain->value, finalChain->id->value, true);
                        dict_add_str(objectChain->value, finalChain->id->value, exp);
                    } else {
                        if (variable == NULL) {
                            dict_add_str((struct Dictionary *) dataStack->top(dataStack), finalChain->id->value, exp);
                        } else {
                            dict_delete((struct Dictionary*)iterator->data, finalChain->id->value, true);
                            dict_add_str((struct Dictionary *) iterator->data, finalChain->id->value, exp);
                        }
                    }
                }
            }
            buffer_free(dataStack, buffer);
        }
        else if (code[pointer] == 0x59) {
            struct Dictionary* buffer = buffer_new(dataStack);
            pointer++;
            if (code[pointer] == 0x01) {
                pointer++;
                expStack->push(expStack, dict_new());
                calculate(1);
                void* retVal = dict_get((struct Dictionary*)expStack->top(expStack), "value");
                dict_add_str(buffer, "retVal", retVal);
                notifyUsageEnded(expStack->pop(expStack));
                if (expStack->stackSize == 0)
                    expStack->push(expStack, dict_new());
                dict_add_str(expStack->top(expStack), "retVal", retVal);
                buffer_free(dataStack, buffer);
                return retVal;
            }
        }
    }
    return NULL;
}

void* executeIntern(bool addNewLayer, char* c, unsigned long length, struct Dictionary* entriesDict) {
    if (addNewLayer) {
        if (entriesDict == NULL) {
            dataStack->push(dataStack, dict_new());
        } else {
            dataStack->push(dataStack, entriesDict);
        }
    }
    if (codeLength > 0) {
        struct CodeBlock *cb = custom_malloc(sizeof(struct CodeBlock));
        cb->base.type = malloc((strlen("CodeBlock") + 1) * sizeof(char));
        strcpy(cb->base.type, "CodeBlock");
        cb->data = code;
        struct CodePack *cp = custom_malloc(sizeof(struct CodePack));
        cp->base.type = malloc((strlen("CodePack") + 1) * sizeof(char));
        strcpy(cp->base.type, "CodePack");
        cp->code = cb;
        cp->loc = codeLength;
        cp->pointer = pointer;
        codeLengthStack->push(codeLengthStack, cp);
    }
    code = c;
    codeLength = length;
    pointer = 0;
    void* returnValue = ride();
    if (!codeLengthStack->isEmpty(codeLengthStack)) {
        struct CodePack *cp = (struct CodePack *) codeLengthStack->pop(codeLengthStack);
        code = cp->code->data;
        codeLength = cp->loc;
        pointer = cp->pointer;
        notifyUsageEnded(cp);
    }
    if (addNewLayer) {
        notifyUsageEnded(dataStack->pop(dataStack));
    }
    return returnValue;
}

void onExit() {
    printf("exiting...\n");
    struct List* threadList = toList(aliveThreads);
    struct ListDataItem* iterator = threadList->startPointer;
    while (iterator != NULL) {
        pthread_cancel(*(pthread_t *)((struct Pair*)iterator->data)->second);
        printf("exited thread : %s\n", ((struct String*)((struct Pair*)iterator->data)->first)->value);
        iterator = iterator->next;
    }
}

struct Dictionary* depsDict;
char* entryPoint = NULL;

struct Pair* readCodeFile(char* path) {
    FILE *filePtr;
    char *buffer;
    long fileLen;
    filePtr = fopen(path, "rb");
    fseek(filePtr, 0, SEEK_END);
    fileLen = ftell(filePtr);
    rewind(filePtr);
    buffer = (char *)malloc((fileLen + 1) * sizeof(char));
    fread(buffer, fileLen, 1, filePtr);
    fclose(filePtr);
    struct Pair* pair = custom_malloc(sizeof(struct Pair));
    pair->first = buffer;
    pair->second = &fileLen;
    return pair;
}

void loadCodeFile(char* file) {
    struct List* fileDeps = dict_get(depsDict, file);
    struct ListDataItem* iterator = fileDeps->startPointer;
    while (iterator != NULL) {
        loadCodeFile(((struct String*)iterator->data)->value);
        iterator = iterator->next;
    }
    struct Pair* result = readCodeFile(file);
    executeIntern(false, (char*)result->first, *(long*)result->second, NULL);
}

void execute(char* fileTreePath) {
    depsDict = dict_new();
    FILE *fp = fopen(fileTreePath, "r");
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* fcontent = malloc(size);
    fread(fcontent, 1, size, fp);
    json_value* fileTree = json_parse(fcontent, strlen(fcontent));
    for (unsigned int counter = 0; counter < fileTree->u.array.length; counter++) {
        json_value* fileDeps = fileTree->u.array.values[counter];
        char* filePath = fileDeps->u.object.values[0].value->u.string.ptr;
        if (entryPoint == NULL) {
            entryPoint = filePath;
        }
        json_value* fileDepsArr = fileDeps->u.object.values[1].value;
        struct List* deps = custom_malloc(sizeof(struct List));
        initList(deps);
        for (unsigned int i = 0; i < fileDepsArr->u.array.length; i++) {
            struct String* str = custom_malloc(sizeof(struct String));
            initString(str, strlen(fileDepsArr->u.array.values[i]->u.string.ptr) + 1);
            strcpy(str->value, fileDepsArr->u.array.values[i]->u.string.ptr);
            deps->append(deps, str);
        }
        dict_add_str(depsDict, filePath, deps);
    }
    atexit(onExit);
    codeLengthStack = custom_malloc(sizeof(struct Stack));
    expStack = custom_malloc(sizeof(struct Stack));
    dataStack = custom_malloc(sizeof(struct Stack));
    initStack(codeLengthStack);
    initStack(expStack);
    expStack->stackName = malloc((strlen("Exp") + 1) * sizeof(char));
    strcpy(expStack->stackName, "Exp");
    initStack(dataStack);
    dataStack->stackName = malloc((strlen("Data") + 1) * sizeof(char));
    strcpy(dataStack->stackName, "Data");
    initGlobalRefs(dataStack);
    dataStack->push(dataStack, dict_new());
    aliveThreads = dict_new();
    loadCodeFile(entryPoint);
    notifyUsageEnded(codeLengthStack);
    notifyUsageEnded(expStack);
    notifyUsageEnded(dataStack);
}

void prepareFunctions() {
    struct StackDataItem* item = dataStack->item;
    while (item->prev != NULL) {
        item = item->prev;
    }
    struct List* list = toList((struct Dictionary*)item->data);
    struct ListDataItem* iterator = list->startPointer;
    EM_ASM({window.parent.page = {...window.parent};});
    while (iterator != NULL) {
        struct Pair* keyValue = (struct Pair*)iterator->data;
        EM_ASM({
            let funcName = UTF8ToString($0);
            window.parent.page[funcName] = function(args) {
                window.parent.executeFunction(funcName, args);
            };
        }, ((struct String*)keyValue->first)->value);
        iterator = iterator->next;
    }
}

void executeFile(char* file) {
    struct Pair* result = readCodeFile(file);
    executeIntern(false, (char*)result->first, *(long*)result->second, NULL);
}

void execFunc(char* funcName, struct Dictionary* args) {
    struct StackDataItem* item = dataStack->item;
    while (item->prev != NULL) {
        item = item->prev;
    }
    struct Function* func = dict_get((struct Dictionary*)item->data, funcName);
    executeIntern(true, func->codes->data, func->loc, args);
}

void prepareDataStructs() {
    atexit(onExit);
    codeLengthStack = custom_malloc(sizeof(struct Stack));
    expStack = custom_malloc(sizeof(struct Stack));
    dataStack = custom_malloc(sizeof(struct Stack));
    initStack(codeLengthStack);
    initStack(expStack);
    expStack->stackName = malloc((strlen("Exp") + 1) * sizeof(char));
    strcpy(expStack->stackName, "Exp");
    initStack(dataStack);
    dataStack->stackName = malloc((strlen("Data") + 1) * sizeof(char));
    strcpy(dataStack->stackName, "Data");
    initGlobalRefs(dataStack);
    dataStack->push(dataStack, dict_new());
    aliveThreads = dict_new();
}