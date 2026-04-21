#include "server.h"
#include "service.nsmap"
#include "soapH.h"
#include "soapStub.h"
#include <libconfig.h>
#include <libstemmer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

int log_fd = -1;

void config_load(const char *path, ServerConfig *cfg) {
  config_t lib_cfg;
  config_init(&lib_cfg);

  cfg->port = 8080;
  strcpy(cfg->log_dir, "logs");

  if (!config_read_file(&lib_cfg, path)) {
    fprintf(stderr, "Eroare la configurare %s:%d - %s, folosim valorile default\n",
            config_error_file(&lib_cfg), config_error_line(&lib_cfg),
            config_error_text(&lib_cfg));
    config_destroy(&lib_cfg);
    return;
  }

  int port;
  if (config_lookup_int(&lib_cfg, "port", &port))
    cfg->port = port;

  const char *log_dir;
  if (config_lookup_string(&lib_cfg, "log_dir", &log_dir))
    strncpy(cfg->log_dir, log_dir, sizeof(cfg->log_dir) - 1);

  config_destroy(&lib_cfg);
}

int ns__hello(struct soap *soap, char *name, char **result) {
  char buf[BUFFER_SIZE];
  snprintf(buf, sizeof(buf), "ns__hello serviciu apelat");
  logger_log(buf);

  // test libstemmer
  struct sb_stemmer *stm = sb_stemmer_new("english", "UTF_8");

  *result = (char *)soap_malloc(soap, BUFFER_SIZE);
  snprintf(*result, BUFFER_SIZE, "Hello %s", name);
  return SOAP_OK;
}

int ns__register(struct soap *soap, char *username, char *password,
                 int *result) {
  char buf[BUFFER_SIZE];
  snprintf(buf, sizeof(buf), "User inregistrat: %s", username);
  logger_log(buf);
  *result = 0;
  return SOAP_OK;
}

int ns__login(struct soap *soap, char *username, char *password, char **token) {
  char buf[BUFFER_SIZE];
  snprintf(buf, sizeof(buf), "User logat %s", username);
  logger_log(buf);
  *token = soap_malloc(soap, BUFFER_SIZE);
  strcpy(*token, "token");
  return SOAP_OK;
}

void logger_init(const char *log_dir) {
  mkdir(log_dir, 0755);

  int pipe_fds[2];
  if (pipe(pipe_fds) < 0)
    return;

  if (fork() == 0) {
    close(pipe_fds[1]);

    char filename[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S.log", tm_info);
    snprintf(filename, sizeof(filename), "%s/%s", log_dir, time_str);

    FILE *f = fopen(filename, "w");
    if (!f)
      exit(1);

    char buffer[BUFFER_SIZE];
    ssize_t n;
    while ((n = read(pipe_fds[0], buffer, sizeof(buffer) - 1)) > 0) {
      buffer[n] = '\0';
      fprintf(f, "%s\n", buffer);
      fflush(f);
    }

    fclose(f);
    close(pipe_fds[0]);
    exit(0);
  }

  close(pipe_fds[0]);
  log_fd = pipe_fds[1];
}

void logger_log(const char *msg) {
  if (log_fd < 0)
    return;

  char timestamp[BUFFER_SIZE];
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

  char final_buf[1024];
  int len = snprintf(final_buf, sizeof(final_buf), "[%s] %s", timestamp, msg);
  write(log_fd, final_buf, len);
}

void start_server(int port, const char *log_dir) {
  struct soap soap;
  soap_init(&soap);

  logger_init(log_dir);

  if (soap_bind(&soap, NULL, port, 100) < 0) {
    soap_print_fault(&soap, stderr);
    return;
  }

  printf("[SERVER] Server ruleaza pe port %d...\n", port);
  logger_log("Server initializat.");

  while (1) {
    if (soap_accept(&soap) < 0)
      break;
    soap_serve(&soap);
    soap_end(&soap);
  }

  close(log_fd);
  soap_done(&soap);
}