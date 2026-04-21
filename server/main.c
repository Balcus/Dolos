#include "server.h"
#include <stdio.h>

int main(void) {
  ServerConfig cfg;
  config_load("config/server.cfg", &cfg);

  printf("Configurare server: port = %d, log_dir = %s\n", cfg.port, cfg.log_dir);

  start_server(cfg.port, cfg.log_dir);
  return 0;
}