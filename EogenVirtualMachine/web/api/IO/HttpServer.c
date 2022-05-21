
#include "HttpServer.h"
#include "../../utils/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h> // for getnameinfo()

// Usual socket headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>

#define SIZE 1024
#define BACKLOG 10  // Passed to listen()
bool work = false;

void http_server_stop() {
    work = false;
}

#define CONNMAX 1000
#define BYTES 1024
int listenfd, clients[CONNMAX];
char *ROOT;

bool startsWith(const char *a, const char *b)
{
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}

char* concatStrs(const char *s1, const char *s2)
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

struct Request *parse_request(const char *raw) {
    struct Request *req = custom_malloc(sizeof(struct Request));
    req->base.type = malloc((strlen("HttpRequest") + 1) * sizeof(char));
    strcpy(req->base.type, "HttpRequest");
    size_t meth_len = strcspn(raw, " ");
    if (memcmp(raw, "GET", strlen("GET")) == 0) {
        req->method = GET;
    } else if (memcmp(raw, "HEAD", strlen("HEAD")) == 0) {
        req->method = HEAD;
    } else if (memcmp(raw, "POST", strlen("POST")) == 0) {
        req->method = POST;
    } else {
        req->method = UNSUPPORTED;
    }
    raw += meth_len + 1; // move past <SP>

    // Request-URI
    size_t url_len = strcspn(raw, " ");
    req->url = malloc(url_len + 1);
    if (!req->url) {
        free_request(req);
        return NULL;
    }
    memcpy(req->url, raw, url_len);
    req->url[url_len] = '\0';
    raw += url_len + 1; // move past <SP>

    // HTTP-Version
    size_t ver_len = strcspn(raw, "\r\n");
    req->version = malloc(ver_len + 1);
    if (!req->version) {
        free_request(req);
        return NULL;
    }
    memcpy(req->version, raw, ver_len);
    req->version[ver_len] = '\0';
    raw += ver_len + 2; // move past <CR><LF>

    struct Header *header = NULL, *last = NULL;
    while (raw[0]!='\r' || raw[1]!='\n') {
        last = header;
        header = malloc(sizeof(Header));
        if (!header) {
            free_request(req);
            return NULL;
        }

        // name
        size_t name_len = strcspn(raw, ":");
        header->name = malloc(name_len + 1);
        if (!header->name) {
            free_request(req);
            return NULL;
        }
        memcpy(header->name, raw, name_len);
        header->name[name_len] = '\0';
        raw += name_len + 1; // move past :
        while (*raw == ' ') {
            raw++;
        }

        // value
        size_t value_len = strcspn(raw, "\r\n");
        header->value = malloc(value_len + 1);
        if (!header->value) {
            free_request(req);
            return NULL;
        }
        memcpy(header->value, raw, value_len);
        header->value[value_len] = '\0';
        raw += value_len + 2; // move past <CR><LF>

        // next
        header->next = last;
    }
    req->headers = header;
    raw += 2; // move past <CR><LF>

    size_t body_len = strlen(raw);
    req->body = malloc(body_len + 1);
    if (!req->body) {
        free_request(req);
        return NULL;
    }
    memcpy(req->body, raw, body_len);
    req->body[body_len] = '\0';


    return req;
}

char* (*stringifyObj)(void*);
void setStringifyObj(char* (*stringifyFunc)(void*)) {
    stringifyObj = stringifyFunc;
}

static void* process_object(json_value* value, int depth)
{
    int length, x;
    if (value == NULL) {
        return NULL;
    }
    length = value->u.object.length;
    struct Object* obj = custom_malloc(sizeof(struct Object));
    obj->base.type = malloc((strlen("Object") + 1) * sizeof(char));
    strcpy(obj->base.type, "Object");
    obj->value = dict_new();
    notifyNewUsage(obj, obj->value);
    obj->funcs = NULL;
    for (x = 0; x < length; x++) {
        char* key = malloc((strlen(value->u.object.values[x].name) + 1) * sizeof(char));
        strcpy(key, value->u.object.values[x].name);
        dict_add_str(obj->value, key, process_value(value->u.object.values[x].value, depth + 1));
    }
    return obj;
}

static void* process_array(json_value* value, int depth)
{
    int length, x;
    if (value == NULL) {
        struct Empty *empty = custom_malloc(sizeof(struct Empty));
        empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
        strcpy(empty->base.type, "Empty");
        return empty;
    }
    length = value->u.array.length;
    struct Array* array = custom_malloc(sizeof(struct Array));
    initArray(array, length);
    for (x = 0; x < length; x++)
        insertArray(array, process_value(value->u.array.values[x], depth));
    return array;
}

void* process_value(json_value* value, int depth)
{
    if (value == NULL) {
        struct Empty* empty = custom_malloc(sizeof(struct Empty));
        empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
        strcpy(empty->base.type, "Empty");
        return empty;
    }
    switch (value->type) {
        case json_null:
        case json_none: {
            struct Empty *empty = custom_malloc(sizeof(struct Empty));
            empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
            strcpy(empty->base.type, "Empty");
            return empty;
        }
        case json_object:
            return process_object(value, depth+1);
            break;
        case json_array:
            return process_array(value, depth+1);
            break;
        case json_integer: {
            struct IntValue *intValue = custom_malloc(sizeof(struct IntValue));
            intValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(intValue->base.base.type, "Value");
            intValue->base.valueType = 0x06;
            intValue->value = value->u.integer;
            return intValue;
        }
        case json_double: {
            struct DoubleValue *dblValue = custom_malloc(sizeof(struct DoubleValue));
            dblValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(dblValue->base.base.type, "Value");
            dblValue->base.valueType = 0x05;
            dblValue->value = value->u.dbl;
            return dblValue;
        }
        case json_string: {
            struct StringValue *strValue = custom_malloc(sizeof(struct StringValue));
            strValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(strValue->base.base.type, "Value");
            strValue->base.valueType = 0x01;
            strValue->value = malloc((strlen(value->u.string.ptr) + 1) * sizeof(char));
            strcpy(strValue->value, value->u.string.ptr);
            return strValue;
        }
        case json_boolean: {
            struct BoolValue *boolValue = custom_malloc(sizeof(struct BoolValue));
            boolValue->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
            strcpy(boolValue->base.base.type, "Value");
            boolValue->base.valueType = 0x07;
            boolValue->value = value->u.boolean;
            return boolValue;
        }
    }
    return NULL;
}

struct Object* produceFriendlyReqObject(Request* req) {
    struct Object *reqObj = custom_malloc(sizeof(struct Object));
    reqObj->base.type = malloc((strlen("Object") + 1) * sizeof(char));
    strcpy(reqObj->base.type, "Object");
    reqObj->value = dict_new();
    notifyNewUsage(reqObj, reqObj->value);
    reqObj->funcs = NULL;

    struct StringValue *method = custom_malloc(sizeof(struct StringValue));
    method->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
    strcpy(method->base.base.type, "Value");
    method->base.valueType = 0x01;
    method->value = malloc((strlen("POST") + 1) * sizeof(char));
    strcpy(method->value, "POST");
    dict_add_str(reqObj->value, "method", method);

    struct StringValue *url = custom_malloc(sizeof(struct StringValue));
    url->base.base.type = malloc((strlen("Value") + 1) * sizeof(char));
    strcpy(url->base.base.type, "Value");
    url->base.valueType = 0x01;
    url->value = malloc((strlen(req->url) + 1) * sizeof(char));
    strcpy(url->value, req->url);
    dict_add_str(reqObj->value, "url", url);

    json_value *jsonValue = json_parse(req->body, strlen(req->body));
    struct Object *body = process_value(jsonValue, 0);
    dict_add_str(reqObj->value, "body", body);

    struct Dictionary *headers = dict_new();
    Header *headerPtr = req->headers;
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
    dict_add_str(reqObj->value, "headers", headersObj);
    return reqObj;
}

void respond(struct Stack* ds, void* (*serve)(bool addNewLayer, char* c, unsigned long length, struct Dictionary* entries), char* (*stringifyObj)(void*),
        struct Object* server, int n) {
    char mesg[99999], data_to_send[BYTES], path[99999];
    int rcvd, fd, bytes_read;
    memset( (void*)mesg, (int)'\0', 99999 );
    rcvd=recv(clients[n], mesg, 99999, 0);
    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    // receive socket closed
        fprintf(stderr,"Client disconnected upexpectedly.\n");
    else
    {
        struct Dictionary* buffer = buffer_new(ds);
        struct Request* req = parse_request(mesg);
        if (req->method == GET) {
            char *filesPath = ((struct StringValue *) dict_get(server->value, "filesPath"))->value;
            if (startsWith(req->url, filesPath)) {
                size_t newUrlLength = (strlen(req->url) - strlen(filesPath));
                char* newUrl = malloc(newUrlLength + 1);
                memcpy(newUrl, &req->url[strlen(filesPath)], newUrlLength);
                newUrl[newUrlLength] = '\0';
                free(req->url);
                req->url = newUrl;
                if (strncmp(req->url, "/\0", 2) == 0) {
                    free(req->url);
                    req->url = malloc((strlen("/index.html") + 1) * sizeof(char));
                    strcpy(req->url, "/index.html");
                    req->url[strlen("/index.html")] = '\0';
                }
                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], filesPath);
                strcpy(&path[strlen(ROOT) + strlen(filesPath)], req->url);
                printf("file: %s\n", path);
                if ((fd = open(path, O_RDONLY)) != -1)    //FILE FOUND
                {
                    send(clients[n], "HTTP/1.0 200 OK\n\n Content-Type: application/json\n\n", 17, 0);
                    while ((bytes_read = read(fd, data_to_send, BYTES)) > 0)
                        write(clients[n], data_to_send, bytes_read);
                } else
                    write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
            } else {
                struct Function *func = (struct Function *) dict_get(
                        (struct Dictionary *) dict_get(server->value, "routes"), req->url);
                if (func != NULL) {
                    struct Dictionary* entries = dict_new();
                    dict_add_str(entries, "request", produceFriendlyReqObject(req));
                    void* resultObj = serve(true, func->codes->data, func->loc, entries);
                    if (resultObj == NULL) {
                        struct Empty* empty = custom_malloc(sizeof(struct Empty));
                        empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
                        strcpy(empty->base.type, "Empty");
                        resultObj = empty;
                    }
                    char *result = stringifyObj(resultObj);
                    if (strcmp(((struct Code*)resultObj)->type, "Object") == 0) {
                        result = concatStrs("Content-Type: application/json\n\n", result);
                        char *response = concatStrs("HTTP/1.0 200 OK\n", result);
                        send(clients[n], response, strlen(response), 0);
                        free(response);
                    }
                    else {
                        char *response = concatStrs("HTTP/1.0 200 OK\n\n", result);
                        send(clients[n], response, strlen(response), 0);
                        free(response);
                    }
                    free(result);
                } else {
                    write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                }
            }
        }
        else if (req->method == POST) {
            struct Function *func = (struct Function *) dict_get(
                    (struct Dictionary *) dict_get(server->value, "routes"), req->url);
            if (func != NULL) {
                struct Dictionary *entries = dict_new();
                struct Object* reqOrig = produceFriendlyReqObject(req);
                dict_add_str(buffer, "request", reqOrig);
                dict_add_str(entries, "request", reqOrig);
                printf("%s\n", stringifyObj(reqOrig));
                void* resultObj = serve(true, func->codes->data, func->loc, entries);
                if (resultObj == NULL) {
                    struct Empty* empty = custom_malloc(sizeof(struct Empty));
                    empty->base.type = malloc((strlen("Empty") + 1) * sizeof(char));
                    strcpy(empty->base.type, "Empty");
                    resultObj = empty;
                }
                char *result = stringifyObj(resultObj);
                if (strcmp(((struct Code*)resultObj)->type, "Object") == 0) {
                    result = concatStrs("Content-Type: application/json\n\n", result);
                    char *response = concatStrs("HTTP/1.0 200 OK\n", result);
                    send(clients[n], response, strlen(response), 0);
                    free(response);
                }
                else {
                    char *response = concatStrs("HTTP/1.0 200 OK\n\n", result);
                    send(clients[n], response, strlen(response), 0);
                    free(response);
                }
                free(result);
            } else {
                write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
            }
        }
        buffer_free(ds, buffer);
        notifyUsageEnded(req);
    }
    shutdown (clients[n], SHUT_RDWR);
    close(clients[n]);
    clients[n]=-1;

    printf("response ended.\n");
}

void* http_server_start(void* httpServerArg)
{
    struct HttpServerArg* hsa = (struct HttpServerArg*) httpServerArg;
    ROOT = getenv("PWD");
    struct addrinfo hints, *res, *p;
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, ((struct StringValue*)dict_get(hsa->server->value, "port"))->value, &hints, &res) != 0)
    {
        perror ("getaddrinfo() error");
        exit(1);
    }
    for (p = res; p!=NULL; p=p->ai_next)
    {
        listenfd = socket (p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1) continue;
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
            perror("setsockopt(SO_REUSEADDR) failed");
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p==NULL)
    {
        perror ("socket() or bind()");
        exit(1);
    }
    freeaddrinfo(res);
    if ( listen (listenfd, 1000000) != 0 )
    {
        perror("listen() error");
        exit(1);
    }
    work = true;
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    int slot=0;
    printf("server started on port %s\n", ((struct StringValue*)dict_get(hsa->server->value, "port"))->value);
    while(work) {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);
        if (clients[slot]<0)
            exit(15);
        else
        {
            respond(hsa->ds, hsa->serve, hsa->stringifyObj, hsa->server, slot);
        }
        while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
    }
    return NULL;
}