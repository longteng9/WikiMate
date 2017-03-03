#include "Request.h"
#include <iostream>
#include <sstream>
#include <fstream>

#ifndef BLOCK_NET_ENTRY_QUERY
asio::io_service Request::s_io_service;

Response::Response(){
    m_errorCode = 0;
    m_errorMsg = "";
    m_statusCode = -1;
    m_statusMsg = "";
    m_message = "";
}

int Response::errorCode() const{
    return m_errorCode;
}

const std::string& Response::errorMsg() const{
    return m_errorMsg;
}

int Response::statusCode() const{
    return m_statusCode;
}

const HeaderList& Response::headerList() const{
    return m_headerList;
}

const std::string& Response::content() const{
    return m_message;
}

void Response::assignHeaderList(const std::string& header){
    std::vector<std::string> lines = split(header, "\r\n", true);
    std::vector<std::string> pair;
    for (int i = 0; i < lines.size(); i++){
        pair = split(lines[i], ":", true);
        if (pair.size() > 2){
            std::string combo = "";
            for (int j = 1; j < pair.size(); j++){
                combo += pair[j] + ":";
            }
            combo.erase(combo.size() - 1);
            m_headerList.push_back(std::make_pair(trim(pair[0]), combo));
        }else{
            m_headerList.push_back(std::make_pair(trim(pair[0]), trim(pair[1])));
        }
    }
}

std::string Response::trim(const std::string& str) const{
    if (str.empty()) {
        return str;
    }
    std::string str2 = str;
    str2.erase(0, str2.find_first_not_of(" \r\n\t"));
    str2.erase(str2.find_last_not_of(" \r\n\t") + 1);
    return str2;
}

std::vector<std::string> Response::split(const std::string& str,
    const std::string& delm,
    bool skipEmpty) const{
    std::vector<std::string> result;
    if (str.empty()){
        return result;
    }

    std::string tmp;
    std::string::size_type pos_begin = str.find_first_not_of(delm);
    std::string::size_type comma_pos = 0;

    while (pos_begin != std::string::npos){
        comma_pos = str.find(delm, pos_begin);
        if (comma_pos != std::string::npos){
            tmp = str.substr(pos_begin, comma_pos - pos_begin);
            pos_begin = comma_pos + delm.length();
        }
        else{
            tmp = str.substr(pos_begin);
            pos_begin = comma_pos;
        }

        if (!tmp.empty() || !skipEmpty){
            result.push_back(tmp);
            tmp.clear();
        }
    }
    return result;
}

Request::Request()
{
}


Request::~Request()
{
}

Response Request::get(const std::string& url){
    Response response;
    try{
        std::string host = "", res = "";
        parseURL(url, &host, &res);

        asio::error_code ec;
        asio::ip::tcp::resolver resolver(s_io_service);
        asio::ip::tcp::resolver::query query(host, "http");
        asio::ip::tcp::resolver::iterator endpoint_iter = resolver.resolve(query);
        asio::ip::tcp::socket socket(s_io_service);
        asio::connect(socket, endpoint_iter, ec);
        if (!socket.is_open()){
            response.m_errorCode = ec.value();
            response.m_errorMsg = ec.message();
            return response;
        }

        asio::streambuf request;
        std::ostream stream_w(&request);
        stream_w << "GET " << res << " HTTP/1.0\r\n";
        stream_w << "Host: " << host << "\r\n";
        stream_w << "Accept: */*\r\n";
        stream_w << "Connection: close\r\n";
        stream_w << "\r\n";

        // 发送请求头
        if (asio::write(socket, request, ec) <= 0){
            response.m_errorCode = ec.value();
            response.m_errorMsg = ec.message();
            return response;
        }

        // 读取所有响应（状态行 + header + message）
        asio::streambuf reply;
        asio::streambuf::const_buffers_type buff = reply.data();
        std::string reply_str = "";
        while (asio::read(socket, reply, asio::transfer_at_least(1), ec)){
        }
        buff = reply.data();
        reply_str.append(std::string(asio::buffers_begin(buff), asio::buffers_end(buff)));
        if (ec != asio::error::eof){
            response.m_errorCode = ec.value();
            response.m_errorMsg = ec.message();
            return response;
        }

        // 分割读取到的响应
        const std::string::size_type pos = reply_str.find("\r\n");
        const std::string::size_type pos2 = reply_str.find("\r\n\r\n", pos + 1);

        if (pos == std::string::npos || pos2 == std::string::npos){
            response.m_errorCode = -1;
            response.m_errorMsg = "invalid header part";
            return response;
        }
        std::string statusLine = reply_str.substr(0, pos);
        std::string header = reply_str.substr(pos + 2, pos2 - (pos + 2));

        // 开始读取回应的状态行
        std::stringstream status_stream(statusLine);
        std::string httpVersion = "";
        response.m_statusCode = -1;
        response.m_statusMsg = "";
        status_stream >> httpVersion;
        status_stream >> response.m_statusCode;
        status_stream >> response.m_statusMsg;

        // 判断状态是否有效
        if (!status_stream
            || httpVersion.substr(0, 5) != "HTTP/"
            || response.m_statusCode != 200){
            return response;
        }

        // 分割响应的头部信息
        response.assignHeaderList(header);

        // 分割响应的消息体（直到EOF）
        if (pos2 + 4 < reply_str.size()){
            response.m_message.append(reply_str.substr(pos2 + 4));
        }
    }catch(std::exception &e){
        std::cerr << "exception while requesting http:"<< e.what();
    }
    return response;
}

void Request::parseURL(const std::string& url, 
    std::string* host,
    std::string* res) const{
    std::string::size_type start = 0;
    std::string::size_type pos = url.find("://");
    if (pos == std::string::npos){
        pos = 0;
    } else {
        pos += 3;
    }
    start = pos;

    pos = url.find("/", pos);
    if (pos == std::string::npos){
        *host = url.substr(start);
        *res = "/";
    } else {
        *host = url.substr(start, pos - start);
        *res = url.substr(pos);
    }
}

Response Request::post(const std::string& url, const PairList & args){
    Response response;

    return response;
}

void Request::on_error(error_cb cb){
    m_error_cb = cb;
}

void Request::on_response(response_cb cb){
    m_response_cb = cb;
}
#endif
