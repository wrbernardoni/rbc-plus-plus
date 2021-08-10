#include <iostream>
#include <fstream>
//#include "http/HTTPRequest.hpp"

#include <string>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "http/httplib.h"

#include "utilities/json.hpp"
using json = nlohmann::json;

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

		auto j = json::parse(res->body);
		std::cout << "As a json:" << std::endl << j << std::endl;
		std::cout << j["usernames"][0] << std::endl;
	} else {
		std::cout << "error code: " << res.error() << std::endl;
	}

	auto j = json::parse("{\"testTrue\" : true, \"testFalse\" : false}");
	std::cout << "Testing json bools:" << j << std::endl;
	if (j["testTrue"].get<bool>())
	{
		std::cout << "testTrue true\n";
	}
	else
	{
		std::cout << "testTrue false\n";
	}

	if (j["testFalse"].get<bool>())
	{
		std::cout << "testFalse true\n";
	}
	else
	{
		std::cout << "testFalse false\n";
	}

	std::string server_url = "https://rbc.jhuapl.edu";
	int gameID = 17;
	std::string test = server_url + "/api/games/" + std::to_string(gameID);
	std::cout << test << std::endl;

	return 0;
}