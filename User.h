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

        userSkt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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

        udpSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (udpSocket == INVALID_SOCKET) {
            std::cerr << "Udp Socket Make Fail Error : " << WSAGetLastError() << std::endl;
            return false;
        }
       
        udpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
        auto bIOCPHandle = CreateIoCompletionPort((HANDLE)udpSocket, udpHandle, (ULONG_PTR)0, 0);

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SERVER_UDP_PORT);
        inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

        std::cout << "Udp Socket Make Success" << std::endl;

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
                udpHandle,
                &dwIoSize,
                nullptr,
                &lpOverlapped,
                INFINITE
            );

            auto overlapped = (OverlappedUDP*)lpOverlapped;

            if (overlapped->taskType == TaskType::UDP_RECV) { // 레이드 몹 hp 동기화 요청
                auto hp = reinterpret_cast<unsigned int*>(overlapped->wsaBuf.buf);
                std::cout <<"Current Mob Hp : " << mobHp << std::endl;
                mobHp.store(*hp);
                delete[] overlapped->wsaBuf.buf;
                delete overlapped;
            }
        }
    }

    void RaidStart() {
            char recvBuffer[PACKET_SIZE];
            memset(recvBuffer, 0, PACKET_SIZE);

            RAID_MATCHING_REQUEST rmReq;
            rmReq.PacketId = (UINT16)PACKET_ID::RAID_MATCHING_REQUEST;
            rmReq.PacketLength = sizeof(RAID_MATCHING_REQUEST);
            rmReq.uuId = uuId;

            send(userSkt, (char*)&rmReq, sizeof(rmReq), 0);
            std::cout << "Match Insert Waitting " << std::endl;
            recv(userSkt, recvBuffer, PACKET_SIZE, 0);

            auto rmReqPacket = reinterpret_cast<RAID_MATCHING_RESPONSE*>(recvBuffer);

            if (rmReqPacket->insertSuccess) { // Mathing Success
                std::cout << "Found Game" << std::endl;
                recv(userSkt, recvBuffer, PACKET_SIZE, 0);
                auto rrReqPacket = reinterpret_cast<RAID_READY_REQUEST*>(recvBuffer);
                delete rrReqPacket;

                uint8_t timer = rrReqPacket->timer; // Minutes
                uint8_t roomNum = rrReqPacket->roomNum; // If Max RoomNum Up to Short Range, Back to Number One
                uint8_t myNum = rrReqPacket->yourNum;
                UINT16 udpPort = rrReqPacket->udpPort;   // Server UDP Port Num
                mobHp = rrReqPacket->mobHp;

                RAID_TEAMINFO_REQUEST rtReq;
                rtReq.PacketId = (UINT16)PACKET_ID::RAID_TEAMINFO_REQUEST;
                rtReq.PacketLength = sizeof(RAID_TEAMINFO_REQUEST);
                rtReq.uuId = uuId;
                rtReq.imReady = true;
                rtReq.myNum = myNum;
                rtReq.roomNum = roomNum;

                send(userSkt, (char*)&rtReq, sizeof(rtReq), 0);
                std::cout << "Team Info Waitting" << std::endl;
                recv(userSkt, recvBuffer, PACKET_SIZE, 0);

                auto rmReqPacket = reinterpret_cast<RAID_TEAMINFO_RESPONSE*>(recvBuffer);
                uint8_t teamLevel = rmReqPacket->teamLevel;
                std::string teamId = rmReqPacket->teamId;

                std::cout << "Team Waitting" << std::endl;
                recv(userSkt, recvBuffer, PACKET_SIZE, 0);

                unsigned int myScore = 0;
                unsigned int teamScore = 0;

                while (1) {
                    std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::steady_clock::now() + std::chrono::minutes(timer);
                    std::cout << "Raid Start" << std::endl;
                    std::cout << "My ID : " << userId <<"Level : " <<level <<  std::endl;
                    std::cout << "Team ID : " << teamId << "Level : " <<teamLevel <<  std::endl;

                    while (mobHp != 0 && (std::chrono::steady_clock::now()<endTime)) { 
                        std::cout << "Input Damage" << std::endl;
                        unsigned int damage;
                        std::cin >> damage;

                        RAID_HIT_REQUEST rhReq;
                        rhReq.myNum = myNum;
                        rhReq.roomNum = roomNum;
                        rhReq.damage = damage;

                        send(userSkt, (char*)&rhReq, sizeof(rhReq),0);
                        recv(userSkt, recvBuffer, PACKET_SIZE, 0);

                        auto rhResPacket = reinterpret_cast<RAID_HIT_RESPONSE*>(recvBuffer);

                        if (mobHp.load() > rhResPacket->currentMobHp) mobHp.store(rhResPacket->currentMobHp);
                        myScore = rhResPacket->yourScore;
                        std::cout << "My Socre : " << myScore << std::endl;
                    }

                    RAID_END_REQUEST_TO_SERVER rerts;
                    rtReq.PacketId = (UINT16)PACKET_ID::RAID_END_REQUEST_TO_SERVER;
                    rtReq.PacketLength = sizeof(RAID_END_REQUEST_TO_SERVER);
                    rtReq.uuId = uuId;
                    rtReq.roomNum = roomNum;

                    send(userSkt,(char*)&rerts, sizeof(rerts),0);
                    std::cout << "Game End Waitting..." << std::endl;
                    recv(userSkt, recvBuffer, PACKET_SIZE, 0);

                    auto rhResPacket = reinterpret_cast<RAID_END_REQUEST*>(recvBuffer);

                    std::cout << "Raid End. Your Score : "<< rhResPacket->userScore << std::endl;
                    std::cout << "Raid End. Team Score : "<< rhResPacket->teamScore << std::endl;
                    break;
                }
                mobHp = 0;
            }
            else { // Server Matching Full
                std::cout << "Server Matching Full. Matching Fail" << std::endl;
            }
    }

    void GetRaidScore() {
        char recvBuffer[PACKET_SIZE];
        memset(recvBuffer, 0, PACKET_SIZE);

        unsigned int startRank_ = 1;

        RAID_RANKING_REQUEST rrReq;
        rrReq.PacketId = (UINT16)PACKET_ID::RAID_RANKING_REQUEST;
        rrReq.PacketLength = sizeof(RAID_RANKING_REQUEST);
        rrReq.uuId = uuId;

        std::vector<std::pair<std::string, unsigned int>> reqScore_;

        while (1) {
            std::cout << "랭킹확인 1~100" << std::endl;
            rrReq.startRank = startRank_;
            send(userSkt, (char*)&rrReq, sizeof(rrReq), 0);
            recv(userSkt, recvBuffer, PACKET_SIZE, 0);
            auto rhResPacket = reinterpret_cast<RAID_RANKING_RESPONSE*>(recvBuffer);
            reqScore_ = rhResPacket->reqScore;

            for(int i = 0; i < reqScore_.size(); i++) {
                std::cout << startRank_+i << "등 아이디 : " << reqScore_[i].first << " 점수 : " << reqScore_[i].second << std::endl;
                startRank_++;
            }

            
            std::cout << "다음 100명 보기 : 1번, 뒤로가기 : 2번" << std::endl;
            uint8_t check;
            std::cin >> check;
            if (check == 1) {
                continue;
            }
            else {
                break;
            }
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
    HANDLE udpHandle;

    SOCKET userSkt;
    SOCKET udpSocket;

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