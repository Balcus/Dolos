#include "client.h"
#include "service.nsmap"
#include "soapH.h"
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void config_load(const char *path, ClientConfig *cfg) {
  config_t lib_cfg;
  config_init(&lib_cfg);

  /* defaults */
  cfg->port = 8080;
  strcpy(cfg->host, "localhost");

  if (!config_read_file(&lib_cfg, path)) {
    fprintf(stderr, "[CONFIG] %s:%d - %s, using defaults\n",
            config_error_file(&lib_cfg), config_error_line(&lib_cfg),
            config_error_text(&lib_cfg));
    config_destroy(&lib_cfg);
    return;
  }

  int port;
  if (config_lookup_int(&lib_cfg, "port", &port))
    cfg->port = port;

  const char *host;
  if (config_lookup_string(&lib_cfg, "host", &host))
    strncpy(cfg->host, host, sizeof(cfg->host) - 1);

  config_destroy(&lib_cfg);
}

void client_call_hello(const char *name, const char *endpoint) {
  struct soap soap;
  soap_init(&soap);
  char *result = NULL;

  if (soap_call_ns__hello(&soap, endpoint, NULL, (char *)name, &result) ==
      SOAP_OK) {
    printf("[CLIENT] Server response: %s\n", result);
  } else {
    soap_print_fault(&soap, stderr);
  }

  soap_destroy(&soap);
  soap_end(&soap);
  soap_done(&soap);
}