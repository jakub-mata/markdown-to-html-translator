#ifndef _BUILDER_INTERFACE_HPP_
#define _BUILDER_INTERFACE_HPP_

#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include "../node.hpp"

/**
 * @class AbstractBuilder
 * @brief An abstract base class (interface) for building documents.
 */
class AbstractBuilder {
public:
    virtual ~AbstractBuilder() = default;
    /**
     * @brief Builds a document and writes it to the output stream.
     * 
     * @param output_stream The output stream for the document.
     * @param stylesheet_name The name of the stylesheet to link in the document.
     * @param root The root node of the document tree.
     */
    virtual void build_document(
        std::ofstream& output_stream,
        const std::string& stylesheet_name,
        std::unique_ptr<Node> root) = 0;
};

#endif