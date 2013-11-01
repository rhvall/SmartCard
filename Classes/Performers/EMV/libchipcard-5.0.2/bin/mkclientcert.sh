#!/bin/bash 
 
cd certs 
echo "Client Cert vorbereiten..." 
openssl req -new -newkey rsa:2048 -out $1_csr.pem -nodes -keyout $1_key.pem -days 3650 
 
echo "" 
echo "Client Cert erstellen..." 
openssl x509 -req -in $1_csr.pem -out $1_cert.pem -CA ca_cert.pem -CAkey ca_key.pem -CAserial serial -days 3650 
echo "" 
echo "CSR Cert loeschen..." 
rm $1_csr.pem 
echo "Clientcert $1_cert.pem und Clientkey $1_key.pem erstellt..." 
cd .. 
 
