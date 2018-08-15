# Proxy-Server
A simple proxy server written in C to practise socket programming.  
It only handles GET request.  

## Build
```
 $ make
 $ make install
```  

## Proxy Server Usage (default port is 3000)
```
 $ proxy <port_no> 
```  

## Helper programs ( installed along with proxy in make install )
```
 $ showip <host_name>                 => returns IPV4 or IPV6 of host.
 $ tcpclient <host_name> <port_no>    => sets up a tcp connection to the host
```  

### Learnings from this project
  * Socket Programming  
  * Working of HTTP Protocol
  * Creating and handling child processes using fork, waitpid, sigaction etc.  
  
[Fork System Call](https://utiny.herokuapp.com/fZ)  
[Zombie and Orphan Processes](https://utiny.herokuapp.com/f9)  
[Preventing Zombie Processes](https://utiny.herokuapp.com/gj)  
