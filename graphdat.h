#ifndef GRAPHDAT_H
#define GRAPHDAT_H

#include <stdbool.h>
#include <stdint.h>

#include <ngx_core.h>

typedef struct {
	char* method;
	int methodlen;
	char* uri;
	int urilen;
	double msec;
	ngx_log_t *log;
} request_t;

void graphdat_init(ngx_str_t,ngx_log_t*);
void graphdat_term();
void graphdat_store(char*,int,char*,int,double,ngx_log_t*);

#endif /* GRAPHDAT_H */

