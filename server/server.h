#ifndef SERVER_H
#define SERVER_H

#include <libconfig.h>

typedef struct {
  int port;
  char log_dir[256];
} ServerConfig;

void config_load(const char *path, ServerConfig *cfg);
void start_server(int port, const char *log_dir);
void logger_init(const char *log_dir);
void logger_log(const char *msg);

#endif