#ifndef __HANDLER_HPP__
#define __HANDLER_HPP__

#define LOG "/var/log/erss/proxy.log"

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <string>
#include <fstream>

#include "Socket.hpp"
#include "Request_Parser.hpp"
#include "Response_Parser.hpp"

using namespace std;

class Handler{
protected:
  int ID;
  int browser_fd;
  ofstream proxylog;
  Client_Socket *server;
  Request_Parser *reqParser;
  Response_Parser *resParser;

public:
  Handler(int _ID, int fd, string req):
    ID(_ID),
    browser_fd(fd),
    server(NULL),
    reqParser(new Request_Parser(req)),
    resParser(NULL)
  {
    // TODO ip time
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(browser_fd, (struct sockaddr *)&addr, &addr_len);
    time_t curr_time = time(NULL);
    struct tm * timeinfo = gmtime(&curr_time);
    write_to_log("\"" + reqParser->get_requestLine() + "\" from " + inet_ntoa(addr.sin_addr) + " @ " + asctime(timeinfo));
  }

  virtual ~Handler(){
    if(browser_fd>0){
      close(browser_fd);
    }
    delete server;
    delete reqParser;
    delete resParser;
  }

  void connect_server(){
    try{
      server = new Client_Socket(reqParser->get_hostname(), reqParser->get_portnumber());
    }catch(bad_alloc &ba){
      cerr << "Bad Allocation!" << endl;
      send_back_to_browser(browser_fd, "HTTP/1.1 404 Not Found\r\n\r\n");
      throw(ba);
    }
    try{
      server->get_addr_info();
      server->connect_to_host();
    }catch(string &msg){
      cerr << "Cannot connect to " << msg << endl;
      send_back_to_browser(browser_fd, "HTTP/1.1 404 Not Found\r\n\r\n");
      throw(msg);
    }
  }

  void write_to_log(string content){
    proxylog.open(LOG, ofstream::out | ofstream::app);
    proxylog << ID << ": " << content.substr(0, content.find('\n')) << endl;
    proxylog.close();
  }

  string get_diff_response(int);
  string get_response_from_web(string);
  void send_back_to_browser(int, string);
};

string Handler::get_diff_response(int web_fd){
  string resHead = recv_header(web_fd);
  string firstLine = resHead.substr(0, resHead.find('\n'));
  write_to_log("Received \"" + firstLine + "\" from " + reqParser->get_hostname());
  string clen_s = find_content_length(resHead);
  string res;
  // has content-length
  if(clen_s.length() != 0){
    int clen = stoi(clen_s);
    // content length = 0
    if(clen == 0){
      return resHead;
    }
    res = recv_content(web_fd, clen);
  }
  // chunked
  else if(resHead.find("chunked") != string::npos){
    res = recv_chunked(web_fd);
  }
  else{
    res = recv_from(web_fd);
  }
  return resHead+res;
}

string Handler::get_response_from_web(string request){
  connect_server();
  int web_fd = server->get_socket_fd();
  write_to_log("Requesting \"" + reqParser->get_requestLine() + "\" from " + reqParser->get_hostname());
  send_to(web_fd, request);
  return get_diff_response(web_fd);
}

void Handler::send_back_to_browser(int browser_fd, string res){
  write_to_log("Responding \"" + res.substr(0, res.find("\r\n")) + "\"");
  send_to(browser_fd, res);
}

string get_browser_request_head(int browser_fd){
  string req = recv_header(browser_fd);
  if(req.length() == 0){
    return "";
  }
  return req;
}

#endif
