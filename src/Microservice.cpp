#include "../hv/HttpServer.h"
#include "../hv/hthread.h" // import hv_gettid
#include "../hv/hasync.h"  // import hv::async
#include <ctime>
#include <iomanip>
#include <sstream>
using namespace hv;

// Функция для получения текущего времени в формате ISO 8601
std::string getCurrentTime()
{
    std::time_t now = std::time(nullptr);
    std::tm *tm_now = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm_now, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

HttpServer server;
HttpService router;
int main(int argc, char **argv)
{
    HV_MEMCHECK;
    int res;
    int port = 0;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    if (port == 0)
        port = 8081;

    router.Static("/", "./html");

    router.GET("/time", [](const HttpContextPtr &ctx)
               {
        hv::Json resp;
        try
        {
            // Получаем текущее время
            std::string currentTime = getCurrentTime();

            // Формируем ответ в формате JSON
            resp["time"] = currentTime;

            // Устанавливаем статус ответа
            ctx->setStatus(http_status_enum("200"));
        }
        catch (const std::exception& e)
        {
            resp["status"] = "error";
            resp["message"] = e.what();
            ctx->setStatus(http_status_enum("500")); // Устанавливаем статус 500 для ошибок сервера
        }
        return ctx->send(resp.dump(2)); });

    router.AllowCORS();

    server.service = &router;
    server.port = port;
    server.start();
    printf("Host started http://127.0.0.1:8081\n");
    printf("Press enter key to exit...\n");
    while (getchar() != '\n')
        ;
    server.stop();
    hv::async::cleanup();
    return 0;
}
