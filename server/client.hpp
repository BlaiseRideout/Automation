#pragma once

#include <string>
#include <functional>
#include <deque>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class server;

using tcp = boost::asio::ip::tcp;

class client : public boost::enable_shared_from_this<client> {
	public:
		using pointer = boost::shared_ptr<client>;
		static client::pointer create(boost::asio::io_service &, server &);
		void start();
		void write(std::string);
		void shutdown();

		tcp::socket socket;
		std::string type;
	private:
		client(boost::asio::io_service &io, server &serv);
		void read_message(std::function<void(client*, std::string)>, const boost::system::error_code&, size_t);
		std::function<void(const boost::system::error_code&, size_t)> wrap_read(std::function<void(client*, std::string)> func);
		void handle_registration(std::string);
		void listen();
		void handle_read(std::string);

		void handle_write(const boost::system::error_code&, size_t);

		server &serv;
		std::deque<std::string> messages;
		boost::asio::streambuf buf;
};
