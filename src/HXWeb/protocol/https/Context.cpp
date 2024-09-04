#include <HXWeb/protocol/https/Context.h>

#include <stdexcept>

#include <openssl/bio.h>
#include <openssl/ssl.h>  
#include <openssl/err.h>

namespace HX { namespace web { namespace protocol { namespace https {

// HttpsVerifyBuilder::HttpsVerifyBuilder()
    // : verifyMod(SSL_VERIFY_PEER)
// {}

void Context::initServerSSL(
    const HttpsVerifyBuilder& verifyBuilder
) {
    SSL_load_error_strings(); // 加载错误字符串
    if (!SSL_library_init()) // 初始化 SSL 库
        throw "SSL_library_init failed";
    _sslCtx = ::SSL_CTX_new(::TLS_server_method()); // 创建新的 SSL 上下文

    // 设置验证模式
    ::SSL_CTX_set_verify(_sslCtx, verifyBuilder.verifyMod, nullptr);

    if (!_sslCtx)
        throw "SSL_CTX_new failed";

    _errBio = ::BIO_new_fd(2, BIO_NOCLOSE); // 创建错误输出的 BIO

    if (::SSL_CTX_use_certificate_file(_sslCtx, verifyBuilder.certificate.c_str(), SSL_FILETYPE_PEM) <= 0) // 加载证书
        throw "SSL_CTX_use_certificate_file " + verifyBuilder.certificate + " failed";
    if (::SSL_CTX_use_PrivateKey_file(_sslCtx, verifyBuilder.privateKey.c_str(), SSL_FILETYPE_PEM) <= 0) // 加载私钥
        throw "SSL_CTX_use_certificate_file " + verifyBuilder.privateKey + " failed";
    if (!::SSL_CTX_check_private_key(_sslCtx)) // 检查私钥
        throw "SSL_CTX_check_private_key failed";
}

void Context::initClientSSL(const HttpsVerifyBuilder& verifyBuilder) {
    SSL_load_error_strings(); // 加载错误字符串
    if (!SSL_library_init()) // 初始化 SSL 库
        throw std::runtime_error("SSL_library_init failed");
    
    _sslCtx = SSL_CTX_new(TLS_client_method()); // 创建新的 SSL 上下文
    if (!_sslCtx)
        throw std::runtime_error("SSL_CTX_new failed");

    return;
    // 设置验证模式
    SSL_CTX_set_verify(_sslCtx, verifyBuilder.verifyMod, nullptr);

    // 创建错误输出的 BIO
    _errBio = BIO_new_fd(2, BIO_NOCLOSE);
    if (!_errBio)
        throw std::runtime_error("BIO_new_fd failed");
    
    // 如果 verifyBuilder 中提供了证书和私钥，则加载它们
    if (verifyBuilder.certificate.size() && 
        SSL_CTX_use_certificate_file(_sslCtx, verifyBuilder.certificate.c_str(), SSL_FILETYPE_PEM) <= 0) 
        throw std::runtime_error("SSL_CTX_use_certificate_file " + verifyBuilder.certificate + " failed");
    
    if (verifyBuilder.privateKey.size() && 
        SSL_CTX_use_PrivateKey_file(_sslCtx, verifyBuilder.privateKey.c_str(), SSL_FILETYPE_PEM) <= 0) 
        throw std::runtime_error("SSL_CTX_use_PrivateKey_file " + verifyBuilder.privateKey + " failed");

    if (verifyBuilder.certificate.size() &&
        verifyBuilder.privateKey.size() &&
        !SSL_CTX_check_private_key(_sslCtx)) // 检查私钥
        throw std::runtime_error("SSL_CTX_check_private_key failed");
}


}}}} // namespace HX::web::protocol::http