// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef COUNTWORDS_PAR_TBB_CONFIG_FILE_H
#define COUNTWORDS_PAR_TBB_CONFIG_FILE_H

#include <boost/program_options.hpp>
#include <string>
#include "configs.h"

class command_line_options_t {
public:
    command_line_options_t();
    command_line_options_t(int ac, char **av);

    //! Explicit is better than implicit:
    command_line_options_t(const command_line_options_t&) = default;
    command_line_options_t& operator=(const command_line_options_t&) = delete;
    command_line_options_t(command_line_options_t&&) = default;
    command_line_options_t& operator=(command_line_options_t&&) = delete;
    ~command_line_options_t() = default;

    void parse(int ac, char **av);

    std::string config_file;

private:
    boost::program_options::variables_map var_map{};
    boost::program_options::options_description opt_conf{
            "Usage:\n\tcountwords_par_tbb [-h|--help] "
            "<config>\nAllowed options:"};
    boost::program_options::positional_options_description popt_conf{};
};

class config_file_options_t {
public:
    config_file_options_t();
    explicit config_file_options_t(const std::string &config_file);

    config_file_options_t(const config_file_options_t &) = default;
    config_file_options_t &operator=(const config_file_options_t &) = delete;
    config_file_options_t(config_file_options_t &&) = default;
    config_file_options_t &operator=(config_file_options_t &&) = delete;
    ~config_file_options_t() = default;

    void parse(const std::string &config_file);

    configs config{};

private:
    boost::program_options::variables_map var_map{};
    boost::program_options::options_description opt_conf{""};

    static void remove_brackets(std::string &path);
};

#endif //COUNTWORDS_PAR_TBB_CONFIG_FILE_H

