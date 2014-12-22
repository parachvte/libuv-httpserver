#include <stdio.h>
#include <stdlib.h>

#include "libuv/include/uv.h"
#include "libuv/test/task.h"
#include "http-parser/http_parser.h"

const static char* HOST     = "0.0.0.0"; /* localhost */
const static int   PORT     = 8888;
const static int   BACKLOG  = 128;

#define RESPONSE                  \
  "HTTP/1.1 200 OK\r\n"           \
  "Content-Type: text/plain\r\n"  \
  "Content-Length: 14\r\n"        \
  "\r\n"                          \
  "Hello, World!\r\n"

/* types */
typedef struct {
  uv_tcp_t handle;
  http_parser parser;
} client_t;

/* variables */
static uv_tcp_t server;
static http_parser_settings parser_settings;

/* forward declaration */
static void close_cb(uv_handle_t *);
static void shutdown_cb(uv_shutdown_t *, int);
static void alloc_cb(uv_handle_t*, size_t, uv_buf_t*);
static void connection_cb(uv_stream_t *, int);
static void read_cb(uv_stream_t*, ssize_t, const uv_buf_t*);
static void write_cb(uv_write_t*, int);

static int headers_complete_cb(http_parser*);

/* tcp callbacks */
void close_cb(uv_handle_t *handle) {
  client_t *client = (client_t *) handle->data;
  free(client);
}

void shutdown_cb(uv_shutdown_t *shutdown_req, int status) {
  uv_close((uv_handle_t *) shutdown_req->handle, close_cb);
  free(shutdown_req);
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = malloc(suggested_size);
  buf->len = suggested_size;
  ASSERT(buf->base != NULL);
}

void connection_cb(uv_stream_t *server, int status) {
  ASSERT(status == 0);

  client_t *client = malloc(sizeof(client_t));
  int r = uv_tcp_init(server->loop, &client->handle);
  ASSERT(r == 0);
  client->handle.data = client;

  r = uv_accept(server, (uv_stream_t *) &client->handle);
  if (r) {
    uv_shutdown_t *shutdown_req = malloc(sizeof(uv_shutdown_t));
    uv_shutdown(shutdown_req, (uv_stream_t *) &client->handle, shutdown_cb);
    ASSERT(r == 0);
  }

  http_parser_init(&client->parser, HTTP_REQUEST);
  client->parser.data = client;
  r = uv_read_start((uv_stream_t *) &client->handle, alloc_cb, read_cb);
}

void read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
  int r = 0;
  client_t *client = (client_t *) handle->data;

  if (nread >= 0) {
    size_t parsed = http_parser_execute(&client->parser, &parser_settings, buf->base, nread);

    if (parsed < nread) {
      LOG("parse error\n");
      uv_close((uv_handle_t *) handle, close_cb);
    }

  } else {
    if (nread == UV_EOF) {
      // do nothing
    } else {
      LOGF("read: %s\n", uv_strerror(nread));
    }

    uv_shutdown_t *shutdown_req = malloc(sizeof(uv_shutdown_t));
    r = uv_shutdown(shutdown_req, handle, shutdown_cb);
    ASSERT(r == 0);
  }
  free(buf->base);
}

void write_cb(uv_write_t* write_req, int status) {
  uv_close((uv_handle_t *) write_req->handle, close_cb);
  free(write_req);
}

/* http callback */
int headers_complete_cb(http_parser* parser) {
  client_t *client = (client_t *) parser->data;

  uv_write_t *write_req = malloc(sizeof(uv_write_t));
  uv_buf_t buf = uv_buf_init(RESPONSE, sizeof(RESPONSE));
  uv_write(write_req, (uv_stream_t *) &client->handle, &buf, 1, write_cb);

  return 1;
}

/* server */
void server_init(uv_loop_t *loop) {
  int r = uv_tcp_init(loop, &server);
  ASSERT(r == 0);

  struct sockaddr_in addr;
  r = uv_ip4_addr(HOST, PORT, &addr);
  ASSERT(r == 0);
  
  r = uv_tcp_bind(&server, (struct sockaddr *) &addr, 0);
  ASSERT(r == 0);

  r = uv_listen((uv_stream_t *) &server, BACKLOG, connection_cb);
  ASSERT(r == 0);
}

int main() {
  uv_loop_t *loop = uv_default_loop();

  parser_settings.on_headers_complete = headers_complete_cb;
  server_init(loop);

  int r = uv_run(loop, UV_RUN_DEFAULT);
  ASSERT(r == 0);

  /* If no more events here, cleanup loop */
  MAKE_VALGRIND_HAPPY();
  return 0;
}
