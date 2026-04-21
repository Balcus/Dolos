#ifndef CLIENT_H
#define CLIENT_H

void client_call_hello(const char *name);
void client_send_files(const char **filepaths, int count);

#endif