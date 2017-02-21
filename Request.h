#ifndef REQUEST_H
#define REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string>

#if defined(__WIN32__)
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

typedef struct {
    std::string headerSend;
    std::string headerReceive;
    std::string message;
} HTTPRequest;

enum class HTTP{
    GET,
    POST
};

class Request
{
public:
    Request();
    virtual ~Request();
    int sendRequest(HTTP type,
                    const std::string &url,
                    const std::string &postMessage,
                    std::string *headerReceive,
                    std::string *recvMessage);

private:
    unsigned long getHostAddress(const char *host);
    bool sendString(SOCKET sock, const char *str);
    bool sendBinary(SOCKET sock, const unsigned char *bytes, size_t size);
    bool validHostChar(char ch);
    void initWinSock();
    void parseURL(const std::string &url,
                  char *proto,
                  size_t proto_len,
                  char *host,
                  size_t host_len,
                  char *req,
                  size_t req_len,
                  int *port);
    int sendHTTP(HTTP type,
                 const std::string &url,
                 const std::string &extraHeader,
                 const std::string &postMessage,
                 HTTPRequest *req);
public:
    std::string errorMessage = "";
};


#endif // REQUEST_H
