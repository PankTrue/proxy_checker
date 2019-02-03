#ifndef CONFIGS_H
#define CONFIGS_H



#define MAX_WORKERS_DEFAULT         256
#define PROXY_TYPES_DEFAULT         "4,5,h"
#define INPUT_FILENAME_DEFAULT      "input.txt"
#define OUTPUT_FILENAME_DEFAULT     "output.txt"
#define PRINT_ONLINE_PROXY_DEFAULT  false
#define TIMEOUT_DEFAULT             16
#define CHECK_ORIGIN_DEFAULT        false
#define RANGE_START_DEFAULT         16777216
#define RANGE_END_DEFAULT           4294967295
#define RANGE_STATUS_FILE_DEFAULT   "status.ini"



#define DEST_HOST                   "httpbin.org"
#define DEST_PORT                   80



#define GET_FREE_THREAD_USLEEP      100
#define SBLIST_BLOCK_SIZE           1024

#endif // CONFIGS_H
