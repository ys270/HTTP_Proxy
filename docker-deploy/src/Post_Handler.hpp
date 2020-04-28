#ifndef __POST_HANDLER_HPP__
#define __POST_HANDLER_HPP__

#include "Socket.hpp"
#include "Handler.hpp"
#include "Response_Parser.hpp"
#include "Request_Parser.hpp"

using namespace std;

class Post_Handler : public Handler{
public:
  Post_Handler(int _ID, int fd, string s):
    Handler(_ID, fd, s)
  {}

  virtual ~Post_Handler(){}
  string get_request_body();
  void handle_post();
};

string Post_Handler::get_request_body(){
  string clen_s = find_content_length(reqParser->get_full_request());
  string body;
  // has content-length
  if(clen_s.length() != 0){
    int clen = stoi(clen_s);
    // content length = 0
    if(clen == 0){
      return "";
    }
    body = recv_content(browser_fd, clen);
  }
  // chunked
  else if(reqParser->get_full_request().find("chunked") != string::npos){
    body = recv_chunked(browser_fd);
  }
  else{
    body = recv_from(browser_fd);
  }
  return body;
}

void Post_Handler::handle_post(){
  string body = get_request_body();
  // body = 0?
  string full = reqParser->get_full_request();
  full += body;
  string response;
  try{
    response = get_response_from_web(full);
  }catch(bad_alloc &ba){
    return;
  }catch(string &s){
    return;
  }
  send_back_to_browser(browser_fd, response);
}

#endif
