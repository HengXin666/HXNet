#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-09-01 22:19:58
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HX_HTTPS_CONTEXT_H_
#define _HX_HTTPS_CONTEXT_H_

#include <string>

#ifdef __GNUC__
#define HOT_FUNCTION [[gnu::hot]]
#else
#define HOT_FUNCTION
#endif

// 前置声明

typedef struct bio_st BIO;
typedef struct ssl_ctx_st SSL_CTX;

namespace HX { namespace web { namespace protocol { namespace https {

/**
 * @brief Https 上下文, 用于管理公钥/秘钥, 以及提供`sslCtx`
 */
class Context {
    Context() {};
    Context& operator=(Context&&) = delete;
public:
    /**
     * @brief 获取 Https 上下文
     * @return Context& 
     */
    HOT_FUNCTION static Context& getContext() {
        thread_local static Context context;
        return context;
    }

    /**
     * @brief 初始化SSL
     * @param certificate 公钥文件路径
     * @param privateKey 私钥文件路径
     * @throw 加载证书出现问题
     * @throw 证书不匹配
     */
    void initSSL(
        const std::string& certificate,
        const std::string& privateKey
    );

    BIO* getErrBio() const {
        return _errBio;
    }

    SSL_CTX* getSslCtx() const {
        return _sslCtx;
    }
private:
    BIO* _errBio = nullptr;     // 用于 SSL 错误输出的 BIO
    SSL_CTX* _sslCtx = nullptr; // 全局 SSL 上下文
};

}}}} // namespace HX::web::protocol::http

#undef HOT_FUNCTION

#endif // !_HX_HTTPS_CONTEXT_H_