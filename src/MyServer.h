#include <string>
#include "../hv/HttpServer.h"
#include "../hv/hthread.h"    // import hv_gettid
#include "../hv/hasync.h"     // import hv::async

#define TEST_HTTPS 0

enum Roles
{
	UserRole,
	AdminRole
};

typedef struct UserStruct
{
	std::string login;
	std::string password;
	Roles role;

	bool operator==(const UserStruct& other) const
	{
		return login == other.login && password == other.password && role == other.role;
	}

	bool operator<(const UserStruct& other) const
	{
		if (login != other.login)
			return login < other.login;
		if (password != other.password)
			return password < other.password;
		return role < other.role;
	}
} User;

class MyServer
{
public:
	MyServer(int argc, char** argv);
	~MyServer();
	bool AddUser(std::string login, std::string password, Roles role);
	std::string Authorization(std::string login, std::string password);
	void DeleteUser(std::string login);
	User GetUser(std::string login);
	void ChangeUser(std::string old_login, std::string new_login, std::string new_password);
	bool CheckUserAccess(std::string key, std::string login);
	char* GetHost();
private:
	std::map<User, std::string> autUsers;
	std::vector<User> users;
};