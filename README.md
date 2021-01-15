# proxy_checker
Multithreaded proxy checker


### Check proxies from file:

"Usage: checker [-w workers] [-t 4,5,h]\n"
*        "  -h, -? --help        this message\n"
*        "  -w --workers         max workers\n"
*        "  -i --input           input proxy file\n"
*        "  -o --output          output proxy file\n"
*        "  -t --types           proxy types (4,5,h)\n"
*        "  -u --timeout         socket read/write timeout\n"
*        "  -p --print           print online proxies\n",
*        "  -c --check_origin    origin != proxy_addr\n",
          

### Check proxies from range:

"Usage: scanner [-w workers] [-t 4,5,h] [-r 16777216-4294967295] [ -s 4[4145,1488]5[1080]h[80,8080] ]\n"
*                    "  -h --help, -?        this message\n"
*                    "  -s --ports           port list for check[]\n"
*                    "  -r --range           checking range(16777216-4294967295 equal 1.0.0.0-255.255.255.255)\n"
*                    "  -w --workers         max workers\n"
*                    "  -o --output          output proxy file\n"
*                    "  -t --types           proxy types (4,5,h)\n"
*                    "  -u --timeout         socket read/write timeout\n"
*                    "  -p --print           print online proxies\n",
*                    "  -c --check_origin    origin != proxy_addr\n",
          
          

