#!/bin/bash
echo "key.pem 是私钥文件, cert.pem 是证书文件; 请注意保管! (in `certs`文件夹): \n"
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes