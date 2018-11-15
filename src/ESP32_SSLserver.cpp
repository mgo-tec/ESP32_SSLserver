/*
  ESP32_SSLserver.cpp
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

openssl_server_example_main.c ( ESP-IDF )
Licensed under The Public Domain
*/

#include "ESP32_SSLserver.h"

//******************************************
void Esp32SSLServer::beginSPIFFS( const char* cert_path, const char* prvt_key_path ){
  int ret;
  uint16_t len = 0;
  char cert_or_prvt_temp[5120] = {};
  //※型宣言はgoto文の前に無ければならない。gotoをまたいで宣言不可なので注意

  log_e( "SSL server context create ......" );
    Serial.println( F("SSL server context create ......") );

    mpCtx = SSL_CTX_new( TLS_server_method() );

    if ( !mpCtx ) {
        log_e( "failed goto1" );
        goto failed1;
    }
    log_v( "OK-1" );

    log_v( "SSL server context set own certification......" );

    len = Esp32SSLServer::readPkiFileSPIFFS( cert_path, cert_or_prvt_temp );
    Serial.printf( "cert_length = %d\r\n", len );
    log_v( "cert\r\n%s", (const char*)cert_or_prvt_temp );

    ret = SSL_CTX_use_certificate_ASN1( mpCtx, len, (const unsigned char*)cert_or_prvt_temp );
    if ( !ret ) {
        log_e( "SSL_CTX_use_CA failed goto2" );
        goto failed2;
    }
    log_v( "OK-2" );

    log_v( "SSL server context set private key......" );

    len = Esp32SSLServer::readPkiFileSPIFFS( prvt_key_path, cert_or_prvt_temp );
    log_v( "prvt_key_length = %d", len );
    log_v( "prvt key\r\n%s", (const char*)cert_or_prvt_temp );

    ret = SSL_CTX_use_PrivateKey_ASN1( 0, mpCtx, (const unsigned char*)cert_or_prvt_temp, len );
    if ( !ret ) {
        log_e( "SSL_CTX_use_PrivateKey failed goto2" );
        goto failed2;
    }
    log_v( "OK-3" );

    log_v( "SSL server create socket ......" );

    mp_sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( mp_sockfd < 0 ) {
        log_e( "failed goto2" );
        goto failed2;
    }
    log_v( "OK-4" );

    log_v( "SSL server socket bind ......" );

    struct sockaddr_in sock_addr;
    memset( &sock_addr, 0, sizeof(sock_addr) );
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = 0;
    //sock_addr.sin_addr.s_addr = INADDR_ANY;
    sock_addr.sin_port = htons( mp_https_port );

    ret = bind( mp_sockfd, (struct sockaddr*)&sock_addr, sizeof(sock_addr) );
    log_v( "bind ret = %d", ret );
    if ( ret > 0 ) {
        log_e( "failed goto3" );
        goto failed3;
    }
    log_v( "OK-5" );

    log_v( "SSL server socket listen ......" );

    ret = listen( mp_sockfd, mp_max_clients );
    if ( ret ) {
        log_e( "failed goto3" );
        goto failed3;
    }
    log_v( "OK-6" );
    fcntl( mp_sockfd, F_SETFL, O_NONBLOCK ); //esp-idf opensslコードにはなく、WiFiServer にある関数。

    mp_isListening = true;
    mp_isNo_delay = false;
    mp_accepted_sockfd = -1;
    return;

failed3:
    log_v( "close(mp_sockfd);" );
    close( mp_sockfd );
    lwip_close_r( mp_sockfd );
    mp_sockfd = -1;
failed2:
    log_v( "SSL_CTX_free(mpCtx);" );
    SSL_CTX_free( mpCtx );
    mpCtx = NULL;
failed1:
    log_v( "exit beginSPIFFS" );
    //vTaskDelete(NULL);
    return ;
}
//*******************************************
boolean Esp32SSLServer::available(){
  if( !mp_isListening ){
    return false;
  }

  int client_sock;
  if ( mp_accepted_sockfd >= 0 ) {
    client_sock = mp_accepted_sockfd;
    mp_accepted_sockfd = -1;
  } else {
    struct sockaddr_in _client;
    int cs = sizeof( struct sockaddr_in );
    client_sock = accept( mp_sockfd, (struct sockaddr *)&_client, (socklen_t*)&cs );
  }

  if( client_sock >= 0 ){
    int val = 1;
    if( setsockopt( client_sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&val, sizeof(int) ) == ESP_OK ) {
      val = mp_isNo_delay;

      if( setsockopt( client_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(int) ) == ESP_OK ){
        mp_new_sockfd = client_sock;
        mp_befor_sockfd = client_sock;
        log_v( "mp_new_sockfd = %d", mp_new_sockfd );
        log_v( "available OK!" );
        return true;
      }
    }
  }
  return false;
}
//************************************************
String Esp32SSLServer::readStrClient(){
  int ret;
  char recv_buf[ mp_ssl_recv_buf_len ];
  //※型宣言はgoto文の前に無ければならない。gotoをまたいで宣言不可なので注意

  if ( !mp_isListening ) {
      log_e( "mp_isListening failed" );
      return "";
  }
  log_v( "sendSimpleHtml in..." );
  log_v( "SSL server create ......" );
  mpSsl = SSL_new( mpCtx );
  if ( !mpSsl ) {
      log_e( "failed goto5" );
      goto failed5;
  }
  log_v( "OK-7" );
  log_v( "SSL server socket accept client ......" );

  if ( mp_new_sockfd < 0 ) {
      log_e( "failed goto5" );
      goto failed5;
  }
  log_v( "OK-8" );

  SSL_set_fd( mpSsl, mp_new_sockfd );

  log_v( "SSL server accept client ......" );

  ret = SSL_accept( mpSsl );
  log_v( "SSL_accept(ssl) ret=%d", ret );
  if ( !ret ) {
      log_e( "failed goto5" );
      goto failed5;
  }
  log_v( "OK-9" );
  log_v( "SSL server read message ......" );
  Serial.println("Wait Access of another terminal...");

  /* Android Google Chrome の場合、接続完了してから、何度もここまでプログラムが走る。
   * 15～30 second くらい待てば、ブラウザ側からタイムアウトしてくる。
   * よって、直ぐに別のブラウザ（例えば iOS)などで、アクセスしてもフリーズしてしまう。
   * available関数を置いても良いが、iOS は必ず false になるので、うまくいかない。
   * 結果的に、Android Google Chrome でアクセスした後は、別の端末でアクセスしない方が無難という考えに落ち着いた。
   */

  memset( recv_buf, 0, mp_ssl_recv_buf_len ); //recv_buf を0x00で初期化
  ret = SSL_read( mpSsl, recv_buf, mp_ssl_recv_buf_len - 1 );
  if ( ret <= 0 ) {
    goto failed5;
  }
  return String( recv_buf );

failed5:
  log_v( "SSL_shutdown" );
  SSL_shutdown( mpSsl );
  log_v( "close(mp_new_sockfd);" );
  close(mp_new_sockfd); //これ重要。これが無いと再起動を繰り返す。
  mp_new_sockfd = -1;
  log_v( "SSL_free(mpSsl);" );
  SSL_free( mpSsl );
  mpSsl = NULL;
  Serial.println("Access of another terminal is OK!");
  return "";
}
//************************************************
void Esp32SSLServer::writeStrClient( String str ){
  if ( !mpSsl ) {
    log_e( "failed mpSSL." );
  }else{

    SSL_write( mpSsl, str.c_str(), str.length() );
  }
}
//************************************************
void Esp32SSLServer::stopClient(){
  log_v( "stopClient in" );
  log_v( "-----SSL_shutdown-----" );
  if ( !mpSsl ) {
    log_e( "failed mpSSL." );
  }else{
    SSL_shutdown( mpSsl );
  }
  log_v( "close(mp_new_sockfd);" );
  close(mp_new_sockfd); //これ重要。これが無いと再起動を繰り返す。
  mp_new_sockfd = -1;
  log_v( "SSL_free(mpSsl);" );
  SSL_free( mpSsl );
  mpSsl = NULL;
  Serial.println("Access of another terminal is OK!");
  delay(200); //これ重要。これが無いとうまくWebページが表示されない。
}
//*******************************************
String Esp32SSLServer::strResponseHeader(){
  String html_res_head = "HTTP/1.1 200 OK\r\n";
         html_res_head += "Content-type:text/html\r\n";
         html_res_head += "Connection:close\r\n\r\n";
  return html_res_head;
}
//*******************************************
String Esp32SSLServer::strBodyHtml( String message2 ){
  String str = "<!DOCTYPE html>\r\n<html>\r\n";
  str += "<head>\r\n";
  str += "<meta name='viewport' content='initial-scale=1.3'>\r\n";
  str += "</head>\r\n";
  str += "<body style='background:#000; color:#fff; font-size:1em;'>\r\n";
  str += message2;
  str += "\r\n";
  str += "</body>\r\n</html>\r\n\r\n";
  return str;
}
//*******************************************
String Esp32SSLServer::strResponseFavicon(){
  String str = "HTTP/1.1 404 Not Found\r\n";
  str += "Connection:close\r\n\r\n";
  return str;
}
//******************************************
uint16_t Esp32SSLServer::readPkiFileSPIFFS( const char *path, char *pki_cstr ){
  Serial.print( F("SPIFFS file reading --- ") ); Serial.println( path );
  File file = SPIFFS.open( path, FILE_READ );
  if( !file ){
    Serial.println( F("Failed to open file for writing") );
    return 0;
  }
  delay(10);
  uint16_t i = 0;
  while( file.available() ){
    pki_cstr[i] = file.read();
    if( pki_cstr[i] == '\0' ) break;
    i++;
  }
  pki_cstr[i] = '\0';
  delay(5);
  file.close();
  delay(5);
  return i;
}
//******************************************
void Esp32SSLServer::stop(){
  Esp32SSLServer::end();
}
//******************************************
void Esp32SSLServer::end(){
  lwip_close_r( mp_sockfd );
  mp_sockfd = -1;
  mp_isListening = false;
}
