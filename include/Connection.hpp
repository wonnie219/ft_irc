#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <cstddef>

class Connection
{
private:
    // 이 TCP 연결을 식별하는 file descriptor
    //
    // A의 Connection(fd)와
    // B의 Client(fd)가 같은 fd를 공통 식별자로 사용한다.
    int         _fd;


    // recv()로 받은 데이터를 누적하는 버퍼
    //
    // TCP에서는 한 번의 recv()가
    // IRC 명령 하나와 정확히 일치한다는 보장이 없다.
    //
    // 예:
    // recv #1 -> "NICK ra"
    // recv #2 -> "in\r\n"
    //
    // 따라서 먼저 이곳에 누적한다.
    std::string _recvBuffer;


    // 아직 전송하지 못한 데이터를 보관하는 버퍼
    //
    // non-blocking socket에서는 send() 한 번으로
    // 전체 데이터가 전송된다는 보장이 없다.
    //
    // 남은 데이터는 여기에 계속 보관한다.
    std::string _sendBuffer;

public:
    Connection(int fd);
    ~Connection();

    // Connection이 관리하는 socket fd 반환
    int getFd(void) const;


    // 현재 receive buffer 확인
    const std::string& getRecvBuffer(void) const;

    // recv()로 새롭게 들어온 데이터를 뒤에 추가
    void appendRecvBuffer(const std::string& data);

    // 처리 완료된 앞부분 제거
    void eraseRecvBuffer(std::size_t count);


    // 현재 send buffer 확인
    const std::string& getSendBuffer(void) const;

    // B가 보낸 OutgoingMessage 데이터를 전송 대기열에 추가
    void appendSendBuffer(const std::string& data);

    // 실제 send()에 성공한 byte 수만큼 앞부분 제거
    void eraseSendBuffer(std::size_t count);


    // 아직 전송해야 할 데이터가 남아있는지 확인
    bool hasPendingSend(void) const;
};

#endif