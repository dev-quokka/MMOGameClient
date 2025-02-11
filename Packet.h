#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string>
#include <ws2tcpip.h>
#include <vector>
#include <chrono>

const UINT16 PACKET_ID_SIZE = 53; // Last Packet_ID Num + 1

struct DataPacket {
	UINT32 dataSize;
	SOCKET userSkt;
	DataPacket(UINT32 dataSize_, SOCKET userSkt_) : dataSize(dataSize_), userSkt(userSkt_) {}
};

struct PacketInfo
{
	UINT16 packetId = 0;
	UINT16 dataSize = 0;
	SOCKET userSkt = 0;
	char* pData = nullptr; // 유저로 부터 받은 데이터
};

struct PACKET_HEADER
{
	UINT16 PacketLength;
	UINT16 PacketId;
	std::string uuId; // UUID For User Check
};


//  ---------------------------- SYSTEM  ----------------------------

struct USER_CONNECT_REQUEST_PACKET : PACKET_HEADER {
	std::string userId;
};

struct USER_CONNECT_RESPONSE_PACKET : PACKET_HEADER {
	bool isSuccess;
	uint8_t level;
	unsigned int currentExp;
	std::string uuId;
};

struct IM_WEB_REQUEST : PACKET_HEADER {

};

struct IM_WEB_RESPONSE : PACKET_HEADER {
	bool isSuccess;
};

struct SYNCRONIZE_LEVEL_REQUEST : PACKET_HEADER {
	uint8_t level;
	UINT16 userPk;
	unsigned int currentExp;
};

struct SYNCRONIZE_LOGOUT_REQUEST : PACKET_HEADER {
	UINT16 userPk;
};

struct SYNCRONIZE_DISCONNECT_REQUEST : PACKET_HEADER {
	UINT16 userPk;
};

//  ---------------------------- USER STATUS  ----------------------------

struct EXP_UP_REQUEST : PACKET_HEADER {
	short mobNum; // Number of Mob
};

struct EXP_UP_RESPONSE : PACKET_HEADER {
	unsigned int expUp;
};

struct LEVEL_UP_RESPONSE : PACKET_HEADER {
	uint8_t increaseLevel;
	unsigned int currentExp;
};

//  ---------------------------- INVENTORY  ----------------------------

struct Equipment {

};

struct Consumable {

};

struct Material {

};


struct ADD_ITEM_REQUEST : PACKET_HEADER {
	uint8_t itemType; // (Max 3)
	uint8_t itemSlotPos; // (Max 50)
	uint8_t itemCount; // (Max 99)
	short itemCode; // (Max 5000)
};

struct ADD_ITEM_RESPONSE : PACKET_HEADER {
	bool isSuccess;
};

struct DEL_ITEM_REQUEST : PACKET_HEADER {
	uint8_t itemType; // (Max 3)
	uint8_t itemSlotPos; // (Max 50)
	short itemCode; // (Max 5000)
};

struct DEL_ITEM_RESPONSE : PACKET_HEADER {
	bool isSuccess;
};

struct MOD_ITEM_REQUEST : PACKET_HEADER {
	uint8_t itemType; // (Max 3)
	uint8_t itemSlotPos; // (Max 50)
	int8_t itemCount; // (Max 99)
	short itemCode; // (Max 5000)
};

struct MOD_ITEM_RESPONSE : PACKET_HEADER {
	bool isSuccess;
};

struct MOV_ITEM_REQUEST : PACKET_HEADER {
	uint8_t dragItemType; // (Max 3)
	uint8_t dragItemSlotPos; // (Max 50)
	uint8_t dragItemCount; // (Max 99)
	uint8_t targetItemType; // (Max 3)
	uint8_t targetItemSlotPos; // (Max 50)
	uint8_t targetItemCount; // (Max 99)
	short dragItemCode; // (Max 5000)
	short targetItemCode; // (Max 5000)
};

struct MOV_ITEM_RESPONSE : PACKET_HEADER {
	bool isSuccess;
};

//  ---------------------------- INVENTORY:EQUIPMENT  ----------------------------

struct ADD_EQUIPMENT_REQUEST : PACKET_HEADER {
	uint8_t itemType; // (Max 3)
	uint8_t itemSlotPos; // (Max 50)
	uint8_t currentEnhanceCount; // (Max 20)
	short itemCode; // (Max 5000)
};

struct ADD_EQUIPMENT_RESPONSE : PACKET_HEADER {
	bool isSuccess;
};

struct DEL_EQUIPMENT_REQUEST : PACKET_HEADER {
	uint8_t itemType; // (Max 3)
	uint8_t itemSlotPos; // (Max 50)
	short itemCode; // (Max 5000)
};

struct DEL_EQUIPMENT_RESPONSE : PACKET_HEADER {
	bool isSuccess;
};

struct ENH_EQUIPMENT_REQUEST : PACKET_HEADER {
	uint8_t itemType; // (Max 3)
	uint8_t itemSlotPos; // (Max 50)
	uint8_t currentEnhanceCount; // (Max 10)
	short itemCode; // (Max 5000)
};

struct ENH_EQUIPMENT_RESPONSE : PACKET_HEADER {
	bool isSuccess;
};


//  ---------------------------- RAID  ----------------------------

struct RAID_MATCHING_REQUEST : PACKET_HEADER { // Users Matching Request

};

struct RAID_MATCHING_RESPONSE : PACKET_HEADER {
	bool insertSuccess; // Insert Into Matching Queue Check
};



struct RAID_READY_REQUEST : PACKET_HEADER {
	uint8_t timer; // Minutes
	uint8_t roomNum; // If Max RoomNum Up to Short Range, Back to Number One
	uint8_t yourNum;
	UINT16 udpPort;   // Server UDP Port Num
	unsigned int mobHp;
	char serverIP[16]; // Server IP Address
};

struct RAID_TEAMINFO_REQUEST : PACKET_HEADER { // User To Server
	bool imReady;
	uint8_t roomNum;
	uint8_t myNum;
	sockaddr_in userAddr;// 유저가 만든 udp 소켓의 sockaddr_in 전달
};

struct RAID_TEAMINFO_RESPONSE : PACKET_HEADER {
	uint8_t teamLevel;
	std::string teamId;
};

struct RAID_START_REQUEST : PACKET_HEADER {
	std::chrono::time_point<std::chrono::steady_clock> endTime;
};

struct RAID_HIT_REQUEST : PACKET_HEADER {
	uint8_t roomNum;
	uint8_t myNum;
	unsigned int damage;
};

struct RAID_HIT_RESPONSE : PACKET_HEADER {
	unsigned int yourScore;
	unsigned int currentMobHp;
};

struct RAID_END_REQUEST : PACKET_HEADER { // Server to USER
	unsigned int userScore;
	unsigned int teamScore;
	std::chrono::time_point<std::chrono::steady_clock> elapsedTime; // Time to Finish Mob
};

struct RAID_END_RESPONSE : PACKET_HEADER { // User to Server (If Server Get This Packet, Return Room Number)

};

struct RAID_RANKING_REQUEST : PACKET_HEADER {
	unsigned int startRank;
};

struct RAID_RANKING_RESPONSE : PACKET_HEADER {
	std::vector<std::pair<std::string, unsigned int>> reqScore;
};

enum class PACKET_ID : UINT16 {
	//SYSTEM
	USER_CONNECT_REQUEST = 1,
	USER_CONNECT_RESPONSE = 2,
	USER_LOGOUT_REQUEST = 3,
	IM_WEB_REQUEST = 4,
	IM_WEB_RESPONSE = 5,
	SYNCRONIZE_LEVEL_REQUEST = 6,
	SYNCRONIZE_LOGOUT_REQUEST = 7,
	SYNCRONIZE_DISCONNECT_REQUEST = 8,

	// USER STATUS (11~)
	EXP_UP_REQUEST = 11,
	EXP_UP_RESPONSE = 12,
	LEVEL_UP_REQUEST = 13,
	LEVEL_UP_RESPONSE = 14,

	// INVENTORY (25~)
	ADD_ITEM_REQUEST = 25,
	ADD_ITEM_RESPONSE = 26,
	DEL_ITEM_REQUEST = 27,
	DEL_ITEM_RESPONSE = 28,
	MOD_ITEM_REQUEST = 29,
	MOD_ITEM_RESPONSE = 30,
	MOV_ITEM_REQUEST = 31,
	MOV_ITEM_RESPONSE = 32,

	// INVENTORY::EQUIPMENT 
	ADD_EQUIPMENT_REQUEST = 33,
	ADD_EQUIPMENT_RESPONSE = 34,
	DEL_EQUIPMENT_REQUEST = 35,
	DEL_EQUIPMENT_RESPONSE = 36,
	ENH_EQUIPMENT_REQUEST = 37,
	ENH_EQUIPMENT_RESPONSE = 38,

	// RAID (45~)
	RAID_MATCHING_REQUEST = 45,
	RAID_MATCHING_RESPONSE = 46,
	RAID_READY_REQUEST = 47,
	RAID_TEAMINFO_REQUEST = 48,
	RAID_TEAMINFO_RESPONSE = 49,
	RAID_START_REQUEST = 50,
	RAID_HEAT_REQUEST = 51,
	RAID_HEAT_RESPONSE = 52,
	RAID_END_REQUEST = 53,
	RAID_END_RESPONSE = 54,
	RAID_RANKING_REQUEST = 55,
	RAID_RANKING_RESPONSE = 56,
};