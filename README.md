# ESP32_SSLserver
This is Arduino core for the ESP32 library using SPIFFS of ESP-WROOM-32 or M5Stack.  
https://github.com/espressif/arduino-esp32  
It can be used with DNS or mDNS library.  
Certificates made with OpenSSL etc. are necessary.  
  
DNS SSL server : soft AP mode  
mDNS server : STA mode OK!  
  
Reference Blog:  
https://www.mgo-tec.com/blog-entry-dns-mdns-ssl-server-esp32-m5stack.html  
  
# Change log:
(1.0.0)  
New Release.  
  
# 【更新履歴】(Japanese)
(1.0.0)  
新規リリース   
  
# Credits and license
*Licensed under LGPL-2.1   
  
Copyright (c) 2018 Mgo-tec  
  
This library has modified WiFiServer library(by the Arduino core for the ESP32) and OpenSSL server Example  (by ESP-IDF).  
  
Server.h(WiFiServer.h) - Server class for Raspberry Pi  
  Copyright (c) 2016 Hristo Gochkov  All right reserved.  
  
openssl_server_example_main.c ( ESP-IDF ) - Modification.  
Licensed under The Public Domain  
  
# My Blog: 
Usage is Japanese, please visit my blog.  
https://www.mgo-tec.com  
