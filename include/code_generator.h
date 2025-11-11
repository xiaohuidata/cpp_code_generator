#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

/**
 * @file code_generator.hpp
 * @brief Main include file for C++ Code Generator library
 * @version 1.0.0
 */

#include "code_generator/zero_copy_stream.h"
#include "code_generator/file_streams.h"
#include "code_generator/formatter.h"
#include "code_generator/cpp_generator.h"
#include "code_generator/config_parser.h"
#include "code_generator/enhanced_cpp_generator.h"

/**
 * @namespace code_generator
 * @brief Main namespace for the C++ Code Generator library
 */
namespace code_generator {

/**
 * @brief Get library version information
 * @return Version string
 */
std::string GetVersion();

/**
 * @brief Get library build information
 * @return Build info string
 */
std::string GetBuildInfo();

} // namespace code_generator

#endif
