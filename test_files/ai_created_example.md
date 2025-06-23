# Markdown Test Document

This document tests various Markdown elements for your converter.

## Headings
# H1
## H2
### H3
#### H4
##### H5
###### H6

## Text Formatting
- *Italic text*
- **Bold text**
- ***Bold and italic***

## Blockquotes
> This is a blockquote.
> It can span multiple lines.

## Lists

### Unordered Lists
- Item 1
- Item 2
    - Subitem 2.1
	- Subitem 2.2
	> Blockquote
- Item 3

### Ordered Lists
1. First item
2. Second item
    1. Nested item
    2. Another nested item
3. Third item

## Code

### Inline Code
Use `printf()` for output.

### Code Blocks
```
def hello_world():
    print("Hello, world!")
```


## Horizontal Rule
---

## Links
[Google](https://www.google.com)
[Relative link](/path/to/file)
[Link with title](https://example.com "Example Title")

## Images
![Alt text](https://via.placeholder.com/150 "Placeholder Image")

## Escaping
Use backslash to escape special characters: \*not italic\*, \[not a link\]

## Tables
| Syntax      | Description |
| ----------- | ----------- |
| Header      | Title       |
| Paragraph   | Text        |

### Complex Table
| Header1 | Header2 | Header3 |
|------------- |----------------|---------------|
| data         | data           | data          |
| longer data  | more data      | numbers       |

## Mixed Elements
1. **List item** with [link](https://example.com)
    > Blockquote inside list
2. *Italic* and `code` combined

## Edge Cases
- Empty table cell:

| Header | Header |
| ------ | ------ |
|        | Cell   |

[Empty link text]()