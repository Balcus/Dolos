#include "client.h"
#include "soapH.h"
#include "sayhello.nsmap"

#include <stdio.h>
#include <stdlib.h>

void client_call_hello(const char *name)
{
    struct soap soap;
    soap_init(&soap);

    char *result = NULL;

    // Call generated SOAP function
    if (soap_call_ns__hello(
            &soap,
            "http://localhost:8080",
            NULL,
            (char*)name,
            &result) == SOAP_OK)
    {
        printf("[CLIENT] Server response: %s\n", result);
    }
    else
    {
        soap_print_fault(&soap, stderr);
    }

    soap_destroy(&soap);
    soap_end(&soap);
    soap_done(&soap);
}