#include "MyServer.h"
#include <iostream>
#include <map>
#include <vector>
#include "../hv/hv.h"
#include "../hv/requests.h"
using json = nlohmann::json;
using namespace hv;

typedef UserStruct User;
std::string generateUniqueKey()
{
    std::stringstream key;
    // Append current timestamp
    std::time_t currentTime = std::time(nullptr);
    key << currentTime;

    // Append random number
    key << "_" << std::rand();

    return key.str();
}

std::map<User, std::string> *localAutUsers;
std::vector<User> *localUsers;

MyServer *myServer;
HttpServer server;
HttpService router;
char *host;

bool fileExists(const std::string &filename)
{
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

void createDatabaseIfNotExists()
{
    const char *dbFilename = "users.db";

    // Проверяем, существует ли файл базы данных
    if (fileExists(dbFilename))
    {
        std::cout << "Database file already exists. No action taken." << std::endl;
        return; // Если файл существует, ничего не делаем
    }

    // Создаем соединение с базой данных
    sqlite3 *db;
    char *errMessage = nullptr;

    int rc = sqlite3_open(dbFilename, &db);
    if (rc)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    std::cout << "Opened database successfully." << std::endl;

    // SQL для создания таблицы
    const char *sqlCreateTable = R"(
        CREATE TABLE users (
            login VARCHAR(50) NOT NULL,
            password VARCHAR(255) NOT NULL,
            role INT NOT NULL
        );
    )";

    // Создаем таблицу
    rc = sqlite3_exec(db, sqlCreateTable, 0, 0, &errMessage);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMessage << std::endl;
        sqlite3_free(errMessage);
    }
    else
    {
        std::cout << "Table created successfully." << std::endl;
    }

    // SQL для добавления пользователя admin
    const char *sqlInsertAdmin = R"(
        INSERT INTO users (login, password, role) VALUES ('admin', '1234', 1);
    )";

    // Добавляем администратора
    rc = sqlite3_exec(db, sqlInsertAdmin, 0, 0, &errMessage);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMessage << std::endl;
        sqlite3_free(errMessage);
    }
    else
    {
        std::cout << "User 'admin' added successfully." << std::endl;
    }

    // Закрываем соединение с базой данных
    sqlite3_close(db);
}
std::string GetServerTime()
{
    // Создание HTTP-запроса
    auto response = requests::get("http://microservice:8081/time");

    // Проверяем, успешно ли выполнен запрос
    if (response != nullptr && response->status_code == 200)
    {
        try
        {
            // Парсим тело ответа как JSON
            json response_data = json::parse(response->body);
            // Извлекаем текущее время
            if (response_data.contains("time"))
            {
                return response_data["time"].get<std::string>();
            }
            else
            {
                return "Error: 'time' key not found in response.";
            }
        }
        catch (const std::exception &e)
        {
            return std::string("Error parsing JSON response: ") + e.what();
        }
    }
    else
    {
        // Обработка ошибки соединения или неверного HTTP-статуса
        return "Error: Unable to fetch time. HTTP status: " +
               (response != nullptr ? std::to_string(response->status_code) : "unknown");
    }
}
void ConnectGetTime(HttpService *router)
{
    (*router).GET("/time", [](const HttpContextPtr &ctx)
                  {
        hv::Json resp;
        try
        {
            // Получаем текущее время
            std::string currentTime = GetServerTime();

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
}

void ConnectGetAllUsers(HttpService *router)
{
    (*router).POST("/users", [](const HttpContextPtr &ctx)
                   {
        hv::Json resp;
        try
        {
            // ��������� ������ ������������ �� ���� �������
            hv::Json KeyData = hv::Json::parse(ctx->body());
            // �������� ������� ����������� ����� � ������ ������������
            if (!KeyData.contains("key")) {
                throw std::invalid_argument("Missing user");
            }
            if (myServer->CheckUserAccess(KeyData["key"].get<std::string>(), "")) {
            //if (true){
                //����������� �� ���� ������������� � ��������� �� � �����
                for (int i = 0; i < localUsers->size(); i++) {
                    resp["users"][i]["login"] = (*localUsers)[i].login;
                    resp["users"][i]["password"] = (*localUsers)[i].password;
                    resp["users"][i]["role"] = (*localUsers)[i].role;
                }

                ctx->setStatus(http_status_enum("200"));
            }
        }
        catch (const std::exception& e)
        {
            resp["status"] = "error";
            resp["message"] = e.what();
            ctx->setStatus(http_status_enum("400"));
        }
        return ctx->send(resp.dump(2)); });
}
void ConnectRegistration(HttpService *router)
{
    (*router).POST("/registration", [](const HttpContextPtr &ctx)
                   {
        hv::Json resp;
        try
        {
            // ��������� ������ ������������ �� ���� �������
            hv::Json UserData = hv::Json::parse(ctx->body());
            // �������� ������� ����������� ����� � ������ ������������
            if (!UserData.contains("login") || !UserData.contains("password") || !UserData.contains("role")) {
                throw std::invalid_argument("EmptyData");
            }

            if(!(*myServer).AddUser(UserData["login"].get<std::string>(), UserData["password"].get<std::string>(), (Roles)UserData["role"].get<int>()))
                throw std::invalid_argument("Wrong User Data");

            ctx->setStatus(http_status_enum("200"));
        }
        catch (const std::exception& e)
        {
            resp["status"] = "error";
            resp["message"] = e.what();
            ctx->setStatus(http_status_enum("400"));
        }
        return ctx->send(resp.dump(2)); });
}
void ConnectAuthorization(HttpService *router)
{
    (*router).POST("/authorization", [](const HttpContextPtr &ctx)
                   {
        hv::Json resp;
        try
        {
            // ��������� ������ ������������ �� ���� �������
            hv::Json UserData = hv::Json::parse(ctx->body());
            // �������� ������� ����������� ����� � ������ ������������
            if (!UserData.contains("login") || !UserData.contains("password")) {
                throw std::invalid_argument("EmptyData");
            }

            std::string key = (*myServer).Authorization(UserData["login"].get<std::string>(), UserData["password"].get<std::string>());
            //std::string key = (*myServer).Authorization("admin", "123");
            if(key.empty())
                throw std::invalid_argument("Wrong login or password");

            resp["key"] = key;
            //resp["role"] = (*myServer).GetUser("admin").role;
            resp["role"] = std::to_string(((*myServer).GetUser(UserData["login"].get<std::string>()).role));
            //printf("%d", (int)(*myServer).GetUser(UserData["login"].get<std::string>()).role);
            ctx->setStatus(http_status_enum("200"));
        }
        catch (const std::exception& e)
        {
            resp["status"] = "error";
            resp["message"] = e.what();
            ctx->setStatus(http_status_enum("400"));
        }
        return ctx->send(resp.dump(2)); });
}
void ConnectChangeUserData(HttpService *router)
{
    (*router).POST("/change", [](const HttpContextPtr &ctx)
                   {
        hv::Json resp;
        try
        {
            // ��������� ������ ������������ �� ���� �������
            hv::Json UserData = hv::Json::parse(ctx->body());
            // �������� ������� ����������� ����� � ������ ������������
            if (!UserData.contains("new_login") || !UserData.contains("new_password")
                || !UserData.contains("old_login") || !UserData.contains("key")) {
                throw std::invalid_argument("EmptyData");
            }

            if ((*myServer).CheckUserAccess(UserData["key"].get<std::string>(), UserData["old_login"].get<std::string>()))
                (*myServer).ChangeUser(UserData["old_login"].get<std::string>(), UserData["new_login"].get<std::string>(),
                    UserData["new_password"].get<std::string>());
            else
                throw std::invalid_argument("Wrong login or password");
            ctx->setStatus(http_status_enum("200"));
        }
        catch (const std::exception& e)
        {
            resp["status"] = "error";
            resp["message"] = e.what();
            ctx->setStatus(http_status_enum("400"));
        }
        return ctx->send(resp.dump(2)); });
}
void ConnectDeleteUserData(HttpService *router)
{
    (*router).POST("/delete", [](const HttpContextPtr &ctx)
                   {
        hv::Json resp;
        try
        {
            // ��������� ������ ������������ �� ���� �������
            hv::Json UserData = hv::Json::parse(ctx->body());
            // �������� ������� ����������� ����� � ������ ������������
            if (!UserData.contains("login") || !UserData.contains("key")){
                throw std::invalid_argument("EmptyData");
            }

            if ((*myServer).CheckUserAccess(UserData["key"].get<std::string>(), UserData["login"].get<std::string>()))
                (*myServer).DeleteUser(UserData["login"].get<std::string>());
            else
                throw std::invalid_argument("Wrong login or dont have access");
            ctx->setStatus(http_status_enum("200"));
        }
        catch (const std::exception& e)
        {
            resp["status"] = "error";
            resp["message"] = e.what();
            ctx->setStatus(http_status_enum("400"));
        }
        return ctx->send(resp.dump(2)); });
}
void MyServer::readData()
{
    const char *sql = "SELECT login, password, role FROM users;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to fetch data: " << sqlite3_errmsg(db) << std::endl;
    }
    else
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            User user;
            user.login = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            user.password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            user.role = static_cast<Roles>(sqlite3_column_int(stmt, 2));
            users.push_back(user);
        }
    }

    sqlite3_finalize(stmt);
}

void MyServer::writeData()
{
    char *errorMessage = 0;
    const char *sql = "DELETE FROM users;"; // Очистка таблицы перед записью
    sqlite3_exec(db, sql, NULL, NULL, &errorMessage);

    const char *insertSQL = "INSERT INTO users (login, password, role) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    }
    else
    {
        for (const auto &user : users)
        {
            sqlite3_bind_text(stmt, 1, user.login.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, user.password.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 3, user.role);

            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_reset(stmt);
        }
    }

    sqlite3_finalize(stmt);
}
MyServer::MyServer(int argc, char **argv)
{
    HV_MEMCHECK;

    localAutUsers = &autUsers;
    localUsers = &users;
    int res;
    createDatabaseIfNotExists();
    res = sqlite3_open("users.db", &db);
    if (res == SQLITE_OK)
        readData();
    int port = 0;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    if (port == 0)
        port = 8080;

    router.Static("/", "./html");

    myServer = this;
    ConnectGetTime(&router);
    ConnectGetAllUsers(&router);
    ConnectRegistration(&router);
    ConnectAuthorization(&router);
    ConnectDeleteUserData(&router);
    ConnectChangeUserData(&router); /**/

    router.AllowCORS();

    server.service = &router;
    server.port = port;
    host = server.host;
    server.start();
}

MyServer::~MyServer()
{
    server.stop();
    hv::async::cleanup();
    sqlite3_close(db);
}

char *MyServer::GetHost()
{
    return host;
}

bool MyServer::AddUser(std::string login, std::string password, Roles role)
{
    for (int i = 0; i < users.size(); i++)
    {
        if (users[i].login == login)
        {
            return false;
        }
    }
    if (login == "" || password == "")
        return false;

    User user;
    user.login = login;
    user.password = password;
    user.role = role;
    users.push_back(user);
    return true;
}

std::string MyServer::Authorization(std::string login, std::string password)
{
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].login == login && users[i].password == password)
        {
            std::string key = generateUniqueKey();
            autUsers[users[i]] = key;
            return key;
        }
    }
    return "";
}
void MyServer::DeleteUser(std::string login)
{
    for (size_t i = 0; i <= users.size(); i++)
    {
        if (users[i].login == login)
        {
            auto it = autUsers.find(users[i]);
            if (it != autUsers.end())
            {
                autUsers.erase(it);
            }
            users.erase(users.begin() + i);
        }
    }
}
User MyServer::GetUser(std::string login)
{
    for (int i = 0; i < users.size(); i++)
    {
        if (users[i].login == login)
        {
            return users[i];
        }
    }
    return {};
}
void MyServer::ChangeUser(std::string old_login, std::string new_login, std::string new_password)
{
    for (int i = 0; i < users.size(); i++)
    {
        if (users[i].login == old_login)
        {
            users[i].login = new_login;
            users[i].password = new_password;
        }
    }
}
bool MyServer::CheckUserAccess(std::string key, std::string login)
{
    User thisUser;
    for (int i = 0; i < users.size(); i++)
    {
        if (autUsers[users[i]] == key)
        {
            thisUser = users[i];
            break;
        }
    }
    if (thisUser.role == Roles::AdminRole)
    {
        return true;
    }
    if (thisUser.login == login)
    {
        return true;
    }
    return false;
}