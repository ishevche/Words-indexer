// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <iostream>
#include <oneapi/tbb/flow_graph.h>
#include "calc_words_functions.h"
#include "exceptions.h"
#include "archive_t.h"
#include "timer.h"

namespace fl = tbb::flow;

void calculate_words(const configs &configs, const std::locale &locale) {
#ifndef NDEBUG
    std::cout << "flow_graph" << std::endl;
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
    size_t token_number = std::min(std::min(configs.filenames_queue_size,
                                            configs.raw_files_queue_size),
                                   configs.dictionaries_queue_size);

    fl::graph g;

    fl::input_node < fs::path > finder(
            g,
            [&generator, &finding_time](tbb::flow_control &fc) {
                auto start = get_current_time_fenced();
                auto file = generator.get_next_file();
                if (file.empty())
                    fc.stop();
                auto end = get_current_time_fenced();
                finding_time += to_ms(end - start);
                return file;
            }
    );

    fl::limiter_node<fs::path> input_limiter(g, token_number);

    using reader_node_t = fl::multifunction_node<fs::path, std::tuple<std::string, std::string>>;
    reader_node_t reader(
            g, fl::serial,
            [&configs, &reading_time](const fs::path &filename,
                                      reader_node_t::output_ports_type &out_ports) {
                auto data = read_file(filename, configs, reading_time);
                if (data.first.empty())
                    return;
                if (data.second)
                    std::get<0>(out_ports).try_put(data.first);
                else
                    std::get<1>(out_ports).try_put(data.first);
            }
    );

    using unzipper_node_t = fl::multifunction_node<std::string, std::tuple<std::string>>;
    unzipper_node_t unzipper(
            g, fl::unlimited,
            [&configs](std::string raw_archive,
                       unzipper_node_t::output_ports_type &out_ports) {
                archive_t archive(std::move(raw_archive));
                while (archive.is_ok()) {
                    auto file_header = archive.next_file();
                    if (!has_extension(file_header.file_name,
                                       configs.indexing_extensions) ||
                        file_header.file_size >= configs.max_file_size)
                        continue;
                    auto file_data = file_header.get_data();
                    if (!file_data.empty())
                        std::get<0>(out_ports).try_put(file_data);
                }
            }
    );

    using indexer_node_t = fl::function_node<std::string, std::unordered_map<std::string, size_t>>;
    indexer_node_t indexer(
            g, fl::unlimited,
            [&locale](const std::string &raw_file) {
                std::unordered_map<std::string, size_t> result;
                index_file(raw_file, result, locale);
                return result;
            }
    );

    using merger_node_t = fl::function_node<std::unordered_map<std::string, size_t>, fl::continue_msg>;
    merger_node_t merger(
            g, fl::unlimited,
            [&result] (const std::unordered_map<std::string, size_t>& words_map) {
                merge_dictionary(words_map, result);
                return fl::continue_msg();
            }
    );

    fl::make_edge(finder, input_limiter);
    fl::make_edge(input_limiter, reader);
    fl::make_edge(fl::output_port<0>(reader), unzipper);
    fl::make_edge(fl::output_port<1>(reader), indexer);
    fl::make_edge(unzipper, indexer);
    fl::make_edge(indexer, merger);
    fl::make_edge(merger, input_limiter.decrementer());

    auto start_total = get_current_time_fenced();
    finder.activate();
    g.wait_for_all();
    auto end_total = get_current_time_fenced();
    save_results(result, configs);
    auto end_write = get_current_time_fenced();
    std::cout << "Total=" << to_ms(end_total - start_total) << std::endl;
    std::cout << "Finding=" << finding_time << std::endl;
    std::cout << "Reading=" << reading_time << std::endl;
    std::cout << "Writing=" << to_ms(end_write - end_total) << std::endl;
}
