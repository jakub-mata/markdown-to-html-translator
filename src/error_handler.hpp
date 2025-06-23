#ifndef _ERROR_HANDLING_HPP
#define _ERROR_HANDLING_HPP

#include <iostream>
#include <fstream>
#include <chrono>

enum ErrorType
{
    IncorrectArgFormat,
    MissingInput,
    UnableToOpenInput,
    UnableToOpenOutput,
};

void handle_error(ErrorType err)
{
    switch (err)
    {
    case IncorrectArgFormat:
        std::cerr << "Arguments provided are not formatted correctly" << std::endl;
        break;
    case MissingInput:
        std::cerr << "No input file has been provided" << std::endl;
        break;
    case UnableToOpenInput:
        std::cerr << "Unable to open the input file. Make sure it exists and is written correctly" << std::endl;
        break;
    case UnableToOpenOutput:
        std::cerr << "Unable to open the output file. Make sure it exists and is written correctly" << std::endl;
        break;
    default:

        break;
    }
}

/**
 * @class Logger
 * @brief A class for logging messages to a file. It provides methods for logging info, warnings, and errors.
 *        The verbosity level determines the level of messages that will be logged.
 */
class Logger
{
public:

    /**
     * @brief Constructs a Logger object when logging disabled.
     */
    Logger() {}
    /**
     * @brief Constructs a Logger object.
     * @param verbosity The verbosity level (0: no logging, 1: errors only, 2: warnings and errors, 3: all messages).
     */
    Logger(size_t verbosity) : log_stream(std::ofstream("logs.log", std::ios::app)), verbosity(verbosity) {}

    /**
     * @brief Logs an info message.
     * @param message The message to log.
     */
    void log_info(std::string&& message)
    {
        if (verbosity < 3)
            return;
        std::time_t curr = get_curr_time();
        log_stream << "INFO at " << std::ctime(&curr) << ": " << message << std::endl;
    }

    /**
     * @brief Logs a warning message.
     * @param message The message to log.
     * @param line The line number where the warning occurred (default is 0).
     */
    void log_warning(const std::string& message, const size_t& line = 0)
    {
        if (verbosity < 2)
            return;
        std::time_t curr = get_curr_time();
        if (line == 0)
            log_stream << "WARNING at " << std::ctime(&curr) << ": " << message << std::endl;
        else
            log_stream << "WARNING at " << std::ctime(&curr) << ": line " << line << ": " << message << std::endl;
    }

    /**
     * @brief Logs an error message.
     * @param message The message to log.
     */
    void log_warning(std::string&& message, const size_t& line = 0)
    {
        if (verbosity < 2)
            return;
        std::time_t curr = get_curr_time();
        if (line == 0)
            log_stream << "WARNING at " << std::ctime(&curr) << ": " << message << std::endl;
        else
            log_stream << "WARNING at " << std::ctime(&curr) << ": line " << line << ": " << message << std::endl;
    }

    /**
     * @brief Logs an error message.
     * @param message The message to log.
     */
    void log_error(std::string&& message)
    {
        if (verbosity < 1)
            return;
        std::time_t curr = get_curr_time();
        log_stream << "ERROR at " << std::ctime(&curr) << ": " << message << std::endl;
    }
private:
    std::ofstream log_stream;
    size_t verbosity;

    std::time_t get_curr_time()
    {
        auto time = std::chrono::system_clock::now();
        return std::chrono::system_clock::to_time_t(time);
    }
};

#endif