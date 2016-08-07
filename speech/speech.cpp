#include <iostream>
#include <boost/asio.hpp>

#include <sphinxbase/err.h>
#include "speech.hpp"

static ps_ptr ps(nullptr, ps_free);
static ad_ptr ad(nullptr, ad_close);


static void run(tcp::socket socket) {
	bool utt_started = false, in_speech;
	int16 adbuf[2048];

	for(;;) {
		boost::array<char, 128> buf;
		int32 k = ad_read(ad.get(), adbuf, 2048);
		if (k < 0)
			fatal_error("Failed to read audio\n");
		ps_process_raw(ps.get(), adbuf, k, FALSE, FALSE);
		in_speech = ps_get_in_speech(ps.get());
		if (in_speech && !utt_started) {
			utt_started = true;
			std::cout << "Listening..." << std::endl;
		}
		if (!in_speech && utt_started) {
			/* speech -> silence transition, time to start new utterance  */
			ps_end_utt(ps.get());
			int32 score;
			const char *hyp = ps_get_hyp(ps.get(), &score);
			if (hyp != NULL) {
				std::string message(hyp);
				if(message.length() > 0) {
					std::cout << hyp << ": " << score << std::endl;
					if(message.find("tv") != std::string::npos || message.find("t.v.") != std::string::npos || message.find("t. v.") != std::string::npos || message.find("television") != std::string::npos){
						if(message.find("turn on") != std::string::npos || message.find("turn off") != std::string::npos || message.find("power") != std::string::npos)
							socket.send(boost::asio::buffer(std::string("{\"to\":\"tv\",\"message\":\"power\"}\n")));
						else if((message.find("volume") != std::string::npos || message.find("turn") != std::string::npos) && message.find("up") != std::string::npos)
							socket.send(boost::asio::buffer(std::string("{\"to\":\"tv\",\"message\":\"volup\"}\n")));
						else if((message.find("volume") != std::string::npos || message.find("turn") != std::string::npos) && message.find("down") != std::string::npos)
							socket.send(boost::asio::buffer(std::string("{\"to\":\"tv\",\"message\":\"voldown\"}\n")));
						else if(message.find("mute") != std::string::npos)
							socket.send(boost::asio::buffer(std::string("{\"to\":\"tv\",\"message\":\"mute\"}\n")));
						else if(message.find("play") != std::string::npos)
							socket.send(boost::asio::buffer(std::string("{\"to\":\"tv\",\"message\":\"play\"}\n")));
						else if(message.find("pause") != std::string::npos)
							socket.send(boost::asio::buffer(std::string("{\"to\":\"tv\",\"message\":\"pause\"}\n")));
					}
				}
			}

			if (ps_start_utt(ps.get()) < 0)
				fatal_error("Failed to start utterance\n");
			utt_started = false;
			printf("READY....\n");
		}


		if(socket.available()) {
			size_t len = socket.receive(boost::asio::buffer(buf));
			std::cout.write(buf.data(), len);
		}
		usleep(100);
	}
}

static void initialize_pocketsphinx() {
	using cmdln_ptr = std::unique_ptr<cmd_ln_t, decltype(&cmd_ln_free_r)>;
	const arg_t cont_args_def[] = {
	    POCKETSPHINX_OPTIONS,
	    /* Argument file. */
	    {"-argfile",
	     ARG_STRING,
	     NULL,
	     "Argument file giving extra arguments."},
	    {"-adcdev",
	     ARG_STRING,
	     NULL,
	     "Name of audio device to use for input."},
	    {"-infile",
	     ARG_STRING,
	     NULL,
	     "Audio file to transcribe."},
	    {"-inmic",
	     ARG_BOOLEAN,
	     "no",
	     "Transcribe audio from microphone."},
	    {"-time",
	     ARG_BOOLEAN,
	     "no",
	     "Print word times in file transcription."},
	    CMDLN_EMPTY_OPTION
	};

	cmdln_ptr config(cmd_ln_init(NULL, cont_args_def, FALSE,
			"-inmic", "yes",
			"-hmm", "training/en-us-adapt",
			"-adcdev", "plughw:1,0",
			NULL), cmd_ln_free_r);
	//config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);

	ps_default_search_args(config.get());
	ps.reset(ps_init(config.get()));
	if (ps == nullptr) {
		fatal_error("Failed to initialize decoder");
	}

	ad.reset(ad_open_dev(cmd_ln_str_r(config.get(), "-adcdev"),
		(int)cmd_ln_float32_r(config.get(), "-samprate")));
	if (ad == nullptr)
			fatal_error("Failed to open audio device");
	if (ad_start_rec(ad.get()) < 0)
			fatal_error("Failed to start recording");

	if (ps_start_utt(ps.get()) < 0)
			fatal_error("Failed to start utterance");
}

static tcp::socket initialize_socket(const boost::program_options::variables_map &options) {
	auto address = options["ip"].as<std::string>();
	auto port = options["port"].as<std::string>();

	tcp::resolver::query query(address, port);
	boost::asio::io_service io;

	tcp::resolver resolver(io);
	tcp::socket socket(io);

	boost::asio::connect(socket, resolver.resolve(query));
	std::cout << "Connected." << std::endl;
	fflush(stdout);
	socket.send(boost::asio::buffer(std::string("{\"type\":\"voice\"}\n")));

	return std::move(socket);
}

static boost::program_options::variables_map parse_options(int argc, char **argv) {
	namespace po = boost::program_options;

	po::positional_options_description positional;
	positional.add("address", -1);

	po::options_description desc("Usage: [options] [address]");
	desc.add_options()
		("help,h", "Show this message")
		("port,p", po::value<std::string>()->required(), "Socket port")
		("ip", po::value<std::string>()->required(), "Socket address to bind");

	po::variables_map options;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(), options);

	po::notify(options);

	if(options.count("help") || !(options.count("ip") && options.count("port")))
		fatal_error(desc);

	return options;
}

int main(int argc, char **argv) {
	auto options = parse_options(argc, argv);

	while(true) {
		auto socket = initialize_socket(options);
		initialize_pocketsphinx();
		run(std::move(socket));
	}
	return 0;
}

template <typename T>
void fatal_error(const T &message) {
	std::cerr << message << std::endl;
	exit(-1);
}
