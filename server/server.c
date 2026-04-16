#include "server.h"
#include "soapH.h"
#include "service.nsmap"
#include "soapStub.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

// file descriptor folosit pentru logging
int log_fd = -1;

int ns__hello(struct soap *soap, char *name, char **result)
{
    char buf[BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "ns__hello service called");
    logger_log(buf);

    *result = (char*)soap_malloc(soap, BUFFER_SIZE);
    snprintf(*result, BUFFER_SIZE, "Hello %s", name);
    return SOAP_OK;
}

// TODO:
int ns__register(struct soap *soap, char* username, char *password, int* result) {
    char buf[BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "Registered user: %s", username);
    logger_log(buf);

    *result = 0;
    return SOAP_OK;
}

// TODO:
int ns__login(struct soap *soap, char *username, char *password, char **token) {
    char buf[BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "Logged in user: %s", username);
    logger_log(buf);

    *token = soap_malloc(soap, BUFFER_SIZE);
    strcpy(*token, "token");
    return SOAP_OK;
}

void logger_init() {
    mkdir("logs", 0755);

    int pipe_fds[2];
    if (pipe(pipe_fds) < 0) return;

    if (fork() == 0) {
        close(pipe_fds[1]);

        char filename[BUFFER_SIZE];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S.log", tm_info);
        
        snprintf(filename, sizeof(filename), "logs/%s", time_str);

        FILE *f = fopen(filename, "w");
        if (!f) exit(1);

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
    if (log_fd < 0) return;

    char timestamp[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    char final_buf[1024];
    int len = snprintf(final_buf, sizeof(final_buf), "[%s] %s", timestamp, msg);
    
    write(log_fd, final_buf, len);
}

void start_server(int port) {
    struct soap soap;
    soap_init(&soap);

    logger_init();

    if (soap_bind(&soap, NULL, port, 100) < 0) {
        soap_print_fault(&soap, stderr);
        return;
    }

    printf("Running server on port %d...\n", port);
    logger_log("Server initialized.");

    while (1) {
        if (soap_accept(&soap) < 0) break;
        soap_serve(&soap);
        soap_end(&soap);
    }

    close(log_fd);
    soap_done(&soap);
}