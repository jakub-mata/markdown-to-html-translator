#ifndef _TOKEN_HPP
#define _TOKEN_HPP

#include <string>
#include <unordered_map>


enum TokenType {
    OpenToken,
    CloseToken,
    ContentToken,
    EOF_Token
};

enum ElementType {
    DOCSTART,
    Content,
    Header_1,
    Header_2,
    Header_3,
    Header_4,
    Header_5,
    Header_6,
    Paragraph,
    Codeblock,
    Horizontalline,
    Hypertext,
    ImageType,
    Span,
    List_Ordered,
    List_Unordered,
    List_Element,
    Table,
    Table_Head,
    Table_Row,
    Table_Cell,
    EOF_Reached,
};

/**
 * @struct Token
 * @brief A struct representing a token in the parsing process. It contains the type, element, content, alt text, and title of the token.
 */
struct Token {
    TokenType type;
    ElementType element;
    std::string content;
    std::string alt;
    std::string title;
    /**
     * @brief Constructs a Token object.
     * @param type The type of the token.
     * @param el The element type of the token.
     * @param text The content of the token.
     * @param alt The alt text of the token (default is an empty string).
     * @param title The title of the token (default is an empty string).
     */
    Token(TokenType type, ElementType el, const std::string& text, const std::string& alt = "", const std::string& title = "")
    : type(type), 
      element(el), 
      content(text),
      alt(alt),
      title(title) {}
};



std::unordered_map<ElementType, std::string> element_to_html_name = 
{
    {DOCSTART, "!DOCTYPE html"},
    {Content, "content"},
    {Header_1, "h1"},
    {Header_2, "h2"},
    {Header_3, "h3"},
    {Header_4, "h4"},
    {Header_5, "h5"},
    {Header_6, "h6"},
    {Paragraph, "p"},
    {Codeblock, "code"},
    {Horizontalline, "hr"},
    {ImageType, "img"},
    {Hypertext, "a"},
    {Span, "span"},
    {List_Ordered, "ol"},
    {List_Unordered, "ul"},
    {List_Element, "li"},
    {Table, "table"},
    {Table_Head, "th"},
    {Table_Row, "tr"},
    {Table_Cell, "td"},
};

#endif