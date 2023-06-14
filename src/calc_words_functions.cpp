// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <boost/locale.hpp>
#include <calc_words_functions.h>
#include <fstream>
#include "archive_t.h"
#include "exceptions.h"
#include "timer.h"

#ifdef COUNTWORDS_FMT_FOUND

#include <fmt/core.h>

#endif //COUNTWORDS_FMT_FOUND

namespace bl = boost::locale;
namespace ba = bl::boundary;

fs::path files_generator::get_next_file() {
    fs::path result;
    for (auto &file: iter) {
        if (file.is_regular_file()) {
            result = file;
            break;
        }
    }
    if (iter != fs::recursive_directory_iterator())
        iter++;
    return result;
}

std::pair<std::string, bool> read_file(const fs::path &filename,
                                       const configs &configs,
                                       size_t &read_time) {
#ifndef NDEBUG
    std::cout << "Processing file " << filename << std::endl;
#endif // NDEBUG
    bool is_archive;
    if (has_extension(filename, configs.archives_extensions)) {
        is_archive = true;
    } else if (has_extension(filename, configs.indexing_extensions) &&
               fs::file_size(filename) < configs.max_file_size) {
        is_archive = false;
    } else {
        return {};
    }
    auto start_time = get_current_time_fenced();
    auto file_data = read_file_binary(filename);
    std::pair<std::string, bool> result{std::move(file_data), is_archive};
    auto end_time = get_current_time_fenced();
    read_time += to_ms(end_time - start_time);
    return result;
}

std::unordered_map<std::string, size_t> index_archive(std::string &&raw_archive,
                                                      const configs &configs,
                                                      const std::locale &locale) {
    archive_t archive(std::move(raw_archive));
    std::unordered_map<std::string, size_t> words;
    while (archive.is_ok()) {
        auto file_header = archive.next_file();
        if (!has_extension(file_header.file_name,
                           configs.indexing_extensions) ||
            file_header.file_size >= configs.max_file_size)
            continue;
        auto file_data = file_header.get_data();
        if (!file_data.empty())
            index_file(file_data, words, locale);
    }
    return words;
}

void index_file(const std::string &raw_files,
                std::unordered_map<std::string, size_t> &words_count,
                const std::locale &locale) {
    ba::segment_index<std::string::const_iterator>
            words_iter(ba::word, raw_files.begin(),
                       raw_files.end(), locale);
    words_iter.rule(ba::word_letters);

    for (auto it = words_iter.begin(), e = words_iter.end(); it != e; ++it) {
        std::string word(*it);
        word = bl::fold_case(
                bl::normalize(word, bl::norm_default, locale),
                locale
        );
        words_count[word]++;
    }
}

void merge_dictionary(const std::unordered_map<std::string, size_t> &map,
                      tbb::concurrent_hash_map<std::string, size_t> &result) {
    tbb::concurrent_hash_map<std::string, size_t>::accessor accessor;
    for (const auto &dict_entry: map) {
        result.insert(accessor, dict_entry.first);
        accessor->second += dict_entry.second;
        accessor.release();
    }
}


bool has_extension(const fs::path &file_path,
                   const std::vector<std::string> &allowed_extensions) {
    std::string extension = file_path.extension();
    auto find_result = std::find(allowed_extensions.begin(),
                                 allowed_extensions.end(), extension);
    return find_result != allowed_extensions.end();
}

void save_results(const tbb::concurrent_hash_map<std::string, size_t> &words_count,
                  const configs &configs) {
    std::vector<std::pair<std::string, size_t>> key_value(words_count.begin(), words_count.end());
    std::sort(key_value.begin(), key_value.end());
    write_in_file(key_value, configs.out_by_alphabet);
    std::sort(key_value.begin(), key_value.end(),
              [](auto a, auto b) {
                  return a.second > b.second ||
                         (a.second == b.second && a.first < b.first);
              });
    write_in_file(key_value, configs.out_by_number);
}

void write_in_file(const std::vector<std::pair<std::string, size_t>> &result,
                   const std::string &file_path) {
    std::ofstream out(file_path);
    if (!out.is_open()) {
        throw ResultFileOpenError("Can not open file " + file_path);
    }
    out << std::left;
    for (const auto &entry: result) {
#ifdef COUNTWORDS_FMT_FOUND
        out << fmt::format("{:20} {}\n", entry.first, entry.second);
#else
        out << std::setw(20) << entry.first << " " << entry.second << "\n";
#endif // COUNTWORDS_FMT_FOUND
    }
    out.close();
}

std::locale setup_boost_locale() {
    bl::localization_backend_manager lbm =
            bl::localization_backend_manager::global();
    lbm.select("icu");
    return bl::generator{}("en_US.UTF-8");
}
