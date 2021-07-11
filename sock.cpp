//
// Created by vir1ibus on 7/7/21.
//

#include "sock.h"
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <vector>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sstream>
#include <fstream>

std::string getProtocol( std::string url ) {
    std::string protocol = "";

    int i = 0;

    for(i = 0; i < url.size(); i++) {
        if( url[i] != '/' || url[i+1] != '/'  ) {
            protocol += url[i];
        }
        else {
            protocol += "//";
            break;
        }
    }

    return protocol;
}

std::string getHost( std::string url ) {
    std::string host = "";

    url.replace(0, getProtocol(url).size(), "");

    int i = 0;
    for(i = 0; i < url.size(); i++) {
        if( url[i] != '/' ) {
            host += url[i];
        }
        else {
            break;
        }

    }

    return host;
}

std::string getAction( std::string url ) {
    std::string parm = "";

    url.replace(0, getProtocol(url).size()+getHost(url).size(), "");

    int i = 0;
    for(i = 0; i < url.size(); i++) {

        if( url[i] != '?' && url[i] != '#' ) {
            parm += url[i];
        }
        else {
            break;
        }

    }

    return parm;
}

std::string getParams( std::vector< std::pair< std::string, std::string> > requestData ) {
    std::string parm = "";

    std::vector< std::pair< std::string, std::string> >::iterator itr = requestData.begin();

    for( ; itr != requestData.end(); ++itr ) {
        if( parm.size() < 1 ) {
            parm += "";
        }
        else {
            parm += "&";
        }
        parm += itr->first + "=" + itr->second;
    }

    return parm;
}


std::string GET( std::string url, std::vector< std::pair< std::string, std::string> > requestData )
{
    std::string http = getProtocol(url);
    std::string host = getHost(url);
    std::string script = getAction(url);
    std::string parm = "";

    std::cout << http + "\n" + host + "\n" + script + "\n" + parm + "\n";

    char buf[1024];

    std::string header = "";

    header += "GET ";
    header += (std::string) script + "\r\n";
    header += (std::string)"HTTP/1.1" + "\r\n";
    header += (std::string)"Host: " + host + "\r\n";
    header += (std::string)"User-Agent: Mozilla/5.0" + "\r\n";
    header += (std::string)"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8" + "\r\n";
    header += (std::string)"Accept-Language: en-US,en;q=0.5" + "\r\n";
    header += (std::string)"Accept-Encoding: gzip, deflate, br" + "\r\n";
    header += (std::string)"Connection: keep-alive " + "\r\n";
    header += (std::string)"Upgrade-Insecure-Requests: 1" + "\r\n";
    header += (std::string)"Cache-Control: max-age=0" + "\r\n";
    header += "\r\n";

    std:: cout << header;

    int sock;
    struct sockaddr_in addr;
    struct hostent* raw_host;
    raw_host = gethostbyname( host.c_str() );
    if (raw_host == NULL) {
        std::cout<<"ERROR, no such host";
        exit(0);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    bcopy( (char*)raw_host->h_addr, (char*)&addr.sin_addr, raw_host->h_length );

    if( connect( sock, (struct sockaddr *)&addr, sizeof(addr) ) < 0) {
        std::cerr<<"connect error"<<std::endl;
        exit(2);
    }

    char * message = new char[ header.size() ];
    for(int i = 0; i < header.size(); i++) {
        message[i] = header[i];
    }

    send(sock, message, header.size(), 0);
    recv(sock, buf, sizeof(buf), 0);

    std::string answer = "";

    for(int j = 0; j < 1024; j++) {
      answer += buf[j];
    }

    return answer;

}

int main(){
    std::string key("user");
    std::string value("me");
    std::pair<std::string, std::string> data { "", ""};
    std::vector<std::pair<std::string, std::string>> request = { data };
    std::cout << GET("https://api.telegram.org/bot1620046312:AAEbb89mVnnnO1KvPwA2YY0hf9lRGQbgg-Q/getMe", request);
    return 0;
}
