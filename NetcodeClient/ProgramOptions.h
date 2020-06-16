#pragma once

#include <Netcode/ModulesConfig.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

void InitProgramOptions(po::options_description & root, Netcode::Config & config);
