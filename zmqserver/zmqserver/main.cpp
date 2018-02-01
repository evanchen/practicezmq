// Hello World server

#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zhelpers.h"
#include "zmq_engine.h"


int main(void)
{
	auto zEngine = ZmqEngine::getInstance();
	zEngine->init(NET_ENGINE);

	while (1) {
		zEngine->update();
		Sleep(1000);
	}
	return 0;
}