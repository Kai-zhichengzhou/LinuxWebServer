#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"
#include <sys/uio.h>

class http_conn
{
public:
    static const int FILENAME_LEN = 200;        // length of filename 
    static const int READ_BUFFER_SIZE = 2048;   // size of read buffer
    static const int WRITE_BUFFER_SIZE = 1024;  // size of write buffer
    

    // the task we are handling in our server is Http connection
    // use enum to define different Http request
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};
    
    /*
        The state of state machine while parsing the request from client 
        CHECK_STATE_REQUESTLINE: currently parsing the request line 
        CHECK_STATE_HEADER: currently parsing the header
        CHECK_STATE_CONTENT: currently parsing the body content
    */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    
    /*
        Enum for indicating the possible response code while server parsing the http request
        NO_REQUEST          :   the request from client is incomplete
        GET_REQUEST         :   indacting received a complete request 
        BAD_REQUEST         :   syntax error for request
        NO_RESOURCE         :   no resource in server
        FORBIDDEN_REQUEST   :   no permission 
        FILE_REQUEST        :   the file request succeed 
        INTERNAL_ERROR      :   internal error 
        CLOSED_CONNECTION   :   indicating the connection is closed
    */
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    
    // status for state machine of reading lines 
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };
public:
    http_conn(){}
    ~http_conn(){}
public:
    void init(int sockfd, const sockaddr_in& addr); // init new connection 
    void close_conn();  // close connection
    void process(); // handle client request 
    bool read();// non-blocking read
    bool write();// non-blocking write 
private:
    void init();    // init
    HTTP_CODE process_read();    //parse http request, return http code
    bool process_write( HTTP_CODE ret );    // write http response to client 

    // functions to parse http request in different section 
    HTTP_CODE parse_request_line( char* text );
    HTTP_CODE parse_headers( char* text );
    HTTP_CODE parse_content( char* text );
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();

    // functions to create http response to write to client 
    void unmap();
    bool add_response( const char* format, ... );
    bool add_content( const char* content );
    bool add_content_type();
    bool add_status_line( int status, const char* title );
    bool add_headers( int content_length );
    bool add_content_length( int content_length );
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;       // all events in all sockets are registered in epoll 
    static int m_user_count;    // user count 

private:
    int m_sockfd;           // the socket of this http connection and the socket address 
    sockaddr_in m_address;
    
    char m_read_buf[ READ_BUFFER_SIZE ];    // reaad buffer 
    int m_read_idx;                         // last byte of reading buffer 
    int m_checked_idx;                      // the index of currently parsing char
    int m_start_line;                       // the start position of currently parsed line 

    CHECK_STATE m_check_state;              // current sate
    METHOD m_method;                        // method of requesting 

    char m_real_file[ FILENAME_LEN ];       // the target file name 
    char* m_url;                            // the path to target file 
    char* m_version;                        // http protocol version 
    char* m_host;                           // name of host 
    int m_content_length;                   // length of content 
    bool m_linger;                          // closed connection or not 

    char m_write_buf[ WRITE_BUFFER_SIZE ];  // write buffer 
    int m_write_idx;                        // the number of ready-to-send bytes
    char* m_file_address;                   // the pos in memory of mmap from target file 
    struct stat m_file_stat;                // the status of file 
    struct iovec m_iv[2];                   // use writev to execute write, the count indicates the number of memory blocks written 
    int m_iv_count;

    int bytes_to_send;              // the bytes number that going to be send 
    int bytes_have_send;            // bytes number already sent
};

#endif
