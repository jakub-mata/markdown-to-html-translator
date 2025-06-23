/**
 * @file markdown_parser.hpp
 * @brief The "controller" part of markdown parsing.
 */

#ifndef _MARKDOWN_PARSER_HPP
#define _MARKDOWN_PARSER_HPP

#include <fstream>
#include "state.hpp"
#include "state_handlers.hpp"
#include "../error_handler.hpp"
#include "parser_interface.hpp"

/**
 * @class Md_Parser
 * @brief The main class for Markdown parsing.
 * 
 * The `Md_Parser` class is responsible for managing the parsing process. It reads the input
 * Markdown file, updates the `Context`, and delegates character processing to the appropriate
 * state handler. The emitted tokens are used to construct a parsing tree.
 * 
 * @details
 * - Implements the **State Pattern** through state handlers.
 * - Emits tokens to the `TreeBuilder` for constructing the parsing tree.
 * - Handles special cases like escape sequences and unexpected characters.
 * 
 * @see Context
 * @see TreeBuilder
 */
class Md_Parser : public AbstractParser
{
public:
    /**
     * @param md_stream A reference to an already established stream of the markdown document.parse_document
     * @param logger A pointer to the overarching Logger instance.
     */
    Md_Parser(std::ifstream& md_stream, Logger* logger)
    : md_stream(md_stream),
      context(logger),
      curr_line(1),
      logger(logger) {}

    /**
     * @brief The starting point for markdown parsing. The method reads the input document char by char, changes
     * its context and calls the corresponding state handler. This handler processes the char and changes
     * the context appropriately. The handlers can emit tokens to a tree builder which creates a parsing tree.
     * @param print_tree A bool for printing the constructed tree to the output (meant for debugging).
     * @return A unique pointer to the root of a parsing tree.
     */
    virtual std::unique_ptr<Node> parse_document(bool print_tree = false) override
    {
        EnumArithmetic enumOp;
        char next;
        int next_flag;
        reset_context();

        while (true)
        {
            int next_flag = md_stream.get();
            if (next_flag != -1)
                next = (char) next_flag;
            else
            {
                next = '\n';
                context.EOF_Reached = true;
            }
            if (next == '\n')
                ++curr_line;
            // Handle escaping
            if (context.is_escaped)
            {
                handle_escape_sequence(next);
                context.is_escaped = false;
                continue;
            }
            if (next == '\\' 
                && (context.state != State::CodeInline 
                && context.state != State::CodeBlock
                && context.state != State::DataBacktick
            ))
            {
                context.is_escaped = true;
                continue;
            }

            // Consume char
            auto&& [state_key, handler] = state_handlers[context.state];
            if (state_key != context.state)
            {
                logger->log_error("State key found in state_handlers and state in context do not match");
                throw std::runtime_error("State error");
            }
            handler(context, next);
            if (!context.warning_msg.empty())
            {
                logger->log_warning(context.warning_msg, curr_line);
                context.warning_msg.clear();
            }

            if (context.EOF_Reached) break;
            if (context.newline_counter != 0 && next != '\n') { context.newline_counter = 0; }
        }
        
        if (print_tree) { context.emitter->print_tree(); }
        return context.emitter->get_builder()->get_root();
    }

private:
    size_t curr_line;
    std::ifstream& md_stream;
    Context context;
    Logger* logger;

    void reset_context()
    {
        context.newline_counter = 0;
        context.counter = 0;
        context.indent_level = 0;
        context.alt_counter = 0;
        context.blockquote_in_list = false;
        context.src.clear();
        context.alt.clear();
        context.consumed.clear();
        context.EOF_Reached = false;
        context.is_escaped = false;
        context.state = State::Data;
        context.is_image = false;
    }

    void handle_escape_sequence(char next)
    {
        if (is_escaped_char(next))
            context.consumed += next;
        else if (next == '\n')
            context.handle_unexpected_newline("\\", EOF_Reached);
        else
            context.consumed += '\\' + next;
    }
};

#endif