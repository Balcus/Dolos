// includerea headerului clientului si a bibliotecilor necesare
#include "client.h"
#include "service.nsmap"
#include "soapH.h"
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// functia care incarca configuratia clientului dintr-un fisier cfg
void config_load(const char *path, ClientConfig *cfg) {
  config_t lib_cfg;
  config_init(&lib_cfg);

  // setam valorile implicite pentru port si host

  // port si host default
  cfg->port = 8080;
  strncpy(cfg->host, "localhost", sizeof(cfg->host) - 1);
  cfg->host[sizeof(cfg->host) - 1] = '\0';

  // incercam sa citim fisierul de configurare
  // daca nu reusim, afisam eroare si pastram valorile default
  if (!config_read_file(&lib_cfg, path)) {
    fprintf(stderr,
            "Eroare la configurare %s:%d - %s, folosim valorile default\n",
            config_error_file(&lib_cfg), config_error_line(&lib_cfg),
            config_error_text(&lib_cfg));
    config_destroy(&lib_cfg);
    return;
  }

  // citim portul din configurare, daca exista
  int port = 0;
  if (config_lookup_int(&lib_cfg, "port", &port)) {
    cfg->port = port;
  }

  // citim adresa host-ului din configurare, daca exista
  const char *host = NULL;
  if (config_lookup_string(&lib_cfg, "host", &host)) {
    strncpy(cfg->host, host, sizeof(cfg->host) - 1);
    cfg->host[sizeof(cfg->host) - 1] = '\0';
  }

  // eliberam resursele libconfig
  config_destroy(&lib_cfg);
}

// functia care face un apel soap de test catre server (hello)
void client_call_hello(const char *name, const char *endpoint) {
  // initializam structura soap pentru comunicare
  struct soap soap;
  soap_init(&soap);
  char *result = NULL;

  // trimitem cererea soap hello si asteptam raspunsul
  if (soap_call_ns__hello(&soap, endpoint, NULL, name, &result) == SOAP_OK) {
    printf("[CLIENT] Raspuns server: %s\n", result);
  } else {
    // daca a aparut o eroare, o afisam
    soap_print_fault(&soap, stderr);
  }

  // eliberam toate resursele alocate de soap
  soap_destroy(&soap);
  soap_end(&soap);
  soap_done(&soap);
}

// functia care trimite fisierele specificate catre server
void client_send_files(const char **filepaths, int count,
                       const ClientConfig *cfg) {
  // parcurgem lista de fisiere si afisam fiecare fisier care va fi trimis
  for (int i = 0; i < count; i++) {
    printf("[CLIENT] Fisierul %s va fi trimis spre server\n", filepaths[i]);
  }
}