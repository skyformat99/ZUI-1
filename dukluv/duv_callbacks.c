#include "duv_callbacks.h"

void duv_close_cb(uv_handle_t* handle) {
  duv_handle_t* data = handle->data;
  duk_context *ctx = data->ctx;

  duv_emit_event(ctx, data, DUV_CLOSED, 0);
  handle->data = duv_cleanup_handle(ctx, data);
}

void duv_timer_cb(uv_timer_t* handle) {
  duv_emit_event(((duv_handle_t*)handle->data)->ctx, handle->data, DUV_TIMEOUT, 0);
}

void duv_connect_cb(uv_connect_t* req, int status) {
  duv_handle_t* data = req->handle->data;
  duk_context *ctx = data->ctx;

  duv_push_status(ctx, status);
  duv_fulfill_req(ctx, (uv_req_t*)req, 1);
  req->data = duv_cleanup_req(ctx, req->data);
}

void duv_shutdown_cb(uv_shutdown_t* req, int status) {
    duv_handle_t* data = req->handle->data;
    duk_context *ctx = data->ctx;
  duv_push_status(ctx, status);
  duv_fulfill_req(ctx, (uv_req_t*)req, 1);
  req->data = duv_cleanup_req(ctx, req->data);
}

void duv_connection_cb(uv_stream_t* handle, int status) {
  duk_context *ctx = ((duv_handle_t*)handle->data)->ctx;
  duv_push_status(ctx, status);
  duv_emit_event(ctx, handle->data, DUV_CONNECTION, 1);
}

void duv_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
  duk_context *ctx = ((duv_handle_t*)handle->data)->ctx;

  if (nread > 0) {
    char* out;
    duk_push_null(ctx);
    out = duk_push_fixed_buffer(ctx, nread);
    memcpy(out, buf->base, nread);
  }

  free(buf->base);
  if (nread == 0) return;

  if (nread == UV_EOF) {
    duk_push_null(ctx); // no error
    duk_push_undefined(ctx); // undefined value to signify EOF
  }
  else if (nread < 0) {
    duv_push_status(ctx, nread);//error
    duk_push_undefined(ctx); // undefined value to signify EOF
  }
  duv_emit_event(ctx, handle->data, DUV_READ, 2);
}

void duv_read_udp_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
    duk_context *ctx = ((duv_handle_t*)handle->data)->ctx;

    if (nread >= 0) {
        char* out;
        duk_push_null(ctx);
        out = duk_push_fixed_buffer(ctx, nread);
        memcpy(out, buf->base, nread);
        duv_push_sockaddr(ctx, (struct sockaddr_storage*)addr, sizeof(*addr));
    }

    free(buf->base);
    
    if (nread < 0) {
        duv_push_status(ctx, nread);//error
        duk_push_undefined(ctx); // undefined value to signify EOF
        duk_push_undefined(ctx); // undefined value to signify EOF
    }
    duv_emit_event(ctx, handle->data, DUV_READ, 3);
}

void duv_write_cb(uv_write_t* req, int status) {
  duk_context *ctx = ((duv_handle_t*)req->handle->data)->ctx;
  duv_push_status(ctx, status);
  duv_fulfill_req(ctx, (uv_req_t*)req, 1);
  duv_cleanup_req(ctx, req->data);
}

void duv_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = malloc(suggested_size);
  assert(buf->base);
  buf->len = suggested_size;
}
