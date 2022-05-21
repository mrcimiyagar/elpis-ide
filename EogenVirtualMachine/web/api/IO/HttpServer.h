
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
#include "../../structures/array/array.h"
#include "../../utils/json.h"

#define SIZE 1024
#define BACKLOG 10  // Passed to listen()

void setHttpHeader(char httpHeader[]);
void* http_server_start(void* httpServerArg);
void setStringifyObj(char* (*stringifyObj)(void*));
void http_server_stop();
void http_server_report(struct sockaddr_in *serverAddress);
void* process_value(json_value* value, int depth);