#ifndef CLIENT_H
#define CLIENT_H

#include <libconfig.h>

typedef struct {
  int port;
  char host[256];
} ClientConfig;

void config_load(const char *path, ClientConfig *cfg);
void client_call_hello(const char *name, const char *endpoint);

#endif