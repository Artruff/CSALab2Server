#include "MyServer.h"
#include <iostream>
#include <map>
#include <vector>
using namespace hv;

typedef UserStruct User;
std::string generateUniqueKey() {
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

MyServer* myServer;
HttpServer server;
HttpService router;
char* host;

void ConnectGetAllUsers(HttpService* router) {
    (*router).POST("/users", [](const HttpContextPtr& ctx) {
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
        return ctx->send(resp.dump(2));
        });
}
void ConnectRegistration(HttpService* router) {
    (*router).POST("/registration", [](const HttpContextPtr& ctx) {
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
        return ctx->send(resp.dump(2));
        });
}
void ConnectAuthorization(HttpService* router) {
    (*router).POST("/authorization", [](const HttpContextPtr& ctx) {
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
        return ctx->send(resp.dump(2)); 
        });
}
void ConnectChangeUserData(HttpService* router) {
    (*router).POST("/change", [](const HttpContextPtr& ctx) {
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
        return ctx->send(resp.dump(2));
        });
}
void ConnectDeleteUserData(HttpService* router) {
    (*router).POST("/delete", [](const HttpContextPtr& ctx) {
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
        return ctx->send(resp.dump(2));
        });
}
MyServer::MyServer(int argc, char** argv)
{
    HV_MEMCHECK;

    localAutUsers = &autUsers;
    localUsers = &users;

    int port = 0;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    if (port == 0) port = 8080;

    router.Static("/", "./html");

    myServer = this;

    AddUser("admin", "123", Roles::AdminRole);

    ConnectGetAllUsers(&router);
    ConnectRegistration(&router);
    ConnectAuthorization(&router);
    ConnectDeleteUserData(&router);
    ConnectChangeUserData(&router);/**/

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
}

char* MyServer::GetHost() {
    return host;
}

bool MyServer::AddUser(std::string login, std::string password, Roles role) {
    for (int i = 0; i < users.size(); i++) {
        if (users[i].login == login) {
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

std::string MyServer::Authorization(std::string login, std::string password) {
	for (size_t i = 0; i < users.size(); i++) {
		if (users[i].login == login && users[i].password == password) {
            std::string key = generateUniqueKey();
            autUsers[users[i]] = key;
			return key;
		}
	}
	return "";
}
void MyServer::DeleteUser(std::string login){
	for (size_t i = 0; i <= users.size(); i++) {
		if (users[i].login == login) {
            auto it = autUsers.find(users[i]);
            if (it != autUsers.end()) {
                autUsers.erase(it);
            }
			users.erase(users.begin() + i);
		}
	}
}
User MyServer::GetUser(std::string login) {
	for (int i = 0; i < users.size(); i++) {
		if (users[i].login == login) {
			return users[i];
		}
	}
	return {};
}
void MyServer::ChangeUser(std::string old_login, std::string new_login, std::string new_password) {
	for (int i = 0; i < users.size(); i++) {
		if (users[i].login == old_login) {
			users[i].login = new_login;
			users[i].password = new_password;
		}
	}
}
bool MyServer::CheckUserAccess(std::string key, std::string login) {
    User thisUser;
    for (int i = 0; i < users.size(); i++) {
        if(autUsers[users[i]] == key) {
			thisUser = users[i];
            break;
		}
    }
    if(thisUser.role == Roles::AdminRole) {
		return true;
	}
	if(thisUser.login == login) {
		return true;
	}
	return false;
}