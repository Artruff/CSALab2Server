#include "MyServer.h"
#include <stdio.h>
using namespace hv;

int main(int argc, char** argv) {
    MyServer* server = new MyServer(argc, argv);
    printf("Host started http://127.0.0.1:8080\n");
    printf("Press enter key to exit...\n");
    while (getchar() != '\n');
    delete server;
    return 0;
}
