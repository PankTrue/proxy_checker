#include "http.h"



int http_connect(proxy_client_t *client, char **response)
{
    if((*response = simple_get_request(client->fd)) == NULL) return 1;

    if(parse_origin(*response) == NULL) return 1;

return 0;
}

enum anonimity_level parse_anonimity_level(char *data)
{
    char *origin = parse_origin(data);

    if(origin == NULL) { log_error("origin is null"); }

    bool have_my_ip         =   strstr(origin,get_global_ip()) == 0 ?   false : true;
    bool isOriginMultiIPs   =   strstr(origin,",") == 0             ?   false : true;
    bool isViaHeader        =   strstr(data,"Via") == 0             ?   false : true;

    free(origin);

    if(have_my_ip)
        return Transparent;
    else if( !isOriginMultiIPs && isViaHeader)
        return Low;
    else if( isOriginMultiIPs )
        return Medium;
    else
        return High;
}
