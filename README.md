# HTTP_Proxy
to see detailed requirement, please go to [hw2.pdf](https://github.com/ys270/HTTP_Proxy/blob/master/hw2.pdf).

This is an HTTP proxy which functions with GET, POST and CONNECT and caches responses to GET requests. Our cache has size of 500, follows the rules of expiration time and revalidation and uses LRU replacement policy. The proxy can handle multiple concurrent requests with multiple threads. Our proxy would produce a proxy.log which contains the information about each requst.
## How to Run
```
sudo ducker-compose up
```
