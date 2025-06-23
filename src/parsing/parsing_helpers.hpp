/**
 * @file parsing_helpers.hpp
 * @brief Contains useful functions and classes for Md_Parser
 */

#ifndef _PARSING_HELPERS_HPP
#define _PARSING_HELPERS_HPP

#include <memory>
#include <stack>
#include <set>
#include "../token.hpp"
#include "state.hpp"

/**
 * @brief A set of characters that can be considered escaped in Markdown.
 */
std::set<char> escaped_chars = {
    {'\\'}, {'`'}, {'*'}, {'_'}, {'{'}, {'}'}, {'['}, {']'}, {'<'}, {'>'}, {'('}, {')'}, {'#'},
    {'+'}, {'-'}, {'.'}, {'!'}, {'|'}
};

/**
 * @brief Checks if a character is an escaped character.
 * 
 * @param c The character to check.
 * @return true if the character is escaped, false otherwise.
 */
bool is_escaped_char(char c)
{
    auto it = escaped_chars.find(c);
    return it != escaped_chars.end();
}

/**
 * @class ReturnStateStack
 * @brief A stack for storing states that a Md_Parser instance will return to.
 */
class ReturnStateStack
{
public:
    /**
     * @brief Constructs a ReturnStateStack object.
     * 
     * @param logger Pointer to the Logger instance for error handling.
     */
    ReturnStateStack(Logger* logger) : logger(logger), return_stack(std::stack<State>()) {}

    /**
     * @brief Pushes a state onto the stack.
     * 
     * @param state The state to push onto the stack.
     * @throws std::runtime_error if the state is not a valid return state.
     * @see is_a_return_state
     */
    void push (State state)
    {
        if (!is_a_return_state(state))
        {
            logger->log_error("Pushing a state that is not a return state: " + std::to_string(state));
            throw std::runtime_error(state + " should not be in the stack");
        }
        return_stack.push(state);
    }

    /**
     * @brief Returns the top state from the stack without removing it.
     * If the stack is empty, it returns State::Data and logs a warning.
     */
    State top()
    {
        if (return_stack.size() == 0)
        {
            logger->log_warning("Topping an empty stack. Returning State::Data instead.");
            return State::Data;
        }
        return return_stack.top();
    }

    /**
     * @brief Returns the top state from the stack and removes it.
     * If the stack is empty, it returns State::Data and logs a warning.
     */
    State top_n_pop()
    {
        if (return_stack.size() == 0)
        {
            logger->log_warning("Popping an empty stack. Returning State::Data instead.");
            return State::Data;
        }
        State top = return_stack.top();
        return_stack.pop();
        return top;
    }
private:
    std::stack<State> return_stack;
    Logger* logger;
    std::vector<State> allowed_return_states = {
        State::Data, 
        State::UnorderedListPrep, 
        State::OrderedListPrep, 
        State::TableHeaderNames, 
        State::TableCellData
    };

    /**
     * @brief Not all states can be return states. Return states usually represent elements which can contain
     * other elements. This method makes sure only these allowed states are passed in.
     */
    bool is_a_return_state(const State& state)
    {
        for (auto&& allowed : allowed_return_states)
        {
            if (allowed == state)
                return true;
        }
        return false;
    }
};

#endif