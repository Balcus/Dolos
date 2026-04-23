#ifndef SERVER_H
#define SERVER_H

#include <libconfig.h>

// structura care retine configuratia serverului (port si directorul de loguri)
typedef struct {
  int port;
  char log_dir[256];
} ServerConfig;

// declaratiile functiilor publice ale serverului
void config_load(const char *path, ServerConfig *cfg);
void start_server(int port, const char *log_dir);
void logger_init(const char *log_dir);

// logging
void logger_log(const char *msg);

#endif