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
	zEngine->init(GAME_ENGINE);

	char data[] = "just a test";
	while (1) {
		zEngine->update();
		Sleep(100);
		zEngine->sync(11, 1001, data, sizeof(data));
	}
	return 0;
}