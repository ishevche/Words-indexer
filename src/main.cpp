// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <iostream>
#include "exceptions.h"
#include "options_parser.h"
#include "calc_words_functions.h"

int main(int argc, char *argv[]) {
    try {
        command_line_options_t command_line_options{argc, argv};
        config_file_options_t config_file_option{
                command_line_options.config_file
        };
        calculate_words(config_file_option.config);
    } catch (BaseException &ex) {
        std::cerr << ex.what() << std::endl;
        return ex.get_exit_code();
    }
    return 0;
}

