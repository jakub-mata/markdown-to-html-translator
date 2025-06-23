#ifndef __HTML_VISITOR_HPP
#define __HTML_VISITOR_HPP

#include <fstream>
#include "../node.hpp"
#include "css_constructor.hpp"


/**
 * @class HTML_Visitor
 * @brief Implements the Visitor pattern for generating HTML from a parsing tree.
 * 
 * The `HTML_Visitor` traverses the parsing tree and generates HTML elements. It works
 * closely with the `CSS_Constructor` to ensure that attributes are styled properly by
 * adding CSS classes to the generated HTML.
 * 
 * @details
 * - Visits different types of nodes (`Node`, `ContentNode`, `ImageNode`, `HyperlinkNode`) 
 *   and generates the corresponding HTML elements.
 * - Handles indentation and formatting of the HTML output.
 * - Ensures that CSS classes are added for attributes using the `CSS_Constructor`.
 * 
 * @see NodeVisitor
 * @see CSS_Constructor
 */
class HTML_Visitor : public NodeVisitor {
    public:
        /**
         * @brief Constructs an `HTML_Visitor` object.
         * 
         * @param stream The output stream to write the generated HTML to.
         * @param css_builder Pointer to a `CSS_Constructor` for managing CSS classes.
         * @param indent The initial indentation level for the HTML output.
         */
        HTML_Visitor(std::ofstream& stream, CSS_Constructor* css_builder, const size_t& indent)
        : stream(stream),
          css_builder(css_builder),
          prev_token_content(false),
          SPACE_INDENT(indent),
          prev_token_indent(0) {}

        /**
         * @brief Visits a generic `Node` and generates the corresponding HTML element.
         * 
         * @param node The node to visit.
         * @param indent The current indentation level.
         * 
         * @throws std::runtime_error If the node's element type is unknown.
         */
        void visit(Node& node, size_t indent) override 
        {
            prev_token_content = false;
            auto it = element_to_html_name.find(node.element);
            if (it == element_to_html_name.end()) {throw std::runtime_error("unknown element");}
            stream << std::endl;
            fill_in_indenting(stream, indent);
            stream << '<' << it->second;
            if (node.element == ElementType::Horizontalline)
            {
                stream << "/>";
                return;
            }
            
            // Adding attributes as CSS classes
            if (!node.attributes.empty())
            {
                stream << " class= \"";
                bool first = true;
                for (auto&& attr : node.attributes)
                {
                    if (first)
                        first = false;
                    else 
                        stream << ' ';
                    stream << attr_enum_to_name[attr];

                    css_builder->add_css_attr_class(attr);
                }
                stream << "\"";
            }

            stream << '>';
            if (node.element == ElementType::Codeblock && node.attributes[0] == Attribute::Block)
                stream << "<pre>";

            for (auto&& child : node.children)
            {
                child->accept(*this, indent + SPACE_INDENT);
            }

            stream << std::endl;
            fill_in_indenting(stream, indent);
            if (node.element == ElementType::Codeblock && node.attributes[0] == Attribute::Block)
                stream << "</pre>";
            stream << "</" << it->second << '>';
        }

        /**
         * @brief Visits a `ContentNode` and generates the corresponding HTML content.
         * 
         * @param node The content node to visit.
         * @param indent The current indentation level.
         */
        void visit(ContentNode& node, size_t indent) override 
        {
            if (!prev_token_content || prev_token_indent != indent)
            {
                prev_token_content = true;
                prev_token_indent = indent;
                stream << std::endl;
                fill_in_indenting(stream, indent);
            }
            stream << node.content;
        }

        /**
         * @brief Visits an `ImageNode` and generates the corresponding HTML image element.
         * 
         * @param node The image node to visit.
         * @param indent The current indentation level.
         */
        void visit(ImageNode& node, size_t indent) override 
        {
            prev_token_content = false;
            stream << std::endl;
            fill_in_indenting(stream, indent);
            stream << "<img src=\"" << node.src << "\" alt=\"" << node.alt << "\" title=\"" << node.title << "\""" class=\"ImageAttr\"" << "/>";
            css_builder->add_css_attr_class(Attribute::ImageAttr);
        }

        /**
         * @brief Visits a `HyperlinkNode` and generates the corresponding HTML hyperlink element.
         * 
         * @param node The hyperlink node to visit.
         * @param indent The current indentation level.
         */
        void visit(HyperlinkNode& node, size_t indent) override
        {
            prev_token_content = false;
            stream << std::endl;
            fill_in_indenting(stream, indent);
            stream << "<a href=\"" << node.href << "\" title=\"" << node.title << "\">" << node.displayed << "</a>";
        }
    
    private:
        std::ofstream& stream;
        CSS_Constructor* css_builder;
        bool prev_token_content;
        size_t prev_token_indent;
        size_t SPACE_INDENT;

        void fill_in_indenting(std::ofstream& stream, size_t indent)
        {
            for (size_t i = 0; i < indent; ++i) {stream << ' ';}
        }
};

#endif