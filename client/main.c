// includerea headerului clientului si a bibliotecilor necesare
#include "client.h"
#include <argtable2.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  // incarcam configuratia clientului din fisierul cfg
  ClientConfig cfg;
  config_load("config/client.cfg", &cfg);

  // construim adresa endpoint-ului serverului (http://host:port)
  char endpoint[512];
  snprintf(endpoint, sizeof(endpoint), "http://%s:%d", cfg.host, cfg.port);

  // definim argumentele acceptate din linia de comanda
  // -f pentru fisiere, -h pentru ajutor
  struct arg_file *files =
      arg_filen("f", "files", "<file>", 0, 100, "fisiere de trimis la server");
  struct arg_lit *help = arg_lit0("h", "help", "afiseaza mesajul de ajutor");
  struct arg_end *end = arg_end(20);
  void *argtable[] = {files, help, end};

  // verificam daca tabelul de argumente a fost alocat corect
  if (arg_nullcheck(argtable) != 0) {
    fprintf(stderr, "error: insufficient memory\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  // parsam argumentele primite din linia de comanda
  int nerrors = arg_parse(argc, argv, argtable);

  // daca utilizatorul a cerut ajutor, afisam modul de utilizare
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

  // daca sunt erori de parsare, afisam ce a fost gresit
  if (nerrors > 0) {
    arg_print_errors(stderr, end, "client");
    printf("Usage: client");
    arg_print_syntax(stdout, argtable, "\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  // daca au fost specificate fisiere, le trimitem la server
  // altfel, facem un apel hello de test
  if (files->count > 0) {
    client_send_files(files->filename, files->count, &cfg);
  } else {
    client_call_hello("William", endpoint);
  }

  // eliberam memoria alocata pentru tabelul de argumente
  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
  return 0;
}