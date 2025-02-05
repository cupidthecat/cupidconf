# cupidconf lib

**Version:** 0.0.2

**Overview:**  
cupidconf is a lightweight configuration parser library written in C. It is designed to load simple configuration files formatted as key-value pairs (using the `key = value` syntax) and supports multiple entries for a single key. The library automatically trims whitespace and ignores comments (lines starting with `#` or `;`), making it easy for both beginners and advanced developers to integrate configuration parsing into their applications.

**New in Version 0.0.2:**  
- Added support for **wildcard patterns** in keys (e.g., `ignore = /path/to/files/*.txt`).

**Intended Audience:**  
- C developers needing a simple yet effective configuration parser.
- Developers looking to integrate configurable parameters into their C applications.
- Anyone interested in learning how to implement a basic parser in C.

---

## Table of Contents
- [Installation](#installation)
- [Quick Start Guide](#quick-start-guide)
- [Usage Examples](#usage-examples)
- [Function Reference](#Function-reference)
  - [`cupidconf_load`](#cupidconf_load)
  - [`cupidconf_get`](#cupidconf_get)
  - [`cupidconf_get_list`](#cupidconf_get_list)
  - [`cupidconf_free`](#cupidconf_free)
- [Configuration Options](#configuration-options)
- [Advanced Topics](#advanced-topics)
- [FAQ / Troubleshooting](#faq--troubleshooting)
- [Contribution Guidelines](#contribution-guidelines)
- [License and Support](#license-and-support)

---

## Installation

### Prerequisites
- **Compiler:** A POSIX-compliant C compiler (e.g., `gcc` or `clang`).
- **POSIX Feature Test Macro:** The library requires `_POSIX_C_SOURCE` defined to at least `200809L`.

### Build Instructions
1. **Download the source code** (typically, `cupidconf.h` and `cupidconf.c`).
2. **Compile the library:**  
   Use the following command to compile the library along with your application:
   ```bash
   gcc -D_POSIX_C_SOURCE=200809L -o myapp myapp.c cupidconf.c
   ```
3. **Include the header:**  
   Ensure that your source files include the header:
   ```c
   #include "cupidconf.h"
   ```

> **Note:** No external dependencies are required.

---

## Quick Start Guide

Below is a minimal working example demonstrating how to load a configuration file and retrieve a value using CupidConf:

```c
#include "cupidconf.h"
#include <stdio.h>

int main(void) {
    // Load the configuration file "config.conf"
    cupidconf_t *conf = cupidconf_load("config.conf");
    if (!conf) {
        fprintf(stderr, "Error: Unable to load configuration file.\n");
        return 1;
    }

    // Retrieve a configuration value for the key "username"
    const char *username = cupidconf_get(conf, "username");
    if (username) {
        printf("Username: %s\n", username);
    } else {
        printf("Key 'username' not found in configuration.\n");
    }

    // Free the configuration object and all allocated memory
    cupidconf_free(conf);
    return 0;
}
```

**Explanation:**
- The example loads a configuration file named `config.conf`.
- It then retrieves the value associated with the key `"username"`.
- Finally, it frees the allocated configuration structure.

---

## Usage Examples

### Example 1: Loading and Reading a Single Key

Suppose you have a configuration file (`config.conf`) with the following contents:

```
# Application Configuration
username = johndoe
timeout = 30
```

The following code demonstrates how to read these values:

```c
#include "cupidconf.h"
#include <stdio.h>

int main(void) {
    cupidconf_t *conf = cupidconf_load("config.conf");
    if (!conf) {
        fprintf(stderr, "Failed to load configuration.\n");
        return 1;
    }

    const char *username = cupidconf_get(conf, "username");
    const char *timeout = cupidconf_get(conf, "timeout");

    if (username) {
        printf("Username: %s\n", username);
    }
    if (timeout) {
        printf("Timeout: %s\n", timeout);
    }

    cupidconf_free(conf);
    return 0;
}
```

### Example 2: Retrieving Multiple Values for a Key

If your configuration file contains multiple entries with the same key, for example:

```
path = /usr/local/bin
path = /usr/bin
path = /bin
```

You can retrieve all values for `path` using `cupidconf_get_list`:

```c
#include "cupidconf.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    cupidconf_t *conf = cupidconf_load("config.conf");
    if (!conf) {
        fprintf(stderr, "Failed to load configuration.\n");
        return 1;
    }

    int count = 0;
    char **paths = cupidconf_get_list(conf, "path", &count);
    if (paths) {
        printf("Found %d path(s):\n", count);
        for (int i = 0; i < count; i++) {
            printf("  %s\n", paths[i]);
        }
        free(paths);  // Free the array (note: the strings are managed by the config object)
    } else {
        printf("No paths found.\n");
    }

    cupidconf_free(conf);
    return 0;
}
```

### Example 3: Using Wildcard Patterns in Keys

You can now use wildcard patterns in keys to match multiple values. For example, consider the following configuration file:

```
ignore = /home/user/project/*.txt
ignore = /home/user/project/temp/*
verbose = true
```

The following code demonstrates how to retrieve values using wildcard patterns:

```c
#include "cupidconf.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    cupidconf_t *conf = cupidconf_load("config.conf");
    if (!conf) {
        fprintf(stderr, "Failed to load configuration.\n");
        return 1;
    }

    int count = 0;
    char **ignores = cupidconf_get_list(conf, "ignore", &count);
    if (ignores) {
        printf("Found %d ignore pattern(s):\n", count);
        for (int i = 0; i < count; i++) {
            printf("  %s\n", ignores[i]);
        }
        free(ignores);
    } else {
        printf("No ignore patterns found.\n");
    }

    cupidconf_free(conf);
    return 0;
}
```

**Explanation:**
- The `ignore` key uses wildcard patterns (`*.txt` and `temp/*`) to match multiple files or directories.
- The `cupidconf_get_list` function retrieves all values for the `ignore` key, including wildcard patterns.

---

## Function Reference

### `cupidconf_load`

```c
cupidconf_t *cupidconf_load(const char *filename);
```

- **Description:**  
  Loads a configuration file specified by `filename`. The file must contain lines in the format `key = value`, and lines starting with `#` or `;` are treated as comments.
  
- **Parameters:**  
  - `filename` (`const char *`): The path to the configuration file.
  
- **Return Value:**  
  - Returns a pointer to a `cupidconf_t` object on success.
  - Returns `NULL` on failure (e.g., if the file cannot be opened or if memory allocation fails).

---

### `cupidconf_get`

```c
const char *cupidconf_get(cupidconf_t *conf, const char *key);
```

- **Description:**  
  Retrieves the value associated with the given key from the configuration object. If the key appears more than once, the first occurrence is returned.
  
- **Parameters:**  
  - `conf` (`cupidconf_t *`): The configuration object returned by `cupidconf_load`.
  - `key` (`const char *`): The key for which the value is requested.
  
- **Return Value:**  
  - Returns a read-only string containing the value if found.
  - Returns `NULL` if the key is not present or if either argument is `NULL`.

---

### `cupidconf_get_list`

```c
char **cupidconf_get_list(cupidconf_t *conf, const char *key, int *count);
```

- **Description:**  
  Retrieves all values associated with a given key from the configuration object.
  
- **Parameters:**  
  - `conf` (`cupidconf_t *`): The configuration object.
  - `key` (`const char *`): The key to search for.
  - `count` (`int *`): A pointer to an integer that will be set to the number of values found.
  
- **Return Value:**  
  - Returns an array of pointers (read-only) to the values associated with the key.
  - Returns `NULL` if the key is not found or if memory allocation for the array fails.
  
- **Memory Management:**  
  The caller is responsible for freeing the returned array (using `free()`), but **should not free** the individual strings as they are managed by the configuration object.

---

### `cupidconf_free`

```c
void cupidconf_free(cupidconf_t *conf);
```

- **Description:**  
  Frees the configuration object along with all associated memory (including key/value entries).
  
- **Parameters:**  
  - `conf` (`cupidconf_t *`): The configuration object to be freed.
  
- **Return Value:**  
  - This function does not return a value.

---

## Configuration Options

While CupidConf itself does not expose runtime configuration options via environment variables or configuration settings, it expects configuration files to follow a simple format:

- **File Format:**  
  - Each configuration line should be in the form:  
    ```
    key = value
    ```
  - Leading and trailing whitespace around both the key and value is automatically trimmed.
  - Empty lines and lines beginning with `#` or `;` are ignored, allowing you to include comments.

- **Wildcard Support:**  
  - Keys can now include wildcard patterns (e.g., `ignore = /path/to/files/*.txt`).
  - Use `cupidconf_get_list` to retrieve all values matching a wildcard pattern.

---

## Advanced Topics

### Extending cupidconf

- **Support for Sections:**  
  If you need configuration sections (e.g., `[section]`), consider enhancing the parser to recognize section headers and group key/value pairs accordingly.

- **Custom Parsing Logic:**  
  For applications with more complex configuration formats (such as nested configurations or value type conversions), you can extend the parser functions or write additional helper functions that post-process the raw string values.

- **Wildcard Matching:**  
  - The library now supports wildcard patterns in keys using the `fnmatch` function.
  - This is particularly useful for filtering or ignoring files/directories in applications like `cupid-clip`.

---

## FAQ / Troubleshooting

**Q:** *My application crashes after loading a configuration file. What could be the issue?*  
**A:** Ensure that the configuration file is correctly formatted. Lines without an `=` separator are skipped, but malformed lines may indicate issues with file encoding or unexpected characters. Also, verify that you are checking for `NULL` returns from API functions.

**Q:** *Why do I need to define `_POSIX_C_SOURCE`?*  
**A:** The library uses POSIX functions (e.g., `fgets`, `strdup`, and others) that require `_POSIX_C_SOURCE` to be defined for proper feature support on some systems.

**Q:** *How do I free the memory allocated by `cupidconf_get_list`?*  
**A:** You only need to free the array pointer returned by `cupidconf_get_list`. Do not attempt to free the individual strings, as they are managed by the configuration object.

---
