#ifndef _NODE_HPP
#define _NODE_HPP

#include <memory>
#include <vector>
#include <string>
#include "token.hpp"
#include <optional>

/**
 * @enum Attribute
 * @brief Contains values corresponding to certain CSS styling.
 * @see CSS_Constructor
 */
enum Attribute
{
    Bold,
    Italic,
    FontSize1,
    FontSize2,
    FontSize3,
    FontSize4,
    FontSize5,
    FontSize6,
    Inline,
    Block,
    BlockQuote,
    TableStyle,
    TableHeader,
    TableRow,
    TableCell,
    ImageAttr,
};

std::vector<std::string> attr_enum_to_name = {
    "Bold", "Italic", "FontSize1", "FontSize2", "FontSize3", "FontSize4", "FontSize5", "FontSize6", "Inline", "Block", "BlockQuote",
    "TableStyle", "TableHeader", "TableRow", "TableCell", "ImageAttr"
};

/**
 * @brief Different kinds of nodes have different needs in terms of what data to store, hence multiple kinds of nodes.
 */
struct Node;
struct ContentNode;
struct ImageNode;
struct HyperlinkNode;
/**
 * @struct NodeVisitor
 * @brief The base class for a Visitor design pattern. In order to traverse the parsing tree during HTML construction and
 * behave differently based the type of node, the Visitor pattern is used. Each abstract method *visit()* represents
 * a different action corresponding to a type of node.
 */
struct NodeVisitor {
    virtual void visit(Node& node, size_t indent) = 0;
    virtual void visit(ContentNode& node, size_t indent) = 0;
    virtual void visit(ImageNode& node, size_t indent) = 0;
    virtual void visit(HyperlinkNode& node, size_t indent) = 0;
    virtual ~NodeVisitor() = default;
};

/**
 * @struct Node
 * @brief A struct representing a node in the parsing tree. It contains information about the node's type, its parent,
 * its children, and its attributes. It also provides methods for adding children and attributes, as well as accepting
 * a visitor for traversal.
 */
struct Node {
    public:
        using child_p = std::unique_ptr<Node>;
    
        ElementType element;
        std::vector<Attribute> attributes; // CSS attributes
        Node* parent;
        std::vector<child_p> children;
    
        /**
         * @brief Constructs a Node object.
         * @param type The type of the node.
         * @param parent The parent node.
         */
        Node(ElementType type, Node* parent)
        : element(type),
          parent(std::move(parent)),
          children(std::vector<child_p>()),
          attributes(std::vector<Attribute>()) {}
    
        virtual ~Node() = default;

        /**
         * @brief Adds a child node to the current node.
         * @param child The child node to add.
         */
        void add_child(std::unique_ptr<Node> child) {
            children.push_back(std::move(child));
        }

        /**
         * @brief Removes the last child node from the current node.
         * @return The removed child node, or std::nullopt if there are no children.
         */
        std::optional<std::unique_ptr<Node>> remove_last_child()
        {
            if (!children.empty())
            {
                std::unique_ptr<Node> last_child = std::move(children.back());
                children.pop_back();
                return std::optional<std::unique_ptr<Node>>{std::move(last_child)};
            }
            return std::nullopt;
        }

        /**
         * @brief Adds an attribute to the current node.
         * @param attribute The attribute to add.
         */
        void add_attribute(Attribute&& attribute)
        {
            attributes.push_back(std::move(attribute));
        }

        /**
         * @brief Accepts a visitor for traversal.
         * @param visitor The visitor to accept.
         * @param indent The indentation level for printing.
         */
        virtual void accept(NodeVisitor& visitor, size_t indent) {
            visitor.visit(*this, indent);
        }
};

/**
 * @struct ContentNode
 * @brief A struct representing a content node in the parsing tree. It contains the content of the node and inherits
 * from Node. It also provides an accept method for traversal.
 */
struct ContentNode : public Node
{
    std::string content;

    /**
     * @brief Constructs a ContentNode object.
     * @param type The type of the node.
     * @param parent The parent node.
     * @param consumed The content of the node.
     */
    ContentNode(ElementType type, Node* parent, std::string&& consumed)
    : Node(type, parent), content(std::move(consumed)) {}

    /**
     * @brief Constructs a ContentNode object.
     * @param type The type of the node.
     * @param parent The parent node.
     * @param consumed The content of the node.
     */
    virtual void accept(NodeVisitor& visitor, size_t indent) override {
        visitor.visit(*this, indent);
    }
};

/**
 * @struct ImageNode
 * @brief A struct representing an image node in the parsing tree. It contains the source, alt text, and title of the image.
 * It inherits from Node and provides an accept method for traversal.
 */
struct ImageNode : public Node
{
    std::string src;
    std::string alt;
    std::string title;

    /**
     * @brief Constructs an ImageNode object.
     * @param parent The parent node.
     * @param src The source of the image.
     * @param alt The alt text of the image.
     * @param title The title of the image.
     */
    ImageNode(Node* parent, std::string&& src, std::string&& alt, std::string&& title)
    : Node(ElementType::ImageType, parent), src(std::move(src)), alt(std::move(alt)), title(std::move(title)) {}

    /**
     * @brief Constructs an ImageNode object.
     * @param parent The parent node.
     * @param src The source of the image.
     * @param alt The alt text of the image.
     * @param title The title of the image.
     */
    virtual void accept(NodeVisitor& visitor, size_t indent) override {
        visitor.visit(*this, indent);
    }
};

/**
 * @struct HyperlinkNode
 * @brief A struct representing a hyperlink node in the parsing tree. It contains the href, displayed text, and title of the hyperlink.
 * It inherits from Node and provides an accept method for traversal.
 */
struct HyperlinkNode : public Node
{
    std::string href;
    std::string displayed;
    std::string title;

    /**
     * @brief Constructs a HyperlinkNode object.
     * @param parent The parent node.
     * @param href The href of the hyperlink.
     * @param displayed_alt The displayed text of the hyperlink.
     * @param title The title of the hyperlink.
     */
    HyperlinkNode(Node* parent, std::string&& href, std::string&& displayed_alt, std::string&& title)
    : Node(ElementType::Hypertext, parent), href(std::move(href)), title(std::move(title)), displayed(std::move(displayed_alt)) {}

    /**
     * @brief Constructs a HyperlinkNode object.
     * @param parent The parent node.
     * @param href The href of the hyperlink.
     * @param displayed_alt The displayed text of the hyperlink.
     * @param title The title of the hyperlink.
     */
    virtual void accept(NodeVisitor& visitor, size_t indent) override {
        visitor.visit(*this, indent);
    }
};

#endif