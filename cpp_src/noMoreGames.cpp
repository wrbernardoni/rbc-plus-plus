#include <iostream>
#include <string>

#include <exception>
#include <typeinfo>
#include <stdexcept>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "http/httplib.h"


#include "../utilities/json.hpp"
using json = nlohmann::json;

using namespace std;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "At least 3 arguments needed: call it like: connect user pass" << endl;
		return 1;
	}
	// TODO Make these cmd line args
	std::string server_url = "https://rbc.jhuapl.edu";
	std::string user = argv[1];
	std::string pass = argv[2];

	httplib::Client cli(server_url);
	cli.set_basic_auth(user.c_str(), pass.c_str());
	cli.enable_server_certificate_verification(false);
	cli.set_follow_location(true);
	cli.set_connection_timeout(1800, 0);

	json maxG;
	maxG["max_games"] = 0;
	if (auto res = cli.Post("/api/users/me/max_games", maxG.dump(), "application/json")) {
		cout << "Successfully told server to stop scheduling games." << endl;
	} else {
		std::cout << "Throwing error " << res.error() << std::endl;
		throw(res.error());
	}

	return 0;
}