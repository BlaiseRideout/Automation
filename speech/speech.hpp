#pragma once

#include <memory>
#include <boost/program_options.hpp>
#include <pocketsphinx/pocketsphinx.h>
#include <sphinxbase/ad.h>

using ps_ptr = std::unique_ptr<ps_decoder_t, decltype(&ps_free)>;
using ad_ptr = std::unique_ptr<ad_rec_t, decltype(&ad_close)>;

using tcp = boost::asio::ip::tcp;

static void run(tcp::socket socket);
static void initialize_pocketsphinx();
static tcp::socket initialize_socket(const boost::program_options::variables_map&);
static boost::program_options::variables_map parse_options(int, char**);
template <typename T>
void fatal_error(const T&);
