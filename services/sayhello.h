//gsoap ns service name: sayhello Service that receives the name from the client and greets him back
//gsoap ns service protocol: SOAP
//gsoap ns service style: rpc
//gsoap ns service encoding: encoded
//gsoap ns service namespace: http://example.com/sayhello.wsdl
//gsoap ns schema namespace: urn:sayhello

//gsoap ns service method: receives the name from the client and sends a hello message back
int ns__hello(char *name, char **result);