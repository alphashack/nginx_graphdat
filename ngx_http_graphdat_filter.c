#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "graphdat.h"

typedef struct {
    ngx_flag_t enable;
} ngx_http_graphdat_conf_t;

static ngx_int_t ngx_http_graphdat_filter_init (ngx_conf_t*);
static ngx_int_t ngx_http_graphdat_header_filter(ngx_http_request_t*);
static char * ngx_http_graphdat_merge_loc_conf(ngx_conf_t*, void*, void*);
static void * ngx_http_graphdat_create_loc_conf(ngx_conf_t*);

static ngx_http_output_header_filter_pt ngx_http_next_header_filter;

static ngx_command_t ngx_http_graphdat_filter_commands[] = {
    {
        ngx_string("graphdat_filter"),
        NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_FLAG,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_graphdat_conf_t, enable),
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t ngx_http_graphdat_filter_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_graphdat_filter_init,         /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_graphdat_create_loc_conf,     /* create location configuration */
    ngx_http_graphdat_merge_loc_conf       /* merge location configuration */
};

ngx_module_t  ngx_http_graphdat_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_graphdat_filter_ctx,         /* module context */
    ngx_http_graphdat_filter_commands,     /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static void *
ngx_http_graphdat_create_loc_conf(ngx_conf_t *cf)
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
ngx_http_graphdat_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_graphdat_conf_t *prev = parent;
    ngx_http_graphdat_conf_t *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_graphdat_header_filter(ngx_http_request_t *r)
{
    struct timeval tv;
    ngx_http_graphdat_conf_t *lrsl;
    ngx_uint_t msec_diff;

    lrsl = ngx_http_get_module_loc_conf(r, ngx_http_graphdat_filter_module);

    if (lrsl->enable == 1)
    {
        ngx_gettimeofday(&tv);
        msec_diff = (tv.tv_sec - r->start_sec) * 1000 + (tv.tv_usec / 1000) - r->start_msec + 123;

        graphdat_send((char*)r->method_name.data, r->method_name.len, (char*)r->uri.data, r->uri.len, msec_diff);
    }
    return ngx_http_next_header_filter(r);
}

static ngx_int_t
ngx_http_graphdat_filter_init (ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_graphdat_header_filter;

    return NGX_OK;
}

