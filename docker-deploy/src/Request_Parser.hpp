#ifndef __REQUEST_PARSER_HPP__
#define __REQUEST_PARSER_HPP__

#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>

using namespace std;

class Request_Parser{
private:
  string s; //input request head
  string requestLine;
  string method;
  string url;
  string hostName;
  string portNum;
public:
  Request_Parser(string input_request) :
    s(input_request)
  {
    parse();
  }
  string get_full_request()const{return s;}
  string get_method()const{return method;}
  string get_url()const{return url;}
  string get_hostname()const{return hostName;}
  string get_portnumber()const{return portNum;}
  string get_requestLine()const{return requestLine;}
  void parse();
  ~Request_Parser(){}
};

void Request_Parser::parse(){
  if(s.length() == 0){
    return;
  }
  // get first line, i.e. request line
  requestLine = s.substr(0, s.find("\r\n"));
  //get method
  method = s.substr(0, s.find(' '));
  //get url
  size_t firspace = s.find_first_of(" ");
  size_t secspace = s.find_first_of(" ", firspace + 1);
  int url_len = secspace - firspace - 1;
  url = s.substr(firspace + 1, (url_len));
  //get hostname and portnumber
  string host = s.substr(s.find("Host: ")+6);
  host = host.substr(0, host.find('\r'));
  if(host.find(':') != string::npos){
    hostName = host.substr(0, host.find(':'));
    portNum  = host.substr(host.find(':')+1);
  }
  else{
    hostName = host;
    portNum = "80";
  }
}

#endif
