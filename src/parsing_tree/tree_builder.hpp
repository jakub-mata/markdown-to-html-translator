#ifndef _TREE_BUILDER_HPP
#define _TREE_BUILDER_HPP

#include "../token.hpp"
#include <memory>
#include "../node.hpp"
#include <iostream>
#include <stack>
#include "../error_handler.hpp"

/**
 * @class TreeBuilder
 * @brief A class reponsible for creating and building a tree from tokens emitted by TokenEmitter.
 * It stores the root of the tree as a unique_ptr and traverses the tree
 * with a raw pointer.
 */
class TreeBuilder {
public:
    /**
     * @brief Constructs a TreeBuilder object.
     * @param logger Pointer to the Logger instance for error handling.
     */
    TreeBuilder (Logger* logger) : root(std::make_unique<Node>(ElementType::DOCSTART, nullptr)), logger(logger) {
        current = root.get();
    }

    /**
     * @brief Consumes a token and builds the tree accordingly.
     * @param token The token to consume.
     */
    void consume_token (Token&& token) {
        switch (token.type)
        {
        case TokenType::OpenToken: 
            if (token.element == ElementType::ImageType)
            {
                auto new_node = std::make_unique<ImageNode>(
                    current, std::move(token.content), std::move(token.alt), std::move(token.title)
                );
                current = new_node.get();
                current->parent->add_child(std::move(new_node));
                break;
            }
            else if (token.element == ElementType::Hypertext)
            {
                auto new_node = std::make_unique<HyperlinkNode>(
                    current, std::move(token.content), std::move(token.alt), std::move(token.title)
                );
                current = new_node.get();
                current->parent->add_child(std::move(new_node));
                break;
            }
            else
            {
                auto new_node = std::make_unique<Node>(token.element, current);
                current = new_node.get();
                current->parent->add_child(std::move(new_node));
                break;
            }
        case TokenType::CloseToken:
            if (token.element != current->element) {
                logger->log_error("Current element ("+element_to_html_name[current->element]+") and \
                    closing tag element ("+element_to_html_name[token.element]+") do not match. This is an error on our side.");
                throw std::runtime_error("incorrectly parsed tree");
            }
            if (current->element == ElementType::DOCSTART)
                logger->log_warning("Moving 'current' above DOCTYPE element making it a nullptr.");
            current = current->parent;
            break;
        case TokenType::ContentToken:
            {
                auto new_node = std::make_unique<ContentNode>(token.element, current, std::move(token.content));
                current->add_child(std::move(new_node));
                break;
            }
        case TokenType::EOF_Token:
            break;
        default:
            logger->log_error("Unknown tokentype found during tree parsing.");
            throw std::runtime_error("huh");
        }
    }

    /**
    * Notes: - the position of current stays the same
    *        - intended for appending a tree created by Managers (e.g. @class TableManager)
    */
    void append_subtree(std::unique_ptr<Node> subtree_root)
    {
        if (current==nullptr)
        {
            logger->log_error("Current is null. The error is on our side, we're working on it.");
            throw std::runtime_error("Error during tree parsing");
        }
        logger->log_info("Appending subtree with root element: " + element_to_html_name[subtree_root->element]);
        current->add_child(std::move(subtree_root));
    }

    /**
     * @brief Returns the element pointed at by the current pointer (the current position in the parsing tree).
     * @return The current element.
     */
    ElementType get_current_element()
    {
        if (current==nullptr)
        {
            logger->log_error("Current is null. The error is on our side, we're working on it.");
            throw std::runtime_error("Error during tree parsing");
        }
        return current->element;
    }

    /**
     * @brief Adds an attribute to the current node.
     * @param att The attribute to add.
     */
    void add_attribute(Attribute&& att)
    {
        if (current==nullptr)
        {
            logger->log_error("Current is null. The error is on our side, we're working on it.");
            throw std::runtime_error("Error during tree parsing");
        }
        current->add_attribute(std::move(att));
    }

    /**
     * @brief Returns the root of the tree.
     * @return A unique_ptr to the root node.
     */
    std::unique_ptr<Node> get_root()
    {
        return std::move(root);
    }

    /**
     * @brief Prints the tree structure to the console.
     */
    void print_tree() const
    {
        std::stack<std::pair<Node*, size_t>> node_stack;
        node_stack.push(std::make_pair<Node*, size_t>(root.get(), 0));
        while (!node_stack.empty())
        {
            std::pair<Node*, size_t> popped = node_stack.top();
            node_stack.pop();
            for (size_t i = 0; i < 2 * popped.second; ++i) {std::cout << ' ';}
            std::cout << element_to_html_name[popped.first->element] << std::endl;

            for (auto it=popped.first->children.rbegin(); it != popped.first->children.rend(); ++it)
            {
                node_stack.push(std::make_pair<Node*, size_t>((*it).get(), popped.second + 1));
            }
        }
    }
private:
    std::unique_ptr<Node> root = nullptr;
    Node* current = nullptr;
    Logger* logger;
};

#endif