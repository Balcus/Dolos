#include "client.h"
#include "service.nsmap"
#include "soapH.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <libgen.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOAP_ENDPOINT "http://localhost:8080"
#define FILE_SERVER_PORT 8081
#define BUFFER_SIZE 4096

void client_call_hello(const char *name) {
  struct soap soap;
  soap_init(&soap);

  char *result = NULL;

  if (soap_call_ns__hello(&soap, SOAP_ENDPOINT, NULL, (char *)name, &result) ==
      SOAP_OK) {
    printf("[CLIENT] Server response: %s\n", result);
  } else {
    soap_print_fault(&soap, stderr);
  }

  soap_destroy(&soap);
  soap_end(&soap);
  soap_done(&soap);
}

/*
 * Trimite fisiere la server prin TCP pe portul FILE_SERVER_PORT.
 * Protocol:
 *   Client -> Server:
 *     - uint32_t num_files
 *     - pentru fiecare fisier:
 *         - uint32_t filename_len
 *         - char[filename_len] filename (doar numele, fara cale)
 *         - uint32_t file_size
 *         - char[file_size] file_content
 *   Server -> Client (echo):
 *     - acelasi format
 */
void client_send_files(const char **filepaths, int count) {
  /* Creare socket TCP */
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("[CLIENT] socket");
    return;
  }

  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(FILE_SERVER_PORT);
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("[CLIENT] connect");
    close(sockfd);
    return;
  }

  printf("[CLIENT] Conectat la server pe portul %d\n", FILE_SERVER_PORT);

  /* Trimite numarul de fisiere */
  uint32_t num_files = (uint32_t)count;
  write(sockfd, &num_files, sizeof(num_files));

  /* Trimite fiecare fisier */
  for (int i = 0; i < count; i++) {
    /* Deschide fisierul cu apel de sistem */
    int fd = open(filepaths[i], O_RDONLY);
    if (fd < 0) {
      fprintf(stderr, "[CLIENT] Nu pot deschide fisierul: %s\n", filepaths[i]);
      close(sockfd);
      return;
    }

    /* Determina dimensiunea fisierului */
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    /* Extrage doar numele fisierului */
    char path_copy[BUFFER_SIZE];
    strncpy(path_copy, filepaths[i], sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    char *fname = basename(path_copy);
    uint32_t fname_len = (uint32_t)strlen(fname);

    /* Trimite: filename_len, filename, file_size, file_content */
    write(sockfd, &fname_len, sizeof(fname_len));
    write(sockfd, fname, fname_len);

    uint32_t fsize = (uint32_t)file_size;
    write(sockfd, &fsize, sizeof(fsize));

    /* Citeste si trimite continutul fisierului */
    char buf[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
      write(sockfd, buf, bytes_read);
    }

    close(fd);
    printf("[CLIENT] Trimis: %s (%u bytes)\n", fname, fsize);
  }

  /* Primeste fisierele inapoi de la server (echo) */
  printf("[CLIENT] Astept raspuns de la server...\n");

  uint32_t recv_count = 0;
  read(sockfd, &recv_count, sizeof(recv_count));

  for (uint32_t i = 0; i < recv_count; i++) {
    uint32_t fname_len = 0;
    read(sockfd, &fname_len, sizeof(fname_len));

    char fname[BUFFER_SIZE];
    read(sockfd, fname, fname_len);
    fname[fname_len] = '\0';

    uint32_t fsize = 0;
    read(sockfd, &fsize, sizeof(fsize));

    /* Citeste continutul primit si il afiseaza ca confirmare */
    uint32_t remaining = fsize;
    char buf[BUFFER_SIZE];
    printf("[CLIENT] Primit inapoi: %s (%u bytes)\n", fname, fsize);

    while (remaining > 0) {
      ssize_t to_read = remaining < sizeof(buf) ? remaining : sizeof(buf);
      ssize_t n = read(sockfd, buf, to_read);
      if (n <= 0)
        break;
      remaining -= n;
    }
  }

  close(sockfd);
  printf("[CLIENT] Transfer complet.\n");
}
