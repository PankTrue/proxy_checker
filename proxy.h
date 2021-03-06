#ifndef PROXY_H
#define PROXY_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <netdb.h>
#include <pthread.h>
#include <regex.h>
#include <unistd.h>

#include "log.h"
#include "sblist.h"
#include "configs.h"


#define SIMPLE_GET_REQUEST_BUFFER_SIZE 1024



union sockaddr_union
{
    struct sockaddr_in  v4;
    struct sockaddr_in6 v6;
};


enum proxy_type
{
    Http,
    Socks4,
    Socks5
};
static const char *proxy_type_str[] =
{
    "Http",
    "Socks4",
    "Socks5"
};

enum anonimity_level
{
    Transparent,
    Low,
    Medium,
    High
};
static const char *anonimity_level_str[] =
{
    "Transparent",
    "Low",
    "Medium",
    "High"
};



typedef struct proxy_client
{
    union sockaddr_union    socks_addr;
    uint32_t                dest_addr;
    uint16_t                dest_port;
    int                     fd;
} proxy_client_t;

typedef struct
{
    char        ip[16];
    uint16_t    port;
} proxy_t;

typedef struct
{
    proxy_client_t      client;
    proxy_t             *proxy;
    char                *output_file;
    enum proxy_type     proxy_type;
    pthread_t           pt;
    int                 done;
}proxy_thread_t;




static char simple_request[] =  "GET /get HTTP/1.1\r\n"
                                "Host: "DEST_HOST"\r\n"
                                "Connection: close\r\n\r\n";



static struct hostent *dest_host = NULL;
static uint32_t dest_host_addr;
static char dest_host_buffer[16];



void checking_from_list(sblist *proxy_list, char *output_filename_proxy,
                        proxy_thread_t *threads, size_t worker_max,
                        enum proxy_type type);

void checking_from_range(uint32_t proxy_addr, uint16_t *ports, size_t ports_count,
                         char *output_filename_proxy,proxy_thread_t *threads,
                         size_t worker_max, enum proxy_type type);



void create_proxy_checker(proxy_thread_t *t);



void set_timeout(int sock_fd, uint16_t sec);
void dest_addr_init(proxy_client_t *c);
int proxy_client_connect(proxy_client_t *client,uint16_t timeout, proxy_t *proxy);



proxy_thread_t *get_free_thread(proxy_thread_t *t, size_t size);
size_t get_worked_threads_count(proxy_thread_t *t, size_t size);


int load_range_status(char *filename, uint32_t *range_start, uint32_t *range_end);
void save_range_status(char *filename, uint32_t range_start, uint32_t range_end);
sblist *load_proxy(const char *filename);
void save_proxy(proxy_t *p,char *filename,enum proxy_type type,enum anonimity_level lvl);



char *get_global_ip();
void decimal_to_ip(char *dest_buf, uint32_t src_addr);
char *simple_get_request(int fd);
int check_origin(int fd,proxy_t *p);
size_t arr_size(void *array, size_t element_size);


char *parse_origin(char *data);
sblist *parse_ports(char *data, enum proxy_type type);
bool parse_proxy_type(enum proxy_type type,char *proxy_kinds);


#endif // PROXY_H
