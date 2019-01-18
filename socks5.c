#include "socks5.h"


int socks5_connect(proxy_client_t *client)
{
    char buf[16];
    char *buf_p;

    //step 1
    buf_p = buf;

    *buf_p++=0x05; //version
    *buf_p++=0x01; //command
    *buf_p++=0x00; //0

    if(write(client->fd,buf,buf_p-buf) <= 0) return 1;
    if(read(client->fd,buf,sizeof(buf)) <= 0) return 1;

    if(buf[0] != 0x05 || buf[1] == 0xFF) return 1;

    //step 2
    buf_p=buf;

    *buf_p++=0x05; //version
    *buf_p++=0x01; //command
    *buf_p++=0x00; //0
    *buf_p++=0x01; //address type

    memcpy(buf_p,&client->dest_addr,sizeof(client->dest_addr)); buf_p+=sizeof(client->dest_addr); //destination addr
    memcpy(buf_p,&client->dest_port,sizeof(client->dest_port)); buf_p+=sizeof(client->dest_port); //destination port

    if(write(client->fd,buf,buf_p-buf) <= 0) return 1;
    if(read(client->fd,buf,sizeof(buf)) <= 0) return 1;

    if(buf[0] != 0x05 || buf[1] != 0x00) return 1;

return 0;
}
