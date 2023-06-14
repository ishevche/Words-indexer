// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <iostream>
#include "exceptions.h"
#include "options_parser.h"

namespace po = boost::program_options;

command_line_options_t::command_line_options_t() {
    opt_conf.add_options()
            ("help,h",
             "Show help message")
            ("conf,c", po::value<std::string>(&config_file)->required(),
             "Path to configuration file");
    popt_conf.add("conf", 1);
}

command_line_options_t::command_line_options_t(int ac, char **av) :
        command_line_options_t() {
    parse(ac, av);
}

void command_line_options_t::parse(int ac, char **av) {
    try {
        auto parsed = po::command_line_parser(ac, av)
                .options(opt_conf).positional(popt_conf).run();
        po::store(parsed, var_map);
        if (var_map.count("help")) {
            std::cout << opt_conf << "\n";
            exit(EXIT_SUCCESS);
        }
        po::notify(var_map);
    } catch (std::exception &ex) {
        throw ArgumentsException(ex.what());
    }
}

config_file_options_t::config_file_options_t() {
    opt_conf.add_options()
            ("indir", po::value<std::string>(&config.directory)->required())
            ("out_by_a", po::value<std::string>(&config.out_by_alphabet)->required())
            ("out_by_n", po::value<std::string>(&config.out_by_number)->required())
            ("indexing_extensions",
             po::value<std::vector<std::string>>(&config.indexing_extensions)->multitoken())
            ("archives_extensions",
             po::value<std::vector<std::string>>(&config.archives_extensions)->multitoken())
            ("max_file_size", po::value<size_t>(&config.max_file_size)->required())
            ("filenames_queue_size", po::value<size_t>(&config.filenames_queue_size)->required())
            ("raw_files_queue_size", po::value<size_t>(&config.raw_files_queue_size)->required())
            ("dictionaries_queue_size", po::value<size_t>(&config.dictionaries_queue_size)->required());
}

config_file_options_t::config_file_options_t(const std::string &config_file) :
        config_file_options_t() {
    parse(config_file);
}

void config_file_options_t::parse(const std::string &config_file) {
    try {
        auto parsed = po::parse_config_file(config_file.c_str(),
                                            opt_conf);
        po::store(parsed, var_map);
        po::notify(var_map);

        remove_brackets(config.directory);
        remove_brackets(config.out_by_number);
        remove_brackets(config.out_by_alphabet);
    } catch (po::reading_file &ex) {
        throw ConfigFileOpenError(ex.what());
    } catch (std::exception &ex) {
        throw FileParseError(ex.what());
    }
}

void config_file_options_t::remove_brackets(std::string &path) {
    size_t size = path.size();
    path = path.substr(1, size - 2);
}
