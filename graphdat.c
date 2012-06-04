#include "graphdat.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <msgpack.h>

int sockfd = -1;

void socket_send(char * data, int len, FILE * fp) {
	if(sockfd < 0)
	{
		sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			fprintf(fp, "socketInit: opening socket (%s)\n", strerror(errno));
			return;
		}
		fcntl(sockfd, F_SETFL, O_NONBLOCK);

		struct sockaddr_un serv_addr;
		int servlen;
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sun_family = AF_UNIX;
		strcpy(serv_addr.sun_path, "/tmp/gd.agent.sock");
		servlen = sizeof(serv_addr.sun_family) + strlen(serv_addr.sun_path) + 1;
		if(connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
		{
			fprintf(fp, "socketInit: binding socket (%s)\n", strerror(errno));
			close(sockfd);
			return;
		}
	}

	unsigned char bytes[4];
	bytes[0] = len >> 24;
	bytes[1] = len >> 16;
	bytes[2] = len >> 8;
	bytes[3] = len;

	int wrote = write(sockfd, bytes, 4);
	if(wrote < 0)
	{
		fprintf(fp, "write error\n");
	}
	else
	{
		fprintf(fp, "wrote: %d\n", wrote);
	}
	wrote = write(sockfd, data, len);
	if(wrote < 0)
	{
		fprintf(fp, "write error\n");
	}
	else
	{
		fprintf(fp, "wrote: %d\n", wrote);
	}

	//close(sockfd);
}

void graphdat_send(char* method, int methodlen, char* uri, int urilen, uintptr_t msec) {
	FILE * fp;
	fp = fopen("/tmp/nginx_graphdat.log", "a");

	fprintf(fp, "%.*s|%.*s|%d\n", methodlen, method, urilen, uri, (unsigned int)msec);

	msgpack_sbuffer* buffer = msgpack_sbuffer_new();
        msgpack_packer* pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);

	msgpack_pack_map(pk, 4); // timestamp, type, route, responsetime
	// timestamp
	msgpack_pack_raw(pk, 9);
        msgpack_pack_raw_body(pk, "timestamp", 9);
	msgpack_pack_int(pk, 1);
	// type
	msgpack_pack_raw(pk, 4);
        msgpack_pack_raw_body(pk, "type", 4);
	msgpack_pack_raw(pk, 6);
        msgpack_pack_raw_body(pk, "Sample", 6);
	// route
	msgpack_pack_raw(pk, 5);
        msgpack_pack_raw_body(pk, "route", 5);
	msgpack_pack_raw(pk, urilen);
        msgpack_pack_raw_body(pk, uri, urilen);
	// responsetime
	msgpack_pack_raw(pk, 12);
        msgpack_pack_raw_body(pk, "responsetime", 12);
	msgpack_pack_int(pk, msec);
        
	size_t i;
	for(i=0;i<buffer->size;i++) {
		fprintf(fp, "%d", buffer->data[i]);
	}
	fprintf(fp, "\n");

	socket_send(buffer->data, buffer->size, fp);

	msgpack_sbuffer_free(buffer);
        msgpack_packer_free(pk);

	fclose(fp);
}

