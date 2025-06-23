/**
* @file emitting_middleware.hpp
* @brief Middleware connecting Markdown parsing and tree building.
* 
* This file contains the `Token_Emitter` and `TableManager` classes, which act as intermediaries
* between the `Md_Parser` and the `TreeBuilder`. They handle token emission and table-specific
* parsing logic, ensuring proper communication and structure generation.
*/

#ifndef _EMITTING_MIDDLEWARE_HPP
#define _EMITTING_MIDDLEWARE_HPP

#include "../parsing_tree/tree_builder.hpp"
#include "../error_handler.hpp"
#include <optional>

/**
 * @enum ParseWarningFlags
 * @brief Flags indicating the status of table parsing.
 * 
 * This enum is used to signal the success or failure of table parsing operations.
 * 
 * @see TableManager
 */
enum ParseWarningFlags
{
    TableFailed,
    TableSuccess,
};

/**
 * @class TableManager
 * @brief Manages table parsing and emits structured table data to the `TreeBuilder`.
 * 
 * The `TableManager` processes tokens related to tables, handles table-specific attributes,
 * and emits the table structure to the `TreeBuilder` on success or failure.
 * 
 * @details
 * - Handles table rows, headers, and cells.
 * - Emits incomplete tables as paragraphs on failure.
 * 
 * @see TreeBuilder
 */
class TableManager
{
public:
    /**
     * @brief Constructs a `TableManager` object.
     * 
     * @param builder_p Pointer to the `TreeBuilder` for emitting table data.
     * @param logger Pointer to the logger for error handling.
     */
    TableManager(std::shared_ptr<TreeBuilder> builder_p, Logger* logger)
    : table_root(nullptr),
      builder(std::move(builder_p)),
      current(nullptr),
      col_dims(0) {}
      
    /**
     * @brief Consumes a token and processes it based on its type.
     * 
     * @param token The token to be consumed.
     * 
     * @throws std::runtime_error If the token type is not recognized or if the table structure is invalid.
     */
    void consume_token(Token&& token)
    {
        switch (token.element)
        {
        case ElementType::Table:
        {
            if (token.type == TokenType::OpenToken)
            {
                if (table_root != nullptr)
                    throw std::runtime_error("Should have been nulled by now");
                table_root = std::make_unique<Node>(ElementType::Table, nullptr);
                table_root->add_attribute(TableStyle);
                current = table_root.get();
            }
            else
                throw std::runtime_error("You should not be here");
            break;
        }
        case ElementType::Table_Row:
            if (token.type == TokenType::OpenToken)
                create_new_node(std::move(token));
            else
            {
                if (col_dims != 0)  // not the header row, current is pointing to <tr>
                {
                    for (size_t t = current->children.size(); t < col_dims; ++t)
                    {
                        auto empty_child = std::make_unique<Node>(ElementType::Table_Cell, current);
                        current->add_child(std::move(empty_child));
                    }
                    current = current->parent;
                }
                else  // the header row
                {
                    current->children.pop_back();
                    col_dims = current->children.size();
                    if (col_dims == 0)
                        throw std::runtime_error("Table header should not be empty");
                    current = current->parent;
                }
            }
            break;
        case ElementType::Table_Head:
            if (token.type == OpenToken)
                create_new_node(std::move(token));
            else
                current = current->parent;
            break;
        case ElementType::Table_Cell:
            if (token.type == OpenToken)
            {
                if (current->children.size() < col_dims)
                    create_new_node(std::move(token));
            }
            else
            {
                if (current->element==ElementType::Table_Cell)
                    current = current->parent;
            }
            break;
        case ElementType::Content:
        {
            if (current->element == ElementType::Table_Row)
                break; // the current amount of data cell is greater than col_dim
            
            auto node = std::make_unique<ContentNode>(ElementType::Content, current, std::move(token.content));
            current->add_child(std::move(node));
            break;
        }
        case ElementType::Hypertext:
        {
            if (token.type == TokenType::OpenToken)
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
                current = current->parent;
            }
        }
        case ElementType::Span:
        case ElementType::Codeblock:
            if (token.type == TokenType::OpenToken)
                create_new_node(std::move(token));
            else
                current = current->parent;
            break;
        default:
            std::cerr << "Unrecognized element in table parsing";
        }
    }

    /**
     * @brief Adds an attribute to the current node.
     * 
     * @param attr The attribute to be added.
     */
    void add_attribute(Attribute&& attr)
    {
        current->add_attribute(std::move(attr));
    }

    /**
     * @brief Emits the table structure to the `TreeBuilder` on success.
     * 
     * This method appends the table root to the tree builder and resets the table root.
     */
    void emit_on_success()
    {
        builder->append_subtree(std::move(table_root));
        table_root = nullptr;
    }

    /**
    * @brief Emits the whole table except for the last row
    * The last row emitted as if not in table (links, styling and 
    * other special content inside cells are kept)
    */
    void emit_on_failure()
    {
        if (table_root == nullptr)
        {
            logger->log_error("Table root is null when emitting.");
            throw std::runtime_error("invalid emitting on failure");
        }
        //print_tree();
        std::optional<std::unique_ptr<Node>> last_row_p = table_root->remove_last_child();
        if (!last_row_p.has_value())
            return;

        // emit the correctly parsed table
        if (table_root->children.size() != 0)
            emit_on_success();

        // emit the last row as paragraph
        emit_row_as_paragraph(std::move(*last_row_p));
        table_root = nullptr;
        col_dims = 0;
    }

    /**
     * @brief Returns the number of columns in the table.
     * 
     * @return The number of columns in the table.
     */
    size_t get_col_dims()
    {
        return col_dims;
    }
private:
    std::unique_ptr<Node> table_root;
    std::shared_ptr<TreeBuilder> builder;
    Node* current;
    Logger* logger;
    // current table state
    size_t col_dims;

    void create_new_node(Token&& token)
    {
        auto node = std::make_unique<Node>(token.element, current);
        if (token.element == ElementType::Table_Row)
            node->add_attribute(Attribute::TableRow);
        else if (token.element == ElementType::Table_Head)
            node->add_attribute(Attribute::TableHeader);
        else if (token.element == ElementType::Table_Cell)
            node->add_attribute(Attribute::TableCell);

        current = node.get();
        current->parent->add_child(std::move(node));
    }

    /**
     * @brief A method called when parsing failed and the last row of the parsing tree is invalid,
     * hence can be emitted as in a paragraph element.
     */
    void emit_row_as_paragraph(std::unique_ptr<Node> row_p)
    {
        if (row_p->children.empty())
            return;
        row_p->element = ElementType::Paragraph;

        std::vector<std::unique_ptr<Node>> new_children;
        for (auto& td_node : row_p->children)
        {
            // Transfer the children of the <td> element to the new_children vector
            new_children.push_back(std::make_unique<ContentNode>(ElementType::Content, row_p.get(), "|"));
            for (auto& child : td_node->children)
            {
                new_children.push_back(std::move(child));
            }
        }
        row_p->children = std::move(new_children);

        builder->append_subtree(std::move(row_p));
    }

    void print_tree() const
    {
        std::stack<std::pair<Node*, size_t>> node_stack;
        node_stack.push(std::make_pair<Node*, size_t>(table_root.get(), 0));
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
};


/**
 * @class Token_Emitter
 * @brief Middleware between `Md_Parser` and `TreeBuilder`.
 * 
 * The `Token_Emitter` handles token emission and delegates table-specific tokens to the `TableManager`.
 * It ensures proper communication between the parser and the tree builder.
 * 
 * @details
 * - Manages table parsing using the `TableManager`.
 * - Emits non-table tokens directly to the `TreeBuilder`.
 * 
 * @see TableManager
 * @see TreeBuilder
 */
class Token_Emitter
{
public:
    Token_Emitter(std::shared_ptr<TreeBuilder> builder_p, Logger* logger)
    : table_manager(std::make_unique<TableManager>(std::shared_ptr<TreeBuilder>(builder_p), logger)),
      builder(std::shared_ptr(builder_p)),
      logger(logger),
      table_parsing_flag(false) {}

    void emit_token(Token&& to_emit) 
    {
        if (table_parsing_flag)
        {
            logger->log_info("Emitting " + element_to_html_name[to_emit.element] + " to table builder.");
            table_manager->consume_token(std::move(to_emit));
            return;
        }
        
        if (to_emit.element == ElementType::Table)
        {
            table_parsing_flag = true;
            logger->log_info("Table parsing has started.");
            logger->log_info("Emitting " + element_to_html_name[to_emit.element] + " to table builder.");
            table_manager->consume_token(std::move(to_emit));
            return;
        }

        logger->log_info("Emitting " + element_to_html_name[to_emit.element] + " to tree builder.");
        builder->consume_token(std::move(to_emit));
    }

    void handle_flag(ParseWarningFlags flag)
    {
        switch (flag)
        {
        case TableFailed:
            logger->log_info("Table parsing has ended");
            table_manager->emit_on_failure();
            table_parsing_flag = false;
            break;
        case TableSuccess:
            logger->log_info("Table parsing has ended");
            table_manager->emit_on_success();
            table_parsing_flag = false;
        default:
            break;
        }
    }

    ElementType fetch_current_element()
    {
        return builder->get_current_element();
    }

    std::shared_ptr<TreeBuilder> get_builder()
    {
        return std::move(builder);
    }

    void print_tree()
    {
        builder->print_tree();
    }

    void add_attribute(Attribute&& attr)
    {
        if (table_parsing_flag)
            table_manager->add_attribute(std::move(attr));
        else
            builder->add_attribute(std::move(attr));
    }

    size_t get_col_dims()
    {
        return table_manager->get_col_dims();
    }
private:
    std::shared_ptr<TreeBuilder> builder;
    std::unique_ptr<TableManager> table_manager;
    Logger* logger;
    bool table_parsing_flag;
};


#endif