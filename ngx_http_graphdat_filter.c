#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "graphdat.h"

// remove:
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
    ngx_flag_t enable;
    ngx_str_t socket_file;
} ngx_http_graphdat_conf_t;

static bool s_enabled = false;

static ngx_int_t ngx_http_graphdat_filter_init(ngx_conf_t*);
static ngx_int_t ngx_http_graphdat_header_filter(ngx_http_request_t*);
static char * ngx_http_graphdat_init_main_conf(ngx_conf_t*, void*);
static void * ngx_http_graphdat_create_main_conf(ngx_conf_t*);
static ngx_int_t ngx_http_graphdat_init_process(ngx_cycle_t*);
static void ngx_http_graphdat_exit_process(ngx_cycle_t*);

static ngx_http_output_header_filter_pt ngx_http_next_header_filter;

static ngx_command_t ngx_http_graphdat_filter_commands[] = {
    {
        ngx_string("graphdat_filter"),
        NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1|NGX_CONF_FLAG,
        ngx_conf_set_flag_slot,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(ngx_http_graphdat_conf_t, enable),
        NULL
    },
    {
        ngx_string("graphdat_socket_file"),
        NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(ngx_http_graphdat_conf_t, socket_file),
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t ngx_http_graphdat_filter_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_graphdat_filter_init,         /* postconfiguration */

    ngx_http_graphdat_create_main_conf,    /* create main configuration */
    ngx_http_graphdat_init_main_conf,      /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};

ngx_module_t  ngx_http_graphdat_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_graphdat_filter_ctx,         /* module context */
    ngx_http_graphdat_filter_commands,     /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    ngx_http_graphdat_init_process,        /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    ngx_http_graphdat_exit_process,        /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static void *
ngx_http_graphdat_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_graphdat_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_graphdat_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    conf->enable = NGX_CONF_UNSET;

    return conf;
}

static char *
ngx_http_graphdat_init_main_conf(ngx_conf_t *cf, void *conf)
{
   ngx_http_graphdat_conf_t *config = conf;

   ngx_conf_init_value(config->enable, 0);
   if(config->socket_file.data == NULL) {
	 ngx_str_set(&config->socket_file, "/tmp/gd.agent.sock");
   }

   return NGX_CONF_OK;
}

void delegate_logger(void * user, const char * fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	ngx_log_t * log = user;
	
	// All this because nginx does not always define a legger with a va_list param
	char *msg = "";
	int res = vasprintf(&msg, fmt, argp);

	if(res >= 0)
	{
		ngx_log_error_core(NGX_LOG_ERR, log, 0, msg);
		free(msg);
	}

	va_end(argp);
}

static ngx_int_t
ngx_http_graphdat_header_filter(ngx_http_request_t *r)
{
    if (s_enabled)
    {
	struct timeval tv;
	double msec_diff;

	ngx_gettimeofday(&tv);
	msec_diff = (tv.tv_sec - r->start_sec) * 1000 + (tv.tv_usec / 1000) - r->start_msec;

	graphdat_store((char*)r->method_name.data, r->method_name.len, (char*)r->uri.data, r->uri.len, msec_diff, delegate_logger, r->connection->log, sizeof(ngx_log_t));
    }

    return ngx_http_next_header_filter(r);
}

static ngx_int_t
ngx_http_graphdat_filter_init (ngx_conf_t *conf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_graphdat_header_filter;

    return NGX_OK;
}

static ngx_int_t ngx_http_graphdat_init_process(ngx_cycle_t* cycle) {
    ngx_http_graphdat_conf_t *conf;
    conf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_graphdat_filter_module);
    s_enabled = conf->enable;

    if(s_enabled) {
        graphdat_init((char *)conf->socket_file.data, conf->socket_file.len, delegate_logger, cycle->log);
    }
    return NGX_OK;
}

static void ngx_http_graphdat_exit_process(ngx_cycle_t* cycle) {
    if(s_enabled) {
        graphdat_term();
    }
}

