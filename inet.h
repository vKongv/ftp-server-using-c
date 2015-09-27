/*inet.h*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "queue.h"
#include "qoperate.h"

#define SERV_TCP_PORT 25000
#define SERV_UDP_PORT 35001
#define CLI_UDP_PORT 35002

const char *STATUS[] = {"LISTENING", "SELECTING", "DOWNLOADING", "UPLOADING", "ERROR"};
const char *ERR_MSG[] = {"Cannot open the file\n", "Unknown Error\n", "Fail to upload\n", "Quit"};
const char *CRASH_MSG = "Unknown Error\n";
const char *NULL_MSG = "NULL";
