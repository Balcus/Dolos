#include "server.h"
#include "service.nsmap"
#include "soapH.h"
#include "soapStub.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// file descriptor folosit pentru logging
int log_fd = -1;

/* Forward declaration */
static void *start_file_server_thread(void *arg);

int ns__hello(struct soap *soap, char *name, char **result) {
  char buf[BUFFER_SIZE];
  snprintf(buf, sizeof(buf), "ns__hello serviciu apelat");
  logger_log(buf);

  *result = (char *)soap_malloc(soap, BUFFER_SIZE);
  snprintf(*result, BUFFER_SIZE, "Hello %s", name);
  return SOAP_OK;
}

// TODO:
int ns__register(struct soap *soap, char *username, char *password,
                 int *result) {
  char buf[BUFFER_SIZE];
  snprintf(buf, sizeof(buf), "User inregistrat: %s", username);
  logger_log(buf);

  *result = 0;
  return SOAP_OK;
}

// TODO:
int ns__login(struct soap *soap, char *username, char *password, char **token) {
  char buf[BUFFER_SIZE];
  snprintf(buf, sizeof(buf), "User logat: %s", username);
  logger_log(buf);

  *token = soap_malloc(soap, BUFFER_SIZE);
  strcpy(*token, "token");
  return SOAP_OK;
}

void logger_init() {
  mkdir("logs", 0755);

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

    snprintf(filename, sizeof(filename), "logs/%s", time_str);

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
  int len = snprintf(final_buf, sizeof(final_buf), "[%s] %s\n", timestamp, msg);

  write(log_fd, final_buf, len);
}

void start_server(int port) {
  struct soap soap;
  soap_init(&soap);

  logger_init();

  /* Porneste serverul TCP pentru fisiere intr-un thread separat */
  pthread_t file_thread;
  int *file_port = malloc(sizeof(int));
  *file_port = port + 1; /* portul pentru fisiere = SOAP port + 1 */
  pthread_create(&file_thread, NULL, start_file_server_thread, file_port);

  if (soap_bind(&soap, NULL, port, 100) < 0) {
    soap_print_fault(&soap, stderr);
    return;
  }

  printf("Server-ul ruleaza pe port-ul %d...\n", port);
  printf("Server de fisiere ruleaza pe port-ul %d...\n", port + 1);

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

/*
 * Functie wrapper pentru thread - porneste serverul de fisiere TCP.
 */
static void *start_file_server_thread(void *arg) {
  int port = *(int *)arg;
  free(arg);
  start_file_server(port);
  return NULL;
}

/*
 * Server TCP dedicat transferului de fisiere.
 * Protocol:
 *   - Primeste: num_files, apoi pentru fiecare: fname_len, fname, fsize,
 * content
 *   - Trimite inapoi (echo): acelasi format
 */
void start_file_server(int port) {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("[FILE SERVER] socket");
    return;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("[FILE SERVER] bind");
    close(server_fd);
    return;
  }

  if (listen(server_fd, 10) < 0) {
    perror("[FILE SERVER] listen");
    close(server_fd);
    return;
  }

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
      perror("[FILE SERVER] accept");
      continue;
    }

    logger_log("Conexiune pentru file transfer acceptata.");

    /* Citeste numarul de fisiere */
    uint32_t num_files = 0;
    read(client_fd, &num_files, sizeof(num_files));

    /* Structura temporara pentru stocarea fisierelor primite */
    typedef struct {
      char filename[1024];
      uint32_t fname_len;
      uint32_t size;
      char *data;
    } FileEntry;

    FileEntry *entries = malloc(num_files * sizeof(FileEntry));

    for (uint32_t i = 0; i < num_files; i++) {
      /* Citeste filename_len + filename */
      read(client_fd, &entries[i].fname_len, sizeof(uint32_t));
      read(client_fd, entries[i].filename, entries[i].fname_len);
      entries[i].filename[entries[i].fname_len] = '\0';

      /* Citeste dimensiunea fisierului */
      read(client_fd, &entries[i].size, sizeof(uint32_t));

      /* Citeste continutul fisierului */
      entries[i].data = malloc(entries[i].size);
      uint32_t remaining = entries[i].size;
      uint32_t offset = 0;
      while (remaining > 0) {
        ssize_t n = read(client_fd, entries[i].data + offset, remaining);
        if (n <= 0)
          break;
        offset += n;
        remaining -= n;
      }

      char log_buf[1024];
      snprintf(log_buf, sizeof(log_buf), "Fisier primit: %s (%u bytes)",
               entries[i].filename, entries[i].size);
      logger_log(log_buf);
    }

    /* Trimite fisierele inapoi (echo) */
    write(client_fd, &num_files, sizeof(num_files));

    for (uint32_t i = 0; i < num_files; i++) {
      write(client_fd, &entries[i].fname_len, sizeof(uint32_t));
      write(client_fd, entries[i].filename, entries[i].fname_len);
      write(client_fd, &entries[i].size, sizeof(uint32_t));
      write(client_fd, entries[i].data, entries[i].size);
      free(entries[i].data);
    }

    free(entries);
    close(client_fd);
    logger_log("Transfer de fisiere complet.");
  }

  close(server_fd);
}
