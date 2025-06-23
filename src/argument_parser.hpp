#ifndef _ARGUMENT_PARSER_HPP
#define _ARGUMENT_PARSER_HPP

#include <string>
#include <sstream>
#include <optional>
#include <vector>

struct Arguments {
    std::string input_file;
    std::string output_file;
    std::string styles_file;
    bool print_tree;
    size_t log_verbosity = 0;
};

enum Arg_Types 
{
    InputFile,
    OutputFile,
    StylesFile,
    Logging
};

class ArgumentParser 
{
public:
    /** 
     * Supported arguments:
     * -i (the path to a input file in a markdown format)
     * -o (the path to the output HTML file made by the program)
     * -s (the path to the styles.css file created)
     * -v (the verbosity of logging, 1 for Errors only, 2 adds Warnings, 3 adds Info)
     * 
     * Arguments can be written separately from their value: -i *input-file*
     * or together: -i*input-file*
     * 
     * If an argument is passed multiple types, only the last one counts.
    */
    static std::optional<Arguments> parse_arguments(const std::vector<std::string>& args) 
    {
        bool arg_set = false;  // whether the previous argument expects a value from the current argument
        Arg_Types last_type = InputFile;
        Arguments parsed;
        
        for (auto&& next_arg : args)
        {
            if (arg_set)
            {
                set_parsed_arg(&parsed, next_arg, last_type);
                arg_set = false;
                continue;
            }

            if (next_arg.length() <= 1) 
                return std::nullopt;

            if (next_arg[0] != '-')
                return std::nullopt;
            
            switch (next_arg[1])
            {
            case 'i':
                last_type = InputFile;
                break;
            case 'o':
                last_type = OutputFile;
                break;
            case 's':
                last_type = StylesFile;
                break;
            case 'v':
                last_type = Logging;
                break;
            default:
                return std::nullopt;
            }

            if (next_arg.length() == 2) 
            {
                arg_set = true;
                continue;
            }

            std::string val = next_arg.substr(2);
            set_parsed_arg(&parsed, val, last_type);
            arg_set = false;
        }
        
        if (parsed.output_file.empty())
        {
            std::cout << "Output file not specified. Defaulting to output.html" << std::endl;
            parsed.output_file = "output.html";
        }
        if (parsed.styles_file.empty())
        {
            std::cout << "Styles file not specified. Defaulting to styles.css" << std::endl;
            parsed.styles_file = "styles.css";
        }
        return std::optional<Arguments>{parsed};
    }
private:
    static void set_parsed_arg(Arguments* parsed, const std::string& val, const Arg_Types& type)
    {
        switch (type)
        {
            case InputFile:
                (*parsed).input_file = val;
                break;
            case OutputFile:
                (*parsed).output_file = val;
                break;
            case StylesFile:
                (*parsed).styles_file = val;
                break;
            case Logging:
                try {
                (*parsed).log_verbosity = std::stoul(val);
                } catch (std::invalid_argument& err) {
                    // ignore
                }
                break;
        }
    }
};

#endif