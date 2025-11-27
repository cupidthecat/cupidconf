# cupidconf lib

**Version:** 0.0.4  

**Overview:**  
cupidconf is a lightweight configuration parser library written in C. It is designed to load simple configuration files formatted as key-value pairs (using the `key = value` syntax) and supports multiple entries for a single key. The library automatically trims whitespace and ignores comments (lines starting with `#` or `;`), making it easy for both beginners and advanced developers to integrate configuration parsing into their applications.

**New in Version 0.0.4:**  
- **Home directory expansion (`~`)** support added. Configuration values starting with `~` or `~/` are automatically expanded to the user's home directory path.
  - For example, if your config has:
    ```
    config_dir = ~/.config/myapp
    cache_dir = ~/cache
    ```
    these will be automatically expanded to `/home/username/.config/myapp` and `/home/username/cache` (or the appropriate home directory path for your system).
  - The expansion uses the `HOME` environment variable. If `HOME` is not set, the value remains unchanged.

**New in Version 0.0.3:**  
- **`cupidconf_value_in_list`** function added to allow **wildcard matching in config values**.  
  - For example, if your config has:
    ```
    ignore = *.txt
    ignore = build_*
    ```
    you can call `cupidconf_value_in_list(conf, "ignore", "among.txt")` to check if `"among.txt"` matches any of those patterns.
- **Clarification on Wildcard Functions:**
  - **`cupidconf_get` / `cupidconf_get_list`**: These interpret **wildcards in the key**. For instance, if your config has keys like `foo.* = ...`, you can retrieve matches by using `cupidconf_get(conf, "foo.bar")`.
  - **`cupidconf_value_in_list`**: This checks if a **given string** matches **wildcard patterns stored in the config values**. Useful for "ignore" or "exclusion" lists.

**Intended Audience:**  
- C developers needing a simple yet effective configuration parser.
- Developers looking to integrate configurable parameters into their C applications.
- Anyone interested in learning how to implement a basic parser in C.

---

## Table of Contents
- [Installation](#installation)
- [Quick Start Guide](#quick-start-guide)
- [Usage Examples](#usage-examples)
- [Function Reference](#function-reference)
  - [`cupidconf_load`](#cupidconf_load)
  - [`cupidconf_get`](#cupidconf_get)
  - [`cupidconf_get_list`](#cupidconf_get_list)
  - [`cupidconf_value_in_list`](#cupidconf_value_in_list)
  - [`cupidconf_free`](#cupidconf_free)
- [Configuration Options](#configuration-options)
- [Advanced Topics](#advanced-topics)
- [FAQ / Troubleshooting](#faq--troubleshooting)

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

You can use wildcard patterns in keys to match multiple values. For example, consider the following configuration file:

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

### Example 4: Checking if a String Matches Wildcard Patterns in Config Values

If your configuration file contains wildcard patterns **as the values**, for example:

```
ignore = *.txt
ignore = build_*
ignore = *.log
```

Use `cupidconf_value_in_list` to check if a string (e.g., `"among.txt"`) matches **any** of these patterns:

```c
#include "cupidconf.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(void) {
    cupidconf_t *conf = cupidconf_load("config.conf");
    if (!conf) {
        fprintf(stderr, "Failed to load configuration.\n");
        return 1;
    }

    const char *filename = "among.txt";
    bool isIgnored = cupidconf_value_in_list(conf, "ignore", filename);

    if (isIgnored) {
        printf("%s is in the ignore list.\n", filename);
    } else {
        printf("%s is NOT ignored.\n", filename);
    }

    cupidconf_free(conf);
    return 0;
}
```

### Example 5: Using Home Directory Expansion (`~`)

You can use `~` in configuration values to reference the user's home directory. The library automatically expands `~` and `~/` to the full home directory path:

```
# Configuration file (config.conf)
config_dir = ~/.config/myapp
cache_dir = ~/cache
log_file = ~/logs/app.log
```

The following code demonstrates how the tilde expansion works:

```c
#include "cupidconf.h"
#include <stdio.h>

int main(void) {
    cupidconf_t *conf = cupidconf_load("config.conf");
    if (!conf) {
        fprintf(stderr, "Failed to load configuration.\n");
        return 1;
    }

    const char *config_dir = cupidconf_get(conf, "config_dir");
    const char *cache_dir = cupidconf_get(conf, "cache_dir");
    const char *log_file = cupidconf_get(conf, "log_file");

    if (config_dir) {
        printf("Config directory: %s\n", config_dir);
        // Output: Config directory: /home/username/.config/myapp
    }
    if (cache_dir) {
        printf("Cache directory: %s\n", cache_dir);
        // Output: Cache directory: /home/username/cache
    }
    if (log_file) {
        printf("Log file: %s\n", log_file);
        // Output: Log file: /home/username/logs/app.log
    }

    cupidconf_free(conf);
    return 0;
}
```

**Explanation:**
- Values starting with `~` or `~/` are automatically expanded to the user's home directory.
- The expansion uses the `HOME` environment variable.
- If `HOME` is not set, the value remains unchanged (e.g., `~` stays as `~`).

---

## Function Reference

### `cupidconf_load`

```c
cupidconf_t *cupidconf_load(const char *filename);
```

- **Description:**  
  Loads a configuration file specified by `filename`. The file must contain lines in the format `key = value`, and lines starting with `#` or `;` are treated as comments. Values starting with `~` or `~/` are automatically expanded to the user's home directory path.
  
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
  Retrieves all values associated with a given key from the configuration object. Keys may contain wildcard patterns (e.g., `"ignore.*"`), and this function will return all entries whose keys match that pattern.
  
- **Parameters:**  
  - `conf` (`cupidconf_t *`): The configuration object.
  - `key` (`const char *`): The key to search for (supports wildcards).
  - `count` (`int *`): A pointer to an integer that will be set to the number of values found.
  
- **Return Value:**  
  - Returns an array of pointers (read-only) to the values associated with the key.
  - Returns `NULL` if the key is not found or if memory allocation for the array fails.
  
- **Memory Management:**  
  The caller is responsible for freeing the returned array (using `free()`), but **should not free** the individual strings as they are managed by the configuration object.

---

### `cupidconf_value_in_list`

```c
bool cupidconf_value_in_list(cupidconf_t *conf, const char *key, const char *value);
```

- **Description:**  
  Checks if a given string (`value`) matches **any** of the wildcard patterns stored as **values** under the specified `key`. This is useful for implementing “ignore” or “exclusion” lists in your configuration.  
  - For example, if your config has:
    ```
    ignore = *.txt
    ignore = build_*
    ```
    then `cupidconf_value_in_list(conf, "ignore", "among.txt")` returns `true`.
  
- **Parameters:**  
  - `conf` (`cupidconf_t *`): The configuration object.
  - `key` (`const char *`): The key whose values represent wildcard patterns.
  - `value` (`const char *`): The string to test against these wildcard patterns.
  
- **Return Value:**  
  - `true` if `value` matches at least one pattern under `key`.
  - `false` if it does not match or if any parameter is `NULL`.

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

- **Home Directory Expansion:**  
  - Values starting with `~` or `~/` are automatically expanded to the user's home directory path.
  - The expansion uses the `HOME` environment variable. If `HOME` is not set, the value remains unchanged.
  - Examples: `~/.config` expands to `/home/username/.config`, `~/cache` expands to `/home/username/cache`.

- **Wildcard Support:**  
  - **Key-based Wildcards:** `cupidconf_get` and `cupidconf_get_list` interpret wildcards in the **config key**.  
  - **Value-based Wildcards:** `cupidconf_value_in_list` interprets wildcards in the **config value**.  

---

## Advanced Topics

### Extending cupidconf

- **Support for Sections:**  
  If you need configuration sections (e.g., `[section]`), consider enhancing the parser to recognize section headers and group key/value pairs accordingly.

- **Custom Parsing Logic:**  
  For applications with more complex configuration formats (such as nested configurations or value type conversions), you can extend the parser functions or write additional helper functions that post-process the raw string values.

- **Wildcard Matching:**  
  - By default, the library uses `fnmatch` for wildcard matching.
  - You can disable or replace this with your own pattern-matching logic if desired.

---

## FAQ / Troubleshooting

**Q:** *My application crashes after loading a configuration file. What could be the issue?*  
**A:** Ensure that the configuration file is correctly formatted. Lines without an `=` separator are skipped, but malformed lines may indicate issues with file encoding or unexpected characters. Also, verify that you are checking for `NULL` returns from API functions.

**Q:** *Why do I need to define `_POSIX_C_SOURCE`?*  
**A:** The library uses POSIX functions (e.g., `fgets`, `strdup`, `fnmatch`, etc.) that require `_POSIX_C_SOURCE` to be defined for proper feature support on some systems.

**Q:** *How do I free the memory allocated by `cupidconf_get_list`?*  
**A:** You only need to free the array pointer returned by `cupidconf_get_list`. Do **not** free the individual strings, as they are managed by the configuration object.

---
