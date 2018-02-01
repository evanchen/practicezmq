#pragma once
#include <vector>

typedef long long int64;
typedef int vfd_t;
typedef std::vector<vfd_t> vfd_vt;

//tcpЭ����Ϣ������л�����
static const int MAX_TCP_MSG_BODY_LEN = 65536;
//tcpЭ��ͷ������
static const int MAX_TCP_HEADER_LEN = 4;
//tcp��Ϣ����󳤶�
static const int MAX_TCP_MSG_LEN = MAX_TCP_HEADER_LEN + MAX_TCP_MSG_BODY_LEN;
//ÿ�δ�socket�������MAX_READ_LEN�ֽ�����
static const int MAX_READ_LEN = 1024;

//���̼���Ϣ������л�����
static const int MAX_SERIALIZE_LEN = 0x600000; //6*1024*1024 bits

//�����Ϣid����
static const int MAX_MSG_ID = 65536;
//�㲥������vfdʱ,vfd�ַ�����󳤶�(game������net����ͨ��)
//ע��,������Ȼ����ƹ㲥����
static const int MAX_VFDS_LEN = 65536;

// 1. net �� game ��ͨ��Ҳ��ͨ�� syncMsg ����ת��,�����ڽű���ִ�д���, �����Զ�������Э��id��100����
// 2. ����Э��id��101--1000,������0����ר������id
static const int MAX_ENGINE_PTO = 100;
static const int MIN_MASTER_PTO = 101;
static const int MAX_MASTER_PTO = 2000;

// ����1--100Э��id
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

//���������
const int MAX_CONN_NUM = 10000;
//���������
const int MAX_HEARTBEAT_TIME = 150;
//��ʼ�������������������ֵ
const int MAX_HEARTBEAT_CHECK_NUM = 1000;