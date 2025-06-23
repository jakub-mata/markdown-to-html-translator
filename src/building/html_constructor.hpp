#ifndef _HTML_CONSTRUCTOR_HPP
#define _HTML_CONSTRUCTOR_HPP

#define ELEMENT_INDENTATION 4

#include <fstream>
#include <iostream>
#include <memory>
#include "../node.hpp"
#include "css_constructor.hpp"
#include "html_visitor.hpp"
#include "../error_handler.hpp"
#include "builder_interface.hpp"

/**
 * @class HTML_Builder
 * @brief A class for building an HTML + CSS document, given a parsing tree.
 * 
 * This class is responsible for constructing an HTML document and linking it to a CSS file.
 * It uses the Visitor design pattern to traverse the parsing tree and generate HTML content.
 * The `CSS_Constructor` class is used to generate the associated CSS file.
 * 
 * @details
 * - The `HTML_Builder` works closely with `CSS_Constructor` to ensure that the generated HTML
 *   document is properly styled. The `CSS_Constructor` is responsible for creating the CSS file
 *   and defining default styles for the document.
 * - The `HTML_Builder` also uses the `HTML_Visitor` class to traverse the parsing tree and
 *   generate HTML elements.
 * 
 * @see CSS_Constructor
 * @see HTML_Visitor
 */
class HTML_Builder : public AbstractBuilder
{
public:
    /**
     * @brief Constructs an HTML_Builder instance.
     * 
     * @param logger A pointer to the Logger instance for logging errors or warnings.
     */
    HTML_Builder(Logger* logger)
    : prev_token_indent(0),
      prev_token_content(false) {}

    /**
     * @brief The main method for building an HTML document.
     * 
     * This method is called after the parsing tree is built. It traverses the tree using the
     * Visitor design pattern (via `HTML_Visitor`) and generates the HTML content. It also
     * initializes the `CSS_Constructor` to create the associated CSS file.
     * 
     * @param output_stream The output stream for the HTML file.
     * @param styles_stream The output stream for the CSS file.
     * @param stylesheet_name The name of the CSS file to link in the HTML document.
     * @param root The root node of the parsing tree.
     * 
     * @throws std::runtime_error If the document does not start with a DOCTYPE element.
     * 
     * @see CSS_Constructor
     * @see HTML_Visitor
     */
    virtual void build_document(
        std::ofstream& output_stream,
        const std::string& stylesheet_name,
        std::unique_ptr<Node> root) override
    {
        if (root->element != ElementType::DOCSTART) {
            logger->log_error("Document is not starting with DOCTYPE. This is an error on our side.");
            throw std::runtime_error("doc not starting with DOCTYPE");
        }
        css_builder->create_default_styling();
        
        output_stream << '<' << element_to_html_name[ElementType::DOCSTART] << '>' << std::endl;

        setup_html_meta_tags(output_stream, stylesheet_name);
        output_stream << "<body>" << std::endl;
        
        HTML_Visitor visitor(output_stream, css_builder.get(), ELEMENT_INDENTATION);
        for (auto&& child : root->children)
        {
            child->accept(visitor, 0); 
        }
        output_stream << std::endl << std::endl << "</body>" << std::endl;
    }

    /**
     * @brief Sets the CSS builder for the HTML_Builder.
     * 
     * This method initializes the `CSS_Constructor` instance that will be used to generate
     * the CSS file. It should be called before building the document.
     * 
     * @param styles_stream The output stream for the CSS file.
     */
    void set_css_builder(std::ofstream& styles_stream)
    {
        this->css_builder = std::make_unique<CSS_Constructor>(styles_stream);
    }

private:
    std::unique_ptr<CSS_Constructor> css_builder; /**< Pointer to the CSS_Constructor instance for generating CSS. */
    bool prev_token_content; /**< Tracks whether the previous token was content. */
    size_t prev_token_indent; /**< Tracks the indentation level of the previous token. */
    Logger* logger; /**< Pointer to the Logger instance for logging. */

    /**
     * @brief Fills the output stream with spaces for indentation.
     * 
     * @param stream The output stream to write to.
     * @param indent The number of spaces to write.
     */
    
    void fill_in_indenting(std::ofstream& stream, const size_t& indent)
    {
        for (size_t i = 0; i < indent; ++i) {stream << ' ';}
    }

    /**
     * @brief Sets up the meta tags for the HTML document.
     * 
     * @param stream The output stream for the HTML file.
     * @param stylesheet_name The name of the CSS file to link in the HTML document.
     */
    void setup_html_meta_tags(std::ofstream& stream, const std::string& stylesheet_name)
    {
        size_t indent = 1;
        stream << "<head>" << std::endl;
        fill_in_indenting(stream, indent);
        stream << "<meta charset=\"utf-8\">" << std::endl;
        fill_in_indenting(stream, indent);
        stream << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" << std::endl;
        fill_in_indenting(stream, indent);
        stream << "<link rel=\"stylesheet\" href=\"" << stylesheet_name << "\">" << std::endl;
        stream << "</head>" << std::endl;
    }
};

#endif