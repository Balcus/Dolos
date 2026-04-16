#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>

void start_server(int port);

// logging
void logger_init();
void logger_log(const char *msg);

#endif