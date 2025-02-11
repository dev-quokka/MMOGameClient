#pragma once
#include "Packet.h"
#include "Define.h"

#include <vector>
#include <atomic>

#include <mysql.h>

#pragma comment (lib, "libmysql.lib") // mysql 연동

class User {
public:
    bool init() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        userSkt = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

        userHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
        auto bIOCPHandle = CreateIoCompletionPort((HANDLE)userSkt, userHandle, (UINT32)0, 0);


        SOCKADDR_IN addr;
        ZeroMemory(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(SERVER_TCP_PORT);
        inet_pton(AF_INET, SERVER_IP, &addr.sin_addr.s_addr);

        if (!connect(userSkt, (SOCKADDR*)&addr, sizeof(addr))) {
            char recvBuffer[PACKET_SIZE];
            memset(recvBuffer, 0, PACKET_SIZE);

            USER_CONNECT_REQUEST_PACKET ucReq;
            ucReq.PacketId = (UINT16)PACKET_ID::USER_CONNECT_REQUEST;
            ucReq.PacketLength = sizeof(USER_CONNECT_REQUEST_PACKET);
            ucReq.userId = userId;
            ucReq.uuId = "";

            send(userSkt, (char*)&ucReq, sizeof(ucReq), 0);
            recv(userSkt, recvBuffer, PACKET_SIZE, 0);

            // GET USER UUID
            auto ucReqPacket = reinterpret_cast<USER_CONNECT_RESPONSE_PACKET*>(recvBuffer);

            level = ucReqPacket->level;
            exp = ucReqPacket->currentExp;
            uuId = ucReqPacket->uuId;
        }

        equipment.resize(30);
        consumable.resize(30);
        material.resize(30);

        CreateUdpThread();
        GetInventory();
    }

    bool CreateUdpThread() {
        workThread = std::thread([this]() {WorkThread();});
    }

    void UDPSend() {

    }

    void UDPRecv() {

    }

    void WorkThread() {
        std::cout << "Start Work Thread" << std::endl;
        LPOVERLAPPED lpOverlapped = NULL;
        DWORD dwIoSize = 0;
        bool gqSucces = TRUE;

        while (WorkRun) {
            gqSucces = GetQueuedCompletionStatus(
                userHandle,
                &dwIoSize,
                nullptr,
                &lpOverlapped,
                INFINITE
            );

            auto overlapped = (OverlappedUDP*)lpOverlapped;

            if (overlapped->taskType == TaskType::UDP_RECV) { // 레이드 몹 hp 동기화 요청
                auto udp = (OverlappedUDP*)lpOverlapped;
                auto hp = reinterpret_cast<unsigned int*>(udp->wsaBuf.buf);
                mobHp.store(*hp);

                delete[] udp->wsaBuf.buf;
                delete udp;
            }
        }
    }

    void RaidStart() {
        while (1) {

        }
    }

    void GetInventory() {
        MYSQL Conn;
        MYSQL* ConnPtr = NULL;
        MYSQL_RES* Result;
        MYSQL_ROW Row;
        int MysqlResult;

        mysql_init(&Conn);
        ConnPtr = mysql_real_connect(&Conn, "127.0.0.1", "root", "1234", "quokka_server", 3306, (char*)NULL, 0);

        if (ConnPtr == NULL) {
            std::cout << "Fail To Connect Mysql" << std::endl;
        }

        std::cout << "Success To Connect Mysql" << std::endl;


    }

    void End() {
        WSACleanup();
    }

private:
    bool WorkRun = false;
    HANDLE userHandle;

    SOCKET userSkt;

    std::atomic<uint8_t> level;
    std::atomic<unsigned int> exp;
    std::atomic<unsigned int> mobHp;

    std::string uuId;
    std::string userId = "quokka";

    std::thread workThread;

    std::vector<Equipment> equipment;
    std::vector<Consumable> consumable;
    std::vector<Material> material;
};