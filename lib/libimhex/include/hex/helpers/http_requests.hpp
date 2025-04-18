#pragma once

#include <hex.hpp>

#include <future>
#include <map>
#include <string>
#include <vector>

#include <wolv/io/file.hpp>
#include <wolv/utils/guards.hpp>

#if defined(OS_WEB)
    #include <emscripten/fetch.h>
#endif

typedef void CURL;

namespace hex {

    class HttpRequest {
    public:

        class ResultBase {
        public:
            ResultBase() = default;
            explicit ResultBase(u32 statusCode) : m_statusCode(statusCode), m_valid(true) { }

            [[nodiscard]] u32 getStatusCode() const {
                return m_statusCode;
            }

            [[nodiscard]] bool isSuccess() const {
                return this->getStatusCode() == 200;
            }

            [[nodiscard]] bool isValid() const {
                return m_valid;
            }

        private:
            u32 m_statusCode = 0;
            bool m_valid = false;
        };

        template<typename T>
        class Result : public ResultBase {
        public:
            Result() = default;
            Result(u32 statusCode, T data) : ResultBase(statusCode), m_data(std::move(data)) { }

            [[nodiscard]]
            const T& getData() const {
                return m_data;
            }

        private:
            T m_data;
        };

        HttpRequest(std::string method, std::string url);
        ~HttpRequest();

        HttpRequest(const HttpRequest&) = delete;
        HttpRequest& operator=(const HttpRequest&) = delete;

        HttpRequest(HttpRequest &&other) noexcept;

        HttpRequest& operator=(HttpRequest &&other) noexcept;

        static void setProxyState(bool enabled);
        static void setProxyUrl(std::string proxy);

        void setMethod(std::string method) {
            m_method = std::move(method);
        }

        void setUrl(std::string url) {
            m_url = std::move(url);
        }

        void addHeader(std::string key, std::string value) {
            m_headers[std::move(key)] = std::move(value);
        }

        void setBody(std::string body) {
            m_body = std::move(body);
        }

        void setTimeout(u32 timeout) {
            m_timeout = timeout;
        }

        float getProgress() const {
            return m_progress;
        }

        void cancel() {
            m_canceled = true;
        }

        template<typename T = std::string>
        std::future<Result<T>> downloadFile(const std::fs::path &path);

        std::future<Result<std::vector<u8>>> downloadFile();

        template<typename T = std::string>
        std::future<Result<T>> uploadFile(const std::fs::path &path, const std::string &mimeName = "filename");

        template<typename T = std::string>
        std::future<Result<T>> uploadFile(std::vector<u8> data, const std::string &mimeName = "filename", const std::fs::path &fileName = "data.bin");

        template<typename T = std::string>
        std::future<Result<T>> execute();

        static std::string urlEncode(const std::string &input);

        static std::string urlDecode(const std::string &input);

        void setProgress(float progress) { m_progress = progress; }
        bool isCanceled() const { return m_canceled; }

        static size_t writeToVector(void *contents, size_t size, size_t nmemb, void *userdata);
        static size_t writeToFile(void *contents, size_t size, size_t nmemb, void *userdata);

        template<typename T>
        Result<T> executeImpl(std::vector<u8> &data);

    private:
        static void checkProxyErrors();
        void setDefaultConfig();

    private:
        #if defined(OS_WEB)
        emscripten_fetch_attr_t m_attr;
        #else
        CURL *m_curl;
        #endif

        std::mutex m_transmissionMutex;

        std::string m_method;
        std::string m_url;
        std::string m_body;
        std::promise<std::vector<u8>> m_promise;
        std::map<std::string, std::string> m_headers;
        u32 m_timeout = 1000;

        std::atomic<float> m_progress = 0.0F;
        std::atomic<bool> m_canceled = false;
    };
}


#if defined(OS_WEB)
#include <hex/helpers/http_requests_emscripten.hpp>
#else
#include <hex/helpers/http_requests_native.hpp>
#endif
