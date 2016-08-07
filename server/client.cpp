#include "server.hpp"

#include <boost/bind.hpp>
#include <json/json.h>

#include <iostream>
#include <sstream>

client::pointer client::create(boost::asio::io_service &io, server &serv) {
	return pointer(new client(io, serv));
}

client::client(boost::asio::io_service &io, server &serv) : socket(io), serv(serv) {
}

void client::start() {
	boost::asio::async_read_until(this->socket, this->buf, "\n", wrap_read(&client::handle_registration));
}

void client::write(std::string message) {
	std::cout << "write: " << message << std::endl;
	boost::asio::async_write(this->socket, boost::asio::buffer(message), boost::bind(&client::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void client::read_message(std::function<void(client*, std::string)> func, const boost::system::error_code &err, size_t size) {
	if(err) {
		std::cerr << err.message() << std::endl;
		shutdown();
		return;
	}

	std::string message;
	std::istream ss(&this->buf);
	std::getline(ss, message);

	func(this, message);
}

std::function<void(const boost::system::error_code&, size_t)> client::wrap_read(std::function<void(client*, std::string)> func) {
	return boost::bind(&client::read_message, this, func, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
}

void client::handle_registration(std::string message) {
	Json::Value root;
	Json::Reader reader;
	if(!reader.parse(message, root)) {
		std::cerr << "Failed to parse registration json:" << reader.getFormattedErrorMessages() << std::endl;
		shutdown();
		return;
	}

	this->type = root.get("type", "").asString();
	if(this->type == "") {
		std::cerr << "Failed to get device type. Disconnecting";
		shutdown();
		return;
	}
	std::cout << "A(n) " << this->type << " connected." << std::endl;

	serv.add_client(this->type, shared_from_this());

	listen();
}

void client::listen() {
	boost::asio::async_read_until(this->socket, this->buf, "\n", wrap_read(&client::handle_read));
}

void client::handle_read(std::string message) {
	Json::Value root;
	Json::Reader reader;
	if(!reader.parse(message, root)) {
		std::cerr << "Failed to parse message from " << this->type << ":" << std::endl << reader.getFormattedErrorMessages();
		return;
	}

	if(!root.isMember("type"))
		root["type"] = this->type;

	serv.send_message(root.get("to", this->type).asString(), shared_from_this(), Json::FastWriter().write(root));
	this->listen();
}

void client::handle_write(const boost::system::error_code &err, size_t bytes_transferred) {
	if(err) {
		std::cerr << err.message() << std::endl;
		this->shutdown();
		return;
	}
}

void client::shutdown() {
	try {
		this->socket.shutdown(tcp::socket::shutdown_both);
		this->socket.close();
	}
	catch(boost::system::system_error) {
	}
	serv.remove_client(shared_from_this());
}
