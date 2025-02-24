#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>

#include "Packet.h"
#include "Define.h"

#pragma comment(lib, "ws2_32.lib") // 비주얼에서 소켓프로그래밍 하기 위한 것

class User {
public:
	~User() {
        char recvBuffer[PACKET_SIZE];
        memset(recvBuffer, 0, PACKET_SIZE);

        USER_LOGOUT_REQUEST_PACKET ulReq;
        ulReq.PacketId = (UINT16)PACKET_ID::USER_LOGOUT_REQUEST;
        ulReq.PacketLength = sizeof(USER_LOGOUT_REQUEST_PACKET);

        send(userSkt, (char*)&ulReq, sizeof(ulReq), 0);

		WorkRun = false;
		if (workThread.joinable()) workThread.join();
        std::this_thread::sleep_for(std::chrono::seconds(3));
		CloseHandle(udpHandle);
		closesocket(userSkt);
		closesocket(udpSocket);
		WSACleanup();
	}

    bool init() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        webSkt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (webSkt == INVALID_SOCKET) {
            std::cout << "Server Socket Make Fail" << std::endl;
            return false;
        }

        SOCKADDR_IN addr;
        ZeroMemory(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(WEB_SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &addr.sin_addr.s_addr);

        std::cout << "Web Server Connecting..." << std::endl;

        if (connect(webSkt, (SOCKADDR*)&addr, sizeof(addr))) {
            std::cout << "WebServer Connect Fail" << std::endl;
            return false;
        }
            std::cout << "Web Server Connect Success" << std::endl;

            memset(recvBuffer, 0, PACKET_SIZE);

            USERINFO_REQUEST uiReq;
            uiReq.PacketId = (UINT16)WEBPACKET_ID::USERINFO_REQUEST;
            uiReq.PacketLength = sizeof(USERINFO_REQUEST);
            strncpy_s(uiReq.userId, userId.c_str(), MAX_USER_ID_LEN);

            send(webSkt, (char*)&uiReq, sizeof(uiReq), 0); // 유저 정보 요청
            recv(webSkt, recvBuffer, PACKET_SIZE, 0); // 유저 정보
            auto uiPacket = reinterpret_cast<USERINFO_RESPONSE*>(recvBuffer);
            USERINFO tempU = uiPacket->UserInfo;
            exp = tempU.exp;
            level = tempU.level;

            if (tempU.level == 0) {
                std::cout << "Get Userinfo Fail" << std::endl;
                return false;
            }
            std::cout << "Get Userinfo Success" << std::endl;

            EQUIPMENT_REQUEST eqReq;
            eqReq.PacketId = (UINT16)WEBPACKET_ID::EQUIPMENT_REQUEST;
            eqReq.PacketLength = sizeof(EQUIPMENT_REQUEST);

            send(webSkt, (char*)&eqReq, sizeof(eqReq), 0); // 장비 정보 요청
            recv(webSkt, recvBuffer, PACKET_SIZE, 0); // 장비 정보

            auto eqPacket = reinterpret_cast<EQUIPMENT_RESPONSE*>(recvBuffer);
            std::cout << eqPacket->eqCount << std::endl;
            std::vector<EQUIPMENT> tempEq(eqPacket->eqCount);
            char* ptr = recvBuffer + sizeof(PACKET_HEADER) + sizeof(uint16_t);
            return false;
            tempEq = eqPacket->Equipments;
            eq = tempEq;

            if (eq.empty()) {
                std::cout << "Get EQUIPMENT Fail" << std::endl;
                return false;
            }

            std::cout << "Get EQUIPMENT Success" << std::endl;

            CONSUMABLES_REQUEST csReq;
            uiReq.PacketId = (UINT16)WEBPACKET_ID::CONSUMABLES_REQUEST;
            uiReq.PacketLength = sizeof(CONSUMABLES_REQUEST);

            send(webSkt, (char*)&uiReq, sizeof(uiReq), 0); // 게임 시작 준비 요청
            recv(webSkt, recvBuffer, PACKET_SIZE, 0); // 소비 정보
            auto csPacket = reinterpret_cast<CONSUMABLES_RESPONSE*>(recvBuffer);
            std::vector<CONSUMABLES> tempCs(csPacket->csCount);
            tempCs = csPacket->Consumables;
            tempCs = cs;

            if (cs.empty()) {
                std::cout << "Get CONSUMABLES Fail" << std::endl;
                return false;
            }

            std::cout << "Get CONSUMABLES Success" << std::endl;

            MATERIALS_REQUEST mtReq;
            mtReq.PacketId = (UINT16)WEBPACKET_ID::MATERIALS_REQUEST;
            mtReq.PacketLength = sizeof(MATERIALS_REQUEST);

            send(webSkt, (char*)&mtReq, sizeof(mtReq), 0); // 게임 시작 준비 요청
            recv(webSkt, recvBuffer, PACKET_SIZE, 0); // 재료 정보
            auto mtPacket = reinterpret_cast<MATERIALS_RESPONSE*>(recvBuffer);
            std::vector<MATERIALS> tempMt(mtPacket->mtCount);
            tempMt = mtPacket->Materials;
            tempMt = mt;

            if (mt.empty()) {
                std::cout << "Get MATERIALS Fail" << std::endl;
                return false;
            }

            std::cout << "Get MATERIALS Success" << std::endl;

            USER_GAMESTART_REQUEST ugReq;
            ugReq.PacketId = (UINT16)WEBPACKET_ID::USER_GAMESTART_REQUEST;
            ugReq.PacketLength = sizeof(USER_GAMESTART_REQUEST);
            strncpy_s(ugReq.userId, userId.c_str(), MAX_USER_ID_LEN);

            send(webSkt, (char*)&ugReq, sizeof(ugReq), 0); // 게임 시작 준비 요청
            recv(webSkt, recvBuffer, PACKET_SIZE, 0); // 게임 시작을 위한 웹 토큰

            // GET USER UUID
            auto ucReqPacket = reinterpret_cast<USER_GAMESTART_RESPONSE*>(recvBuffer);

			std::string webToken = ucReqPacket->webToken;

            if (webToken=="") { // 웹서버에서 토큰 생성 실패
                std::cout << "Get WebToken Fail" << std::endl;
                return false;
            }

            std::cout << "Connect Success" << std::endl;

            shutdown(webSkt, SD_BOTH);
			closesocket(webSkt); // 웹서버 소켓 닫기
            
            std::cout << "If you Press 1, game start. If you want out, press any key" << std::endl;

            int k = 0;
            std::cin >> k;
            if (k != 1) exit(0);

        userSkt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        ZeroMemory(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(SERVER_TCP_PORT);
        inet_pton(AF_INET, SERVER_IP, &addr.sin_addr.s_addr);

        std::cout << "Quokka Server Connecting..." << std::endl;

        if (connect(userSkt, (SOCKADDR*)&addr, sizeof(addr))) {
            std::cout << "Quokka Server Connect Fail" << std::endl;
            return false;
        }

        USER_CONNECT_REQUEST_PACKET ucReq;
        ucReq.PacketId = (UINT16)PACKET_ID::USER_CONNECT_REQUEST;
        ucReq.PacketLength = sizeof(USER_CONNECT_REQUEST_PACKET);
        strncpy_s(ucReq.userId, userId.c_str(), MAX_USER_ID_LEN);
        strncpy_s(ucReq.userToken, webToken.c_str(), MAX_JWT_TOKEN_LEN);

        std::cout << "Connect Requset To Game Server.." << std::endl;

        send(userSkt, (char*)&ucReq, sizeof(ucReq), 0);
        recv(userSkt, recvBuffer, PACKET_SIZE, 0);

        auto ucResPacket = reinterpret_cast<USER_CONNECT_RESPONSE_PACKET*>(recvBuffer);

        if (ucResPacket->isSuccess == false) return false;

        std::cout << "Connect Success In Game Server" << std::endl;

        udpSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (udpSocket == INVALID_SOCKET) {
            std::cout << "Udp Socket Make Fail Error : " << WSAGetLastError() << std::endl;
            return false;
        }
       
        udpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
        auto bIOCPHandle = CreateIoCompletionPort((HANDLE)udpSocket, udpHandle, (ULONG_PTR)0, 0);

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SERVER_UDP_PORT);
        inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

        std::cout << "Udp Socket Make Success" << std::endl;

        CreateUdpThread();
    }

    bool CreateUdpThread() {
        workThread = std::thread([this]() {UdpWorkThread();});
        return true;
    }

    void MonsterNum(uint16_t mobNum_) {
        EXP_UP_REQUEST euReq;
        euReq.PacketId = (UINT16)PACKET_ID::EXP_UP_REQUEST;
        euReq.PacketLength = sizeof(EXP_UP_REQUEST);
        euReq.mobNum = mobNum_;

        send(userSkt, (char*)&euReq, sizeof(euReq), 0);
        recv(userSkt, recvBuffer, PACKET_SIZE, 0);

        auto ucResPacket = reinterpret_cast<EXP_UP_RESPONSE*>(recvBuffer);

        if (ucResPacket->increaseLevel == 0) { // Only Exp Up
            exp = ucResPacket->currentExp;
            std::cout << mobNum_ << " 몬스터를 잡았습니다 !" << std::endl;
            std::cout << "현재 레벨 : " << level.load() << std::endl;
            std::cout << "현재 경험치 : " << exp << std::endl;
        }
        else { // Level up
            level.fetch_add(ucResPacket->increaseLevel);
            exp = ucResPacket->currentExp;
            std::cout << mobNum_ << " 몬스터를 잡았습니다 !" << std::endl;
            std::cout << "레벨업 했습니다 !" << std::endl;
            std::cout << "현재 레벨 : " << level.load() << std::endl;
            std::cout << "현재 경험치 : " << exp << std::endl;
        }
    }

    std::pair<uint16_t, unsigned int> GetUserLevelExp() {
        std::cout << "현재 레벨 : " << level.load() << std::endl;
        std::cout << "현재 경험치 : " << exp.load() << std::endl;
        return {level, exp};
    }

    //void UDPSend() {

    //}

    //void UDPRecv() {

    //}

    void UdpWorkThread() {
        std::cout << "Start Udp Work Thread" << std::endl;
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

    //void RaidStart() {
    //        char recvBuffer[PACKET_SIZE];
    //        memset(recvBuffer, 0, PACKET_SIZE);

    //        RAID_MATCHING_REQUEST rmReq;
    //        rmReq.PacketId = (UINT16)PACKET_ID::RAID_MATCHING_REQUEST;
    //        rmReq.PacketLength = sizeof(RAID_MATCHING_REQUEST);
    //        rmReq.uuId = uuId;

    //        send(userSkt, (char*)&rmReq, sizeof(rmReq), 0);
    //        std::cout << "Match Insert Waitting " << std::endl;
    //        recv(userSkt, recvBuffer, PACKET_SIZE, 0);

    //        auto rmReqPacket = reinterpret_cast<RAID_MATCHING_RESPONSE*>(recvBuffer);

    //        if (rmReqPacket->insertSuccess) { // Mathing Success
    //            std::cout << "Found Game" << std::endl;
    //            recv(userSkt, recvBuffer, PACKET_SIZE, 0);
    //            auto rrReqPacket = reinterpret_cast<RAID_READY_REQUEST*>(recvBuffer);
    //            delete rrReqPacket;

    //            uint8_t timer = rrReqPacket->timer; // Minutes
    //            uint8_t roomNum = rrReqPacket->roomNum; // If Max RoomNum Up to Short Range, Back to Number One
    //            uint8_t myNum = rrReqPacket->yourNum;
    //            UINT16 udpPort = rrReqPacket->udpPort;   // Server UDP Port Num
    //            mobHp = rrReqPacket->mobHp;

    //            RAID_TEAMINFO_REQUEST rtReq;
    //            rtReq.PacketId = (UINT16)PACKET_ID::RAID_TEAMINFO_REQUEST;
    //            rtReq.PacketLength = sizeof(RAID_TEAMINFO_REQUEST);
    //            rtReq.uuId = uuId;
    //            rtReq.imReady = true;
    //            rtReq.myNum = myNum;
    //            rtReq.roomNum = roomNum;

    //            send(userSkt, (char*)&rtReq, sizeof(rtReq), 0);
    //            std::cout << "Team Info Waitting" << std::endl;
    //            recv(userSkt, recvBuffer, PACKET_SIZE, 0);

    //            auto rmReqPacket = reinterpret_cast<RAID_TEAMINFO_RESPONSE*>(recvBuffer);
    //            uint8_t teamLevel = rmReqPacket->teamLevel;
    //            std::string teamId = rmReqPacket->teamId;

    //            std::cout << "Team Waitting" << std::endl;
    //            recv(userSkt, recvBuffer, PACKET_SIZE, 0);

    //            unsigned int myScore = 0;
    //            unsigned int teamScore = 0;

    //            while (1) {
    //                std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::steady_clock::now() + std::chrono::minutes(timer);
    //                std::cout << "Raid Start" << std::endl;
    //                std::cout << "My ID : " << userId <<"Level : " <<level <<  std::endl;
    //                std::cout << "Team ID : " << teamId << "Level : " <<teamLevel <<  std::endl;

    //                while (mobHp != 0 && (std::chrono::steady_clock::now()<endTime)) { 
    //                    std::cout << "Input Damage" << std::endl;
    //                    unsigned int damage;
    //                    std::cin >> damage;

    //                    RAID_HIT_REQUEST rhReq;
    //                    rhReq.myNum = myNum;
    //                    rhReq.roomNum = roomNum;
    //                    rhReq.damage = damage;

    //                    send(userSkt, (char*)&rhReq, sizeof(rhReq),0);
    //                    recv(userSkt, recvBuffer, PACKET_SIZE, 0);

    //                    auto rhResPacket = reinterpret_cast<RAID_HIT_RESPONSE*>(recvBuffer);

    //                    if (mobHp.load() > rhResPacket->currentMobHp) mobHp.store(rhResPacket->currentMobHp);
    //                    myScore = rhResPacket->yourScore;
    //                    std::cout << "My Socre : " << myScore << std::endl;
    //                }

    //                //RAID_END_REQUEST_TO_SERVER rerts;
    //                //rtReq.PacketId = (UINT16)PACKET_ID::RAID_END_REQUEST_TO_SERVER;
    //                //rtReq.PacketLength = sizeof(RAID_END_REQUEST_TO_SERVER);
    //                //rtReq.uuId = uuId;
    //                //rtReq.roomNum = roomNum;

    //                //send(userSkt,(char*)&rerts, sizeof(rerts),0);
    //                std::cout << "Game End Waitting..." << std::endl;
    //                recv(userSkt, recvBuffer, PACKET_SIZE, 0);

    //                auto rhResPacket = reinterpret_cast<RAID_END_REQUEST*>(recvBuffer);

    //                std::cout << "Raid End. Your Score : "<< rhResPacket->userScore << std::endl;
    //                std::cout << "Raid End. Team Score : "<< rhResPacket->teamScore << std::endl;
    //                break;
    //            }
    //            mobHp = 0;
    //        }
    //        else { // Server Matching Full
    //            std::cout << "Server Matching Full. Matching Fail" << std::endl;
    //        }
    //}

    //void GetRaidScore() {
    //    char recvBuffer[PACKET_SIZE];
    //    memset(recvBuffer, 0, PACKET_SIZE);

    //    unsigned int startRank_ = 1;

    //    RAID_RANKING_REQUEST rrReq;
    //    rrReq.PacketId = (UINT16)PACKET_ID::RAID_RANKING_REQUEST;
    //    rrReq.PacketLength = sizeof(RAID_RANKING_REQUEST);
    //    rrReq.uuId = uuId;

    //    std::vector<std::pair<std::string, unsigned int>> reqScore_;

    //    while (1) {
    //        std::cout << "랭킹확인 1~100" << std::endl;
    //        rrReq.startRank = startRank_;
    //        send(userSkt, (char*)&rrReq, sizeof(rrReq), 0);
    //        recv(userSkt, recvBuffer, PACKET_SIZE, 0);
    //        auto rhResPacket = reinterpret_cast<RAID_RANKING_RESPONSE*>(recvBuffer);
    //        reqScore_ = rhResPacket->reqScore;

    //        for(int i = 0; i < reqScore_.size(); i++) {
    //            std::cout << startRank_+i << "등 아이디 : " << reqScore_[i].first << " 점수 : " << reqScore_[i].second << std::endl;
    //            startRank_++;
    //        }

    //        std::cout << "다음 100명 보기 : 1번, 뒤로가기 : 2번" << std::endl;
    //        uint8_t check;
    //        std::cin >> check;
    //        if (check == 1) {
    //            continue;
    //        }
    //        else {
    //            break;
    //        }
    //    }
    //}

    //void GetInventory() {

    //}

    void End() {
        char recvBuffer[PACKET_SIZE];
        memset(recvBuffer, 0, PACKET_SIZE);

        USER_LOGOUT_REQUEST_PACKET ulReq;
        ulReq.PacketId = (UINT16)PACKET_ID::USER_LOGOUT_REQUEST;
        ulReq.PacketLength = sizeof(USER_LOGOUT_REQUEST_PACKET);

        send(userSkt, (char*)&ulReq, sizeof(ulReq), 0);
    }

private:
    bool WorkRun = false;
    HANDLE udpHandle;

    SOCKET webSkt;
    SOCKET userSkt;
    SOCKET udpSocket;

    std::string userId = "quokka";

    std::atomic<uint16_t> level;
    std::atomic<unsigned int> exp;
    std::atomic<unsigned int> mobHp;

    std::thread workThread;

    std::vector<EQUIPMENT> eq;
    std::vector<CONSUMABLES> cs;
    std::vector<MATERIALS> mt;

    char recvBuffer[PACKET_SIZE];
};