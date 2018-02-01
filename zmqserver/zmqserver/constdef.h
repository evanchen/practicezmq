#pragma once
#include <vector>

typedef long long int64;
typedef int vfd_t;
typedef std::vector<vfd_t> vfd_vt;

//tcp协议消息最大序列化长度
static const int MAX_TCP_MSG_BODY_LEN = 65536;
//tcp协议头部长度
static const int MAX_TCP_HEADER_LEN = 4;
//tcp消息体最大长度
static const int MAX_TCP_MSG_LEN = MAX_TCP_HEADER_LEN + MAX_TCP_MSG_BODY_LEN;
//每次从socket读出最多MAX_READ_LEN字节数据
static const int MAX_READ_LEN = 1024;

//进程间消息最大序列化长度
static const int MAX_SERIALIZE_LEN = 0x600000; //6*1024*1024 bits

//最大消息id数量
static const int MAX_MSG_ID = 65536;
//广播给所有vfd时,vfd字符串最大长度(game进程与net进程通信)
//注意,这个长度会限制广播人数
static const int MAX_VFDS_LEN = 65536;

// 1. net 和 game 的通信也是通过 syncMsg 进行转发,但不在脚本层执行处理, 保留自定义请求协议id在100以内
// 2. 请求协议id在101--1000,是属性0进程专属处理id
static const int MAX_ENGINE_PTO = 100;
static const int MIN_MASTER_PTO = 101;
static const int MAX_MASTER_PTO = 2000;

// 定义1--100协议id
enum NetPtoId {
	StartNetPtoId			= 1,
	GsIdentity				= 2,
	PlayerInProcess			= 3, 
	EngineCloseVfd			= 4,
	GameCloseVfd			= 5,
	EngineCmd				= 6,
	SyncIp					= 7,

	EndNetPtoId				= MAX_ENGINE_PTO,
};

#define NET_ENGINE			1
#define GAME_ENGINE			2
#define LOG_ENGINE			4
#define DB_ENGINE			8
#define TEST_ENGINE			16
#define BATTLE_ENGINE		32

//最大连接数
const int MAX_CONN_NUM = 10000;
//心跳检测间隔
const int MAX_HEARTBEAT_TIME = 150;
//开始检测心跳的连接数量阀值
const int MAX_HEARTBEAT_CHECK_NUM = 1000;