#ifndef _PARSER_INTERFACE_HPP
#define _PARSER_INTERFACE_HPP

#include <memory>
#include "../node.hpp"

/**
 * @class AbstractParser
 * @brief Interface for parsers.
 */
class AbstractParser
{
public:
    virtual ~AbstractParser() = default;
    virtual std::unique_ptr<Node> parse_document(bool print_tree = false) = 0;
};

#endif