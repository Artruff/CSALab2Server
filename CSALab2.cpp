#include "MyServer.h"
#include "hv.h"
#include <stdio.h>
#include "requests.h"
#include "axios.h"
using namespace hv;

#include "hthread.h"

int main(int argc, char** argv) {
    MyServer* server = new MyServer(argc, argv);
    printf("Host started http://127.0.0.1:8080\n");
    printf("Press enter key to exit...\n");
    while (getchar() != '\n');
    //{
    //    HttpRequest req;

    //    // Set the request URL
    //    req.SetUrl("http://localhost:8080/api/endpoint");

    //    // Set the request method to GET
    //    req.SetMethod("GET");

    //    // Send the request
    //    HttpResponse res = req.

    //    // Check if the request was successful
    //    if (res.statusCode() == 200) {
    //        // Get the response body
    //        std::string responseBody = res.body();

    //        // Print the response body
    //        std::cout << "Response Body: " << responseBody << std::endl;
    //    }
    //    else {
    //        // Request failed
    //        std::cout << "Request failed with status code: " << res.statusCode() << std::endl;
    //    }
    //}

   /* {
        auto resp = requests::get("http://127.0.0.1:8080/authorization");
        if (resp == NULL) {
            printf("request failed!\n");
        }
        else {
            printf("%d %s\r\n", resp->status_code, resp->status_message());
            printf("%s\n", resp->body.c_str());
        }
    }*/
    delete server;
    return 0;
}
