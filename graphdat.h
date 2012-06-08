#ifndef GRAPHDAT_H
#define GRAPHDAT_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*logger_delegate_t)(void * user, const char * fmt, ...);

typedef struct {
	char* method;
	int methodlen;
	char* uri;
	int urilen;
	double msec;
	logger_delegate_t logger;
	void * log_context;
} request_t;

void graphdat_init(char *, int, logger_delegate_t, void *);
void graphdat_term();
void graphdat_store(char *, int, char *, int, double, logger_delegate_t, void *, int);

#endif /* GRAPHDAT_H */

