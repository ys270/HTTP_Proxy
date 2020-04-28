#ifndef __GET_HANDLER_HPP__
#define __GET_HANDLER_HPP__

#include "Socket.hpp"
#include "Handler.hpp"
#include "Response_Parser.hpp"
#include "Request_Parser.hpp"
#include "Cache.hpp"

using namespace std;

class Get_Handler : public Handler{
private:
  Cache_List *myCache;
public:
  Get_Handler(int _ID, int fd, string s, Cache_List *c):
    Handler(_ID, fd, s), myCache(c)
  {}

  virtual ~Get_Handler(){}
  
  void handle_get();
  string get_chunked_response();
  string revalidate(Cache_Block *);
  string get_diff_response(int);
};

void Get_Handler::handle_get(){
  // find in cache
  Cache_Block *exist = myCache->search_response_lock(reqParser->get_requestLine());
  string response;
  // not in cache
  if(exist == NULL){
    write_to_log("not in cache");
    try{
      response = get_response_from_web(reqParser->get_full_request());
    }catch(bad_alloc &ba){
      return;
    }catch(string &s){
      return;
    }
    if(response == ""){
      send_back_to_browser(browser_fd, "HTTP/1.1 400 Bad Request\r\n\r\n");
      return;
    }
    if(response.find("\r\n\r\n") == string::npos){
      send_back_to_browser(browser_fd, "HTTP/1.1 502 Bad Gateway\r\n\r\n");
      return;
    }
    resParser = new Response_Parser(response);
    // check cachability
    if(resParser->cache_able){
      // create a response parser for cache
      Response_Parser *rp = new Response_Parser(response);
      myCache->add_new_response_lock(reqParser->get_requestLine(), rp, ID);
    }
    // cannot cache, reason
    else{
      string reason = "not cacheable because ";
      if(resParser->no_store){
	write_to_log(reason + "no store");
      }
      else if(resParser->isPrivate){
	write_to_log(reason + "is private");
      }
    }
    send_back_to_browser(browser_fd, response);
    return;
  }
  // in cache
  else{
    time_t currTime = time(NULL);
    time_t expTime = 0;
    // has expire_info
    if(exist->res->expire_info.length() != 0){
      struct tm expire_time;
      strptime(exist->res->expire_info.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &expire_time);
      expTime = mktime(&expire_time);
    }
    // create date + max-age (include max-age=0)
    if(exist->res->age != -1 && exist->res->date.length() != 0){
      struct tm create_time;
      strptime(exist->res->date.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &create_time);
      time_t createTime = mktime(&create_time);
      expTime = createTime + exist->res->age;
    }
    if(expTime!=0){
      if( expTime <= currTime){
	struct tm *exp = gmtime(&expTime);
	string t(asctime(exp));
	write_to_log("in cache, but expired at " + t);
	try{
	  response = revalidate(exist);
	}catch(bad_alloc &ba){
	  return;
	}catch(string &s){
	  return;
	}
	if(response == ""){
	  send_back_to_browser(browser_fd, "HTTP/1.1 400 Bad Request\r\n\r\n");
	  return;
	}
	if(response.find("\r\n\r\n") == string::npos){
	  send_back_to_browser(browser_fd, "HTTP/1.1 502 Bad Gateway\r\n\r\n");
	}
	resParser = new Response_Parser(response);
	// for cache
	Response_Parser *rp = new Response_Parser(response);
	myCache->update_response_to_head_lock(exist, reqParser->get_requestLine(), rp);
	send_back_to_browser(browser_fd, response);
	return;
      }
    }
    // revalidate
    if(exist->res->must_revalid == true){
      write_to_log("in cache, requires validation");
      try{
	response = revalidate(exist);
      }catch(bad_alloc &ba){
	return;
      }catch(string &s){
	return;
      }
      if(response == ""){
	send_back_to_browser(browser_fd, "HTTP/1.1 400 Bad Request\r\n\r\n");
	return;
      }
      if(response.find("\r\n\r\n") == string::npos){
	send_back_to_browser(browser_fd, "HTTP/1.1 502 Bad Gateway");
      }
      // create a new response parser for cache
      resParser = new Response_Parser(response);
      Response_Parser *rp = new Response_Parser(response);
      myCache->update_response_to_head_lock(exist, reqParser->get_requestLine(), rp);
      send_back_to_browser(browser_fd, response);
      return;
    }
    // response already in cache, and is valid
    write_to_log("in cache, valid");
    send_back_to_browser(browser_fd, exist->res->s);
  }
}

string Get_Handler::revalidate(Cache_Block *exist){
  string modified_request = reqParser->get_full_request();
  // if has etag
  string etag = exist->res->etag;
  if(etag.length() > 0){
    modified_request = modified_request.substr(0, modified_request.find("\r\n\r\n")+2);
    modified_request += "If-None-Match: " + etag + "\r\n\r\n";
  }
  // if has last_modified
  string last_modified = exist->res->last_modified;
  if(last_modified.length() > 0){
    modified_request = modified_request.substr(0, modified_request.find("\r\n\r\n")+2);
    modified_request += "If-Modified-Since" + last_modified + "\r\n\r\n";
  }
  string response = get_response_from_web(modified_request);
  // if 304 not modified
  if(response.find("304 Not Modified") != string::npos){
    return exist->res->s;
  }
  else{
    return response;
  }
}

#endif
