#include "server.h"
#include "soapH.h"
#include "sayhello.nsmap"

#include <stdio.h>
#include <stdlib.h>

// -----------------------------
// SOAP SERVICE IMPLEMENTATION
// -----------------------------
int ns__hello(struct soap *soap, char *name, char **result)
{
    *result = (char*)soap_malloc(soap, 256);
    snprintf(*result, 256, "Hello %s", name);
    return SOAP_OK;
}

// -----------------------------
// SERVER CORE LOGIC
// -----------------------------
void start_server(int port)
{
    struct soap soap;
    soap_init(&soap);

    int master = soap_bind(&soap, NULL, port, 100);

    if (master < 0)
    {
        soap_print_fault(&soap, stderr);
        return;
    }

    printf("[SERVER] Running on port %d\n", port);

    while (1)
    {
        int slave = soap_accept(&soap);
        if (slave < 0)
        {
            soap_print_fault(&soap, stderr);
            break;
        }

        soap_serve(&soap);   // dispatch request
        soap_end(&soap);     // cleanup per request
    }

    soap_done(&soap);
}