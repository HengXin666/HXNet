#include <openssl/ssl.h>
#include <openssl/async.h>
#include <openssl/err.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * @brief Set the up async io object
 * 抛弃, 我还是先开发WebSocket好了qwq
 */

void setup_async_io() {
    // 初始化 OpenSSL 异步环境
    // ASYNC_INIT;
    ASYNC_WAIT_CTX *wait_ctx = ASYNC_WAIT_CTX_new();
    
    // 设置异步等待上下文
    if (!wait_ctx) {
        fprintf(stderr, "Failed to create ASYNC_WAIT_CTX\n");
        exit(EXIT_FAILURE);
    }

    // 注册异步 I/O 操作
    // 使用 io_uring 或其他异步库
}

int main() {
    SSL_library_init();
    SSL_load_error_strings();

    setup_async_io();

    // 创建 SSL_CTX
    SSL_CTX *ctx = SSL_CTX_new(TLS_method());
    if (!ctx) {
        fprintf(stderr, "Failed to create SSL_CTX\n");
        ERR_print_errors_fp(stderr);
        return 1;
    }

    // 创建 SSL 对象
    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "Failed to create SSL object\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 1;
    }

    // 进一步设置和操作 SSL 连接
    // 异步处理数据传输等

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    // ASYNC_WAIT_CTX_free(wait_ctx);
    return 0;
}
