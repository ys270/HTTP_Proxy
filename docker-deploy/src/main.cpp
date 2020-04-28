#include <cstdio>
#include <cstdlib>
#include <signal.h>

#include <iostream>
#include <thread>

#include "Proxy.hpp"
#include "Handler.hpp"
#include "Get_Handler.hpp"
#include "Post_Handler.hpp"
#include "Connect_Handler.hpp"
#include "Socket.hpp"
#include "Request_Parser.hpp"
#include "Response_Parser.hpp"

using namespace std;

void proxy_func(int thread_id, int fd, Cache_List *myCache){
  string firstReq = get_browser_request_head(fd);
  Request_Parser reqParser(firstReq);
  string method = reqParser.get_method();
  if(method == "CONNECT"){
    Connect_Handler ch(thread_id, fd, firstReq);
    // no-throw
    ch.handle_connect();
  }
  else if(method == "GET"){
    Get_Handler gh(thread_id, fd, firstReq, myCache);
    // no-throw
    gh.handle_get();
  }
  // POST
  else if(method == "POST"){
    Post_Handler ph(thread_id, fd, firstReq);
    // no-throw
    ph.handle_post();
  }
}

int main(int argc, char *argv[]){
  Proxy myProxy;
  string hn = "0.0.0.0";
  string pn = "12345";
  myProxy.build(hn, pn);
  int thread_id = 0;
  signal(SIGPIPE, SIG_IGN);
  Cache_List *myCache = new Cache_List();
  while(true){
    // new connection/request
    int new_fd = myProxy.accept();
    // invalid connection
    if(new_fd == -1){
      continue;
    }
    thread new_thread(proxy_func, thread_id, new_fd, myCache);
    new_thread.detach();
    thread_id++;
  }
  delete myCache;
  return EXIT_SUCCESS;
}
