// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef COUNTWORDS_PAR_TBB_CONFIGS_H
#define COUNTWORDS_PAR_TBB_CONFIGS_H

#include <vector>

struct configs {
    std::string directory;
    std::string out_by_alphabet;
    std::string out_by_number;
    std::vector<std::string> indexing_extensions;
    std::vector<std::string> archives_extensions;
    size_t max_file_size;
    size_t filenames_queue_size;
    size_t raw_files_queue_size;
    size_t dictionaries_queue_size;
};

#endif //COUNTWORDS_PAR_TBB_CONFIGS_H
