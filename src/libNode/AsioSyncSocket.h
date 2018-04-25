#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
using boost::asio::ip::tcp;
using std::string;

class AsioSyncSocket
{
public:
	AsioSyncSocket(boost::asio::io_service& io_service);
	~AsioSyncSocket(void);

	bool Connect(const std::string& server, int port);
	std::string SendCommand(const std::string& command, const std::string& hash, const std::string& content = "", const std::string& identity = "");

private:
	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_service& io_service_;
};
