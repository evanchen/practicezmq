#pragma once
#include <map>
#include "constdef.h"
class Msg;
struct zmq_msg_t;

class ZmqEngine {
public:
	typedef void(ZmqEngine::*recvFunc)(vfd_t vfd, char* buf, size_t size);
protected:
	ZmqEngine();
public:
	~ZmqEngine();
	static ZmqEngine* getInstance();
	void shutdown();
	void init(int type);
	int sync(vfd_t vfd, int msgId, char* data, size_t len);
	int doSync(vfd_t vfd, int msgId, const char* body, int len);
	int doSyncNobody(vfd_t vfd, int msgId);
	int doRealSync(const char* syncNum, const char* fd, const char* msgId, const char* body, int lent);
	void update();
	int recv(zmq_msg_t& msg, char* buf, size_t maxSize, size_t& outSize);
	int cast2value_net(vfd_t& vfd, vfd_vt& vfds, int& msgId, char* buf, int maxBufLen, size_t& outBufLen);
	int cast2value_game(vfd_t& vfd, vfd_vt& vfds, int& msgId, char* buf, int maxBufLen, size_t& outBufLen);

	void incPtoNum() { m_recvPtoNum++; }

	int getEngineType() { return m_enginType; }
	void send_gs_identity(vfd_t vfd, const char* identity);
	void recv_gs_identity(vfd_t vfd, char* buf, size_t size);

	void handleEnginePto(vfd_t vfd, int msgId, char* buf, size_t size);
	void registerRecvFunc();
private:
	static ZmqEngine* m_instance;
	void* m_ctx;
	void* m_skt;
	std::string m_addr;
	std::map<std::string, bool> m_identity;
	std::map<vfd_t, std::string> m_vfd2identity;
	std::map<NetPtoId, recvFunc> m_recvFunc;
	int m_syncNum;

	int m_recvPtoNum;
	int m_enginType;
};
