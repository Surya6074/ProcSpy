#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 8080

#define BUFFER_SIZE 8192

#define ROOT_DIR "./web"


int init_web_server();

const char *get_content_type(const char *path);

#endif 
