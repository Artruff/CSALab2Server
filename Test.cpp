#include <assert.h>
#include <check.h>
#include <stdlib.h>
#include <time.h>
#include "hv/requests.h"

#include "MyServer.h"

START_TEST(test_authorization) {
    MyServer* server = new MyServer(0, nullptr);
    std::string key = server->Authorization("admin_wrong", "123");
    ck_assert_int_eq(key == "", 1);
    key = server->Authorization("admin", "123");
    ck_assert_int_eq(key != "", 1);
    delete server;
}

START_TEST(test_delete) {
    MyServer* server = new MyServer(0, nullptr);
    server->DeleteUser("admin");
    User user = server->GetUser("admin");
    ck_assert_int_eq(user.login == "", 1);
    delete server;
}

START_TEST(test_connect) {
    MyServer* server = new MyServer(0, nullptr);
    
    auto resp = requests::get("http://127.0.0.1:8080/ping");
    ck_assert_int_eq(resp != NULL, 1);
    delete server;
}

START_TEST(test_host) {
    MyServer* server = new MyServer(0, nullptr);
    
    char* host = server->GetHost();
    ck_assert_ptr_nonnull(host);
    delete server;
}

START_TEST(test_add_user) {
    MyServer* server = new MyServer(0, nullptr);
    
    ck_assert_int_eq(server->AddUser("user", "123", Roles::AdminRole), 1);
    ck_assert_int_eq(server->AddUser("admin", "123", Roles::AdminRole), 0);
    delete server;
}

Suite *server_suite(void) {
  Suite *s;
  TCase *tc_core;

  s = suite_create("Server");

  tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_authorization);
  tcase_add_test(tc_core, test_delete);
  tcase_add_test(tc_core, test_host);
  tcase_add_test(tc_core, test_connect);
  tcase_add_test(tc_core, test_add_user);

  suite_add_tcase(s, tc_core);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = server_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);

  number_failed = srunner_ntests_failed(sr);

  srunner_free(sr);

  return (number_failed == 0) ? 0 : 1;
  }