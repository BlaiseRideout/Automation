#pragma once

#include "client.hpp"

#include <set>
#include <map>

class server {
	public:
		server(boost::asio::io_service &io, const boost::asio::ip::address&, const unsigned short);

		std::map<std::string, std::set<client::pointer>> clients;
	private:
		void start_accept();
		void handle_accept(client::pointer, const boost::system::error_code&);

		boost::asio::ip::tcp::acceptor acceptor;
};
