#include <HXWeb/protocol/https/Context.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>  
#include <openssl/err.h>

namespace HX { namespace web { namespace protocol { namespace https {

void Context::initSSL(
    const std::string& certificate,
    const std::string& privateKey
) {
    SSL_load_error_strings(); // 加载错误字符串
    if (!SSL_library_init()) // 初始化 SSL 库
        throw "SSL_library_init failed";
    _sslCtx = SSL_CTX_new(SSLv23_method()); // 创建新的 SSL 上下文
    if (!_sslCtx)
        throw "SSL_CTX_new failed";
    _errBio = BIO_new_fd(2, BIO_NOCLOSE); // 创建错误输出的 BIO
    if (SSL_CTX_use_certificate_file(_sslCtx, certificate.c_str(), SSL_FILETYPE_PEM) <= 0) // 加载证书
        throw "SSL_CTX_use_certificate_file " + certificate + " failed";
    if (SSL_CTX_use_PrivateKey_file(_sslCtx, privateKey.c_str(), SSL_FILETYPE_PEM) <= 0) // 加载私钥
        throw "SSL_CTX_use_certificate_file " + privateKey + " failed";
    if (!SSL_CTX_check_private_key(_sslCtx)) // 检查私钥
        throw "SSL_CTX_check_private_key failed";
}

}}}} // namespace HX::web::protocol::http