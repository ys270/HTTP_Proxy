#ifndef __CONNECT_HANDLER_HPP__
#define __CONNECT_HANDLER_HPP__

#include "Socket.hpp"
#include "Handler.hpp"
#include "Request_Parser.hpp"
#include "Response_Parser.hpp"

class Connect_Handler : public Handler{
public:
  Connect_Handler(int _ID, int fd, string s):
    Handler(_ID, fd, s)
  {}

  virtual ~Connect_Handler(){}
  void handle_connect();
  void create_tunnel();
  void tunneling();
};

void Connect_Handler::handle_connect(){
  try{
    connect_server();
  }catch(bad_alloc &ba){
    return;
  }catch(string &s){
    return;
  }
  create_tunnel();
  tunneling();
}

void Connect_Handler::create_tunnel(){
  string ok = "HTTP/1.1 200 Connection Established\r\n\r\n";
  send_to(browser_fd, ok);  
}

void Connect_Handler::tunneling(){
  fd_set readfds;
  while(true){
    FD_ZERO(&readfds);
    FD_SET(browser_fd, &readfds);
    FD_SET(server->get_socket_fd(), &readfds);
    int max_fd = max(browser_fd, server->get_socket_fd());
    
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    if(select(max_fd+1, &readfds, NULL, NULL, &tv) == -1){
	cerr << "Cannot select" << endl;
    }
    if(FD_ISSET(browser_fd, &readfds)){
      string s = recv_from(browser_fd);
      if(s.length() == 0){
	break;
      }
      send_to(server->get_socket_fd(), s);
      }
    else if(FD_ISSET(server->get_socket_fd(), &readfds)){
      string s = recv_from(server->get_socket_fd());
      if(s.length() == 0){
	break;
      }
      send_to(browser_fd, s);
    }
  }
  write_to_log("Tunnel closed");
}

#endif
