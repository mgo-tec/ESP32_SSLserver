/*
  ESP32_SSLserver.h
  Beta version 1.0.0

This library has modified WiFiServer library(by the Arduino core for the ESP32) and OpenSSL server Example  (by ESP-IDF).

License : LGPL-2.1

Copyright (c) 2018 mgo-tec

My Blog Site --> https://www.mgo-tec.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Reference LGPL-2.1 license statement --> https://opensource.org/licenses/LGPL-2.1   

Server.h(WiFiServer.h) - Server class for Raspberry Pi
  Copyright (c) 2016 Hristo Gochkov  All right reserved.

openssl_server_example_main.c ( ESP-IDF ) - Modification.
Licensed under The Public Domain
*/

#ifndef ESP32_SSL_SERVER_H_INCLUDED
#define ESP32_SSL_SERVER_H_INCLUDED

#include <Arduino.h>
#include <WiFi.h>
#include <lwip/sockets.h>
#include <FS.h>
#include <SPIFFS.h>
#include <openssl/ssl.h>

class Esp32SSLServer
{
public:
  Esp32SSLServer( uint16_t port = 443, uint16_t ssl_recv_buf_len = 1024, uint8_t max_clients = 4 ){
    mp_sockfd = -1;
    mp_accepted_sockfd = -1;
    mp_https_port = port;
    mp_max_clients = max_clients;
    mp_isListening = false;
    mp_isNo_delay = false;
    mp_ssl_recv_buf_len = ssl_recv_buf_len;
  };
  ~Esp32SSLServer(){
    end();
  };

private:
  uint16_t mp_ssl_recv_buf_len;
  uint16_t mp_https_port;

  SSL_CTX *mpCtx;
  SSL *mpSsl;

  int mp_new_sockfd;
  int mp_befor_sockfd;

  //----The same function name as WiFiServer----------
  int mp_sockfd;
  uint8_t mp_max_clients;
  bool mp_isListening;
  int mp_accepted_sockfd;
  bool mp_isNo_delay;

public:
  void beginSPIFFS( const char* cert_path, const char* prvt_key_path );
  boolean available(); //WiFiServer.cpp function

  String readStrClient();
  void writeStrClient( String str );
  void stopClient();

  String strBodyHtml( String message2 );
  String strResponseFavicon();
  String strResponseHeader();
  uint16_t readPkiFileSPIFFS( const char *path, char *pki_cstr );
  void stop();
  //void close(); //※これは別のライブラリで使用しているので使えない。
  void end();

};

#endif
