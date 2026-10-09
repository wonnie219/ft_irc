#ifndef COMMON_HPP
#define COMMON_HPP

#include <string>

struct IncomingMessage
{
    int         fd;
    std::string line;
};

struct OutgoingMessage
{
    int         fd;
    std::string data;
};

#endif

/*
A → B

IncomingMessage
fd   : 어느 연결에서 왔는가
line : A가 \r\n 기준으로 완성시킨 IRC 한 줄
*/

/*
B → A

OutgoingMessage
fd   : 어느 연결로 보낼 것인가
data : 실제 전송할 IRC 문자열
*/