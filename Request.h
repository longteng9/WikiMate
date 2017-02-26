#ifndef REQUEST_H
#define REQUEST_H

#include "asio.hpp"
#include <string>
#include <stdint.h>
#include <vector>
#include <functional>

typedef std::vector<std::pair<std::string, std::string> > HeaderList;
typedef std::vector<std::pair<std::string, std::string> > PairList;

class Response{
    friend class Request;
public:
    Response();
    int errorCode() const;
    const std::string& errorMsg() const;
    int statusCode() const;
    const HeaderList& headerList() const;
    const std::string& content() const;

private:
    void assignHeaderList(const std::string& header);
    std::string trim(const std::string& str) const;
    std::vector<std::string> split(const std::string& str, 
        const std::string& delm, 
        bool skipEmpty = false) const;

private:
    int m_errorCode;
    std::string m_errorMsg;
    int m_statusCode;
    std::string m_statusMsg;
    HeaderList m_headerList;
    std::string m_message;
};

typedef std::function<void(int, const std::string &)> error_cb;
typedef std::function<void(Response)> response_cb;

class Request
{
public:
    Request();
    ~Request();
    Response get(const std::string& url);
    Response post(const std::string& url, const PairList & args);

    void on_error(error_cb cb);
    void on_response(response_cb cb);

protected:
    void parseURL(const std::string& url,
        std::string* host,
        std::string* res) const;

private:
    error_cb m_error_cb;
    response_cb m_response_cb;
    static asio::io_service s_io_service;
};


#endif //REQUEST_H
