#ifndef _STATE_HANDLERS_HPP
#define _STATE_HANDLERS_HPP
#define INDENTATION 4

#include "state.hpp"
#include "../node.hpp"
#include <string>
#include <set>
#include "parsing_helpers.hpp"
#include "../error_handler.hpp"
#include "emitting_middleware.hpp"
#include "../parsing_tree/tree_builder.hpp"
#include <functional>

/**
 * @struct Context
 * @brief A struct for containing all parsing information. Used by Md_Parser. It also
 * provides functions useful for state manipulation, which are used by state handler functions.
 */
struct Context {
    bool EOF_Reached;
    std::string consumed;
    short counter = 0;
    short alt_counter = 0;
    short indent_level = 0;
    size_t newline_counter = 0;
    std::string src;
    std::string alt;
    bool blockquote_in_list;
    bool is_escaped;
    bool is_image;
    State state;
    std::string warning_msg;
    std::unique_ptr<Token_Emitter> emitter;
    std::unique_ptr<ReturnStateStack> return_stack;

    /**
     * @brief Constructs a Context object.
     * @param logger Pointer to the Logger instance for error handling.
     */
    Context(Logger* logger)
    : emitter(std::make_unique<Token_Emitter>(std::make_unique<TreeBuilder>(logger), logger)),
      return_stack(std::make_unique<ReturnStateStack>(logger)),
      state(State::Data) {}

    /**
     * @brief Emits a token to the emitter.
     * @param type The type of the token to emit.
     * @param element The element associated with the token.
     */
    void emit_token(const TokenType& type, const ElementType& element)
    {
        std::string text;
        switch (type)
        {
        case TokenType::OpenToken:
            break;
        case TokenType::CloseToken:
            consumed.clear();
            break;
        case TokenType::ContentToken:
            text = consumed;
            consumed.clear();
            break;
        default:
            break;
        }
        emitter->emit_token(Token(type, element, text));
    }

    /**
     * @brief Emits a content token. It is a shortcut to emit_token with TokenType::ContentToken.
     */
    void emit_content_token()
    {
        if (consumed.empty()) { return; }
        if (emitter->fetch_current_element() == ElementType::DOCSTART)
        {
            emit_token(TokenType::OpenToken, ElementType::Paragraph);
            emit_token(TokenType::ContentToken, ElementType::Content);
        }
        else
        {
            emit_token(TokenType::ContentToken, ElementType::Content);
        }
    }

    /**
     * @brief Emits a token when a pipe character is found in a table.
     * @param to_emit The string to emit.
     * @param full Whether to emit the full string or just the new content.
     */
    void handle_pipe_in_table(std::string&& to_emit, bool full = false)
    {
        ElementType to_close = return_stack->top() == State::TableCellData
            ? ElementType::Table_Cell
            : Table_Head;
        consumed = full ? to_emit : to_emit + consumed;
        emit_token(TokenType::ContentToken, ElementType::Content);
        state = return_stack->top_n_pop();
        emit_token(TokenType::CloseToken, to_close);
        emit_token(TokenType::OpenToken, to_close);
    }

    /**
     * @brief Handles unexpected newlines in the parsing process.
     */
    void handle_unexpected_newline(std::string&& to_emit, bool EOF_Reached)
    {
        if (return_stack->top() == State::TableHeaderNames || return_stack->top() == State::TableCellData)
        {
            consumed += to_emit;
            emit_token(TokenType::ContentToken, ElementType::Content);
            emitter->handle_flag(ParseWarningFlags::TableFailed);
            state = return_stack->top_n_pop();
            return;
        }

        if (!to_emit.empty())
        {
            consumed += to_emit;
            emit_content_token();
        }

        ElementType currElement = emitter->fetch_current_element();
        if (currElement == ElementType::Paragraph && !EOF_Reached)
        {
            ++newline_counter;
        }
        else
        {
            newline_counter = 0;
            emit_token(TokenType::CloseToken, currElement);
        }
        counter = 0;
        state = return_stack->top_n_pop();
    }

    /**
     * @brief Sets up the list parsing context by resetting the counter and indent level.
     */
    void setup_list_parsing()
    {
        counter = 0;
        indent_level = 0;
    }

    /**
     * @brief Emits a closing token which moves the pointer up in the parsing tree.
     */
    void move_up_the_tree()
    {
        emit_token(CloseToken, emitter->fetch_current_element());
    }

    /**
     * @brief Checks if the consumed string contains only whitespace characters.
     * @return true if the consumed string contains only whitespace characters, false otherwise.
     */
    bool consumed_only_whitespace()
    {
        for (auto&& c : consumed)
        {
            if (c != ' ' && c != '\t')
                return false;
        }
        return true;
    }
};

/**
 * @brief Emits an image token.
 */
void emit_image(Context& context)
{
    context.emitter->emit_token(Token(TokenType::OpenToken, ElementType::ImageType, context.src, context.alt, context.consumed));
    context.emit_token(TokenType::CloseToken, ElementType::ImageType);
    context.src.clear();
    context.alt.clear();
    context.consumed.clear();
}

/**
 * @brief Emits an hyperlink token.
 */
void emit_hyperlink(Context& context)
{
    context.emitter->emit_token(Token(TokenType::OpenToken, ElementType::Hypertext, context.src, context.alt, context.consumed));
    context.emit_token(TokenType::CloseToken, ElementType::Hypertext);
    context.src.clear();
    context.alt.clear();
    context.consumed.clear();
}

/**
 * @namespace handlers
 * @brief This namespace contains static functions corresponding to each state. The functions 
 * are responsible for manipulating Context and emitting tokens. All handlers follow
 * a naming convention handle*state_name*. This is basically the State design pattern implemented
 * functionally.
 */
namespace handlers 
{   
    /**
     * @brief A function handler for State::Data.
     */
    static void handleData(Context& context, char next)
    {
        switch (next)
        {
        case '#':
            if (context.consumed.empty()) { 
                context.return_stack->push(State::Data); 
                context.counter++;
                context.state = State::DataHashtag;
            }
            else { context.consumed += next; }
            break;
        case '*':
            context.emit_content_token(); 
            context.return_stack->push(State::Data);
            context.state = State::DataAsterisk;
            break;
        case '-':
            if (context.consumed.empty())
            {
                ++context.counter;
                context.return_stack->push(State::Data);
                context.state = State::HorizontalLine;
            }
            else
            {
                context.consumed += next;
            }
            break;
        case '`':
            context.emit_content_token();
            context.return_stack->push(State::Data);
            context.state = DataBacktick;
            break;
        case '>':
            if (context.consumed.empty())
            {
                context.emit_token(TokenType::OpenToken, ElementType::Span);
                context.emitter->add_attribute(Attribute::BlockQuote);
                context.return_stack->push(State::Data);
            }
            else
            {
                context.consumed += next;
            }
            break;
        case '[':
            context.emit_content_token();
            context.return_stack->push(State::Data);
            context.is_image = false;
            context.state = State::AltOpenSquared;
            break;
        case '!':
            context.emit_content_token();
            context.state = Image;
            context.is_image = true;
            context.return_stack->push(State::Data);
            break;
        case '|':
            if (context.consumed_only_whitespace())
            {
                context.consumed.clear();
                context.state = TableHeaderNames;
                context.return_stack->push(State::Data);
                context.emit_token(TokenType::OpenToken, ElementType::Table);
                context.emit_token(TokenType::OpenToken, ElementType::Table_Row);
                context.emit_token(TokenType::OpenToken, ElementType::Table_Head);
            }
            else
                context.consumed += '|';
            break;
        case '\n':
        {
            ElementType curr_el = context.emitter->fetch_current_element();
            context.emit_content_token();

            if (context.newline_counter == 1 && curr_el == ElementType::Paragraph) 
            {
                context.emit_token(TokenType::CloseToken, ElementType::Paragraph);
            }
            else if (curr_el != ElementType::DOCSTART && curr_el != ElementType::Paragraph)
            {
                context.emit_token(TokenType::CloseToken, curr_el);
                if (context.blockquote_in_list)
                {
                    context.emit_token(TokenType::CloseToken, ElementType::List_Element);
                    context.blockquote_in_list = false;
                }
                context.state = context.return_stack->top_n_pop();
            }
            
            if (curr_el == ElementType::Paragraph)
                ++context.newline_counter;
            break;
        }
        default:
            if (context.consumed.empty() && isdigit(next)) {
                context.consumed += next;
                context.return_stack->push(State::Data);
                context.state = State::DataConsumingNumber; 
                break;
            }
            context.consumed += next;
            break;
        }
    }

    /**
     * @brief A function handler for State::DataHashtag.
     */
    static void handleHashtag(Context& context, char next)
    {
        if (next == '#' && context.counter < 6) { context.counter++; }
        else if (next == ' ') { 
            EnumArithmetic enumOp;
            context.emit_token(TokenType::OpenToken, enumOp(ElementType::Header_1, context.counter-1));
            context.emitter->add_attribute(Attribute::Bold);
            context.emitter->add_attribute(enumOp(Attribute::FontSize1, context.counter-1));
            context.counter = 0;
            context.return_stack->push(State::Data);
            context.state = State::Data;
        }
        else if (next == '\n') {
            for (short i = 0; i < context.counter; ++i) { context.consumed += '#'; }
            context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
        }
        else { 
            for (short i = 0; i < context.counter; ++i) { context.consumed += '#'; }
            context.state = context.return_stack->top_n_pop();
        }
    }

    /**
     * @brief A function handler for State::DataAsterisk.
     */
    static void handleDataAsterisk(Context& context, char next) 
    {
        if (next == '*') 
            context.state = State::DataDoubleAsterisk; 
        else if (next == '\n') {
            context.warning_msg = "Unclosed asterisk signifying bold text - converting '*' to plain text";
            context.handle_unexpected_newline("*", context.EOF_Reached);
        }
        else if (next == '|') {
            if (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData)
            {
                context.warning_msg = "Unclosed asterisk signifying bold text - converting '*' to plain text";
                context.handle_pipe_in_table("*");
                return;
            }
            context.consumed += next;
            context.state = State::DataAsteriskData;
        }
        else {
            context.consumed += next;
            context.state = State::DataAsteriskData;
        }
    }
    /**
     * @brief A function handler for State::DataAsteriskData.
     */
    static void handleAsteriskData(Context& context, char next) 
    {
        switch (next)
        {
        case '*':
            context.emit_token(TokenType::OpenToken, ElementType::Span);
            context.emitter->add_attribute(Attribute::Italic);
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.emit_token(TokenType::CloseToken, ElementType::Span);
            context.state = context.return_stack->top_n_pop();
            break;
        case '\n':
            context.warning_msg = "Unclosed asterisk signifying bold text - converting to plain text";
            context.handle_unexpected_newline('*' + context.consumed, context.EOF_Reached);
            break;
        case '|':
            if (context.return_stack->top() == TableCellData || context.return_stack->top() == TableHeaderNames) {
                context.warning_msg = "Unclosed asterisk signifying bold text - converting to plain text";
                context.handle_pipe_in_table("*");
                break;
            }
        default:
            context.consumed += next;
        }
    }

    /**
     * @brief A function handler for State::DataDoubleAsterisk.
     */
    static void handleDoubleAsterisk(Context& context, char next) 
    {
        switch (next)
        {
            case '*':
                context.state = DataTripleAsterisk;
                break;
            case '\n':
                context.warning_msg = "Unclosed asterisk signifying bold text - converting '**' to plain text";
                context.handle_unexpected_newline("**", context.EOF_Reached);
                break;
            case '|':
                if (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData) {
                    context.warning_msg = "Unclosed asterisk signifying bold text - converting '**' to plain text";
                    context.handle_pipe_in_table("**");
                    break;
                }
            default:
                context.consumed += next;
                context.state = DataDoubleAsteriskData;
                break;
        }
    }

    /**
     * @brief A function handler for State::DataDoubleAsteriskData.
     */
    static void handleDataDoubleAsteriskData(Context& context, char next) 
    {
        switch (next)
        {
            case '*':
                context.counter++;
                if (context.counter == 2)
                {
                    context.counter = 0;
                    context.emit_token(TokenType::OpenToken, ElementType::Span);
                    context.emitter->add_attribute(Attribute::Bold);
                    context.emit_token(TokenType::ContentToken, ElementType::Content);
                    context.emit_token(TokenType::CloseToken, ElementType::Span);
                    context.state = context.return_stack->top_n_pop();
                }
                break;
            case '\n':
                context.warning_msg = "Unclosed asterisk signifying bold text - converting to plain text";
                context.consumed = "**" + context.consumed;
                if (context.counter == 1) context.consumed += '*';
                context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
                break;
            case '|':
                if (context.return_stack->top() == TableCellData || context.return_stack->top() == TableHeaderNames) {
                    context.warning_msg = "Unclosed asterisk signifying bold text - converting to plain text";
                    context.handle_pipe_in_table("**");
                    break;
                }
            default:
                if (context.counter != 0) context.counter = 0;
                context.consumed += next;
                break;
        }
    }

    /**
     * @brief A function handler for State::DataTripleAsterisk.
     */
    static void handleDataTripleAsterisk(Context& context, char next) 
    {
        switch (next)
        {
        case '*':
            context.consumed += "****";
            context.state = context.return_stack->top_n_pop();
            break;
        case '\n':
            context.warning_msg = "Unclosed asterisk signifying bold text - converting '***' to plain text";
            context.handle_unexpected_newline("***", context.EOF_Reached);
            break;
        case '|':
            if (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData) {
                context.warning_msg = "Unclosed asterisk signifying bold text - converting '***' to plain text";
                context.handle_pipe_in_table("***");
                break;
            }
        default:
            context.consumed += next;
            context.state = DataTripleAsteriskData;
            break;
        }
    }

    /**
     * @brief A function handler for State::DataTripleAsteriskData.
     */
    static void handleDataTripleAsteriskData(Context& context, char next) 
    {
        switch (next)
        {
            case '*':
                context.counter++;
                if (context.counter == 3)
                {
                    context.counter = 0;
                    context.emit_token(TokenType::OpenToken, ElementType::Span);
                    context.emitter->add_attribute(Attribute::Bold);
                    context.emitter->add_attribute(Attribute::Italic);
                    context.emit_token(TokenType::ContentToken, ElementType::Content);
                    context.emit_token(TokenType::CloseToken, ElementType::Span);
                    context.state = context.return_stack->top_n_pop();
                }
                break;
            case '\n':
                context.warning_msg = "Unclosed asterisk signifying bold text - converting to plain text";
                context.consumed = "***" + context.consumed;
                for (short i = 0; i < context.counter; ++i) {context.consumed += '*';}
                context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
                break;
            case '|':
                if (context.return_stack->top() == TableCellData || context.return_stack->top() == TableHeaderNames) {
                    context.warning_msg = "Unclosed asterisk signifying bold text - converting to plain text";
                    context.handle_pipe_in_table("***");
                    break;
                }
            default:
                if (context.counter != 0) context.counter = 0;
                context.consumed += next;
                break;
        }
    }

    /**
     * @brief A function handler for State::DataConsumingNumber.
     */
    static void handleDataConsumingNumber(Context& context, char next) 
    {
        switch (next)
        {
        case '.':
            context.consumed += '.';
            context.state = State::DataOrdinalNumber;
            break;
        case '\n':
            context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
            break;
        default:
            context.consumed += next;
            context.state = context.return_stack->top_n_pop();
            break;
        }
    }

    /**
     * @brief A function handler for State::DataOrdinalNumber.
     */
    static void handleDataOrdinalNumber(Context& context, char next) 
    {
        switch (next)
        {
        case ' ':
            context.consumed.clear();

            if (context.emitter->fetch_current_element() == ElementType::List_Ordered || context.emitter->fetch_current_element() == ElementType::List_Unordered)
            {
                if (context.counter >= context.indent_level + INDENTATION)
                {
                    context.indent_level = context.counter;
                    context.counter = 0;
                    context.emit_token(TokenType::OpenToken, ElementType::List_Ordered);
                    context.emit_token(TokenType::OpenToken, ElementType::List_Element);
                    if (context.blockquote_in_list)
                    {
                        context.emit_token(TokenType::OpenToken, ElementType::Span);
                        context.emitter->add_attribute(Attribute::BlockQuote);
                    }
                    context.return_stack->push(State::OrderedListPrep);
                    context.state = State::Data;
                    break;
                }
                if (context.counter % INDENTATION != 0) 
                    --context.counter;
                short curr_indent = (context.indent_level - context.counter) / INDENTATION;
                context.indent_level = context.counter;
                context.counter = 0;

                for (size_t i = 0; i < curr_indent; ++i)
                    context.move_up_the_tree();

                context.return_stack->push(State::OrderedListPrep);
                context.emit_token(TokenType::OpenToken, ElementType::List_Element);
                if (context.blockquote_in_list)
                {
                    context.emit_token(TokenType::OpenToken, ElementType::Span);
                    context.emitter->add_attribute(Attribute::BlockQuote);
                }
                context.state = State::Data;
            }
            else
            {
                context.emit_token(TokenType::OpenToken, ElementType::List_Ordered);
                context.emit_token(TokenType::OpenToken, ElementType::List_Element);
                context.return_stack->push(State::OrderedListPrep);
                context.state = State::Data;
            }
            
            break;
        case '\n':
            context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
            break;
        default:
            context.consumed += next;
            context.state = context.return_stack->top_n_pop();
            break;
        }
    }

    /**
     * @brief A function handler for State::HorizontalLine.
     */
    static void handleHorizontalLine(Context& context, char next) 
    {
        switch (next)
        {
        case '-':
            ++context.counter;
            break;
        case '\n':
            if (context.counter >= 3)
            {
                context.emit_token(TokenType::OpenToken, ElementType::Horizontalline);
                context.emit_token(TokenType::CloseToken, ElementType::Horizontalline);
                context.state = context.return_stack->top_n_pop();
                context.counter = 0;
                break;
            }
            for (short i = 0; i < context.counter; ++i) { context.consumed += '-';}
            context.counter = 0;
            context.emit_content_token();
            context.state = context.return_stack->top_n_pop();
            break;
        default:
            if ((next == ' ' || next == '\t') && context.counter == 1)
            {
                context.emit_token(TokenType::OpenToken, ElementType::List_Unordered);
                context.emit_token(TokenType::OpenToken, ElementType::List_Element);
                context.state = State::Data;
                context.return_stack->push(State::UnorderedListPrep);
                context.setup_list_parsing();
                break;
            }

            for (short i = 0; i < context.counter; ++i) { context.consumed += '-';}
            context.counter = 0;
            context.consumed += next;
            context.emit_content_token();
            context.state = context.return_stack->top_n_pop();
            break;
        }
    }

    /**
     * @brief A function handler for State::DataBacktick.
     */
    static void handleDataBacktick(Context& context, char next) 
    {
        switch (next)
        {
        case '`':
            if (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData)
            {
                context.consumed = "``";
                context.emit_token(TokenType::ContentToken, ElementType::Content);
                context.state = context.return_stack->top_n_pop();
                break;
            }
            context.state = DataDoubleBacktick;
            break;
        case '\n':
            context.warning_msg = "Unclosed backtick signifying a code element - handling as plain text";
            context.handle_unexpected_newline("`", context.EOF_Reached);
            break;
        case '|':
            if (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData) {
                context.warning_msg = "Unclosed backtick signifying a code element - handling as plain text";
                context.handle_pipe_in_table("`");
                break;
            }
        default:
            context.consumed += next;
            context.state = State::CodeInline;
            break;
        }
    }
    /**
     * @brief A function handler for State::DataDoubleBacktick.
     */
    static void handleDataDoubleBacktick(Context& context, char next) 
    {
        switch (next)
        {
        case '`':
            context.state = State::CodeBlock;
            break;
        case '\n':
            context.handle_unexpected_newline("``", context.EOF_Reached);
            break;
        default:
            context.consumed = "``";
            context.emit_content_token();
            context.state = context.return_stack->top_n_pop();
            break;
        }
    }

    /**
     * @brief A function handler for State::CodeInline.
     */
    static void handleCodeInlineState(Context& context, char next) 
    {
        switch (next)
        {
        case '`':
            context.emit_token(TokenType::OpenToken, ElementType::Codeblock);
            context.emitter->add_attribute(Attribute::Inline);
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.emit_token(TokenType::CloseToken, ElementType::Codeblock);
            context.state = context.return_stack->top_n_pop();
            break;
        case '\n':
            context.warning_msg = "Unclosed backtick signifying a code element - handling as plain text";
            context.handle_unexpected_newline('`' + context.consumed, context.EOF_Reached);
            break;
        case '|':
            if (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData)
            {
                context.warning_msg = "Unclosed backtick signifying a code element - handling as plain text";
                context.handle_pipe_in_table("`");
                break;
            }
        default:
            context.consumed += next;
            break;
        }
    }

    /**
     * @brief A function handler for State::CodeBlock.
     */
    static void handleCodeBlock(Context& context, char next) 
    {
        switch (next)
        {
        case '`':
            ++context.counter;
            if (context.counter == 3) {
                context.emit_token(TokenType::OpenToken, ElementType::Codeblock);
                context.emitter->add_attribute(Attribute::Block);
                context.emit_token(TokenType::ContentToken, ElementType::Content);
                context.emit_token(TokenType::CloseToken, ElementType::Codeblock);
                context.counter = 0;
                context.state = context.return_stack->top_n_pop();
            }
            break;
        case '\n':
            // Newline should be ignored, since codeblocks can span multiple lines
        default:
            if (context.counter != 0) { 
                for (short i=0; i<context.counter; ++i) {context.consumed += '`';}
                context.counter = 0; 
            }
            context.consumed += next;
            break;
        }
    }

    /**
     * @brief A function handler for State::UnorderedListPrep.
     */
    static void handleUnorderedListPrep(Context& context, char next) 
    {
        switch (next)
        {
            case '\n':
                context.state = context.return_stack->top_n_pop();
                for (short i = 0; i <= context.indent_level; i = i + INDENTATION)
                    context.move_up_the_tree();
                context.setup_list_parsing();
                break;
            case ' ':
                ++context.counter;
                break;
            case '\t':
                context.counter += INDENTATION;
                break;
            case '*':
            case '+':
            case '>':
            case '-':
                if (context.counter > context.indent_level + INDENTATION)
                {
                    for (short i = 0; i <= context.indent_level; i = i + INDENTATION)
                        context.move_up_the_tree();
                    context.state = context.return_stack->top_n_pop();
                    context.setup_list_parsing();
                    context.consumed = next;
                    break;
                }
                if (next == '>')
                    context.blockquote_in_list = true;
                context.state = UnorderedList;
                break;
            default:
                context.consumed += next;
                if (std::isdigit(next))
                {
                    context.state = State::DataConsumingNumber;
                    break;
                }
                for (short i = 0; i <= context.indent_level; i = i + INDENTATION)
                    context.move_up_the_tree();
                
                context.state = context.return_stack->top_n_pop();
                context.setup_list_parsing();
                break;
        }
    }

    /**
     * @brief A function handler for State::UnorderedList.
     */
    static void handleUnorderedList(Context& context, char next) 
    {
        switch (next)
        {
        case '\t':
        case ' ':
        {
            if (context.counter >= context.indent_level + INDENTATION)
            {
                context.indent_level = context.counter;
                context.counter = 0;
                context.emit_token(TokenType::OpenToken, ElementType::List_Unordered);
                context.emit_token(TokenType::OpenToken, ElementType::List_Element);
                context.state = State::Data;
                if (context.blockquote_in_list)
                {
                    context.emit_token(TokenType::OpenToken, ElementType::Span);
                    context.emitter->add_attribute(Attribute::BlockQuote);
                }
                context.return_stack->push(UnorderedListPrep);
                break;
            }
            if (context.counter % INDENTATION != 0) --context.counter;
            short curr_indent = (context.indent_level - context.counter) / INDENTATION;
            context.indent_level = context.counter;
            context.counter = 0;
            
            for (size_t i = 0; i < curr_indent; ++i)
                context.move_up_the_tree();

            context.state = State::Data;
            context.emit_token(TokenType::OpenToken, ElementType::List_Element);
            if (context.blockquote_in_list)
            {
                context.emit_token(TokenType::OpenToken, ElementType::Span);
                context.emitter->add_attribute(Attribute::BlockQuote);
            }
            context.return_stack->push(UnorderedListPrep);
            break;
        }
        default:
            for (short i = 0; i < context.indent_level; i = i + INDENTATION)
                context.move_up_the_tree();
            context.consumed = '-' + next;
            context.counter = 0;
            context.state = context.return_stack->top_n_pop();
            context.indent_level = 0;
            break;
        }
    }

    /**
     * @brief A function handler for State::OrderedListPrep.
     */
    static void handleOrderedListPrep(Context& context, char next) 
    {
        switch (next)
        {
        case '\n':
            context.state = context.return_stack->top_n_pop();
            for (short i = 0; i <= context.indent_level; i = i + INDENTATION)
                context.move_up_the_tree();
            context.setup_list_parsing();
            break;
        case '\t':
            context.counter += INDENTATION;
            break;
        case ' ':
            ++context.counter;
            break;
        case '+':
        case '*':
        case '-':
        case '>':
            if (context.counter > context.indent_level + INDENTATION)
            {
                for (short i = 0; i <= context.indent_level; i = i + INDENTATION)
                    context.move_up_the_tree();
                context.state = context.return_stack->top_n_pop();
                context.setup_list_parsing();
                context.consumed = next;
                break;
            }
            if (next == '>')
                context.blockquote_in_list = true;
            context.state = UnorderedList;
            break;
        default:
            if (std::isdigit(next))
            {
                context.consumed += next;
                context.state = State::DataConsumingNumber;
                break;
            }
            
            for (short i = 0; i <= context.indent_level; i = i + INDENTATION)
                context.move_up_the_tree();
            context.consumed += next;
            context.state = context.return_stack->top_n_pop();
            context.setup_list_parsing();
            break;
        }
    }

    /**
     * @brief A function handler for State::Image.
     */
    static void handleImage(Context& context, char next) 
    {
        switch (next)
        {
        case '[':
            context.state = AltOpenSquared;
            break;
        case '\n':
            context.handle_unexpected_newline("!", context.EOF_Reached);
            break;
        default:
            context.consumed = '!' + next;
            context.state = context.return_stack->top_n_pop();
            break;
        }
    }

    /**
     * @brief A function handler for State::AltOpenSquared.
     */
    static void handleAltOpenSquared(Context& context, char next) 
    {
        switch (next)
        {
        case ']':
            context.state = AltClosedSquared;
            break;
        case '\n':
            context.consumed = "[" + context.alt;
            if (context.is_image)
                context.consumed = '!' + context.consumed;
            context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
            context.alt.clear();
            break;
        case '|':
            if (!context.is_image && (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData)) {
                context.handle_pipe_in_table('['+std::move(context.alt), true);
            }
        default:
            context.alt += next;
            break;
        }
    }

    /**
     * @brief A function handler for State::AltClosedSquared.
     */
    static void handleAltClosedSquared(Context& context, char next) 
    {
        switch (next)
        {
        case '(':
            context.state = UrlOpenRound;
            break;
        case '\n':
            context.consumed = "[" + context.alt + "]" + context.consumed;
            if (context.is_image)
                context.consumed = '!' + context.consumed;
            context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
            context.alt.clear();
            break;
        case '|':
            if (!context.is_image && (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData)) {
                context.handle_pipe_in_table('['+std::move(context.alt)+']', true);
                break;
            }
        default:
            context.consumed = "[" + context.alt + "]" + next;
            if (context.is_image)
                context.consumed = '!' + context.consumed;
            context.state = context.return_stack->top_n_pop();
            context.alt.clear();
            break;
        }
    }

    /**
     * @brief A function handler for State::UrlOpenRound.
     */
    static void handleUrlOpenRound(Context& context, char next) 
    {
        switch (next)
        {
        case ')':
            if (context.is_image) 
                emit_image(context);
            else 
                emit_hyperlink(context);

            context.state = context.return_stack->top_n_pop();
            break;
        case ' ':
            context.state = TitleOpenRound;
            break;
        case '\n':
            context.consumed = "[" + context.alt + "](" + context.consumed;
            if (context.is_image)
                context.consumed = '!' + context.consumed;
            context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
            context.alt.clear();
            context.src.clear();
            break;
        case '|':
            if (!context.is_image && (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData)) {
                context.handle_pipe_in_table('['+context.alt+"[(");
                break;
            }
        default:
            context.src += next;
            break;
        }
    }

    /**
     * @brief A function handler for State::TitleOpenRound.
     */
    static void handleTitleOpenRound(Context& context, char next) 
    {
        switch (next)
        {
        case '"':
            context.state = TitleConsuming;
            break;
        case '\n':
            context.consumed = "[" + context.alt + "](" + context.src + ' ';
            if (context.is_image)
                context.consumed = '!' + context.consumed;
            context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
            context.alt.clear();
            context.src.clear();
            break;
        case '|':
            if (!context.is_image && (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData)) {
                context.handle_pipe_in_table("[" + context.alt + "](" + context.src + ' ', true);
                break;
            }
        default:
            context.consumed = "[" + context.alt + "](" + context.src + ' ' + next;
            if (context.is_image)
                context.consumed = '!' + context.consumed;
            context.alt.clear();
            context.src.clear();
            context.state = context.return_stack->top_n_pop();
            break;
        }
    }

    /**
     * @brief A function handler for State::TitleConsuming.
     */
    static void handleTitleConsuming(Context& context, char next) 
    {
        switch (next)
        {
            case '"':
                context.state = TitleClosedRound;
                break;
            case '\n':
                context.consumed = "![" + context.alt + "](" + context.src + " \"" + context.consumed;
                if (context.is_image)
                    context.consumed = '!' + context.consumed;
                context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
                context.alt.clear();
                context.src.clear();
                break;
            case '|':
                if (!context.is_image && (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData)) {
                    context.handle_pipe_in_table("[" + context.alt + "](" + context.src + "\"");   
                    break;
                }                 
            default:
                context.consumed += next;
                break;
        }
    }

    /**
     * @brief A function handler for State::TitleClosedRound.
     */
    static void handleTitleClosedRound(Context& context, char next) 
    {
        switch (next)
        {
        case ')':
            if (context.is_image) 
                emit_image(context);
            else 
                emit_hyperlink(context);
            
            context.state = context.return_stack->top_n_pop();
            break;
        case '\n':
            context.consumed = "[" + context.alt + "](" + context.src + " \"" + context.consumed + '"';
            if (context.is_image)
                context.consumed = '!' + context.consumed;
            context.handle_unexpected_newline(std::move(context.consumed), context.EOF_Reached);
            context.alt.clear();
            context.src.clear();
            break;
        case '|':
            if (!context.is_image && (context.return_stack->top() == TableHeaderNames || context.return_stack->top() == TableCellData)) {
                context.handle_pipe_in_table("[" + context.alt + "](" + context.src + " \"" + context.consumed + '"');
                break;
            }
        default:
            context.consumed = "![" + context.alt + "](" + context.src + " \"" + context.consumed + next;
            context.alt.clear();
            context.src.clear();
            context.state = context.return_stack->top_n_pop();
            break;
        }
    }

    /**
     * @brief A function handler for State::TableHeaderNames.
     */
    static void handleTableHeaderNames(Context& context, char next) 
    {
        switch (next)
        {
        case '\n':
            if (context.consumed_only_whitespace())
            {
                context.emit_token(TokenType::CloseToken, ElementType::Table_Head);
                context.emit_token(TokenType::CloseToken, ElementType::Table_Row);
                context.state = State::TableHeaderSeparationPipeAwaiting;
                context.counter = 0;
            }
            else
            {
                context.emit_token(TokenType::ContentToken, ElementType::Content);
                context.emitter->handle_flag(ParseWarningFlags::TableFailed);
                context.state = context.return_stack->top_n_pop();
            }
            break;
        case '|':
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.emit_token(TokenType::CloseToken, ElementType::Table_Head);
            context.emit_token(TokenType::OpenToken, ElementType::Table_Head);
            break;
        case '*':
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.state = State::DataAsterisk;
            context.return_stack->push(State::TableHeaderNames);
            break;
        case '`':
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.state = State::DataBacktick;
            context.return_stack->push(State::TableHeaderNames);
            break;
        case '[':
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.state = State::AltOpenSquared;
            context.return_stack->push(State::TableHeaderNames);
            break;
        default:
            context.consumed += next;
            break;
        }
    }

    /**
     * @brief A function handler for State::TableHeaderSeparationPipeAwaiting.
     */
    static void handleTableHeaderSeparationPipeAwaiting(Context& context, char next) 
    {
        switch (next)
        {
        case '\n':
            context.emitter->handle_flag(ParseWarningFlags::TableFailed);
            context.emit_content_token();
            context.state = context.return_stack->top_n_pop();
            break;
        case ' ':
        case '\t':
            context.consumed += next;
            break;
        case '|':
            context.consumed += '|';
            context.counter = 0;
            context.alt_counter = 0;
            context.state = TableHeaderSeparation;
            break;
        default:
            context.consumed += next;
            context.emitter->handle_flag(ParseWarningFlags::TableFailed);
            context.emit_content_token();
            context.state = context.return_stack->top_n_pop();
            break;
        }
    }

    /**
     * @brief A function handler for State::TableHeaderSeparation.
     */
    static void handleTableHeaderSeparation(Context& context, char next) 
    {
        switch (next)
        {
        case '\n':
            if (context.emitter->get_col_dims() == context.alt_counter)
            {
                context.alt_counter = 0;
                context.consumed.clear();
                context.counter = 0;
                context.state = State::TableCellPipeAwaiting;
                context.emit_token(TokenType::OpenToken, ElementType::Table_Row);
                if (context.EOF_Reached)
                    context.emitter->handle_flag(ParseWarningFlags::TableFailed);
                break;
            }
            context.alt_counter = 0;
            context.counter = 0;
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.emitter->handle_flag(ParseWarningFlags::TableFailed);
            context.state = context.return_stack->top_n_pop();
            break;
        case '|':
            ++context.alt_counter;
            context.consumed += '|';
            if (context.counter < 3)
            {
                context.emitter->handle_flag(ParseWarningFlags::TableFailed);
                context.emit_content_token();
                context.state = context.return_stack->top_n_pop();
                break;
            }
            context.counter = 0;
            break;
        case ' ':
        case '\t':
            context.consumed += next;
            break;
        case '-':
            ++context.counter;
            context.consumed += '-';
            break;
        default:
            context.consumed += next;
            context.emitter->handle_flag(ParseWarningFlags::TableFailed);
            context.emit_content_token();
            context.state = context.return_stack->top_n_pop();
            break;
        }
    }

    /**
     * @brief A function handler for State::TableCellPipeAwaiting.
     */
    static void handleTableCellPipeAwaiting(Context& context, char next) 
    {
        switch (next)
        {
        case '\n':
            context.emitter->handle_flag(ParseWarningFlags::TableFailed);
            context.state = context.return_stack->top_n_pop();
            break;
        case ' ':
        case '\t':
            break;
        case '|':
            context.emit_token(TokenType::OpenToken, ElementType::Table_Cell);
            context.state = TableCellData;
            break;
        default:
            context.emitter->handle_flag(ParseWarningFlags::TableFailed);   
            context.state = context.return_stack->top_n_pop();
            context.consumed += next;
            break;
        }
    }

    /**
     * @brief A function handler for State::TableCellData.
     */
    static void handleTableCellData(Context& context, char next) 
    {
        switch (next)
        {
        case '\n':
            if (!context.consumed_only_whitespace())
            {
                context.emit_token(TokenType::ContentToken, ElementType::Content);
                context.emitter->handle_flag(ParseWarningFlags::TableFailed);
                context.state = context.return_stack->top_n_pop();
                break;
            }
            context.emit_token(TokenType::CloseToken, ElementType::Table_Cell);
            context.emit_token(TokenType::CloseToken, ElementType::Table_Row);
            context.emit_token(TokenType::OpenToken, ElementType::Table_Row);
            if (context.EOF_Reached)
                context.emitter->handle_flag(ParseWarningFlags::TableFailed);
            context.state = TableCellPipeAwaiting;
            context.consumed.clear();
            break;
        case '|':
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.emit_token(TokenType::CloseToken, ElementType::Table_Cell);
            context.emit_token(TokenType::OpenToken, ElementType::Table_Cell);
            break;
        case '*':
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.state = State::DataAsterisk;
            context.return_stack->push(State::TableCellData);
            break;
        case '`':
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.state = State::DataBacktick;
            context.return_stack->push(State::TableCellData);
            break;
        case '[':
            context.emit_token(TokenType::ContentToken, ElementType::Content);
            context.state = State::AltOpenSquared;
            context.return_stack->push(State::TableCellData);
            break;
        default:
            context.consumed += next;
            break;
        }
    }
}

/**
 * @brief The linkage connecting states to their handlers. For better efficiency, it is implemented
 * with a vector, not a map. This works IFF we follow the same order of states as in the State enum,
 * since the value of State is used as an index during lookup.
 */
std::vector<std::pair<State, std::function<void(Context&, char)>>> state_handlers = {
    {State::Data, handlers::handleData},
    {State::DataHashtag, handlers::handleHashtag},
    {State::DataAsterisk, handlers::handleDataAsterisk},
    {State::DataAsteriskData, handlers::handleAsteriskData},
    {State::DataDoubleAsterisk, handlers::handleDoubleAsterisk},
    {State::DataDoubleAsteriskData, handlers::handleDataDoubleAsteriskData},
    {State::DataTripleAsterisk, handlers::handleDataTripleAsterisk},
    {State::DataTripleAsteriskData, handlers::handleDataTripleAsteriskData},
    {State::DataConsumingNumber, handlers::handleDataConsumingNumber},
    {State::DataOrdinalNumber, handlers::handleDataOrdinalNumber},
    {State::HorizontalLine, handlers::handleHorizontalLine},
    {State::DataBacktick, handlers::handleDataBacktick},
    {State::DataDoubleBacktick, handlers::handleDataDoubleBacktick},
    {State::CodeInline, handlers::handleCodeInlineState},
    {State::CodeBlock, handlers::handleCodeBlock},
    {State::UnorderedListPrep, handlers::handleUnorderedListPrep},
    {State::UnorderedList, handlers::handleUnorderedList},
    {State::OrderedListPrep, handlers::handleOrderedListPrep},
    {State::Image, handlers::handleImage},
    {State::AltOpenSquared, handlers::handleAltOpenSquared},
    {State::AltClosedSquared, handlers::handleAltClosedSquared},
    {State::UrlOpenRound, handlers::handleUrlOpenRound},
    {State::TitleOpenRound, handlers::handleTitleOpenRound},
    {State::TitleConsuming, handlers::handleTitleConsuming},
    {State::TitleClosedRound, handlers::handleTitleClosedRound},
    {State::TableHeaderNames, handlers::handleTableHeaderNames},
    {State::TableHeaderSeparationPipeAwaiting, handlers::handleTableHeaderSeparationPipeAwaiting},
    {State::TableHeaderSeparation, handlers::handleTableHeaderSeparation},
    {State::TableCellPipeAwaiting, handlers::handleTableCellPipeAwaiting},
    {State::TableCellData, handlers::handleTableCellData}
};

#endif