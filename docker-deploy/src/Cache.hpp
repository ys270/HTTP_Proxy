#ifndef __CACHE_HPP__
#define __CACHE_HPP__

#define LOG "/var/log/erss/proxy.log"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <mutex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Response_Parser.hpp"
#include "Request_Parser.hpp"

using namespace std;

mutex cache_lock; 

class Cache_Block{
public:
  int ID;
  string request;//the first line of the request
  Response_Parser *res;
  Cache_Block * prev;
  Cache_Block * next;
public:
  Cache_Block(string req, Response_Parser *r, int _ID):
    ID(_ID),
    request(req),
    res(r),
    prev(NULL),
    next(NULL)
  {}
  ~Cache_Block(){
    delete res;
  }
};

class Cache_List{
public:
  Cache_Block * head;
  Cache_Block * tail;
  int len;   //max length of the cache = 500
  ofstream proxylog;
public:
  Cache_List():
    head(NULL),
    tail(NULL),
    len(0)
  {}
  ~Cache_List(){
    // clean up all the blocks
    while(head != NULL){
      Cache_Block *next = head->next;
      delete head;
      head = next;
    }
  }
  
  Cache_Block * search_response_lock(string req);
  void rmv_tail();
  void add_new_response_lock(string req,Response_Parser * r,int ID);
  void add_new_response(string req,Response_Parser * r,int ID);
  void update_response_to_head_lock(Cache_Block* e,string req,Response_Parser * r);
};

Cache_Block* Cache_List::search_response_lock(string req){
  Cache_Block * curr = head;
  while(curr != NULL){
    if(curr->request == req){
      break;
    }
    curr = curr->next;
  }
  return curr;
}

void Cache_List::rmv_tail(){
  string key = tail->request;
  Request_Parser p(key);
  tail->prev->next = NULL;
  tail = tail->prev;
  len = len - 1;
  proxylog.open(LOG, ofstream::out | ofstream::app);
  string evicted = p.get_url() + p.get_hostname();
  proxylog << "(no-id): "<< "NOTE evicted " << evicted << " from cache " << endl;
  proxylog.close();
}

void Cache_List::add_new_response_lock(string req, Response_Parser *r, int ID){
  lock_guard<mutex> lk(cache_lock);
  add_new_response(req, r, ID);
}

void Cache_List::add_new_response(string req, Response_Parser *r, int ID){
  if(len>=500){
    rmv_tail();
  }
  Cache_Block * new_response = new Cache_Block(req, r, ID);
  if(len==0){
    head = new_response;
    tail = new_response;
  }
  else{
    head->prev = new_response;
    new_response->next = head;
    new_response->prev = NULL;
    head = new_response;
  }
  len = len + 1;
  if(r->expire_info != ""){
    proxylog.open(LOG, std::ofstream::out | std::ofstream::app);
    proxylog << ID << ": cached, expires at " << r->expire_info << endl;
    proxylog.close();
    return;
  }
  if(r->must_revalid == true){
    proxylog.open(LOG, std::ofstream::out | std::ofstream::app);
    proxylog << ID << ": cached, but requires re-validation " << endl;
    proxylog.close();
  }
}

void Cache_List::update_response_to_head_lock(Cache_Block* exist, string req, Response_Parser *r){
  lock_guard<mutex> lk(cache_lock);
  int id = exist->ID;
  if(exist == head){
    head = head->next;
    if(head){
      head->prev = NULL;
    }
    len = len - 1;
    add_new_response(req, r, id);
  }
  else if(exist == tail){
    rmv_tail();
    add_new_response(req, r, id);
  }
  else{
    exist->prev->next = exist->next;
    exist->next->prev = exist->prev;
    len = len - 1;
    add_new_response(req, r, id);
  }
}

#endif
