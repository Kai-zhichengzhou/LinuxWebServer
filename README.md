# LinuxWebServer

## Description
This is a web server developed in Linux environment scripting in C++ to handle multiple http connections based on the implemented threadspool. 
The server implemented utilize the technique of epoll to listen to all sockets connecting to clients, and use finite-state-machine to handle and process different type of http requesting, responding to clients with different http response. 
