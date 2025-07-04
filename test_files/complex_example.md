# Strings and String Constants

We already know what a
[string](https://github.com/devnull-cz/c-prog-lang/blob/master/modules/string.md)
is.  It is a sequence of non-null characters terminated by a null byte. The null character is part of the string.

- A *character string literal* is a sequence of zero or more characters enclosed in double-quotes, as in `"xxx"`.  It is also called a *string constant*.  We may often call it just a *string literal*.
	- Do not confuse it with a *character constant*, e.g. `'A'`, as it uses single quotes.  In contrast to Python, for example, single and double quotes are two different things in C.
- **A string constant internally initializes an array of characters, with a null character appended.**  See C99, 6.4.5 String literals, paragraph 5.
- That also means that a string literal may include multiple null characters, thus defining multiple strings.  Note that a *string literal* and a *string* are two different things even that people mostly both call a string.

```C
$ cat main.c
#include <stdio.h>

int
main(void)
{
	printf("%s\n", "hello\0world.");
}
$ cc main.c
$ ./a.out
hello
```

- The `'\0'` is just a special case of octal representation `'\ooo'`, called *octal escape sequence*, where `o` is an octal figure (0-7).  You can use any ASCII character like that.

	- The full syntax is `'\o'`, `'\oo'`, or `'\ooo'`.  `\6` is another example of an octal character sequence while `\8` is illegal.

```C
$ cat main.c
/*
 * The code assumes that our environment uses ASCII but that is not mandatory.
 * See 5.2.1 Character Sets in the C99 standard for more information.
 */
#include <stdio.h>

int
main(void)
{
	/* Check the ascii manual page that 132 is 'Z', and 71 is '9'. */
	printf("%c\n", '\132');
	printf("%c\n", '\71');
	/* Will print "a<tab>b" */
	printf("a\11b\n");
}
$ ./a.out
Z
9
a	b
```

- A string constant may be used to initialize a `char` array and usually that is how the string initialization is used (in contrast to `{ 'a', 'b', ... }`).

```C
int
main(void)
{
	char s[] = "hello, world.";

	printf("%s\n", s);
}
```

```
$ gcc -Wall -Wextra main.c
$ ./a.out
hello, world.
```

- Remember that if `{}` is used for the initialization, **you must add the terminating zero yourself** unless you use the size of the array and the string was shorter (in which case the rest would be initialized to zero as usual):

```C
char s[] = { 'h', 'e', 'l', 'l', 'o', '\0' };
```

- So, the following is still a properly terminated string but do not use it like that, it is confusing:

```C
char s[10] = { 'h', 'e', 'l', 'l', 'o' };
```

Anyway, you would probably never do it this way.  Just use `= "hello"`.

:eyes: [array-fill.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/array-fill.c)
:eyes: [array-fill2.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/array-fill2.c)

- We already know that a string is printed via `printf()` using the `%s` conversion specifier:

```C
printf("%s\n", "hello, world");
```

- Experiment with what `%5s` and `%.5s` do (use any reasonable number)

:eyes: [string-format.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/string-format.c)

- To include a double quote in a string literal, you must escape it with `\`. You may or may not escape a single quote within a string literal (C99, 6.4.5 String literals, paragraph 3).  The following must produce the same output:

```C
printf("%s\n", "\"'");
printf("%s\n", "\"\'");
```


# Warm-up
## Create a special string

- :wrench: write code to print the output further below using a single `printf` with a single conversion specification like this:

```
printf("%s\n", str);
```

- Your code must not have `x` in it.  The only string literal allowed in your code must be `"%s\n"` in `printf`.  If you use `grep` on your code, its output must be empty as follows:

```
$ grep x main.c
$
$ grep '".*"' main.c | grep -v 'printf.*%s'
$
```

When you compile your code, the expected output follows (`<tab>` means a literal tabelator character):

```
hello
world
<tab>x
'
```

:key: [create-string.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/solutions/create-string.c)


# The `for` loop

Formally defined as:

```
for (<init>; <predicate>; <update>)
	<body>
```

It is the same as:

```
<init>
while (<predicate>) {
	<body>
	<update>
}
```

Using the `for` loop is often easier and more readable than a while loop.

Example:
```C
int i;
for (i = 0; i < 10; ++i) {
	printf("%d\n", i);
}
```

Or in C99:

```C
for (int i = 0; i < 10; ++i) {
	printf("%d\n", i);
}
```

- The `break` statement terminates execution of the loop (i.e. it jumps right **after** the enclosing `}`)
- With the `continue` statement, the execution jumps to the end of the loop body (i.e. it jumps **at** the enclosing `}`) but the `<update>` part is executed. The execution then continues with the `<predicate>` test.  Example:

```C
for (int i = 0; i < 3; ++i) {
	printf("%d\n", i);
	continue;
	putchar('a');
}
```
actual output:
```
$ ./a.out
0
1
2
$
```

- Both `break` and `continue` statements relate to the innermost loop they are executed in.

:eyes: [for.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/for.c)

:wrench: Print a subset of an ASCII table for characters `a` to `z` using `for` cycle:

```
$ ./a.out
Dec	Oct	Char
97	141	a
98	142	b
99	143	c
100	144	d
101	145	e
...
...
```

:key: [print-ascii-table-subset.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/solutions/print-ascii-table-subset.c)

:wrench: Compute minimum of averages of lines in 2-D integer array (of arbitrary dimensions) that have positive values (if there is negative value in given line, do not use the line for computation).

:key: [2darray-min-avg.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/solutions/2darray-min-avg.c)

# Expressions

> "In mathematics, an expression or mathematical expression is a finite combination of symbols that is well-formed according to rules that depend on the context."

In C99, section 6.5 expressions:

>"An expression is a sequence of operators and operands that specifies computation of a value, or that designates an object or a function, or that generates side effects, or that performs a combination thereof."

In C, expressions are, amongst others:

- identifiers
- constants
- string literals
- expressions in parentheses
- arithmetic expressions
- relational expressions
- function calls
- `sizeof`
- assignments
- ternary expressions

http://en.cppreference.com/w/c/language/expressions

In C99, an expression can produce results (`2 + 2` gets `4`) or generate [side effects](https://github.com/devnull-cz/c-prog-lang/blob/master/modules/side-effect.md)(e.g. upon evaluation, an expression `printf("foo")` sends a string literal to the standard output as a side effect).

# Statements

Statements are (list not complete): expressions, selections (`if`, `switch` (not introduced yet)), `{}` blocks (known as *compounds*), iterations (`while`, `for`), and jumps (`goto` (not introduced yet), `continue`, `break`, `return`).

http://en.cppreference.com/w/c/language/statements

Basically, statements are pieces of code executed in sequence. The function body is a compound statement. The compound statement (a.k.a. *block*) is a sequence of statements and declarations.  Blocks can be nested.  Blocks inside a function
body can be used for variable reuse but should be used for that with care, if at all.

A semicolon is usually not used after a compound statement but it is allowed. The following is valid code and essentially does nothing:

```C
int
main(void)
{
	{ }
	{ };
}
```

**A declaration is not a statement** (there are subtle consequences, we can show
them later).

:eyes: [null-statement.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/null-statement.c)
:eyes: [compound-statement.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/compound-statement.c)

Some statements must end with `;`.  For example, expression statements.  The following are all valid expression statements.  They do not make much sense though and may generate a warning about an unused result in some compilers.

```C
/* this one is not a statement */
char c;

/* these are all expression statements */
c;
1 + 1;
1000;
"hello";
```

:eyes: [expression-statement.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/expression-statement.c)

:wrench: Make the warning an error with your choice of compiler (would be a variant of `-W` in GCC)

# Pointers

A *pointer* is a reference to an entity, e.g. to an object of type `int`.  By itself it is an object whose value is the reference (= memory address) to the entity.

Motivation:

- Memory allocation / shared memory.
- Protocol buffer parsing.
- Pointer arithmetics.
- The value stored in a pointer object is the memory address storing the given pointer type.
  - Declared as follows, for example:

```C
int *p; // pointer to an int
```

- Note on style:

```C
int * p;
int *p;  // preferred in this lecture
```

- A pointer is always associated with a type.
- To access a value of the object the pointer points to, use a *dereference* operator `*`:

```C
printf("%d", *p);
```

- The dereference is also used to write a value to the object the pointer points to:

```C
*p = 5;
```

- You may assign a value to a pointer being declared:

```C
int i = 5;
int *p = &i;	// assign an address of 'i'
```

- Good practice is to prefix pointers with the 'p' letter, e.g.:

```C
int val = 5;
int *pval = &val;
```

- The `&` above is an *address-of* operator and gets the address of the object (i.e. the memory address of where the value is stored).

- The pointer itself is obviously stored in memory too (it is just another object as already said).  With the declarations above, it looks as follows:

```
     pval
     +---------------+
     |     addr2     |
     +---------------+        val
     ^                        +-------+
     |                        | 5     |
   addr1                      +-------+
                              ^
                              |
                            addr2
```

- The size of the pointer depends on the architecture and the way the program was compiled (see `-m32` / `-m64` command line switches of gcc)
- `sizeof (p)` returns the amount of memory to store the address of the object the pointer points to.  E.g. on 64 bit architecture, the pointer size is usually 8 bytes.
- `sizeof (*p)` returns the amount needed to store the object the pointer points to.  If a pointer points to an `int`, then `sizeof (*p)` will be usually 4 bytes.

:wrench: Write a program to print:

   - Address of the pointer.
   - The address where it points to.
   - The value of the pointed to variable.

Use the `%p` formatting for the first two.

:key: [ptr-basics.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/solutions/ptr-basics.c)

## Null pointer

- The real danger with pointers is that invalid memory access usually results in a crash where the program is terminated by the kernel.
- We can assign a number directly to a pointer even that should trigger a warning.  We will get to casting and how to fix that later.

```C
int *p = 0x1234;
```

- Zero pointer value cast to `void *` is called *a null pointer constant* and is also defined as a macro `NULL` [in the C specification.](https://github.com/devnull-cz/c-prog-lang/blob/master/modules/c99-standard.md) If a null pointer constant is converted to a pointer type such a pointer is called a *null pointer* and it is guaranteed in C not to point to any object or a function.  In other words, dereferencing a null pointer is guaranteed to terminate the program.
  - We will talk about `void` later but it is **a type that has no values**.  A pointer to such a type can never be dereferenced (as that would give us a value of the type but we just defined such a value does not exist).
  - This is because zero address is left unmapped on purpose, or a page that cannot be accessed maps to the address.
  - The C specification says that the macro `NULL` must be defined in `<stddef.h>`

```
$ cat main.c
int
main(void)
{
        void *p = 0;

        printf("%p\n", *p);
}

$ cc main.c
main.c: In function ‘main’:
main.c:8:24: warning: dereferencing ‘void *’ pointer
    8 |         printf("%p\n", *p);
      |                        ^~
main.c:8:24: error: invalid use of void expression
```

:wrench: Create a null pointer (i.e. assign `0` to a pointer) to a non-void type (say an `int`) and try to read from and/or write to it, i.e. dereference such a pointer.

:key: [null-ptr.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/solutions/null-ptr.c)

## Basic operations

- Note the difference:

```C
int i;
int *p = &i;
```

vs.

```C
int i;
int *p;

// set value of the pointer (i.e. the address where it points to).
p = &i;
```

- Operator precedence gotcha:

```C
*p		// value of the pointed to variable
*p + 1		// the value + 1
*(p + 1)	// the value on the address + 1 (see below for what it means)
```

- Pointers of the same type can be assigned to one another.

```C
int *p = 13;	// probably will trigger a warning
int *p2 = p;
```

- Note that the `&` address-of operator is possible to use only on objects that may be assigned values to.  The following is invalid then:

```C
p = &(i + 1);	// we cannot assign to (i + 1) like this: "(i + 1) = 1;"
```

:eyes: [ptr-lvalue.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/ptr-lvalue.c)

- Store value to the address pointed to by a pointer:

```C
*p = 1;
```

## Changing pointer values:

Pointers can be moved forward and backward:

```C
p = p + 1;
p++;
p--;
```

**The pointer is moved by the amount of underlying data type when using arithmetics.**

:wrench: Create a pointer to an `int`, print its value, create a new pointer that points to `p + 1`. See what is the difference between the 2 pointers.  To print a pointer value, use the `%p` conversion specifier.

:key: [ptr-diff.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/solutions/ptr-diff.c)

## Operator gotchas

- The `*` dereference operator has higher precedence than `+`:

```C
i = *p + 1;
```

is equal to:

```C
i = (*p) + 1;
```

- Postfix `++` has higher precedence than `*`:

```C
i = *p++;
```

is evaluated as `*(p++)` but it still does the following because `++` is used in postfix mode, ie. the value of expression `p++` is `p`:

```C
i = *p; p++;
```


# `err`() family of functions

- Not defined by any standard however handy.
- Present in BSD, glibc (= Linux distros), Solaris, macOS, etc.
- Use the `<err.h>` include, see the err(3) man page.
- This is especially handy when working with I/O.

Instead of writing:

```C
if (error) {
	fprintf(stderr, "Error: %s\n", strerror(errno));
	exit(1);
}
```

Use the following.  It will append ": " string and then the error message itself, based on the value of `errno`.

```C
if (error)
     err(1, "Error");
```

- Notice that a newline is inserted automatically at the end of the error message.
- For functions that do not modify the `errno` value, use the *x* variant:

```C
if (some_error)
	errx(1, "ERROR: %d", some_error);
```

There's also `warn()`/`warnx()` that do not exit the program.


# :wrench: Home assignment

Note that home assignments are entirely voluntary but writing code is the only way to learn a programming language.

## Extend `tr -d`

- :wrench: extend the program
:eyes: [tr-d-chars.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/tr-d-chars.c) from last time to translate character input, e.g.
```
tail /etc/passwd | tr 'abcdefgh' '123#'
```
  - use character arrays defined with string literals to represent the 2 strings
    - see the tr(1) man page on what needs to happen if the 1st string is longer than the 2nd
      - do not store the expanded 2nd string as literal in your program !
        (i.e. not `123#####`)

:key: [tr.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/solutions/tr.c)

:wrench: (bonus): refactor the code into a function(s)

- remember that arrays are passed into a function as a pointer (to be explained soon, not needed now) which can be used inside the function with an array subscript.
