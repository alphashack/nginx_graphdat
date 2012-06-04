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

static bool s_init = false;
static char * s_sockfile = NULL;
static int s_sockfd = -1;
static bool s_lastwaserror = false;

void socket_close() {
debug("socket_close\n");
	close(s_sockfd);
	s_sockfd = -1;
}

void socket_term() {
debug("socket_term\n");
        if(s_init) {
	    socket_close();
	    free(s_sockfile);
        }
}

bool socket_connect(ngx_log_t *log) {
	if(s_sockfd < 0)
	{
debug("socket_connect\n");
		s_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (s_sockfd < 0)
		{
			if(!s_lastwaserror) {
				ngx_log_error(NGX_LOG_ERR, log, 0, "nginx_graphdat error: could not create socket (%s)", strerror(s_sockfd));
				s_lastwaserror = true;
			}
			return false;
		}
		fcntl(s_sockfd, F_SETFL, O_NONBLOCK);

		struct sockaddr_un serv_addr;
		int servlen;
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sun_family = AF_UNIX;
		strcpy(serv_addr.sun_path, s_sockfile);
		servlen = sizeof(serv_addr.sun_family) + strlen(serv_addr.sun_path) + 1;
		int result = connect(s_sockfd, (struct sockaddr *)&serv_addr, servlen);
		if(result < 0)
		{
			if(!s_lastwaserror) {
				ngx_log_error(NGX_LOG_ERR, log, 0, "nginx_graphdat error: could not connect socket (%s)", strerror(result));
				s_lastwaserror = true;
			}
			socket_close();
			return false;
		}
	}
	return true;
}

bool socket_check(ngx_log_t *log) {
	if(!s_init) {
		if(!s_lastwaserror) {
			ngx_log_error(NGX_LOG_ERR, log, 0, "nginx_graphdat error: not initialised");
			s_lastwaserror = true;
		}
		return false;
	}
	return socket_connect(log);
}

void socket_init(ngx_str_t file, ngx_log_t *log) {
debug("socket_init\n");
	s_sockfile = malloc(file.len + 1);
	memcpy(s_sockfile, file.data, file.len);
	s_sockfile[file.len] = 0;
	s_sockfd = -1;
	s_init = true;
}

void socket_send(char * data, int len, ngx_log_t *log) {
	if(!socket_check(log)) return;

	unsigned char bytes[4];
	bytes[0] = len >> 24;
	bytes[1] = len >> 16;
	bytes[2] = len >> 8;
	bytes[3] = len;

	int wrote = write(s_sockfd, bytes, 4);
	if(wrote < 0)
	{
		ngx_log_error(NGX_LOG_ERR, log, 0, "nginx_graphdat error: could not write socket (%s)", strerror(wrote));
		socket_close();
	}
        else
	{
		wrote = write(s_sockfd, data, len);
		if(wrote < 0)
		{
			ngx_log_error(NGX_LOG_ERR, log, 0, "nginx_graphdat error: could not write socket (%s)", strerror(wrote));
			socket_close();
		}
		else
		{
			s_lastwaserror = false;
		}
        }
}

void graphdat_init(ngx_str_t file, ngx_log_t *log) {
	socket_init(file, log);
}

void graphdat_term() {
	socket_term();
}

void graphdat_send(char* method, int methodlen, char* uri, int urilen, uintptr_t msec, ngx_log_t *log) {
	msgpack_sbuffer* buffer = msgpack_sbuffer_new();
        msgpack_packer* pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);

	msgpack_pack_map(pk, 4); // timestamp, type, route, responsetime, source
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
	// source
	msgpack_pack_raw(pk, 6);
        msgpack_pack_raw_body(pk, "source", 6);
	msgpack_pack_raw(pk, 5);
        msgpack_pack_raw_body(pk, "nginx", 5);
        
	socket_send(buffer->data, buffer->size, log);

	msgpack_sbuffer_free(buffer);
        msgpack_packer_free(pk);
}

