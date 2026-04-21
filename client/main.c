#include "client.h"
#include <argtable2.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  ClientConfig cfg;
  config_load("config/client.cfg", &cfg);

  char endpoint[512];
  snprintf(endpoint, sizeof(endpoint), "http://%s:%d", cfg.host, cfg.port);

  struct arg_file *files =
      arg_filen("f", "files", "<file>", 0, 100, "fisiere de trimis la server");
  struct arg_lit *help = arg_lit0("h", "help", "afiseaza mesajul de ajutor");
  struct arg_end *end = arg_end(20);
  void *argtable[] = {files, help, end};

  if (arg_nullcheck(argtable) != 0) {
    fprintf(stderr, "error: insufficient memory\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  int nerrors = arg_parse(argc, argv, argtable);

  if (help->count > 0) {
    printf("Usage: client [OPTIONS] <file>...\n\n");
    printf("Options:\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    printf("\nExamples:\n");
    printf("  client -f file1.txt -f file2.txt   Trimite fisiere la server\n");
    printf("  client                              Apel hello implicit\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
  }

  if (nerrors > 0) {
    arg_print_errors(stderr, end, "client");
    printf("Usage: client");
    arg_print_syntax(stdout, argtable, "\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  if (files->count > 0) {
    client_send_files(files->filename, files->count, &cfg);
  } else {
    client_call_hello("William", endpoint);
  }

  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
  return 0;
}