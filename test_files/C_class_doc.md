> These notes are copied from my [C programming course](https://github.com/devnull-cz/c-prog-lang/blob/notes/2025/06.md).

# Strings and String Constants

We already know what a
[string](https://github.com/devnull-cz/c-prog-lang/blob/master/modules/string.md)
is.  It is an array of non-null characters terminated by a null byte.  The null
character is part of the string.

- A *string literal* is a sequence of zero or more characters enclosed in double-quotes, as in `"xxx"`.  It is also called a *string constant*.
	- Do not confuse it with a *character constant*, e.g `'A'`, as it uses single quotes.  In contrast to Python, for example, single and double quotes are two different things in C.

- **A string constant internally initializes an array of characters, with a null character appended.**

- That also means that a string literal may include multiple null characters, thus defining multiple strings.  Note that a *string literal* and a *string* are two different things even that people mostly both call a string.

```
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

```
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

```
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

```
char s[] = { 'h', 'e', 'l', 'l', 'o', '\0' };
```

- So, the following is still a properly terminated string but do not use it like that, it is confusing:

```
char s[10] = { 'h', 'e', 'l', 'l', 'o' };
```

Anyway, you would probably never do it this way.  Just use `= "hello"`.

[array-fill.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/array-fill.c)
[array-fill2.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/array-fill2.c)

- We already know that a string is printed via `printf()` using the `%s` conversion specifier:

```
printf("%s\n", "hello, world");
```

- Experiment with what `%5s` and `%.5s` do (use any reasonable number)

[string-format.c](https://github.com/devnull-cz/c-prog-lang/blob/master/src/string-format.c)
