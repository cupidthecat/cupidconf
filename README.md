# CupidConf

CupidConf is a C parser for ordered `key = value` configuration files. Include `cupidconf.h` and compile `cupidconf.c` with your application, or build the static library with `make`.

## Build and test

From the source checkout:

```sh
make
make test
```

The parser uses POSIX APIs. Define `_POSIX_C_SOURCE=200809L` when compiling it directly. Tests cover literal filenames, home expansion, long lines, duplicate order, wildcard queries, and read failures.

## File syntax

```ini
path = /usr/local/bin
path = /usr/bin
color.background = dark
ignore = *.tmp
cache = ~/.cache/myapp
```

Whitespace around keys and values is trimmed. Empty lines and lines beginning with `#` or `;` are ignored. Those characters also begin inline comments in values. Quoting and escaping are not part of this format. Lines without `=` and empty keys are skipped.

A filename may contain spaces, wildcard characters, environment references, or shell syntax; they are literal and never execute commands. A leading `~` or `~/` in filenames and values expands using `HOME`. `~user` is literal. If `HOME` is unset, the original spelling is retained.

## Queries and ownership

- `cupidconf_load(path)` returns a configuration object, or `NULL` on open, read, allocation, close, or embedded-NUL errors.
- `cupidconf_get(conf, pattern)` returns the first matching value in file order. Wildcards belong in the query argument: `color.*` matches the stored key `color.background`.
- `cupidconf_get_list(conf, pattern, &count)` returns all matching values in file order. It sets `count` to zero when no result is returned. Pass a non-null count pointer.
- `cupidconf_value_in_list(conf, key, value)` uses an exact key and checks the supplied value against wildcard patterns stored under it. With the example above, `cupidconf_value_in_list(conf, "ignore", "draft.tmp")` is true.
- `cupidconf_free(conf)` releases the object and its stored strings.

Query strings are borrowed from the configuration and remain valid until it is freed. Free the array returned by `cupidconf_get_list`, but keep its individual strings owned by the configuration.

The public declarations are in [cupidconf.h](https://github.com/cupidthecat/cupidconf/blob/main/cupidconf.h).

## C example

```c
#include "cupidconf.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    cupidconf_t *conf = cupidconf_load(argc > 1 ? argv[1] : "config.conf");
    if (!conf) { perror("configuration"); return 1; }
    const char *color = cupidconf_get(conf, "color.*");
    if (color) puts(color);
    int count = 0;
    char **paths = cupidconf_get_list(conf, "path", &count);
    for (int i = 0; i < count; i++) puts(paths[i]);
    free(paths);
    cupidconf_free(conf);
    return 0;
}
```

```sh
cc -std=c11 -D_POSIX_C_SOURCE=200809L -I. example.c cupidconf.c -o example
./example config.conf
```
