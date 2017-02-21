#include "Request.h"
#include <iostream>
#include <vector>
#include <QDebug>
#define RECV_BUF_SIZE 8196
#define MEM_BUFFER_INIT_SIZE 8192
#define PROTO_BUF_LEN 20
#define HOST_BUF_LEN 256
#define REQUEST_BUF_LEN 1024

Request::Request(){
    errorMessage = "";
}

Request::~Request(){

}

unsigned long Request::getHostAddress(const char *host){
    struct hostent *phost;
    char *p;

    phost = gethostbyname(host);
    if(phost == NULL){
        errorMessage += "failed to call 'gethostname()'\n";
        return 0;
    }
    p = *phost->h_addr_list;
    return *((unsigned long*)p);
}

bool Request::sendString(SOCKET sock, const char *str){
    size_t len = strlen(str);
    size_t sent = 0;
    int code = 0;
    int tried = 0;
    while(sent < len){
        code = send(sock, str + sent, len - sent, 0);
        if(code > 0){
            sent += code;
        }else{
            if(tried >= 10){
                char tmp[256] = {0};
                sprintf(tmp, "failed to send string, send() returns '%d', sent '%d', left '%d'\n", code, sent, len - sent);
                errorMessage += tmp;
                return false;
            }else{
                tried++;
            }
        }
    }
    return true;
}

bool Request::sendBinary(SOCKET sock, const unsigned char *bytes, size_t size){
    size_t sent = 0;
    int code = 0;
    int tried = 0;
    while(sent < size){
        code = send(sock, (const char*)(bytes + sent), size - sent, 0);
        if(code > 0){
            sent += code;
        }else{
            if(tried >= 10){
                char tmp[256] = {0};
                sprintf(tmp, "failed to send binary, send() returns '%d', sent '%d', left '%d'\n", code, sent, size - sent);
                errorMessage += tmp;
                return false;
            }else{
                tried++;
            }
        }
    }
    return true;
}

bool Request::validHostChar(char ch){
    return (isalpha(ch)
            || isdigit(ch)
            || ch == '-'
            || ch == '.'
            || ch == ':');
}

char *proto_end(char *str){
    int len = strlen(str);
    for(int i = 0; i < len - 2; i++){
        if(str[i] == '.'){
            return NULL;
        }else if(str[i] == ':' && str[i + 1] == '/' && str[i + 2] == '/'){
            return str + i;
        }
    }
    return NULL;
}

void Request::parseURL(const std::string &url,
                       char *proto,
                       size_t proto_len,
                       char *host,
                       size_t host_len,
                       char *req,
                       size_t req_len,
                       int *port){
    char *work, *ptr, *ptr2;
    *proto = *host = *req = 0;
    *port = 80;

    work = strdup(url.c_str());
    strlwr(work);

    ptr = proto_end(work);

    if(ptr != NULL && (ptr - work) < proto_len){
        *(ptr++) = 0;
        strcpy(proto, work);
    }else{
        strcpy(proto, "http");
        ptr = work;
    }

    if((*ptr=='/') && (*(ptr+1)=='/') ){
        ptr+=2;
    }

    ptr2 = ptr;
    while(validHostChar(*ptr2) && *ptr2){
        ptr2++;
    }

    *ptr2 = 0;
    if(strlen(ptr) <= host_len){
        strcpy(host, ptr);
    }

    const char *req_start = url.c_str() + (ptr2 - work);
    if(strlen(req_start) <= req_len){
        strcpy(req, req_start);
    }
    if(*req == '\0'){
        req = "/";
    }

    ptr = strchr(host, ':');
    if(ptr != NULL){
        *ptr = 0;
        *port = atoi(ptr + 1);
    }

    free(work);
}

void Request::initWinSock(){
    static bool inited = false;
    if(!inited){
        WSADATA wsaData;
        if(WSAStartup(0x0101, &wsaData) != 0){
            return;
        }
        inited = true;
    }
}

int Request::sendHTTP(HTTP type,
                      const std::string &url,
                      const std::string &extraHeader,
                      const std::string &postMessage,
                      HTTPRequest *req){


#if defined(__WIN32__)
    initWinSock();
#endif
    struct sockaddr_in sock_in;
    SOCKET sock;
    char recv_buff[RECV_BUF_SIZE] = {0};
    std::string headerToSend;
    char proto[PROTO_BUF_LEN], host[HOST_BUF_LEN], request[REQUEST_BUF_LEN];
    int port, chars, recved;
    bool finished;

    memset(proto, 0, PROTO_BUF_LEN);
    memset(host, 0, HOST_BUF_LEN);
    memset(request, 0, REQUEST_BUF_LEN);

    parseURL(url,
             proto, PROTO_BUF_LEN,
             host, HOST_BUF_LEN,
             request, REQUEST_BUF_LEN,
             &port);
    if(strcmp(proto, "http") != 0){
        errorMessage += "protocol is not HTTP\n";
        return -1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET){
        errorMessage += "failed to create socket instance\n";
        return -1;
    }

    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons((unsigned short)port);
    sock_in.sin_addr.S_un.S_addr = getHostAddress(host);

    if(0 != connect(sock, (struct sockaddr*)&sock_in, sizeof(struct sockaddr))){
        char tmp[256] = {0};
        sprintf(tmp, "failed to connect to remote, host=%s, port=%d [htons(port)=%d]\n", host, port, sock_in.sin_port);
        errorMessage += tmp;
        return -1;
    }

    if(*request == 0){
        request[0] = '/';
        request[1] = '\0';
    }

    // 开始构造请求header
    if(type == HTTP::GET){
        headerToSend.append("GET HTTP/1.1/r/n");
    }else if(type == HTTP::POST){
        headerToSend.append("POST HTTP/1.1/r/n");
    }else{
        errorMessage += "HTTP type missing, only GET and POST supported for now\n";
        return -1;
    }

    headerToSend.append(request);
    headerToSend.append("User-Agent: Neeao/4.0/r/n");

    if(!postMessage.empty()){
        memset(recv_buff, 0, RECV_BUF_SIZE);
        sprintf(recv_buff, "Content-Length: %ld/r/n", postMessage.length());
        headerToSend.append(recv_buff);
    }

    memset(recv_buff, 0, RECV_BUF_SIZE);
    sprintf(recv_buff, "Host: %s/r/n", host);
    headerToSend.append(recv_buff);

    if(!extraHeader.empty()){
        headerToSend.append(extraHeader);
    }

    headerToSend.append("/r/n");

    // 发送请求header
    if(!sendString(sock, headerToSend.c_str())){
        errorMessage += "failed to send request header\n";
        return -1;
    }

    // 发送可选的请求消息体
    if(!postMessage.empty()){
        if(!sendBinary(sock, (const unsigned char*)postMessage.c_str(), postMessage.size())){
            errorMessage += "failed to send request message body\n";
            return -1;
        }
    }

    req->headerSend = headerToSend;

    // 开始接收响应header
    chars = 0;
    finished = false;
    while(!finished){
        recv_buff[0] = '\0';
        recv_buff[1] = '\0';

        recved = recv(sock, recv_buff, 1, 0);

        if(recved <= 0){
            finished = true;
            char tmp[256] = {0};
            if(recved == 0){
                sprintf(tmp, "remote closed connection, errno=%d", errno);
            }else{
                sprintf(tmp, "failed to call first recv() which returns '%d', errno=%d", recved, errno);
            }
            errorMessage += tmp;
            return -1;
        }

        switch(*recv_buff){
        case '/r':
            break;
        case '/n':
            if(chars == 0){
                finished = true;
            }
            chars = 0;
            break;
        default:
            chars++;
            break;
        }

        req->headerReceive.append(recv_buff, 1);
    }

    qDebug() << req->headerReceive.c_str();

    // 开始接收响应消息体
    do{
        recved = recv(sock, recv_buff, sizeof(recv_buff), 0);
        if(recved <= 0){
            char tmp[256] = {0};
            sprintf(tmp, "failed to call second recv() which returns '%d', errno=%d", recved, errno);
            errorMessage += tmp;
            return -1;
        }
        req->message.append(recv_buff, recved);
    }while(recved > 0);
    qDebug() << req->message.c_str();

    closesocket(sock);
    return 0;
}

int Request::sendRequest(HTTP type,
                const std::string &url,
                const std::string &postMessage,
                std::string *headerReceive,
                std::string *recvMessage){
    HTTPRequest req;
    int code;

    req.headerReceive.clear();
    req.headerSend.clear();
    req.message.clear();

    if(type == HTTP::POST){
        code = sendHTTP(type, url, "", postMessage, &req);
    }else if(type == HTTP::GET){
        code = sendHTTP(type, url, "", "", &req);
    }

    if(code == 0){
        *headerReceive = req.headerReceive;
        if(recvMessage != NULL){
            *recvMessage = req.message;
        }
        return 0;
    }else{
        return -1;
    }
}






