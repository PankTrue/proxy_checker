#include "proxy.h"

extern uint timeout;
extern bool print_online_proxy;
extern bool check_orig;



char *get_global_ip()
{
    static char *global_ip = NULL;
    if(global_ip!=NULL)
        return global_ip;


    struct sockaddr_in addr;
    char *data,*origin;
    int fd;

    if((dest_host = gethostbyname(DEST_HOST)) == NULL) { log_fatal("error resolv "DEST_HOST); }
    memcpy(&addr.sin_addr,*(dest_host->h_addr_list),sizeof(addr.sin_addr));
    addr.sin_port = htons(DEST_PORT);
    addr.sin_family = AF_INET;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { log_error("error create socket"); return NULL; }

    set_timeout(fd,16);

    if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { log_error("error connect to "DEST_HOST); return NULL; }


    if((data = simple_get_request(fd)) == NULL) { log_error("error simple get request"); return NULL; }

    if((global_ip = parse_origin(data)) == NULL) { log_error("error parse global ip"); return NULL; }

    log_info("global ip: %s",global_ip);

return global_ip;
}

int proxy_client_connect(proxy_client_t *client,uint16_t timeout, proxy_t *proxy)
{
    if(proxy != NULL)
    {
        inet_pton(AF_INET,proxy->ip,&client->socks_addr.v4.sin_addr);
        client->socks_addr.v4.sin_port = htons(proxy->port);
    }

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
    char *data;
    enum anonimity_level anonlvl;
    if(proxy_client_connect(&t->client,timeout,t->proxy) != 0) goto done;


    switch (t->proxy_type)
    {
        case Socks4: if(socks4_connect(&t->client)      != 0) { goto done; } anonlvl = High; break;
        case Socks5: if(socks5_connect(&t->client)      != 0) { goto done; } anonlvl = High; break;
        case Http:   if(http_connect(&t->client,&data)  != 0) { goto done; } anonlvl = parse_anonimity_level(data); free(data); break;
    default:
        log_error("proxy not support!");
        break;
    }

    if( check_orig &&
        t->proxy_type != Http) // ignore http because already checked in parse_anonimity_level
    {
        if(check_origin(t->client.fd,t->proxy) != 0) goto done;
    }

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
        proxy_t curr_proxy;
        sblist *proxy_list;
        FILE *proxy_file;

        int c;
        char line[128];
        char *line_p = line;
        bool end_file = false;
        char *spliter;


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

            memset(&curr_proxy,0,sizeof (proxy_t));

            memcpy(curr_proxy.ip,line,(size_t)(spliter-line)); //copy ip
            spliter++; // skip ':'

            curr_proxy.port = (uint16_t)atoi(spliter); //copy port

            sblist_add(proxy_list,&curr_proxy);
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

     if((start_origin = strstr(data,"\"origin\":")) == NULL) { goto error; };

     start_origin += sizeof("\"origin\": "); //skip "origin:"

     if((end_origin = strstr(start_origin, "\"")) == NULL) { log_error("end_origin == NULL"); goto error; }

     memcpy(origin,start_origin,end_origin-start_origin);
return origin;

error:
    free(origin);
    return NULL;
}

int check_origin(int fd, proxy_t *p)
{
    static pthread_mutex_t l;
    char *origin,*data;

    if((data = simple_get_request(fd))  == NULL) return 1;
    if((origin = parse_origin(data))    == NULL) return 1;

    if(strstr(origin,p->ip) == NULL)
    {
        pthread_mutex_lock(&l);

        FILE *file = fopen("invalid_origin.txt","a");
        fprintf(file,"%s=>%s\r\n",p->ip,origin);
        fclose(file);

        pthread_mutex_unlock(&l);
    }

free(data);
free(origin);

return 0;
}

void checking_from_list(sblist *proxy_list, char *output_filename_proxy, proxy_thread_t *threads, size_t workers_max, enum proxy_type type)
{
    proxy_thread_t* current_thread  = NULL;

    for (proxy_t *it = proxy_list->items; it != proxy_list->items+(proxy_list->count * proxy_list->itemsize); it++)
    {
        current_thread = get_free_thread(threads,workers_max);

        current_thread->proxy_type  = type;
        current_thread->proxy       = it;
        current_thread->output_file = output_filename_proxy;

        if(pthread_create(&current_thread->pt, NULL, &create_proxy_checker, current_thread) != 0)
            log_error("pthread_create failed.");
    }

}

void checking_from_range(uint32_t proxy_addr_begin, uint16_t *ports, size_t ports_count,
                         char *output_filename_proxy, proxy_thread_t *threads,
                         size_t workers_max, enum proxy_type type)
{
    proxy_thread_t* current_thread  = NULL;

    for(uint32_t proxy_addr = proxy_addr_begin; proxy_addr < (proxy_addr_begin + workers_max); proxy_addr++)
    {
        for (uint proxy_port = 0; proxy_port < ports_count; ++proxy_port)
        {
            current_thread = get_free_thread(threads,workers_max);

            current_thread->proxy_type  = type;
            current_thread->proxy       = NULL;
            current_thread->output_file = output_filename_proxy;

            //set addr and port
            current_thread->client.socks_addr.v4.sin_addr.s_addr    = htonl(proxy_addr);
            current_thread->client.socks_addr.v4.sin_port           = htons(ports[proxy_port]);

            if(pthread_create(&current_thread->pt, NULL, &create_proxy_checker, current_thread) != 0)
                log_error("pthread_create failed.");
        }

    }
}
