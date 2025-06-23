## Markdown to HTML converter

### Specification
This project is aimed at creating a markdown to HTML converter. If you are unsure
about the Markdown standard, go [here](https://www.markdownguide.org/cheat-sheet/). 
Supported markdown syntax includes:
- headings of all sizes
- italic and bold text
- blockquotes (not nested)
- numbered and regular lists
- code blocks (both inline and on mutiple lines)
- horizontal separation line
- hyperlinks
- images
- escaping with `\`
- tables (alignment not supported)

The output of this program are two files: an HTML file and a linked CSS file. 

### Dependencies

This project requires C++17 and newer. `gcc` for GNU/Linux and `mingw` for Windows work but you can choose any compiler as long as it support C++17. No additional libraries are needed.

### Usage
You can `git clone` this repository and create the program in two ways:

Find `build.sh` in the root directory and run it:
```
./build.sh
```
Or use `CMakeLists.txt` by calling `cmake *path-to-the-src-directory*`. E.g. if your newly created directory were on the same level as `/src`, then you would call `cmake ../src` from it. A `Makefile` will then appear in your directory, call with with `make`.

Now you have the executable. Call it (let's name the executable `markdown_converter` in accordance with `CMakeLists.txt`) with these arguments.

```
./markdown_converter -i ../test_files/test_file.md -o ../test_files/test_file.html -s ../test_files/styles.css
```

New HTML and CSS files should appear in your `test_files` directory. Of course, depending on where you call the 
command from, the relative paths in the command change.

The program accepts four arguments:
- `-i *input-file-path*` - a **required** argument for the input markdown file to be converted
- `-o *output-file-path*` - the name of the output HTML file (defaults to `output.html` if none provided)
- `-s *styles-file-path*` - the name of the styles file (defaults to `styles.css` if none provided)
- `-v *{1, 2, 3}*` - the verbosity of a logger. The logger provides logs to a `logs.log` file created in the directory of the executable. If `-v` flag is passed, it has to provide a value, simply passing `-v` will result in an error. Value `1` logs only *error-level* logs, `2` adds *warnings*, `3` adds *info*. Use this for debugging or if interested in the inner workings. If not used, no logging is done.

The argument and its value can be written separately: `-i some-path` or together `-isomepath`.

You can try to run the program on provided example input files in `test_files` directory.


### Quirks
If you use list indentation (a list inside a list), use **4 spaces** or a **tab**. This is in accordance with the Markdown guide (some other parsers would allow for some other combination, this one does not). If you're using tabs, make sure your
editor does not change its value to a preset amount of spaces.

Futhermore, if you wish to employ indented listing, do not put any empty lines between the indented list elements:
```
DO:

- Base level
	- Indented level

DON'T:

- Base level

	- Indented level
```

Tables have to start and end their rows with pipe symbols `|`. A table header is separated by 3 or more dashes from the body. E.g.
```
| Header 1 | Header 2 |
|---|----------|
| Row 1    | Row 2|
```

### Code structure

The program is made up of three parts:
1. *markdown parsing*: a markdown parser parses the input document and emits tokens to a connected tree builder
2. *tree building*: the tree builder catches tokens and creates and changes its parsing tree
3. *html construction*: after all tokens have been emitted and the tree is built, an HTML constructor creates an HTML
and a CSS file based on the provided parsing tree.

#### Markdown parsing
The Markdown Parsing phase is responsible for reading the input Markdown file and converting it into a series of tokens. These tokens represent the structural elements of the Markdown document, such as headings, paragraphs, lists, tables, and more.


***How it works***:

1. The `Md_Parser` class reads the Markdown file character by character.
2. It uses a state machine to determine the current context (e.g., parsing a heading, list, or table).
3. Tokens are emitted using the `Token_Emitter` class, which connects the parser to the tree builder.
4. Special cases like escape sequences (`\`), inline code, and block-level elements (e.g., blockquotes, tables) are handled explicitly.
5. Tables are parsed with support for rows starting and ending with a pipe (`|`) symbol. While attempting to parse a table, a completely new, separate tree is being constructed. If table parsing is successful, this tree gets appended to the overarching tree. Otherwise, the tree gets boiled down into a simple content token.

#### Tree building

The Tree Building phase constructs a hierarchical representation of the Markdown document based on the tokens emitted by the parser. This phase uses the TreeBuilder class to build a parsing tree.


***How it works***:

1. The `TreeBuilder` class receives tokens from the `Token_Emitter`.
2. It creates nodes (`Node`, `ContentNode`, `ImageNode`, etc.) for each token and organizes them into a tree structure.
3. The tree structure represents the logical hierarchy of the document (e.g., headings contain paragraphs, lists contain list items).
4. `Attributes` (e.g., bold, italic, blockquote) are added to nodes as needed.
5. Subtrees (e.g., tables) can be appended using helper classes like `TableManager`.

***Example***: 
```
# Heading 1

This is a paragraph.

- List item 1
- List item 2
```

```
DOCSTART
├── Header_1
│   └── Content: "Heading 1"
├── Paragraph
│   └── Content: "This is a paragraph."
└── List_Unordered
    ├── List_Element
    │   └── Content: "List item 1"
    └── List_Element
        └── Content: "List item 2"
```
#### HTML construction

The HTML Construction phase generates the final HTML and CSS files from the parsing tree. This phase uses the `HTML_Builder` and `CSS_Constructor` classes.

***How it works***:

1. The `HTML_Builder` traverses the parsing tree using the `HTML_Visitor` class.
2. For each node, it generates the corresponding HTML tags and writes them to the output file.
3. The `CSS_Constructor` generates a default CSS file and adds styles for attributes like bold, italic, and table formatting.
4. Special elements like tables and blockquotes are styled using predefined CSS classes.

### Example

Example document are provided in the `test_files` directory. If you try to convert the `complex_table.md` file, you will get.
```
user@PC:...path/build$ ./markdown_converter -i ../test_files/complex_table.md 

Output file not specified. Defaulting to output.html
Styles file not specified. Defaulting to styles.css
Your HTML document has been built successfully!
```

If you had run it with the `-v` set to 2 or higher, you would also get these warnings in your log file.

```
WARNING at Thu May  1 19:24:35 2025
: line 1: Unclosed asterisk signifying bold text - converting to plain text
WARNING at Thu May  1 19:24:35 2025
: line 1: Unclosed backtick signifying a code element: handling as plain text
WARNING at Thu May  1 19:24:35 2025
: line 3: Unclosed asterisk signifying bold text - converting to plain text
WARNING at Thu May  1 19:24:35 2025
: line 4: Unclosed asterisk signifying bold text - converting to plain text
WARNING at Thu May  1 19:24:35 2025
: line 5: Unclosed asterisk signifying bold text - converting to plain text
```
That's by design to show you the converter will warn you if it finds incorrect syntax and defaults to something reasonable. And hey, you can try to run it on this README as well!
