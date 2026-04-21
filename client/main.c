#include "client.h"
#include <stdio.h>

int main(void) {
  ClientConfig cfg;
  config_load("config/client.cfg", &cfg);

  char endpoint[512];
  snprintf(endpoint, sizeof(endpoint), "http://%s:%d", cfg.host, cfg.port);

  client_call_hello("William", endpoint);
  return 0;
}