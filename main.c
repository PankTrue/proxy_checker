#include <stdio.h>
#include <getopt.h>

#include "log.h"
#include "socks4.h"
#include "sblist.h"
#include "configs.h"




size_t      workers_max             = MAX_WORKERS_DEFAULT;
char*       proxy_types             = PROXY_TYPES_DEFAULT;
char*       input_filename_proxy    = INPUT_FILENAME_DEFAULT;
char*       output_filename_proxy   = OUTPUT_FILENAME_DEFAULT;
bool        print_online_proxy      = PRINT_ONLINE_PROXY_DEFAULT;
bool        check_orig              = CHECK_ORIGIN_DEFAULT;
uint        timeout                 = TIMEOUT_DEFAULT;
uint32_t    range_start             = RANGE_START_DEFAULT;
uint32_t    range_end               = RANGE_END_DEFAULT;
sblist      *ports_socks4           = NULL;
sblist      *ports_socks5           = NULL;
sblist      *ports_http             = NULL;






static struct option long_opts_checker[] =
{
    {"help",            no_argument,        0, 'h'},
    {"workers",         required_argument,  0, 'w'},
    {"input",           required_argument,  0, 'i'},
    {"output",          required_argument,  0, 'o'},
    {"types",           required_argument,  0, 't'},
    {"timeout",         required_argument,  0, 'u'},
    {"print",           no_argument,        0, 'p'},
    {"check_origin",    no_argument,        0, 'c'},
    {0,0,0,0}
};

static struct option long_opts_scanner[] =
{
    {"help",            no_argument,        0, 'h'},
    {"range",           required_argument,  0, 'r'},
    {"ports",           required_argument,  0, 's'},
    {"workers",         required_argument,  0, 'w'},
    {"output",          required_argument,  0, 'o'},
    {"types",           required_argument,  0, 't'},
    {"timeout",         required_argument,  0, 'u'},
    {"print",           no_argument,        0, 'p'},
    {"check_origin",    no_argument,        0, 'c'},
    {0,0,0,0}
};


void init()
{
    log_set_level(2);
    get_global_ip();
}

int main(int argc, char **argv)
{
    if(argc < 2) { log_fatal("too few arguments"); }
    int opt,opti = 0;

init();


if(strstr(argv[1],"checker"))
{
    /* parse arguments */
    for(; (opt = getopt_long(argc,argv,"h?w:i:o:t:u:pc",long_opts_checker,&opti)) != -1; )
    {
        switch (opt)
        {
            case 'w':
                workers_max = (size_t)atoi(optarg);
                break;
            case 'i':
                input_filename_proxy = optarg;
                break;
            case 'o':
                output_filename_proxy = optarg;
                break;
            case 't':
                proxy_types = optarg;
                break;
            case 'u':
                timeout = (uint)atoi(optarg);
                break;
            case 'p':
                print_online_proxy = true;
                break;
            case 'c':
                check_orig = true;
                break;
            default:
            printf( "Usage: %s checker [-w workers] [-t 4,5,h]\n"
                    "  -h, -? --help        this message\n"
                    "  -w --workers         max workers\n"
                    "  -i --input           input proxy file\n"
                    "  -o --output          output proxy file\n"
                    "  -t --types           proxy types (4,5,h)\n"
                    "  -u --timeout         socket read/write timeout\n"
                    "  -p --print           print online proxies\n",
                    "  -c --check_origin    origin != proxy_addr\n",
                    argv[0]);
            return (opt == '?' || opt == 'h') ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    }

    bool check_socks4   = parse_proxy_type(Socks4,proxy_types);
    bool check_socks5   = parse_proxy_type(Socks5,proxy_types);
    bool check_http     = parse_proxy_type(Http,  proxy_types);


    sblist*         proxy_list      = load_proxy(input_filename_proxy);
    proxy_thread_t* threads         = malloc(sizeof(proxy_thread_t)*workers_max);


    for (proxy_thread_t *it = threads; it != threads + workers_max; it++)
        it->done = -1;


    if(check_socks4)
    {
        log_info("Start checking socks4");
        checking_from_list(proxy_list,output_filename_proxy,threads,workers_max,Socks4);
    }

    if(check_socks5)
    {
        log_info("Start checking socks5");
        checking_from_list(proxy_list,output_filename_proxy,threads,workers_max,Socks5);
    }

    if(check_http)
    {
        log_info("Start checking http");
        checking_from_list(proxy_list,output_filename_proxy,threads,workers_max,Http);
    }

}
else if (strstr(argv[1],"scanner"))
{
    load_range_status(RANGE_STATUS_FILE_DEFAULT,&range_start,&range_end);

    ports_socks4 = parse_ports(PORTS_DEFAULT,Socks4);
    ports_socks5 = parse_ports(PORTS_DEFAULT,Socks5);
    ports_http   = parse_ports(PORTS_DEFAULT,Http);

    /* parse arguments */
    for(; (opt = getopt_long(argc,argv,"h?r:w:o:t:u:pc",long_opts_checker,&opti)) != -1; )
    {
        switch (opt)
        {
            case 's':
                ports_socks4 = parse_ports(optarg,Socks4);
                ports_socks5 = parse_ports(optarg,Socks5);
                ports_http   = parse_ports(optarg,Http);

            case 'r':
                range_start = atoi(optarg);
                range_end   = atoi(strstr(optarg,"-")+1);
                break;
            case 'w':
                workers_max = (size_t)atoi(optarg);
                break;
            case 'o':
                output_filename_proxy = optarg;
                break;
            case 't':
                proxy_types = optarg;
                break;
            case 'u':
                timeout = (uint)atoi(optarg);
                break;
            case 'p':
                print_online_proxy = true;
                break;
            case 'c':
                check_orig = true;
                break;
            default:
            printf( "Usage: %s scanner [-w workers] [-t 4,5,h] [-r 16777216-4294967295] [ -s 4[4145,1488]5[1080]h[80,8080] ]\n"
                    "  -h --help, -?        this message\n"
                    "  -s --ports           port list for check[]\n"
                    "  -r --range           checking range(16777216-4294967295 equal 1.0.0.0-255.255.255.255)\n"
                    "  -w --workers         max workers\n"
                    "  -o --output          output proxy file\n"
                    "  -t --types           proxy types (4,5,h)\n"
                    "  -u --timeout         socket read/write timeout\n"
                    "  -p --print           print online proxies\n",
                    "  -c --check_origin    origin != proxy_addr\n",
                    argv[0]);
            return (opt == '?' || opt == 'h') ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    }

    bool check_socks4   = parse_proxy_type(Socks4,proxy_types);
    bool check_socks5   = parse_proxy_type(Socks5,proxy_types);
    bool check_http     = parse_proxy_type(Http,  proxy_types);


    proxy_thread_t* threads = malloc(sizeof(proxy_thread_t)*workers_max);


    for (proxy_thread_t *it = threads; it != threads + workers_max; it++)
        it->done = -1;


    log_info("staring checking range %u-%u",range_start,range_end);


    for(uint32_t proxy_addr = range_start; proxy_addr <= range_end; proxy_addr += workers_max)
    {
        if(check_socks4)
            checking_from_range(proxy_addr,ports_socks4->items,ports_socks4->count,
                                output_filename_proxy,threads,workers_max,Socks4);

        if(check_socks5)
            checking_from_range(proxy_addr,ports_socks5->items,ports_socks5->count,
                                output_filename_proxy,threads,workers_max,Socks5);

        if(check_http)
            checking_from_range(proxy_addr,ports_http->items,ports_http->count,
                                output_filename_proxy,threads,workers_max,Http);


        log_info("status: %u/%u | %lf%%", proxy_addr,range_end, ((double)proxy_addr/range_end)*100.0);

        save_range_status(RANGE_STATUS_FILE_DEFAULT, proxy_addr, range_end);
    }


}
else
{
    printf( "Usage: %s <command> [<args>]\n"
            "\nCommands list:\n"
            "   checker\t checking proxies from file\n"
            "   scanner\t checking proxies from range\n",
              argv[0]);
      return EXIT_FAILURE;
}

return 0;
}
