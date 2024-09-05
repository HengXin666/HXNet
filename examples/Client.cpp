#include <iostream>
#include <HXWeb/client/Client.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXSTL/utils/StringUtils.h>

using namespace std::chrono;

/**
 * @brief 简单的客户端示例
 */

HX::STL::coroutine::task::Task<> startClient() {
    try {
        auto ptr = co_await HX::web::client::Client::request({
            .url = "https://github.com/HengXin666/HXNet",
            .head = { // Host 内部已经自动填写~
                {"User-Agent", "curl/8.8.0"},
                {"Accept", "*/*"}
            },
            .proxy = "socks5://127.0.0.1:2333",
        });
        std::cout << ptr->getStatusCode() << '\n';
        // for (auto&& [k, v] : ptr->getResponseHeaders())
        //     std::cout << k 
        //     << " -> " << v 
        //     << "\n";
        std::string body = ptr->getResponseBody();
        printf("等我写入 (body.size %lu)\n", body.size());
        // std::cout << body << '\n';
        co_await HX::STL::utils::FileUtils::asyncPutFileContent(
            "github.html",
            body, 
            HX::STL::utils::FileUtils::OpenMode::Append
        );
        printf("写入完毕~\n");
    } catch (const std::system_error& e) {
        std::cerr << e.what() << '\n';
    } catch (const char* e) {
        std::cerr << e << '\n';
    }
    co_return;
}

#ifdef CLIENT_MAIN
int main() {
    HX::STL::coroutine::task::runTask(
        HX::STL::coroutine::loop::AsyncLoop::getLoop(),
        startClient()
    );
    return 0;
}
#endif

// Content-Security-Policy: default-src 'none'; base-uri 'self'; child-src github.com/assets-cdn/worker/ github.com/webpack/ github.com/assets/ gist.github.com/assets-cdn/worker/; connect-src 'self' uploads.github.com www.githubstatus.com collector.github.com raw.githubusercontent.com api.github.com github-cloud.s3.amazonaws.com github-production-repository-file-5c1aeb.s3.amazonaws.com github-production-upload-manifest-file-7fdce7.s3.amazonaws.com github-production-user-asset-6210df.s3.amazonaws.com *.rel.tunnels.api.visualstudio.com wss://*.rel.tunnels.api.visualstudio.com api.githubcopilot.com objects-origin.githubusercontent.com copilot-proxy.githubusercontent.com/v1/engines/github-completion/completions proxy.enterprise.githubcopilot.com/v1/engines/github-completion/completions *.actions.githubusercontent.com wss://*.actions.githubusercontent.com productionresultssa0.blob.core.windows.net/ productionresultssa1.blob.core.windows.net/ productionresultssa2.blob.core.windows.net/ productionresultssa3.blob.core.windows.net/ productionresultssa4.blob.core.windows.net/ productionresultssa5.blob.core.windows.net/ productionresultssa6.blob.core.windows.net/ productionresultssa7.blob.core.windows.net/ productionresultssa8.blob.core.windows.net/ productionresultssa9.blob.core.windows.net/ productionresultssa10.blob.core.windows.net/ productionresultssa11.blob.core.windows.net/ productionresultssa12.blob.core.windows.net/ productionresultssa13.blob.core.windows.net/ productionresultssa14.blob.core.windows.net/ productionresultssa15.blob.core.windows.net/ productionresultssa16.blob.core.windows.net/ productionresultssa17.blob.core.windows.net/ productionresultssa18.blob.core.windows.net/ productionresultssa19.blob.core.windows.net/ github-production-repository-image-32fea6.s3.amazonaws.com github-production-release-asset-2e65be.s3.amazonaws.com insights.github.com wss://alive.github.com; font-src github.githubassets.com; form-action 'self' github.com gist.github.com copilot-workspace.githubnext.com objects-origin.githubusercontent.com; frame-ancestors 'none'; frame-src viewscreen.githubusercontent.com notebooks.githubusercontent.com; img-src 'self' data: blob: github.githubassets.com media.githubusercontent.com camo.githubusercontent.com identicons.github.com avatars.githubusercontent.com private-avatars.githubusercontent.com github-cloud.s3.amazonaws.com objects.githubusercontent.com secured-user-images.githubusercontent.com/ user-images.githubusercontent.com/ private-user-images.githubusercontent.com opengraph.githubassets.com github-production-user-asset-6210df.s3.amazonaws.com customer-stories-feed.github.com spotlights-feed.github.com objects-origin.githubusercontent.com *.githubusercontent.com; manifest-src 'self'; media-src github.com user-images.githubusercontent.com/ secured-user-images.githubusercontent.com/ private-user-images.githubusercontent.com github-production-user-asset-6210df.s3.amazonaws.com gist.github.com; script-src github.githubassets.com; style-src 'unsafe-inline' github.githubassets.com; upgrade-insecure-requests; worker-src github.com/assets-cdn/worker/ github.com/webpack/ github.com/assets/ gist.github.com/assets-cdn/worker/