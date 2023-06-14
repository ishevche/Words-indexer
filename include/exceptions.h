// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef COUNTWORDS_PAR_TBB_EXCEPTIONS_H
#define COUNTWORDS_PAR_TBB_EXCEPTIONS_H

#include <stdexcept>

class BaseException : public std::runtime_error {
public:
    virtual int get_exit_code() = 0;
    using runtime_error::runtime_error;
};

class ArgumentsException : public BaseException {
public:
    int get_exit_code() override { return 1; }
    using BaseException::BaseException;
};

class ConfigFileOpenError : public BaseException {
public:
    int get_exit_code() override { return 3; }
    using BaseException::BaseException;
};

class ResultFileOpenError : public BaseException {
public:
    int get_exit_code() override { return 4; }
    using BaseException::BaseException;
};

class FileParseError : public BaseException {
public:
    int get_exit_code() override { return 5; }
    using BaseException::BaseException;
};

class WriteError : public BaseException {
public:
    int get_exit_code() override { return 6; }
    using BaseException::BaseException;
};

class DirectoryAccessError : public BaseException {
public:
    int get_exit_code() override { return 26; }
    using BaseException::BaseException;
};

#endif //COUNTWORDS_PAR_TBB_EXCEPTIONS_H
