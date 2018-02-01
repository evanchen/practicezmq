#include<assert.h>
#include<string.h>
#include<stdlib.h>
#include "zmq_engine.h"
#include "zmq.h"
#include "constdef.h"
#pragma warning(disable:4996)

ZmqEngine* ZmqEngine::m_instance = NULL;
const int vfd_msgId_str_len = 30;

ZmqEngine::ZmqEngine():
	m_ctx(nullptr),
	m_skt(nullptr),
	m_syncNum(0),
	m_recvPtoNum(0){

}

ZmqEngine::~ZmqEngine() {
	shutdown();
}

ZmqEngine* ZmqEngine::getInstance() {
	if (!m_instance) {
		m_instance = new ZmqEngine();
	}
	return m_instance;
}

void ZmqEngine::shutdown() {
	int ret = 0;
	if (m_skt) {
		ret = zmq_close(m_skt);
		printf("[ZmqEngine::shutdown]: close m_skt, ret: %d",ret);
		m_skt = nullptr;
	}

	if (m_ctx) {
		ret = zmq_term(m_ctx);
		printf("[ZmqEngine::shutdown]: close m_ctx, ret: %d", ret);
		m_ctx = nullptr;
	}
}

void ZmqEngine::init(int type) {
	registerRecvFunc();

	m_enginType = type;
	int major, minor, patch;
	zmq_version(&major, &minor, &patch);
	//printf("[init] zmq version: %d.%d.%d\n", major, minor, patch);

	m_ctx = zmq_ctx_new();
	assert(m_ctx);

	m_addr = "tcp://*:6666";

	if (m_addr.length() <= 0) {
		assert(false);
		return;
	}

	int engineType = getEngineType();
	if (engineType == NET_ENGINE) {
		m_skt = zmq_socket(m_ctx, ZMQ_XREP);
		assert(m_skt);
		int ret = zmq_bind(m_skt, m_addr.c_str());
		assert(ret == 0);
		int rhm = 0;
		int shm = 0;
		size_t s = sizeof(shm);
		zmq_getsockopt(m_skt, ZMQ_RCVHWM, &rhm, &s);
		zmq_getsockopt(m_skt, ZMQ_SNDHWM, &shm, &s);
	}
	else if (engineType == GAME_ENGINE || engineType == BATTLE_ENGINE) {
		m_skt = zmq_socket(m_ctx, ZMQ_XREQ);
		assert(m_skt);
		//send identity
		char identity[vfd_msgId_str_len] = {'\0'};
		snprintf(identity,sizeof(identity), "%d", 1);
		zmq_setsockopt(m_skt, ZMQ_IDENTITY, identity, strlen(identity));

		std::string::size_type star = m_addr.find('*');
		if (star != std::string::npos) {
			m_addr.erase(star, 1);
		}
		
		int ret = zmq_connect(m_skt, m_addr.c_str());
		assert(ret == 0);

		send_gs_identity(1, identity);
	}
}

int ZmqEngine::sync(vfd_t vfd,int msgId,char* data, size_t len) {
	int type = getEngineType();
	if (!m_skt) {
		return -1;
	}
	
	//:TODO: std::string 优化
	std::string identity = "0";
	if (type == NET_ENGINE) { //router发送给指定的dealer
		if (!(msgId >= MIN_MASTER_PTO && msgId <= MAX_MASTER_PTO)) { // to game 1,2,3,...
			auto it = m_vfd2identity.find(vfd);
			if (it != m_vfd2identity.end()) {
				identity = it->second;
			}
		}
		else { // to game 0, battle
			identity = "1";
		}

		//game进程还未注册
		if (m_identity.find(identity) == m_identity.end()) {
			return -1;
		}
		zmq_send(m_skt, identity.c_str(), identity.length(), ZMQ_SNDMORE);
	}
	int ret = 0;
	if (len > 0) {
		ret = doSync(vfd, msgId, data, len);
	}
	else {
		ret = doSyncNobody(vfd, msgId);
	}
	if (ret < 0) {
		return -1;
	}
	return 0;
}

//同步消息到进程(注意,指定进程的要先把 identity 通过 zmq_send 发送出去
int ZmqEngine::doSync(vfd_t vfd, int msgId, const char* body, int len) {
	m_syncNum++;
	char syncNum[vfd_msgId_str_len] = { '\0' };
	snprintf(syncNum, sizeof(syncNum), "%d", m_syncNum);
	char fd[vfd_msgId_str_len] = { '\0' };
	snprintf(fd, sizeof(fd), "%d", vfd);
	char msgIdstr[vfd_msgId_str_len] = { '\0' };
	snprintf(msgIdstr, sizeof(msgIdstr), "%d", msgId);

	return doRealSync(syncNum, fd, msgIdstr, body, len);
}

int ZmqEngine::doRealSync(const char* syncNum, const char* fd, const char* msgIdstr, const char* body, int len) {
	int ret = 0;
	ret = zmq_send(m_skt, syncNum, strlen(syncNum), ZMQ_SNDMORE);
	if (ret <= 0) {
		return -1;
	}
	ret = zmq_send(m_skt, fd, strlen(fd), ZMQ_SNDMORE);
	if (ret <= 0) {
		return -1;
	}
	ret = zmq_send(m_skt, msgIdstr, strlen(msgIdstr), ZMQ_SNDMORE);
	if (ret <= 0) {
		return -1;
	}
	ret = zmq_send(m_skt, body, len, 0);
	if (ret <= 0) {
		return -1;
	}
	return 0;
}

//为了接收时格式相同,body 为空时,默认是"0",长度为1
int ZmqEngine::doSyncNobody(vfd_t vfd, int msgId) {
	const char* body = "0";
	int len = 1;
	return doSync(vfd, msgId, body, len);
}

void ZmqEngine::update() {
	int count = 100;
	int ret = 0;
	char buf[MAX_TCP_MSG_BODY_LEN] = {'\0'};
	size_t size = 0;
	vfd_t vfd;
	static vfd_vt vfds;
	int msgId;

	int type = getEngineType();
	if (type == NET_ENGINE) {
		while (count > 0) {
			count--;
			ret = cast2value_net(vfd, vfds, msgId, buf, MAX_TCP_MSG_BODY_LEN, size);
			if (ret != 0) return;
			if (msgId > StartNetPtoId && msgId <= EndNetPtoId) {
				handleEnginePto(vfd, msgId, buf, size);
			}
			else {
			}
		}
	}
	else {
		while (count > 0) {
			count--;
			ret = cast2value_game(vfd, vfds, msgId, buf, MAX_TCP_MSG_BODY_LEN, size);
			if (ret != 0) return;
			if (msgId > StartNetPtoId && msgId <= EndNetPtoId) {
				handleEnginePto(vfd, msgId, buf, size);
			}
			else {
				
			}
		}
	}
}

int ZmqEngine::recv(zmq_msg_t& msg, char* buf,size_t maxSize, size_t& outSize) {
	int ret = zmq_msg_init(&msg);
	if (-1 == ret) {
		return -1;
	}

	ret = zmq_msg_recv(&msg, m_skt, ZMQ_NOBLOCK);
	if (-1 == ret) {
		return -1;
	}
	char* data = (char*)zmq_msg_data(&msg);
	outSize = zmq_msg_size(&msg);
	if (outSize >= maxSize) {
		return -1;
	}
	memcpy(buf, data, outSize);
	buf[outSize] = '\0'; //enclose
	return ret;
}

int ZmqEngine::cast2value_net(vfd_t& vfd, vfd_vt& vfds, int& msgId, char* buf, int maxBufLen, size_t& outBufLen) {
	char identity[vfd_msgId_str_len] = { '\0' };

	zmq_msg_t msg;
	size_t outSize = 0;
	int ret = recv(msg, identity, vfd_msgId_str_len, outSize);
	if (-1 == ret) return -1;
	ret = zmq_msg_more(&msg);
	zmq_msg_close(&msg);
	if (ret == 0) return -1;

	return cast2value_game(vfd, vfds, msgId, buf, maxBufLen, outBufLen);
}

int ZmqEngine::cast2value_game(vfd_t& vfd, vfd_vt& vfds, int& msgId, char* buf, int maxBufLen, size_t& outBufLen) {
	int syncNumId = 0;
	vfd = 0;
	vfds.clear();
	msgId = 0;
	outBufLen = 0;

	char syncNum[vfd_msgId_str_len] = { '\0' };
	char fd[MAX_VFDS_LEN] = { '\0' }; // :TODO:改成 static ?
	char msgIdstr[vfd_msgId_str_len] = { '\0' };
	const char* delim = ",";
	char* result = nullptr;

	zmq_msg_t msg;
	int idx = 0;
	int ret = 0;

	size_t outSize = 0;
	int maxSize = 0;
	char* tmpBuf = nullptr;
	while (1) {
		idx++;
		maxSize = vfd_msgId_str_len;
		if (idx == 1) { // syncNum
			tmpBuf = syncNum;
		}
		else if (idx == 2) { // vfd
			tmpBuf = fd;
			maxSize = MAX_VFDS_LEN;
		}
		else if (idx == 3) { // msgId
			tmpBuf = msgIdstr;
		}
		else if (idx == 4) { // body
			tmpBuf = buf;
			maxSize = maxBufLen;
		}
		else {
			return -1;
		}
		ret = recv(msg, tmpBuf, maxSize, outSize);
		if (-1 == ret) return -1;
		if (idx == 1) { // syncNum
			syncNumId = atoi(syncNum);
		}
		else if (idx == 2) { // vfd, 可能有多个, 用逗号隔开
			//注意,strtok 也是非线程安全函数,线程安全用 strtok_r
			//只有game进程广播时,才会出现多个 vfd; net->game的转发 或者 非广播协议,vfd只有一个; net的update, 除进程特殊消息外,只用 vfds 容器
			//game的update不会使用 vfds 容器
			result = strtok(fd, delim);
			while (result != nullptr) {
				vfd = atoi(result);
				vfds.push_back(vfd);
				result = strtok(NULL, delim);
			}
		}
		else if (idx == 3) { // msgId
			msgId = atoi(msgIdstr);
		}
		else if (idx == 4) { // body
			outBufLen = outSize;
		}

		ret = zmq_msg_more(&msg);
		zmq_msg_close(&msg);
		if (!ret) break;
	}

	return 0;
}

//除了vfd和msgId之后的数据按照各自协议格式读出
void ZmqEngine::handleEnginePto(vfd_t vfd, int msgId, char* buf, size_t size) {
	NetPtoId id = (NetPtoId)msgId;
	auto it = m_recvFunc.find(id);
	if (it == m_recvFunc.end()) {
		return;
	}
	auto func = it->second;
	(this->*func)(vfd, buf, size);
}

void ZmqEngine::send_gs_identity(vfd_t vfd, const char* identity) {
	doSync(vfd, int(GsIdentity), identity, strlen(identity));
}

void ZmqEngine::recv_gs_identity(vfd_t vfd, char* buf, size_t size) {
	(void)vfd;
	char identityId[vfd_msgId_str_len] = { '\0' };
	if (size >= vfd_msgId_str_len) {
		return;
	}
	memcpy(identityId, buf, size);
	std::string identity = std::string(identityId);
	auto it_identity = m_identity.find(identity);
	if (it_identity == m_identity.end()) {
		m_identity[identity] = true;
	}
}

void ZmqEngine::registerRecvFunc() {
	m_recvFunc[GsIdentity] = &ZmqEngine::recv_gs_identity;
}