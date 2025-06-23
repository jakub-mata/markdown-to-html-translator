/**
 * @file main.cpp
 * @brief The entry point
 */

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "error_handler.hpp"
#include "argument_parser.hpp"
#include "./parsing/markdown_parser.hpp"
#include "./building/html_constructor.hpp"


int main(int argc, char** argv) {
    std::vector<std::string> args_v(argv+1, argv+argc);
    std::optional<Arguments> args = ArgumentParser::parse_arguments(args_v);
    if (!args.has_value()) {
        handle_error(ErrorType::IncorrectArgFormat);
        return 0;
    }
    
    if (args->input_file.empty()) {
        handle_error(ErrorType::MissingInput);
        return 0;
    }

    std::ifstream input_stream(args->input_file);
    if (input_stream.fail()) {
        handle_error(ErrorType::UnableToOpenInput);
        return 0;
    }

    std::ofstream output_stream(args->output_file);
    std::ofstream styles_stream(args->styles_file);
    if (output_stream.fail() || styles_stream.fail()) {
        handle_error(ErrorType::UnableToOpenOutput);
        return 0;
    }
    Logger logger = (args->log_verbosity == 0) ? Logger() : Logger(args->log_verbosity);
    Md_Parser parser(input_stream, &logger);
    try {
        logger.log_info("Starting parsing.");
        std::unique_ptr<Node> root = parser.parse_document();

        HTML_Builder html_builder(&logger);
        html_builder.set_css_builder(styles_stream);
        logger.log_info("Starting html building");
        html_builder.build_document(output_stream, args->styles_file, std::move(root));
        
        logger.log_info("HTML building has finished successfully");
        std::cout << "Your HTML document has been built successfully!" << std::endl;
    } catch (std::runtime_error& err) {
        std::cerr << "Error during document parsing / html construction: " << err.what() << std::endl;
        return 0;
    }
}
