#include "Request.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/StreamCopier.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/URI.h"

std::string Request::httpGET(const std::string& url){
    std::string result;
    try{
        Poco::URI poco_url(url);
        Poco::Net::HTTPClientSession session(poco_url.getHost(), poco_url.getPort());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_1_1);
        session.sendRequest(request);
        Poco::Net::HTTPResponse response;
        std::istream &in = session.receiveResponse(response);
        Poco::StreamCopier::copyToString(in, result);
    }catch(Poco::Net::NetException &ex){
        errorMessage = ex.displayText();
        return "";
    }

    return result;
}

std::string Request::httpPOST(const std::string& url,
                                   const std::map<std::string, std::string> &data){
    std::string result;
    try{
        Poco::URI poco_url(url);
        Poco::Net::HTTPClientSession session(poco_url.getHost(), poco_url.getPort());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, poco_url.getPath(), Poco::Net::HTTPRequest::HTTP_1_1);
        Poco::Net::HTMLForm form;
        for(auto iter = data.begin(); iter != data.end(); iter++){
            form.add(iter->first, iter->second);
        }
        form.prepareSubmit(request);
        form.write(session.sendRequest(request));
        Poco::Net::HTTPResponse response;
        std::istream &in = session.receiveResponse(response);
        Poco::StreamCopier::copyToString(in, result);
    }catch(Poco::Net::NetException &ex){
        errorMessage = ex.displayText();
        return "";
    }
    return result;
}
