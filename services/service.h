// definitia serviciilor soap pentru generatorul soapcpp2

//gsoap ns service name: service
//gsoap ns service protocol: SOAP
//gsoap ns service style: rpc
//gsoap ns service encoding: encoded
//gsoap ns service namespace: http://example.com/service.wsdl
//gsoap ns schema namespace: urn:service

// serviciul hello - primeste un nume si returneaza un mesaj de salut
//gsoap ns service method: Receives the name from the client and sends a hello message back
int ns__hello(char *name, char **result);

// serviciul register - creeaza un utilizator nou pe server
//gsoap ns service method: Creates new user on the server
int ns__register(char* username, char *password, int* result);

// serviciul login - autentifica un utilizator si returneaza un token
//gsoap ns service method: Login for the user
int ns__login(char* username, char *password, char** token);