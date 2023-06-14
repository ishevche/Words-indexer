// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <oneapi/tbb/parallel_pipeline.h>
#include <iostream>
#include "calc_words_functions.h"
#include "exceptions.h"
#include "timer.h"


void calculate_words(const configs &configs, const std::locale &locale) {
#ifndef NDEBUG
    std::cout << "pipeline" << std::endl;
#endif
    fs::path root_directory{configs.directory};
    if (!exists(root_directory)) {
        throw DirectoryAccessError("No such root_directory " +
                                   root_directory.string());
    } else if (!is_directory(root_directory)) {
        throw DirectoryAccessError(root_directory.string() +
                                   " is not a root_directory");
    }

    size_t reading_time{};
    size_t finding_time{};
    tbb::concurrent_hash_map<std::string, size_t> result;
    files_generator generator(root_directory);

    tbb::filter<void, std::shared_ptr<fs::path>> finder(
            tbb::filter_mode::serial_in_order,
            [&generator, &finding_time](tbb::flow_control &fc) {
                auto start = get_current_time_fenced();
                auto file = generator.get_next_file();
                if (file.empty())
                    fc.stop();
                auto end = get_current_time_fenced();
                finding_time += to_ms(end - start);
                return std::make_shared<fs::path>(file);
            }
    );

    tbb::filter<std::shared_ptr<fs::path>, std::shared_ptr<std::pair<std::string, bool>>> reader(
            tbb::filter_mode::serial_out_of_order,
            [&configs, &reading_time](const std::shared_ptr<fs::path> &file) {
                return std::make_shared<std::pair<std::string, bool>>(
                        read_file(*file, configs, reading_time)
                );
            }
    );

    tbb::filter<std::shared_ptr<std::pair<std::string, bool>>,
            std::shared_ptr<std::unordered_map<std::string, size_t>>> indexer(
            tbb::filter_mode::parallel,
            [&configs, &locale](const std::shared_ptr<std::pair<std::string, bool>> &in_file) {
                if (in_file->second) {
                    return std::make_shared<std::unordered_map<std::string, size_t>>(
                            index_archive(std::move(in_file->first), configs, locale)
                    );
                } else {
                    std::unordered_map<std::string, size_t> resulting_map;
                    index_file(in_file->first, resulting_map, locale);
                    return std::make_shared<std::unordered_map<std::string, size_t>>(resulting_map);
                }
            }
    );

    tbb::filter<std::shared_ptr<std::unordered_map<std::string, size_t>>, void> merger(
            tbb::filter_mode::parallel,
            [&result](const std::shared_ptr<std::unordered_map<std::string, size_t>> &words_map) {
                merge_dictionary(*words_map, result);
            }
    );

    auto resulting_pipeline = finder & reader & indexer & merger;
    auto tokens_number = std::min(std::min(configs.filenames_queue_size,
                                           configs.raw_files_queue_size),
                                  configs.dictionaries_queue_size);

    auto start_total = get_current_time_fenced();
    tbb::parallel_pipeline(tokens_number, resulting_pipeline);
    auto end_total = get_current_time_fenced();
    save_results(result, configs);
    auto end_write = get_current_time_fenced();
    std::cout << "Total=" << to_ms(end_total - start_total) << std::endl;
    std::cout << "Finding=" << finding_time << std::endl;
    std::cout << "Reading=" << reading_time << std::endl;
    std::cout << "Writing=" << to_ms(end_write - end_total) << std::endl;
}
