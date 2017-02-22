#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include <string>
#include <map>

class Request
{
public:
    std::string httpGET(const std::string& url);
    std::string httpPOST(const std::string& url,
                                       const std::map<std::string, std::string> &data);

public:
    std::string errorMessage = "";
};

#endif // REQUEST_H
