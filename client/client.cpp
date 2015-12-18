#include <iostream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void start_client(const boost::program_options::variables_map &options) {
	using boost::asio::ip::tcp;

	auto address = options["address"].as<std::string>();
	auto port = options["port"].as<std::string>();

	boost::asio::io_service io;

	tcp::resolver resolver(io);
	tcp::resolver::query query(address, port);
	tcp::socket socket(io);

	boost::asio::connect(socket, resolver.resolve(query));
	std::cout << "Connected." << std::endl;
	fflush(stdout);
	socket.write_some(boost::asio::buffer(std::string("Hi, I'm a test\n")));
	socket.write_some(boost::asio::buffer(std::string("Hello\n")));

	for(;;) {
		boost::array<char, 128> buf;
		boost::system::error_code err;

		size_t len = socket.read_some(boost::asio::buffer(buf), err);
		if(err)
			break;

		std::cout.write(buf.data(), len);
		std::cout << std::endl;
	}

}

int main(int argc, char **argv) {
	namespace po = boost::program_options;

	po::positional_options_description positional;
	positional.add("address", -1);

	po::options_description desc("Usage: [options] [address]");
	desc.add_options()
		("help,h", "Show this message")
		("port,p", po::value<std::string>()->required(), "Socket port")
		("address", po::value<std::string>()->required(), "Socket address to bind");

	po::variables_map options;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(), options);

	po::notify(options);

	if(options.count("help") || !(options.count("address") && options.count("port"))) {
		std::cout << desc << std::endl;
		return 0;
	}

	start_client(options);

	return 0;
}
