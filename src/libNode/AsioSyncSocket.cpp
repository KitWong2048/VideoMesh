#include "AsioSyncSocket.h"

AsioSyncSocket::AsioSyncSocket(boost::asio::io_service& io_service)
: io_service_(io_service)
, socket_(io_service)
{	

}

AsioSyncSocket::~AsioSyncSocket(void)
{
	socket_.close();
}

bool AsioSyncSocket::Connect(const string& server, int port)
{
	string port_string;
	try
	{
		port_string = boost::lexical_cast<string>(port);
	}
	catch (boost::bad_lexical_cast& e)
	{
		port_string = "10080";
	}

	tcp::resolver resolver(io_service_);
	tcp::resolver::query query(server.c_str(), port_string.c_str());
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endpoint_iterator != end)
	{
		socket_.close();
		socket_.connect(*endpoint_iterator++, error);
	}
	if (error)
		return false;
	
	return true;
}

string AsioSyncSocket::SendCommand(const string& command, const string& hash, const string& content, const string& identity)
{
	// Form the request. We specify the "Connection: close" header so that the
	// server will close the socket after transmitting the response. This will
	// allow us to treat all data up until the EOF as the content.
	boost::system::error_code error;

	boost::asio::streambuf request;
	std::ostream request_stream(&request);

	request_stream << command << " " << hash << " VMesh/1.0" << " " << identity << "\r\n\r\n";
	request_stream << content;

	// Send the request.
	boost::asio::write(socket_, request);

	// Read the response status line.
	boost::asio::streambuf response;
	boost::asio::read_until(socket_, response, "\r\n");

	// Check that response is OK.
	std::istream response_stream(&response);
	string http_version;
	response_stream >> http_version;
	unsigned int status_code;
	response_stream >> status_code;
	string status_message;
	std::getline(response_stream, status_message);
	if (!response_stream || http_version.substr(0, 5) != "HTTP/")
	{
		std::cout << "Invalid response\n";
		return "";
	}
	if (status_code != 200)
	{
		std::cout << "Response returned with status code " << status_code << "\n";
		return "";
	}

	// Read the response headers, which are terminated by a blank line.
	boost::asio::read_until(socket_, response, "\r\n\r\n");

	// Process the response headers.
	std::stringstream output;
	string responseString;
	string header;
	while (std::getline(response_stream, header) && header != "\r")
		{}	//std::cout << header << "\n";
	//std::cout << "\n";

	// Write whatever content we already have to output.
	if (response.size() > 0)
		output << &response;

	// Read until EOF, writing data to output as we go.
	while (boost::asio::read(socket_, response,
		boost::asio::transfer_at_least(1), error))
		output << &response;
	if (error != boost::asio::error::eof)
	{
		std::cout << "Error occur while reading data stream\n";
		return "";
	}
	responseString = output.str();
	return responseString;
}