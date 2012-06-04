#ifndef GRAPHDAT_H
#define GRAPHDAT_H

#include <stdbool.h>
#include <stdint.h>

#include <ngx_core.h>

#define debug(...) {}; /*{ \
FILE * fp = fopen("/usr/local/nginx/logs/debug.log", "a"); \
fprintf(fp, __VA_ARGS__); \
fclose(fp); \
}*/

typedef struct {
	char* method;
	int methodlen;
	char* uri;
	int urilen;
	uintptr_t msec;
	ngx_log_t *log;
} request_t;

void graphdat_init(ngx_str_t,ngx_log_t*);
void graphdat_term();
//void graphdat_send(char*,int,char*,int,uintptr_t,ngx_log_t*);
void graphdat_store(char*,int,char*,int,uintptr_t,ngx_log_t*);

#endif /* GRAPHDAT_H */

