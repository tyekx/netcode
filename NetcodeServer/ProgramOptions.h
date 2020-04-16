#include <boost/program_options.hpp>
#include <Netcode/Network/Config.h>

namespace po = boost::program_options;

void InitProgramOptions(po::options_description & root, Netcode::Network::Config & config);
