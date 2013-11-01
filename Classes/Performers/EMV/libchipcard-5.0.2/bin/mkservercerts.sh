#!/bin/bash 
 
mkdir certs 
cd certs 
echo "CA Cert erstellen..." 
echo "Wichtig: Common Name sollte \"libchipcard\" lauten." 
openssl genrsa -aes256 -out ca_key.pem 2048 
openssl req -new -x509 -days 3650 -key ca_key.pem -out ca_cert.pem -set_serial 1 
chmod 700 ../certs 
touch serial 
echo "01" > serial 
 
echo "" 
echo "Server Cert erstellen..." 
echo "Wichtig: Common Name sollte IP-Adresse des Servers haben." 
echo "" 
openssl req -new -newkey rsa:2048 -out server_csr.pem -nodes -keyout server_key.pem -days 3650 
openssl x509 -req -in server_csr.pem -out server_cert.pem -CA ca_cert.pem -CAkey ca_key.pem -CAserial serial -days 3650 
rm server_csr.pem 
 
echo "" 
echo "Zufallszahlen erstellen..." 
openssl dhparam -out ../dh/dh2048.pem 2048 
echo "" 
 
echo "Client Certs mit folgendem Commando vorbereiten:" 
echo "./mkclientcert.sh <client-IP>" 

