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
            char* ptr = recvBuffer + sizeof(PACKET_HEADER) + sizeof(uint16_t);

            eq.resize(10);

            for (int i = 0; i < eqPacket->eqCount; i++) {
                EQUIPMENT tempE;
                memcpy((char*)&tempE, ptr, sizeof(EQUIPMENT));
                eq[tempE.position] = tempE;
                ptr += sizeof(EQUIPMENT);
            }

            std::cout << "Get EQUIPMENT Success" << std::endl;

            CONSUMABLES_REQUEST csReq;
            csReq.PacketId = (UINT16)WEBPACKET_ID::CONSUMABLES_REQUEST;
            csReq.PacketLength = sizeof(CONSUMABLES_REQUEST);

            send(webSkt, (char*)&csReq, sizeof(csReq), 0);
            recv(webSkt, recvBuffer, PACKET_SIZE, 0); // 소비 정보

            auto csPacket = reinterpret_cast<CONSUMABLES_RESPONSE*>(recvBuffer);

            std::vector<CONSUMABLES> tempCs(csPacket->csCount);
            char* ptr2 = recvBuffer + sizeof(PACKET_HEADER) + sizeof(uint16_t);

            cs.resize(10);

            for (int i = 0; i < eqPacket->eqCount; i++) {
                CONSUMABLES tempCon;
                memcpy((char*)&tempCon, ptr2, sizeof(CONSUMABLES));
                cs[tempCon.position] = tempCon;
                ptr2 += sizeof(CONSUMABLES);
            }

            //if (cs.empty()) {
            //    std::cout << "Get CONSUMABLES Fail" << std::endl;
            //    return false;
            //}

            std::cout << "Get CONSUMABLES Success" << std::endl;

            MATERIALS_REQUEST mtReq;
            mtReq.PacketId = (UINT16)WEBPACKET_ID::MATERIALS_REQUEST;
            mtReq.PacketLength = sizeof(MATERIALS_REQUEST);

            send(webSkt, (char*)&mtReq, sizeof(mtReq), 0); 
            recv(webSkt, recvBuffer, PACKET_SIZE, 0); // 재료 정보

            auto mtPacket = reinterpret_cast<MATERIALS_RESPONSE*>(recvBuffer);

            std::vector<MATERIALS> tempMt(mtPacket->mtCount);
            char* ptr3 = recvBuffer + sizeof(PACKET_HEADER) + sizeof(uint16_t);

            mt.resize(10);

            for (int i = 0; i < eqPacket->eqCount; i++) {
                MATERIALS tempM;
                memcpy((char*)&tempM, ptr3, sizeof(MATERIALS));
                mt[tempM.position] = tempM;
                ptr3 += sizeof(MATERIALS);
            }

            //if (mt.empty()) {
            //    std::cout << "Get MATERIALS Fail" << std::endl;
            //    return false;
            //}

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

        udpAddr.sin_family = AF_INET;
        udpAddr.sin_port = htons(SERVER_UDP_PORT);
        inet_pton(AF_INET, SERVER_IP, &udpAddr.sin_addr);

        std::cout << "Udp Socket Make Success" << std::endl;

        CreateUdpThread();

        std::cout << userId << " 게임 접속 성공 !" << std::endl;
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
            std::cout << "들어옴?" << std::endl;
            if (overlapped->taskType == TaskType::UDP_RECV) { // 레이드 몹 hp 동기화 요청
                auto hp = reinterpret_cast<unsigned int*>(overlapped->wsaBuf.buf);
                std::cout <<"Current Mob Hp : " << mobHp << std::endl;
                mobHp.store(*hp);
                delete[] overlapped->wsaBuf.buf;
                delete overlapped;
            }
        }
    }

    void GetInventory(uint16_t invenNum_) {
        if (invenNum_ == 1) {
            std::cout << "장비 인벤토리" << std::endl;
            for (int i = 0; i < eq.size(); i++) {
                if (eq[i].itemCode == 0) continue;
                std::cout << eq[i].position << "번 위치에 +" << eq[i].enhance << "강화 되어있는 " << eq[i].itemCode << "번 아이템 존재" << std::endl;
            }
        }
        else if (invenNum_ == 2) {
            std::cout << "소비 인벤토리" << std::endl;
            for (int i = 0; i < eq.size(); i++) {
                if (cs[i].itemCode == 0) continue;
                std::cout << cs[i].position << "번 위치에 " << cs[i].itemCode << "번 아이템 " << cs[i].count <<"개 존재"<< std::endl;
            }
        }
        else if (invenNum_ == 3) {
            std::cout << "재료 인벤토리" << std::endl;
            for (int i = 0; i < eq.size(); i++) {
                if (mt[i].itemCode == 0) continue;
                std::cout << mt[i].position << "번 위치에 " << mt[i].itemCode << "번 아이템 " << mt[i].count << "개 존재" << std::endl;
            }
        }
        return;
    }

    bool MoveItem(uint16_t invenNum_, uint16_t currentpos_, uint16_t movepos_) {
        if (invenNum_ == 1) { // 장비
            MOV_EQUIPMENT_REQUEST miReq;
            miReq.PacketId = (UINT16)PACKET_ID::MOV_EQUIPMENT_REQUEST;
            miReq.PacketLength = sizeof(MOV_EQUIPMENT_REQUEST);

            EQUIPMENT currentE = eq[currentpos_];
            EQUIPMENT moveE = eq[movepos_];
            
            miReq.dragItemCode = moveE.itemCode;
            miReq.dragItemEnhance = moveE.enhance;
            miReq.dragItemSlotPos = currentE.position;
            miReq.targetItemCode = currentE.itemCode;
            miReq.targetItemEnhance = currentE.enhance;
            miReq.targetItemSlotPos = moveE.position;

            send(userSkt, (char*)&miReq, sizeof(miReq), 0);
            recv(userSkt, recvBuffer, PACKET_SIZE, 0);

            auto miResPacket = reinterpret_cast<MOV_EQUIPMENT_RESPONSE*>(recvBuffer);

            if (!miResPacket->isSuccess) return false;
            return true;
        }
        else { // 소비 or 재료
            MOV_ITEM_REQUEST miReq;
            miReq.PacketId = (UINT16)PACKET_ID::MOV_ITEM_REQUEST;
            miReq.PacketLength = sizeof(MOV_ITEM_REQUEST);

            if (invenNum_ == 2) { // 소비
                CONSUMABLES currentC = cs[currentpos_];
                CONSUMABLES moveC = cs[movepos_];

                miReq.ItemType = invenNum_ - 1;
                miReq.dragItemCode = moveC.itemCode;
                miReq.dragItemCount = moveC.count;
                miReq.dragItemSlotPos = currentC.position; 
                miReq.targetItemCode = currentC.itemCode;
                miReq.targetItemCount = currentC.count;
                miReq.targetItemSlotPos = moveC.position;

                send(userSkt, (char*)&miReq, sizeof(miReq), 0);
                recv(userSkt, recvBuffer, PACKET_SIZE, 0);

                auto miResPacket = reinterpret_cast<MOV_ITEM_RESPONSE*>(recvBuffer);

                if (!miResPacket->isSuccess) return false;
                return true;
            }
            else if (invenNum_== 3) {
                MATERIALS currentE = mt[currentpos_];
                MATERIALS moveE = mt[movepos_];

                miReq.ItemType = invenNum_ - 1;
                miReq.dragItemCode = moveE.itemCode;
                miReq.dragItemCount = moveE.count;
                miReq.dragItemSlotPos = currentE.position;
                miReq.targetItemCode = currentE.itemCode;
                miReq.targetItemCount = currentE.count;
                miReq.targetItemSlotPos = moveE.position;

                send(userSkt, (char*)&miReq, sizeof(miReq), 0);
                recv(userSkt, recvBuffer, PACKET_SIZE, 0);

                auto miResPacket = reinterpret_cast<MOV_ITEM_RESPONSE*>(recvBuffer);

                if (!miResPacket->isSuccess) return false;
                return true;
            }
        }
    }

    bool AddItem(uint16_t invenNum_,uint16_t itemCode_, uint16_t count_) {

    }

    bool DeleteItem(uint16_t invenNum_,uint16_t pos_) {

    }

    void RaidStart() {
        RAID_MATCHING_REQUEST rmReq;
        rmReq.PacketId = (UINT16)PACKET_ID::RAID_MATCHING_REQUEST;
        rmReq.PacketLength = sizeof(RAID_MATCHING_REQUEST);

        send(userSkt, (char*)&rmReq, sizeof(rmReq), 0);
        std::cout << "Match Insert Waitting " << std::endl;
        recv(userSkt, recvBuffer, PACKET_SIZE, 0);

            auto rmReqPacket = reinterpret_cast<RAID_MATCHING_RESPONSE*>(recvBuffer);

            if (!rmReqPacket->insertSuccess) { // Mathing Success
                std::cout << "Server Matching Full. Matching Fail" << std::endl;
                return;
            }

            std::cout << "Match Insert Success" << std::endl;
            std::cout << "Team Waitting" << std::endl;
            recv(userSkt, recvBuffer, PACKET_SIZE, 0);
            auto rrReqPacket = reinterpret_cast<RAID_READY_REQUEST*>(recvBuffer);

            timer = rrReqPacket->timer; // Minutes
            roomNum = rrReqPacket->roomNum; // If Max RoomNum Up to Short Range, Back to Number One
            myNum = rrReqPacket->yourNum;
            mobHp = rrReqPacket->mobHp;

            RAID_TEAMINFO_REQUEST rtReq;
            rtReq.PacketId = (UINT16)PACKET_ID::RAID_TEAMINFO_REQUEST;
            rtReq.PacketLength = sizeof(RAID_TEAMINFO_REQUEST);
            rtReq.imReady = true;
            rtReq.myNum = myNum;
            rtReq.roomNum = roomNum;
            rtReq.userAddr = udpAddr;

            send(userSkt, (char*)&rtReq, sizeof(rtReq), 0);
            std::cout << "Team Info Waitting" << std::endl;
            recv(userSkt, recvBuffer, PACKET_SIZE, 0);

            auto rtiReqPacket = reinterpret_cast<RAID_TEAMINFO_RESPONSE*>(recvBuffer);
            uint16_t teamLevel = rtiReqPacket->teamLevel;
            std::string teamId = (std::string)rtiReqPacket->teamId;

            std::cout << "Team Waitting" << std::endl;
            recv(userSkt, recvBuffer, PACKET_SIZE, 0);

            auto rsReqPacket = reinterpret_cast<RAID_START_REQUEST*>(recvBuffer);

            unsigned int myScore = 0;
            unsigned int teamScore = 0;

            std::cout << "Raid Start !" << std::endl;
            std::cout << "Mob Hp : " << mobHp << std::endl;
            std::cout << "My ID : " << userId << " / Level : " << level << std::endl;
            std::cout << "Team ID : " << teamId << " / Level : " << teamLevel << std::endl;

            rEndTime = rsReqPacket->endTime;

            while (1) {
                while (mobHp >= 0 || (std::chrono::steady_clock::now() < rEndTime)) {
                    std::cout << "Input Damage" << std::endl;
                    unsigned int damage;
                    std::cin >> damage;

                    RAID_HIT_REQUEST rhReq;
                    rhReq.PacketId = (UINT16)PACKET_ID::RAID_HIT_REQUEST;
                    rhReq.PacketLength = sizeof(RAID_HIT_REQUEST);
                    rhReq.myNum = myNum;
                    rhReq.roomNum = roomNum;
                    rhReq.damage = damage;

                    send(userSkt, (char*)&rhReq, sizeof(rhReq), 0);
                    recv(userSkt, recvBuffer, PACKET_SIZE, 0);

                    auto rhResPacket = reinterpret_cast<RAID_HIT_RESPONSE*>(recvBuffer);

                    if (rhResPacket->currentMobHp <= 0) { // mob dead
                        if (rhResPacket->yourScore !=0) {
                            std::cout << "My Socre : " << rhResPacket->yourScore << std::endl;
                        }
                        std::cout << "Game End Waitting..." << std::endl;

                        recv(userSkt, recvBuffer, PACKET_SIZE, 0);

                        auto reReq = reinterpret_cast<RAID_END_REQUEST*>(recvBuffer);
                        std::cout << "Raid End. Your Score : " << reReq->userScore << std::endl;
                        std::cout << "Raid End. Team Score : " << reReq->teamScore << std::endl;
                        std::cout << "Raid End." << std::endl;
                        break;
                    }
                    else {
                        if (mobHp.load() > rhResPacket->currentMobHp) mobHp.store(rhResPacket->currentMobHp);
                        myScore = rhResPacket->yourScore;
                        std::cout << "My Socre : " << rhResPacket->yourScore << std::endl;
                    }
                }
                break;
            }
            mobHp = 0;
            timer = 0;
            roomNum = 0;
            myNum = 0;
    }

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
    std::atomic<uint16_t> level;
    std::atomic<unsigned int> exp;

    // Raid
    std::atomic<int> mobHp;
    uint16_t timer;
    uint16_t roomNum; 
    uint16_t myNum;

    SOCKET webSkt;
    SOCKET userSkt;
    SOCKET udpSocket;
    HANDLE udpHandle;
    std::thread workThread;

    sockaddr_in udpAddr;
    std::string userId = "quokka";

    std::chrono::time_point<std::chrono::steady_clock> rEndTime;

    std::vector<EQUIPMENT> eq;
    std::vector<CONSUMABLES> cs;
    std::vector<MATERIALS> mt;

    char recvBuffer[PACKET_SIZE];
};