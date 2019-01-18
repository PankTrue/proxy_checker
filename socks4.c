#include "socks4.h"




int socks4_connect(proxy_client_t *client)
{
    char buf[16];
    char *buf_p=buf;

    *buf_p++ = 0x04;                                                                                //version
    *buf_p++ = 0x01;                                                                                //command
    memcpy(buf_p,&client->dest_port,sizeof(client->dest_port)); buf_p+=sizeof(client->dest_port);   //port
    memcpy(buf_p,&client->dest_addr,sizeof(client->dest_addr)); buf_p+=sizeof(client->dest_addr);   //address
    *buf_p++ = 0x00;

    //send request
    if(write(client->fd,buf,buf_p-buf) <= 0)  { log_debug("socks4 write <= 0"); return 1; }
    //receive response
    if(read(client->fd,buf,sizeof(buf)) <= 0) { log_debug("socks4 read <= 0"); return 1; }

    if(buf[0] != 0x00 || buf[1] < 0x5A || buf[1] > 0x5E) { log_debug("socks4 recv is not valid"); return 1; }
return 0;
}
