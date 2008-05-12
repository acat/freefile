// FreeFile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "http_server/server.hpp"

int main(int argc, char* argv[])
{
    http::server3::server server("0.0.0.0", "4490", "/", 2);

    server.run();

	return 0;
}

