#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "MyServer.h"
#include "../hv/requests.h"

TEST_CASE("Authorization test") {
MyServer* server = new MyServer(0, nullptr);
std::string key = server->Authorization("admin_wrong", "123");
REQUIRE(key == "");
key = server->Authorization("admin", "123");
REQUIRE(key != "");
delete server;
}

TEST_CASE("Delete user test") {
MyServer* server = new MyServer(0, nullptr);
server->DeleteUser("admin");
User user = server->GetUser("admin");
REQUIRE(user.login == "");
delete server;
}

TEST_CASE("Get host test") {
MyServer* server = new MyServer(0, nullptr);
char* host = server->GetHost();
REQUIRE(host != nullptr);
delete server;
}
TEST_CASE("Add user test") {
MyServer* server = new MyServer(0, nullptr);
REQUIRE(server->AddUser("user", "123", Roles::AdminRole) == true);
REQUIRE(server->AddUser("admin", "123", Roles::AdminRole) == false);
delete server;
}