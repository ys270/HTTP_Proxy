#ifndef __PROXY_HPP__
#define __PROXY_HPP__

#include "Socket.hpp"
#include "Request_Parser.hpp"
#include "Response_Parser.hpp"

using namespace std;

class Proxy{
private:
  Host_Socket *browser;

public:
  Proxy(): 
    browser(NULL)
  {}

  ~Proxy(){
    delete browser;
  }

  void build(string, string);
  
  int accept(){
    return browser->accept_client();
  }
};

void Proxy::build(string hostname, string port){
  browser = new Host_Socket(hostname, port);
  browser->get_addr_info();
  browser->get_host_fd();
  browser->listen_on_port();
}

#endif
