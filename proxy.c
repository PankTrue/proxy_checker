#include "proxy.h"

extern uint timeout;
extern bool print_online_proxy;



char *get_global_ip()
{
    static char *global_ip = NULL;
    if(global_ip!=NULL)
        return global_ip;


    struct sockaddr_in addr;
    char *data,*origin;
    int fd;

    dest_host = gethostbyname(DEST_HOST);
    memcpy(&addr.sin_addr,*(dest_host->h_addr_list),4);
    addr.sin_port = htons(DEST_PORT);
    addr.sin_family = AF_INET;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { log_error("error create socket"); return 0; }

    set_timeout(fd,16);

    if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { log_error("error connect to "DEST_HOST); return 0; }


    if((data = simple_get_request(fd)) == NULL) { log_error("error simple get request"); return 0; }

    if((global_ip = parse_origin(data)) == NULL) { log_error("error parse global ip"); return 0; }

    log_info("global ip: %s",global_ip);

return global_ip;
}

int proxy_client_connect(proxy_client_t *client,uint16_t timeout, char *proxy_ip, uint16_t proxy_port)
{
    inet_pton(AF_INET,proxy_ip,&client->socks_addr.v4.sin_addr);
    client->socks_addr.v4.sin_port = htons(proxy_port);
    client->socks_addr.v4.sin_family = AF_INET;

    if ((client->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { return 1; }

    set_timeout(client->fd,timeout);

    if(connect(client->fd, (struct sockaddr *)&client->socks_addr.v4, sizeof(client->socks_addr.v4)) < 0) { return 1; }

    dest_addr_init(client);

return 0;
}

void dest_addr_init(proxy_client_t *c)
{
    if(dest_host == NULL)
    {
        if((dest_host = gethostbyname(DEST_HOST)) == NULL){ log_fatal("dest addr init (gethostbyname)"); }

        memcpy(&dest_host_addr,*(dest_host->h_addr_list),4);
        decimal_to_ip(dest_host_buffer,dest_host_addr);
        log_warn("dest addr: %s:%d", dest_host_buffer,DEST_PORT);
    }


    memcpy(&c->dest_addr,*(dest_host->h_addr_list),4);
    c->dest_port = htons(DEST_PORT);
}

void set_timeout(int sock_fd, uint16_t sec)
{
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = 0;

    if (setsockopt (sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
        log_warn("setsockopt failed");

    if (setsockopt (sock_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
        log_warn("setsockopt failed\n");
}

proxy_thread_t *get_free_thread(proxy_thread_t *t, size_t size)
{
    for(;;)
    {
        for (proxy_thread_t *it = t; it != t + size; it++)
        {
            if(it->done != 0)
            {
                if(it->done==1)
                    pthread_detach(it->pt);

                memset(it,0,sizeof (proxy_thread_t));
                return it;
            }
        }
    usleep(GET_FREE_THREAD_USLEEP);
    }
}

size_t get_worked_threads_count(proxy_thread_t *t, size_t size)
{
    size_t count = 0;
    for (proxy_thread_t *it = t; it != t + size; it++)
    {
        if(it->done == 0)
            count++;
    }
return count;
}

char *simple_get_request(int fd)
{
    char *buffer = malloc(SIMPLE_GET_REQUEST_BUFFER_SIZE);
    memset(buffer,0,SIMPLE_GET_REQUEST_BUFFER_SIZE);

    if(write(fd,simple_request,sizeof(simple_request)) <= 0) goto error;
    if(read(fd,buffer,SIMPLE_GET_REQUEST_BUFFER_SIZE) <= 0) goto error;

return buffer;


error:
    free(buffer);
    return NULL;
}

void create_proxy_checker(proxy_thread_t *t)
{
    enum anonimity_level anonlvl;
    if(proxy_client_connect(&t->client,timeout,t->proxy->ip,t->proxy->port) != 0) goto done;


    switch (t->proxy_type)
    {
        case Socks4: if(socks4_connect(&t->client) != 0) {goto done;} anonlvl = High; break;
        case Socks5: break;
        case Http:
    default:
        log_warn("proxy not support!");
        break;
    }

    //TODO: add load httpbin.org and check response

    save_proxy(t->proxy,t->output_file,t->proxy_type,anonlvl);


    static uint counter = 0;
    if(print_online_proxy)
        log_info("%s:%d worked! [%u]",t->proxy->ip,t->proxy->port,++counter);

done:
    t->done = 1;
    close(t->client.fd);
}

bool parse_proxy_type(enum proxy_type type,char *proxy_kinds)
{
    char buf[8];
    memset(buf,0,sizeof(buf));
    memcpy(buf,proxy_kinds,strlen(proxy_kinds));

    for (char *p = buf; *p; p++) *p = tolower(*p);

    switch (type)
    {
        case Socks4:
            if(strstr(buf,"4") != 0 )
                return true;
            else
                return false;
        case Socks5:
            if(strstr(buf,"5") != 0 )
                return true;
            else
                return false;
        case Http:
            if(strstr(buf,"h") != 0 )
                return true;
            else
                return false;
    }
}

void save_proxy(proxy_t *p, char *filename, enum proxy_type type, enum anonimity_level lvl)
{
    static pthread_mutex_t l;

pthread_mutex_lock(&l);

    FILE *file = fopen(filename,"a");

    fprintf(file,"%s:%d:%s:%s\r\n",p->ip,p->port,proxy_type_str[type],anonimity_level_str[lvl]); //try copy proxy type and anon lvl to buffer

    fclose(file);

pthread_mutex_unlock(&l);
}

sblist *load_proxy(const char *filename)
{
        proxy_t *curr;
        sblist *proxy_list;
        FILE *proxy_file;

        int c;
        char line[128];
        char *line_p = line;
        bool end_file = false;
        char *spliter,*end_port;


        proxy_file = fopen(filename,"r"); if(proxy_file <= 0) { log_error("proxy file (%s) not found!",filename); };
        proxy_list = sblist_create(sizeof (proxy_t), SBLIST_BLOCK_SIZE);

        for(;;)
        {
            if(end_file) goto end;

            for(;;)
            {
                c=getc(proxy_file);
                if(c == EOF)  {end_file=true;break;}
                if(c == '\n') break;
                *line_p++=(char)c;
            }

            *line_p++='\0';

            if((spliter = strstr(line,":")) == NULL)  goto end;

            curr = (proxy_t *)malloc(sizeof (proxy_t));
            memset(curr,0,sizeof (proxy_t));

            memcpy(curr->ip,line,(size_t)(spliter-line)); //copy ip
            spliter++; // skip ':'

            curr->port = (uint16_t)atoi(spliter); //copy port

            sblist_add(proxy_list,curr);
            line_p = line;
        }

end:
    fclose(proxy_file);
    log_info("proxy list size: %d",proxy_list->count);
return proxy_list;
}

void decimal_to_ip(char *dest_buf, uint32_t src_addr)
{
    sprintf(dest_buf, "%d.%d.%d.%d",
      (src_addr >> 24) & 0xFF,
      (src_addr >> 16) & 0xFF,
      (src_addr >>  8) & 0xFF,
      (src_addr      ) & 0xFF);
}

char *parse_origin(char *data)
{
     char *origin = malloc(64);
     memset(origin,0,64);

     char *start_origin,*end_origin;

     if((start_origin = strstr(data,"\"origin\":")) == NULL) { log_error("page is not have origin");goto error; };

     start_origin += sizeof("\"origin\": "); //skip "origin:"

     if((end_origin = strstr(start_origin, "\"")) == NULL) { log_error("end_origin == NULL"); goto error; }

     memcpy(origin,start_origin,end_origin-start_origin);
return origin;

error:
    free(origin);
    return NULL;
}