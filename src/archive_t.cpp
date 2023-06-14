// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <fstream>
#include "archive_t.h"

std::string read_file_binary(const std::filesystem::path &file_path) {
    std::ifstream raw_file(file_path, std::ios::binary);
    return [&raw_file] {
        std::ostringstream ss{};
        ss << raw_file.rdbuf();
        return ss.str();
    }();
}

archive_t::archive_t() : archive_t(std::move(std::string(""))) {}

archive_t::archive_t(const std::filesystem::path &archive_path) : archive_t() {
    data = read_file_binary(archive_path);
    result = archive_read_open_memory(archive_instance, data.data(), data.size());
}

archive_t::archive_t(std::string &&archive_data)
        : data(std::move(archive_data)),
          archive_instance{archive_read_new()} {
    archive_read_support_format_all(archive_instance);
    archive_read_support_filter_all(archive_instance);
    result = archive_read_open_memory(archive_instance, data.data(), data.size());
}

archive_t::file archive_t::next_file() {
    archive_t::file header(*this);
    result = archive_read_next_header(archive_instance, &archive_entry);
    if (result == ARCHIVE_OK) {
        header.file_name = archive_entry_pathname(archive_entry);
        header.file_size = archive_entry_size(archive_entry);
    }
    return header;
}

bool archive_t::is_ok() const {
    return result == ARCHIVE_OK;
}

bool archive_t::is_eof() const {
    return result == ARCHIVE_EOF;
}

archive_t::~archive_t() {
    archive_read_free(archive_instance);
}

std::string archive_t::file::get_data() {
    std::string file_data;
    file_data.resize(file_size);
    auto bytes_read = archive_read_data(parent.archive_instance, &file_data[0], file_size);
    if (bytes_read < 0) {
        parent.result = static_cast<int>(bytes_read);
    } else if (!bytes_read) {
        parent.result = ARCHIVE_EOF;
    }
    return file_data;
}
