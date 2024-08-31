#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <libssh2.h>

#ifdef HTTPS_SERVER_MAIN

#define PORT 4456

void initialize_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX* create_context() {
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    return ctx;
}

void configure_context(SSL_CTX* ctx) {
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    // Verify private key
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

void handle_client(SSL* ssl) {
    char buf[4096];
    int bytes;

    // https 握手
    if (SSL_accept(ssl) <= 0) {
        printf("这个鸟, 你倒是连接啊!\n");
        // ERR_print_errors_fp(stderr);
    } else {
        bytes = SSL_read(ssl, buf, sizeof(buf));
        printf("读取成功: %s\n", buf);
        if (bytes > 0) {
            buf[bytes] = 0;
            std::string s = "HTTP/1.1 OK 200\r\nContent-Length: 13\r\n\r\nHello, world!";
            SSL_write(ssl, s.data(), s.size());
            printf("写入结束\n");
        } else {
            printf("这个鸟, 你倒是写啊!\n");
            // ERR_print_errors_fp(stderr);
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
}

void custom_error_handler(const char* file, int line, const char* func, unsigned long err, void* arg) {
    // Implement custom error handling logic here
    // For example, you can ignore specific error codes
    if (err != SSL_ERROR_SSL) {
        fprintf(stderr, "Custom error handler: %s:%d:%s: %lu\n", file, line, func, err);
    }
}

int main() {
    chdir("../certs");
    int server_fd, client_fd;
    struct sockaddr_in addr;
    SSL_CTX* ctx;
    SSL* ssl;

    initialize_openssl();
    ctx = create_context();
    configure_context(ctx);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);
        handle_client(ssl);

        close(client_fd);
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 0;
}

#endif // HTTPS_SERVER_MAIN