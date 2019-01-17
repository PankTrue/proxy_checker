#ifndef SOCKS4_H
#define SOCKS4_H


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <memory.h>

#include "log.h"
#include "proxy.h"
#include "socks4.h"




int socks4_connect(proxy_client_t *client);



#endif // SOCKS4_H
