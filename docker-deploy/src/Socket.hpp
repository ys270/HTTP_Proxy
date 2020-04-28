#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Socket{
protected:
  int socket_fd;
  string hostname;
  string port;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  
public:
  Socket(string _hostname, string _port):
    socket_fd(-1),
    hostname(_hostname),
    port(_port),
    host_info_list(NULL)
  {
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
  }

  virtual ~Socket(){
    if(socket_fd > 0){
      close(socket_fd);
    }
    if(host_info_list){
      freeaddrinfo(host_info_list);
    }
  }
  
  virtual void get_addr_info()=0;
  
  int get_socket_fd()const{ return socket_fd; }
};

class Host_Socket : public Socket{
public:
  Host_Socket(string hostname, string port):
    Socket(hostname, port)
  {}

  virtual ~Host_Socket(){}

  virtual void get_addr_info(){
    host_info.ai_flags = AI_PASSIVE;
    if(getaddrinfo(hostname.c_str(), port.c_str(), &host_info, &host_info_list) != 0){
      cerr << "Cannot get host addr info" << endl;
      exit(EXIT_FAILURE);
    }
  }
  
  void get_host_fd(){
    struct addrinfo *curr;
    for(curr = host_info_list; curr!=NULL; curr = curr->ai_next){
      socket_fd = socket(host_info_list->ai_family, 
			 host_info_list->ai_socktype, 
			 host_info_list->ai_protocol);
      if(socket_fd == -1){
	continue;
      }
      // set socket reusable
      int optval = 1;
      setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
      if(bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) != 0){
	continue;
      }
      break;
    }
    if(curr == NULL){
      cerr << "Cannot bind" << endl;
      exit(EXIT_FAILURE);
    }
  }
  
  void listen_on_port(){
    if(listen(socket_fd, 10) != 0){
      cerr << "Cannot listen" << port << endl;
      exit(EXIT_FAILURE);
    }
  }
  
  int accept_client(){
    int accept_fd;
    struct sockaddr_storage sock_addr;
    socklen_t sock_addr_len = sizeof(sock_addr);
    accept_fd = accept(socket_fd, (struct sockaddr *)&sock_addr, &sock_addr_len);
    return accept_fd;
  }
};

class Client_Socket : public Socket{
public:
  Client_Socket(string hostname, string port):
    Socket(hostname, port)
  {}

  virtual ~Client_Socket(){}
  
  virtual void get_addr_info(){
    if(getaddrinfo(hostname.c_str(), port.c_str(), &host_info, &host_info_list) != 0){
      throw hostname + ":" + port;
    }
  }
  
  void connect_to_host(){
    struct addrinfo *curr;
    for(curr = host_info_list; curr!=NULL; curr = curr->ai_next){
      socket_fd = socket(host_info_list->ai_family, 
			 host_info_list->ai_socktype, 
			 host_info_list->ai_protocol);
      if(socket_fd == -1){
	continue;
      }
      if(connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) != 0){
	continue;
      }
      break;
    }
    if(curr == NULL){
      throw hostname + ":" + port;
    }
  }
};

void send_to(int fd, string buffer){
  long long buf_len = buffer.size();
  long long len = 0;
  while(len < buf_len){
    int status = send(fd, buffer.substr(len).c_str(), buf_len-len, 0);
    if(status == -1){
      continue;
    }
    len += status;
  }
}

string recv_from(int fd){
  int size = 0;
  int len = 0;
  int increase = 4096;
  vector<char> buffer;
  buffer.resize(size);
  while(true){
    size += increase;
    buffer.resize(size);
    len = recv(fd, &buffer.data()[size-increase], increase, 0);
    if(len == -1){
      continue;
    }
    else if(len == 0){
      return "";
    }
    else if(len < increase){
      break;
    }
    increase *= 2;
  }
  size = size + len - increase;
  buffer.resize(size);
  string ans(buffer.begin(), buffer.end());
  return ans;
}

string recv_header(int fd){
  vector<char> buffer;
  int len=0;
  int status = 0;
  string s = "";
  while(true){
    buffer.resize(len+1);
    status = recv(fd, &buffer.data()[len], 1, 0);
    if(status == -1){
      continue;
    }
    else if(status == 0){
      return "";
    }
    len += status;
    s += buffer[len-1];
    if(s.length() >= 4){
      if(s.find("\r\n\r\n") != string::npos){
	break;
      }
    }
  }
  return s;
}

string recv_content(int fd, int clen){
  vector<char> buffer;
  buffer.resize(clen);
  int status = 0;
  do{
    status = recv(fd, &buffer.data()[0], clen, MSG_WAITALL);
    // status = 0?
  }while(status == -1);
  string s(buffer.begin(), buffer.end());
  return s;
}

string recv_chunked(int fd){
  vector<char> buffer;
  int len=0;
  int status = 0;
  string s = "";
  while(true){
    buffer.resize(len+1);
    status = recv(fd, &buffer.data()[len], 1, 0);
    if(status == -1){
      continue;
    }
    else if(status == 0){
      return "";
    }
    len += status;
    s += buffer[len-1];
    if(s.length() >= 5){
      if(s.find("0\r\n\r\n") != string::npos){
	break;
      }
    }
  }
  return s;
}

#endif
