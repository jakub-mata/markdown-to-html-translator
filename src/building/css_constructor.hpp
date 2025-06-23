#ifndef _CSS_CONSTRUCTOR_HPP
#define _CSS_CONSTRUCTOR_HPP

#include <string>
#include <fstream>
#include <set>
#include <unordered_map>
#include "../node.hpp"

/**
 * @class CSS_Constructor
 * @brief A class responsible for creating a CSS file linked to the main HTML file.
 * 
 * This class is used by `HTML_Builder` to generate a CSS file that defines styles for various
 * attributes in the HTML document. It ensures that all required CSS classes are created and
 * written to the CSS file. The `CSS_Constructor` also provides default styling for the document.
 * 
 * @details
 * - The `CSS_Constructor` maintains a set of used attributes to avoid duplicating CSS class definitions.
 * - It maps attributes to their corresponding CSS styles using an internal `std::unordered_map`.
 * - The `HTML_Builder` relies on this class to ensure that the generated HTML document is properly styled.
 * 
 * @see HTML_Builder
 */
class CSS_Constructor
{
public:
    /**
     * @brief Constructs a `CSS_Constructor` instance.
     * 
     * @param stream A reference to the output stream where the CSS file will be written.
     */
    CSS_Constructor(std::ofstream& stream)
    : used_attributes(std::set<Attribute>()),
      styles_stream(stream) {}

    /**
     * @brief Adds a CSS class for a given attribute.
     * 
     * If the attribute has not already been added, this method creates a new CSS class
     * and writes it to the CSS file.
     * 
     * @param attr The attribute for which the CSS class should be created.
     */
    void add_css_attr_class(Attribute attr)
    {
        auto it = used_attributes.find(attr);
        if (it != used_attributes.end())
            return;

        used_attributes.emplace(attr);
        setup_css_class(attr);
    }

    /**
     * @brief Creates default styling for the HTML document.
     * 
     * This method writes basic CSS rules for the `<body>` element to the CSS file.
     */
    void create_default_styling()
    {
        styles_stream << "body {" << std::endl;
        styles_stream << "margin: 2rem auto;" << std::endl;
        styles_stream << "width: 80%;" << std::endl;
        styles_stream << '}' << std::endl;
    }

private:
    std::set<Attribute> used_attributes; /**< A set of attributes that have already been added as CSS classes. */
    std::ofstream& styles_stream; /**< The output stream for writing the CSS file. */

    /**
     * @brief Maps attributes to their corresponding CSS styles.
     * 
     * This map defines the CSS rules for each attribute. If a new attribute needs to be styled,
     * it should be added to this map.
     */
    std::unordered_map<Attribute, std::string> attr_to_css = {
        {Bold, "font-weight: bold;"},
        {Italic, "font-style: italic;"},
        {FontSize1, "font-size: 32px;"},
        {FontSize2, "font-size: 24px;"},
        {FontSize3, "font-size: 20.8px;"},
        {FontSize4, "font-size: 16px;"},
        {FontSize5, "font-size: 12.8px;"},
        {FontSize6, "font-size: 11.2px;"},
        {Inline, "display: inline;"},
        {Block, "display: block;"},
        {BlockQuote, "padding-left: 1em;\nborder-left: 2px solid purple;\ndisplay: block;"},
        {TableRow, "border-bottom: 1px solid #ddd;"},
        {TableHeader, "background-color: #ddd;\npadding: .4rem .8rem"},
        {TableStyle, "border-collapse: collapse;"},
        {TableCell, "padding: .4rem, .8rem"},
        {ImageAttr, "max-width: 100%;\nheight: auto;"},
    };

    /**
     * @brief Creates a CSS class for a given attribute and writes it to the CSS file.
     * 
     * @param attr The attribute for which the CSS class should be created.
     * 
     * @throws std::runtime_error If the attribute is not defined in `attr_to_css`.
     */
    void setup_css_class(Attribute attr)
    {
        styles_stream << '.' << attr_enum_to_name[attr] << " {" << std::endl;
        auto attr_it = attr_to_css.find(attr);
        if (attr_it == attr_to_css.end()) { throw std::runtime_error("unknown attribute"); }

        styles_stream << attr_it->second << std::endl << '}' << std::endl;
    }
};

#endif