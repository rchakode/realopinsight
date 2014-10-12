#!/bin/bash

/usr/share/ssl/misc/CA.sh -newca
openssl req -new -nodes -keyout newreq.pem -out newreq.pem
/usr/share/ssl/misc/CA.sh -sign
cp demoCA/cacert.pem /etc/ssl/certs/ldap.cert
mv newcert.pem /etc/ssl/certs/ldap.csr
mv newreq.pem /etc/ssl/certs/ldap.key
chmod 600 /etc/ssl/certs/ldap.key
