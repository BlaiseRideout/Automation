#include "server.hpp"

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


server::server(boost::asio::io_service &io, const boost::asio::ip::address &addr, const unsigned short port) : acceptor(io, tcp::endpoint(addr, port)) {
	this->start_accept();
}

void server::start_accept() {
	client::pointer new_client = client::create(this->acceptor.get_io_service(), *this);

	this->acceptor.async_accept(new_client->socket,
			boost::bind(&server::handle_accept, this, new_client, boost::asio::placeholders::error));
}

void server::handle_accept(client::pointer new_client, const boost::system::error_code &err) {
	if(!err) {
		new_client->start();
		this->active_clients.emplace(new_client);
	}
	this->start_accept();
}

void server::add_client(std::string type, client::pointer c) {
	if(this->active_clients.insert(c).second)
		std::cout << "Double added" << std::endl;
	this->clients[type].insert(c);
}

void server::remove_client(client::pointer c) {
	if(!this->active_clients.erase(c))
		std::cout << "Couldn't remove " << c->type << " client" << std::endl;
	if(this->clients[c->type].erase(c))
		std::cout << "Deregistered " << c->type << " client" << std::endl;
}

void server::send_message(std::string type, client::pointer c, std::string message) {
	auto of_type = this->clients[type];
	for(auto i = of_type.begin(); i != of_type.end(); ++i)
		if(*i != c) {
			try {
				(*i)->write(message);
			}
			catch(boost::system::system_error e){
				std::cerr << e.what() << std::endl;
				(*(i--))->shutdown();
			}
		}
}


void start_server(const boost::program_options::variables_map &options) {
	auto port = options["port"].as<int>();
	auto ip = boost::asio::ip::address::from_string(options["ip"].as<std::string>());

	boost::asio::io_service io;
	server serv(io, ip, port);
	io.run();
}

int main(int argc, char **argv) {
	namespace po = boost::program_options;

	po::positional_options_description positional;
	positional.add("address", -1);

	po::options_description desc("Usage: [options] [address]");
	desc.add_options()
		("help,h", "Show this message")
		("port,p", po::value<int>()->required(), "Socket port")
		("ip", po::value<std::string>()->required(), "Socket IP address");

	po::variables_map options;
	try {
		po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(), options);
		po::notify(options);
	}
	catch(const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	if(options.count("help") || !(options.count("ip") && options.count("port"))) {
		std::cout << desc << std::endl;
		return 1;
	}

	start_server(options);

	return 0;
}
