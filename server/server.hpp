#pragma once

#include "client.hpp"

#include <set>
#include <map>

class server {
	public:
		server(boost::asio::io_service &io, const boost::asio::ip::address&, const unsigned short);
		void add_client(std::string, client::pointer);
		void remove_client(client::pointer);
		void send_message(std::string, client::pointer, std::string);
	private:
		void start_accept();
		void handle_accept(client::pointer, const boost::system::error_code&);

		boost::asio::ip::tcp::acceptor acceptor;
		std::set<client::pointer> active_clients;
		std::map<std::string, std::set<client::pointer>> clients;
};
