//gsoap ns service name: service
//gsoap ns service protocol: SOAP
//gsoap ns service style: rpc
//gsoap ns service encoding: encoded
//gsoap ns service namespace: http://example.com/service.wsdl
//gsoap ns schema namespace: urn:service

//gsoap ns service method: Receives the name from the client and sends a hello message back
int ns__hello(char *name, char **result);

//gsoap ns service method: Creates new user on the server
int ns__register(char* username, char *password, int* result);

//gsoap ns service method: Login for the user
int ns__login(char* username, char *password, char** token);