#include <iostream>
#include <fstream>
//#include "http/HTTPRequest.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT


#include "http/httplib.h"



int main(int argc, const char* argv[])
{	
	// HTTPS
	httplib::Client cli("http://httpbin.org");
	cli.set_basic_auth("user", "pass");
	//cli.set_ca_cert_path("./ca-bundle.crt");
	cli.enable_server_certificate_verification(false);
	cli.set_follow_location(true);

	if (auto res = cli.Post("/post", "{\"foo\": 1, \"bar\": \"baz\"}","application/json")) {
		std::cout << res->status << std::endl;
		std::cout << res->get_header_value("Content-Type") << std::endl;
		std::cout << res->body << std::endl;
	} else {
		std::cout << "error code: " << res.error() << std::endl;
	}

	// Test a post
	httplib::Client cl2("https://rbc.jhuapl.edu");
	cl2.set_basic_auth("user", "pass"); // <-- Change this to log in
	//cli.set_ca_cert_path("./ca-bundle.crt");
	cl2.enable_server_certificate_verification(false);
	cl2.set_follow_location(true);

	if (auto res = cl2.Get("/api/users")) {
		std::cout << res->status << std::endl;
		std::cout << res->get_header_value("Content-Type") << std::endl;
		std::cout << res->body << std::endl;
	} else {
		std::cout << "error code: " << res.error() << std::endl;
	}

	return 0;
}