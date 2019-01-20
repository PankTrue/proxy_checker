#ifndef HTTP_H
#define HTTP_H

#include "proxy.h"

int http_connect(proxy_client_t *client, char **response);


enum anonimity_level parse_anonimity_level(char *data);

#endif // HTTP_H
