#ifndef __RESPONSE_PARSER_HPP__
#define __RESPONSE_PARSER_HPP__

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <string>

using namespace std;

class Response_Parser{
public:
  string s;//input response's string
  string response_head;//input string's head
  bool chunked;
  bool cache_able;
  string status_number;
  string expire_info;
  string etag;
  string cache_control;
  string date;
  string last_modified;
  bool must_revalid;
  bool no_cache;
  bool no_store;
  bool isPrivate;  
  long long int content_len;
  double age;
  
  Response_Parser(string input):
    s(input),
    response_head(""),
    chunked(false),
    cache_able(true),
    status_number(""),
    expire_info(""),
    etag(""),
    cache_control(""),
    date(""),
    last_modified(""),
    must_revalid(false),
    no_cache(false),
    no_store(false),
    isPrivate(false),
    content_len(0),
    age(-1)
  {
    parse();
  }
  
  void get_response_head();
  void check_chunked();
  string get_info_nohead(string response_head);
  string get_info_with_head(string response_head);
  void parse();
  ~Response_Parser(){}
};

void Response_Parser::get_response_head(){
  response_head = s.substr(0, s.find("\r\n\r\n"));
  }

void Response_Parser::check_chunked(){
  if(response_head.find("chunked") != string::npos){
    chunked = true;
  }
  else{
    chunked = false;
  }
}

string Response_Parser::get_info_nohead(string sign){
  size_t left = response_head.find(sign) + sign.length();
  size_t right = response_head.find_first_of("\r\n", left);
  return response_head.substr(left, right-left);
}

string Response_Parser::get_info_with_head(string sign){
  size_t left = response_head.find(sign);
  size_t right = response_head.find_first_of("\r\n",left);
  return response_head.substr(left,right-left);
}

void Response_Parser::parse(){
  get_response_head();
  check_chunked();
  //get status number
  size_t numberbegin = response_head.find_first_of(" ") + 1;
  size_t numberend = response_head.find_first_of(" ", numberbegin);
  status_number = response_head.substr(numberbegin, numberend - numberbegin);
  if(status_number!="200"){
    cache_able = false;
  }
  //get expire information
  if(response_head.find("Expires: ") != string::npos){
    expire_info = get_info_nohead("Expires: ");
  }
  //get etag(did not get rid of the "ETag: ")
  if(response_head.find("ETag: ") != string::npos){
    etag = get_info_nohead("ETag: ");
  }
  //get cache_control(did not get rid of "Cache Control: ")
  if(response_head.find("Cache-Control: ") != string::npos){
    cache_control = get_info_with_head("Cache-Control: ");
    //get age(double)
    if(cache_control.find("max-age=") != string::npos){
      string s_age = cache_control.substr(cache_control.find("max-age=")+8);
      if(s_age.find(",")!=string::npos){
        s_age = s_age.substr(0,s_age.find_first_of(","));
      }
      age = stod(s_age);
    }
    if(cache_control.find("must-revalidate") != string::npos){
      must_revalid = true;
    }
    else{
      must_revalid = false;
    }
    if(cache_control.find("no-cache")!=string::npos){
      no_cache = true;
      cache_able = false;
    }
    if(cache_control.find("no-store")!=string::npos){
      no_store = true;
      must_revalid = true;
    }
    if(cache_control.find("private")!=string::npos){
      isPrivate = true;
      cache_able = false;
    }
  }
  //get date
  if(response_head.find("Date: ") != string::npos){
    date = get_info_nohead("Date: ");
  }
  //get last modified
  if(response_head.find("Last-Modified: ") != string::npos){
    last_modified = get_info_nohead("Last-Modified: ");
  }
  //get content length(long long int)
  if(response_head.find("Content-Length: ") != string::npos){
    string s_content_len = get_info_nohead("Content-Length: ");
    content_len = stoll(s_content_len);
  }
}

string find_content_length(string s){
  string pattern = "Content-Length: ";
  size_t left;
  // not find
  if((left = s.find(pattern)) == string::npos){
    return "";
  }
  left += pattern.length();
  size_t right = s.find_first_of("\r\n", left);
  return s.substr(left, right-left);
}

#endif
