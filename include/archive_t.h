// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef COUNTWORDS_PAR_TBB_ARCHIVE_T_H
#define COUNTWORDS_PAR_TBB_ARCHIVE_T_H

#include <archive.h>
#include <archive_entry.h>
#include <filesystem>

namespace fs = std::filesystem;

std::string read_file_binary(const fs::path &file_path);

class archive_t {
    struct archive *archive_instance;
    struct archive_entry *archive_entry = nullptr;
    int result{ARCHIVE_OK};
    std::string data;
public:

    class file {
        archive_t &parent;
    public:
        std::filesystem::path file_name;
        size_t file_size{};

        explicit file(archive_t &parent) : parent(parent) {}
        std::string get_data();
    };

    archive_t();
    explicit archive_t(const fs::path &archive_path);
    explicit archive_t(std::string &&archive_data);
    archive_t::file next_file();
    [[nodiscard]] bool is_ok() const;
    [[nodiscard]] bool is_eof() const;
    ~archive_t();
};

#endif //COUNTWORDS_PAR_TBB_ARCHIVE_T_H
