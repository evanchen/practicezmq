// Hello World server

#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zhelpers.h"

int main(void)
{
	int major, minor, patch;
	zmq_version(&major, &minor, &patch);
	printf("当前ZMQ版本号为 %d.%d.%d\n", major, minor, patch);
	// Socket to talk to clients
	void *context = zmq_ctx_new();
	void *responder = zmq_socket(context, ZMQ_REP);
	int rc = zmq_bind(responder, "tcp://*:6666");
	assert(rc == 0);

	while (1) {
		char buffer[10];
		zmq_recv(responder, buffer, 10, 0);
		printf("Received Hello\n");
		zmq_send(responder, "World", 5, 0);
		Sleep(1000); // Do some 'work'
	}
	return 0;
}