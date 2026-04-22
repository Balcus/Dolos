// includerea headerului serverului si a tuturor bibliotecilor necesare
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

// descriptorul de fisier global pentru scrierea in pipe-ul de logging
int log_fd = -1;

// functia care incarca configuratia serverului dintr-un fisier cfg
void config_load(const char *path, ServerConfig *cfg) {
  config_t lib_cfg;
  config_init(&lib_cfg);

  // setam valorile implicite pentru port si directorul de loguri
  cfg->port = 8080;
  strcpy(cfg->log_dir, "logs");

  // incercam sa citim fisierul de configurare
  // daca nu reusim, afisam eroare si pastram valorile default
  if (!config_read_file(&lib_cfg, path)) {
    fprintf(stderr, "Eroare la configurare %s:%d - %s, folosim valorile default\n",
            config_error_file(&lib_cfg), config_error_line(&lib_cfg),
            config_error_text(&lib_cfg));
    config_destroy(&lib_cfg);
    return;
  }

  // citim portul din configurare, daca exista
  int port;
  if (config_lookup_int(&lib_cfg, "port", &port))
    cfg->port = port;

  // citim directorul de loguri din configurare, daca exista
  const char *log_dir;
  if (config_lookup_string(&lib_cfg, "log_dir", &log_dir))
    strncpy(cfg->log_dir, log_dir, sizeof(cfg->log_dir) - 1);

  // eliberam resursele libconfig
  config_destroy(&lib_cfg);
}

// implementarea serviciului soap hello - primeste un nume si returneaza un salut
int ns__hello(struct soap *soap, char *name, char **result) {
  // logam faptul ca serviciul hello a fost apelat
  char buf[BUFFER_SIZE];
  snprintf(buf, sizeof(buf), "ns__hello serviciu apelat");
  logger_log(buf);

  // cream un stemmer de test pentru limba engleza (demo libstemmer)

  // test libstemmer
  struct sb_stemmer *stm = sb_stemmer_new("english", "UTF_8");

  // alocam memorie pentru raspuns si construim mesajul de salut
  *result = (char *)soap_malloc(soap, BUFFER_SIZE);
  snprintf(*result, BUFFER_SIZE, "Hello %s", name);
  return SOAP_OK;
}

// implementarea serviciului soap register - inregistreaza un utilizator nou
int ns__register(struct soap *soap, char *username, char *password,
                 int *result) {
  // logam inregistrarea utilizatorului
  char buf[BUFFER_SIZE];
  snprintf(buf, sizeof(buf), "User inregistrat: %s", username);
  logger_log(buf);

  // returnam cod de succes (0 = totul a mers bine)
  *result = 0;
  return SOAP_OK;
}

// implementarea serviciului soap login - autentifica un utilizator
int ns__login(struct soap *soap, char *username, char *password, char **token) {
  // logam autentificarea utilizatorului
  char buf[BUFFER_SIZE];
  snprintf(buf, sizeof(buf), "User logat %s", username);
  logger_log(buf);

  // alocam memorie si returnam un token de sesiune
  *token = soap_malloc(soap, BUFFER_SIZE);
  strcpy(*token, "token");
  return SOAP_OK;
}

// initializeaza sistemul de logging folosind fork si pipe
// creeaza un proces copil dedicat scrierii in fisierul de log
void logger_init(const char *log_dir) {
  // cream directorul de loguri daca nu exista
  mkdir(log_dir, 0755);

  // cream un pipe pentru comunicarea intre procesul principal si cel de logging
  int pipe_fds[2];
  if (pipe(pipe_fds) < 0)
    return;

  // facem fork: procesul copil va scrie in fisierul de log
  if (fork() == 0) {
    // procesul copil: inchidem capatul de scriere, nu avem nevoie de el
    close(pipe_fds[1]);

    // construim numele fisierului de log pe baza datei si orei curente
    char filename[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S.log", tm_info);
    snprintf(filename, sizeof(filename), "%s/%s", log_dir, time_str);

    // deschidem fisierul de log pentru scriere
    FILE *f = fopen(filename, "w");
    if (!f)
      exit(1);

    // citim in bucla din pipe si scriem in fisierul de log
    char buffer[BUFFER_SIZE];
    ssize_t n;
    while ((n = read(pipe_fds[0], buffer, sizeof(buffer) - 1)) > 0) {
      buffer[n] = '\0';
      fprintf(f, "%s\n", buffer);
      fflush(f);
    }

    // inchidem fisierul si capatul de citire, apoi iesim din procesul copil
    fclose(f);
    close(pipe_fds[0]);
    exit(0);
  }

  // procesul parinte: inchidem capatul de citire si salvam cel de scriere
  close(pipe_fds[0]);
  log_fd = pipe_fds[1];
}

// scrie un mesaj in log cu timestamp-ul curent
void logger_log(const char *msg) {
  // daca pipe-ul nu e initializat, nu facem nimic
  if (log_fd < 0)
    return;

  // generam timestamp-ul curent
  char timestamp[BUFFER_SIZE];
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

  // formatam mesajul final cu timestamp si il scriem in pipe
  char final_buf[1024];
  int len = snprintf(final_buf, sizeof(final_buf), "[%s] %s", timestamp, msg);
  write(log_fd, final_buf, len);
}

// functia principala care porneste serverul soap
void start_server(int port, const char *log_dir) {
  // initializam structura soap
  struct soap soap;
  soap_init(&soap);

  // pornim sistemul de logging (creeaza procesul copil)
  logger_init(log_dir);

  // facem bind pe portul configurat, cu backlog de 100 conexiuni
  if (soap_bind(&soap, NULL, port, 100) < 0) {
    soap_print_fault(&soap, stderr);
    return;
  }

  // afisam confirmarea pornirii si logam evenimentul
  printf("[SERVER] Server ruleaza pe port %d...\n", port);
  logger_log("Server initializat.");

  // bucla principala: asteptam conexiuni si procesam cererile soap
  while (1) {
    // acceptam o conexiune noua de la un client
    if (soap_accept(&soap) < 0)
      break;
    // procesam cererea soap primita
    soap_serve(&soap);
    // eliberam resursele alocate pentru cererea curenta
    soap_end(&soap);
  }

  // inchidem pipe-ul de logging si eliberam resursele soap
  close(log_fd);
  soap_done(&soap);
}