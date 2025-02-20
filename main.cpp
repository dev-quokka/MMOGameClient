#pragma once

#define SERVER_IP "127.0.0.1"
#define WEB_SERVER_PORT 9000
#define SERVER_TCP_PORT 8080
#define SERVER_UDP_PORT 50000
#define PACKET_SIZE 1024

#include "Packet.h"
#include "Define.h"
#include "User.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <thread>
#include <iostream>
#include <string>
#include <vector>

int main() {
    User user;

    if (!user.init()) {
        std::cout << "Connect Fail" << std::endl;
        return 0;
    }

    bool outCheck = false;
    outCheck = true;

    while (outCheck) {
        std::cout << std::endl;
        uint8_t select;

        std::cout << "========================" << std::endl;
        std::cout << "===   1. 내 정보     ===" << std::endl;
        std::cout << "===   2. 인벤토리    ===" << std::endl;
        std::cout << "===   3. 레이드 매칭 ===" << std::endl;
        std::cout << "===   4. 레이드 랭킹 ===" << std::endl;
        std::cout << "===   5. 로그아웃    ===" << std::endl;
        std::cout << "========================" << std::endl;

        std::cin >> select;

        switch (select) {
        case 1: {

            break;
        }
        case 2: {
            int checknum;
            std::cout << "원하는 인벤토리 번호 누르고 엔터를 눌러주세요." << std::endl;
            std::cout << "1. 장비 " << "2. 소비 " << "3. 재료 " << "4. 뒤로가기" << std::endl;
            std::cin >> checknum;

            if (checknum == 1) {

            }
            else if (checknum == 2) {

            }
            else if (checknum == 3) {

            }

            else break;
        }

        case 3:
        {
            //user.RaidStart();
            break;
        }
        case 4: {
            //user.GetRaidScore();
            break;
        }
        case 5: {
            outCheck = false;
            user.End();
            break;
        }

        }
    }
    return 0;
}

// ========================== USER STRUCT ==========================

// 1. 웹에서 로그인 하면, 웹 서버는 로그인 체크후에 ui에 보여야할 정보를 전송해준다.
// 2. 게임시작 버튼을 누르면 웹서버에서 mysql로 해당 유저 정보 가져와서 레디스 클러스터에 올려주기 (해당 레디스 정보들은 짧은 ttl 설정하기) 
// 레디스에 해당 유저 UUID로 pk등록해두고 나머지 값들은 userinfo:{pk}, invenory:{pk} 등으로 저장. 

// 위에 까지는 웹에서 처리 됬다고 가정. 게임시작을 누르면 웹서버에서 uuid를 생성해서 레디스 클러스터에 UUID:"userId", "만든uuid", pk값으로 저장해두고
// 서버에서 해당 데이터를 저장후에 바로 삭제 처리. 이제 해당 uuid로 유저와 통신.
// uuid는 유저가 게임 시작 할 때마다 새로 생성해줄거고, 같은 아이디 이중 접속을 막아서 보안성 유지.

// 3. 유저는 항상 본인의 경험치와 레벨 확인이 가능하고, { 장비, 소비, 재료 }로 이루어진 인벤토리를 확인이 가능하다.
// 4. 인벤토리는 아이템 추가, 아이템 삭제, 아이템 이동이 가능하며 장비 아이템은 확장성으로 다른 아이템들과 차별된 packet 전송
// 5. 유저는 총 2명으로 이루어진 2분짜리 레이드가 가능하다.
// 6. 공격기능과 현재 몹 hp를 확인 가능하며, 2분전에 몹을 잡으면 게임이 종료되며 본인 점수와 팀 점수를 확인 할 수 있다.

// 나중에 추가 구현 : 
// 레이드 매칭 중간 취소 및 유저 연결 끊김 처리
// 유저 상태 처리 (게임중, 접속중 등)


// ========================== REDIS CLUSTER SLOTS STRUCT ==========================
// 
// UUID:quokka, "Made UUID" , quokka pk num
// 
// 1. userInfo / pk,id,level,exp,lastlogin
// 2. inventory:equipment / equipment:pk, code:position, enhance...
// 3. inventory:else / else:pk, code:position, count


// If login success, Insert into Redis pk,id, level, exp,last_login

// Level Up Check
//std::pair<uint8_t, unsigned int> InGameUserManager::LevelUp(UINT16 connObjNum_, unsigned int expUp_) {
//	uint8_t currentLevel = inGmaeUser[connObjNum_]->currentLevel;
//	unsigned int currentExp = inGmaeUser[connObjNum_]->currentExp;
//	uint8_t levelUpCnt = 0;
//
//	if (enhanceProbabilities[currentLevel] <= currentExp + expUp_) { // LEVEL UP
//		currentExp += expUp_;
//		while (currentExp >= enhanceProbabilities[currentLevel]) {
//			currentExp -= enhanceProbabilities[currentLevel];
//			currentLevel++;
//			levelUpCnt++;
//		}
//	}
//	else { // Just Exp Up
//		currentLevel = 0;
//		currentExp = currentExp + expUp_;
//	}
//
//	return { levelUpCnt , currentExp }; // Level Up Increase, currenExp
//}

// 이거 클라이언트로 옮겨서 거기서 체크하기
    //try {
    //    auto existUserInfo = redis.exists("user:" + uuidCheck->uuId); // Check If a UserInfo Value Exists
    //    auto existUserInven = redis.exists("user:" + uuidCheck->uuId); // Check If a UserInventory Value Exists

    //    USERINFO_RESPONSE_PACKET urp;
    //    urp.PacketId = (UINT16)PACKET_ID::USERINFO_RESPONSE;
    //    urp.PacketLength = sizeof(USERINFO_RESPONSE_PACKET);

    //    if (existUserInfo > 0 && existUserInven>0) {
    //        /*urp.userInfo = ;
    //        urp.inventory = ;*/
    //        TempConnUser->PushSendMsg(sizeof(USERINFO_RESPONSE_PACKET), (char*)&urp);
    //    }

    //    else TempConnUser->PushSendMsg(sizeof(USERINFO_RESPONSE_PACKET), (char*)&urp); // Send Nullptr If User Does Not Exist In Redis (Connect Fail)
    //}
    //catch (const sw::redis::Error& e) {
    //    std::cerr << "Redis Error: " << e.what() << std::endl;
    //}

//#include <boost/uuid/uuid.hpp>
//#include <boost/uuid/uuid_generators.hpp>
//#include <boost/uuid/uuid_io.hpp>
//#include <iostream>
//#include <sstream>
//
//std::string GenerateCustomUUID(int pk, const std::string& id, int level) {
//    // 랜덤 UUID 생성
//    boost::uuids::random_generator generator;
//    boost::uuids::uuid uuid = generator();
//
//    // UUID와 유저 데이터를 결합
//    std::ostringstream oss;
//    oss << to_string(uuid) << "|pk:" << pk << "|id:" << id << "|level:" << level;
//
//    return oss.str();
//}
//
//int main() {
//    // 유저 데이터
//    int pk = 12345;
//    std::string id = "player123";
//    int level = 10;
//
//    // 커스텀 UUID 생성 및 출력
//    std::string customUUID = GenerateCustomUUID(pk, id, level);
//    std::cout << "Generated Custom UUID: " << customUUID << std::endl;
//
//    return 0;
//}


//<서버쪽 코드>
//void ParseCustomUUID(const std::string& customUUID, int& pk, std::string& id, int& level) {
//    size_t pkPos = customUUID.find("|pk:");
//    size_t idPos = customUUID.find("|id:");
//    size_t levelPos = customUUID.find("|level:");
//
//    if (pkPos == std::string::npos || idPos == std::string::npos || levelPos == std::string::npos) {
//        throw std::runtime_error("Invalid custom UUID format.");
//    }
//
//    // 데이터 추출
//    pk = std::stoi(customUUID.substr(pkPos + 4, idPos - (pkPos + 4)));
//    id = customUUID.substr(idPos + 4, levelPos - (idPos + 4));
//    level = std::stoi(customUUID.substr(levelPos + 7));
//}