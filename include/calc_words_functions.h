// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef COUNTWORDS_PAR_TBB_CALC_WORDS_FUNCTIONS_H
#define COUNTWORDS_PAR_TBB_CALC_WORDS_FUNCTIONS_H

#include <filesystem>
#include <unordered_map>
#include <oneapi/tbb/concurrent_hash_map.h>
#include "configs.h"

namespace fs = std::filesystem;

class files_generator {
private:
    fs::recursive_directory_iterator iter;
public:
    files_generator() = default;
    ~files_generator() = default;
    files_generator(const files_generator &) = delete;
    files_generator &operator=(const files_generator &) = delete;

    explicit files_generator(const std::string &path) : iter(path) {}

    fs::path get_next_file();
};

std::pair<std::string, bool> read_file(const fs::path &filename,
                                       const configs &configs,
                                       size_t &read_time);

std::unordered_map<std::string, size_t> index_archive(std::string &&raw_archive,
                                                      const configs &configs,
                                                      const std::locale &locale);

void index_file(const std::string &raw_files,
                std::unordered_map<std::string, size_t> &dictionary,
                const std::locale &locale);

void merge_dictionary(const std::unordered_map<std::string, size_t> &map,
                      tbb::concurrent_hash_map<std::string, size_t> &result);

bool has_extension(const fs::path &file_path, const std::vector<std::string> &allowed_extensions);

void save_results(const tbb::concurrent_hash_map<std::string, size_t> &words_count, const configs &configs);

void write_in_file(const std::vector<std::pair<std::string, size_t>> &result,
                   const std::string &file_path);

std::locale setup_boost_locale();

void calculate_words(const configs& configs,
                     const std::locale& locale = setup_boost_locale());


#endif //COUNTWORDS_PAR_TBB_CALC_WORDS_FUNCTIONS_H
