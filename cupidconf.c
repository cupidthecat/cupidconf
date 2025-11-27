#define _POSIX_C_SOURCE 200809L

#include "cupidconf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <wordexp.h>

/* Helper: Check if a key matches a wildcard pattern. */
static int match_wildcard(const char *pattern, const char *key) {
    return fnmatch(pattern, key, 0) == 0;
}

/* Helper: Trim leading and trailing whitespace in place. */
static char *trim_whitespace(char *str) {
    char *end;

    /* Trim leading space */
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  /* All spaces? */
        return str;

    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    /* Write new null terminator */
    *(end+1) = '\0';

    return str;
}

/* Helper: Expand ~ to home directory. Returns a newly allocated string that must be freed. */
static char *expand_tilde(const char *path) {
    if (!path || path[0] != '~')
        return strdup(path);

    /* Check if it's ~/ or just ~ */
    if (path[1] == '/' || path[1] == '\0') {
        const char *home = getenv("HOME");
        if (!home) {
            /* If HOME is not set, return the original path */
            return strdup(path);
        }

        size_t home_len = strlen(home);
        size_t path_len = strlen(path);
        size_t new_len = home_len + path_len; /* path_len includes the ~, so this is correct */

        char *expanded = malloc(new_len + 1);
        if (!expanded)
            return NULL;

        if (path[1] == '/') {
            /* ~/path -> /home/user/path */
            snprintf(expanded, new_len + 1, "%s%s", home, path + 1);
        } else {
            /* ~ -> /home/user */
            strcpy(expanded, home);
        }

        return expanded;
    }

    /* ~username not supported, return as-is */
    return strdup(path);
}

/* Create a new entry and add it to the linked list. */
static int add_entry(cupidconf_t *conf, const char *key, const char *value) {
    cupidconf_entry *entry = malloc(sizeof(cupidconf_entry));
    if (!entry) return -1;

    entry->key = strdup(key);
    entry->value = strdup(value);
    entry->next = NULL;

    if (!entry->key || !entry->value) {
        free(entry->key);
        free(entry->value);
        free(entry);
        return -1;
    }

    // Add to the front of the list for simplicity.
    entry->next = conf->entries;
    conf->entries = entry;
    return 0;
}

cupidconf_t *cupidconf_load(const char *filename) {
    /* Expand ~ in filename using wordexp() */
    wordexp_t p;
    const char *expanded_filename = filename;
    int ret = wordexp(filename, &p, 0);
    if (ret == 0 && p.we_wordc > 0) {
        /* Use the first expanded word (should be only one for a filename) */
        expanded_filename = p.we_wordv[0];
    }

    FILE *fp = fopen(expanded_filename, "r");
    if (!fp) {
        if (ret == 0) {
            wordfree(&p);
        }
        perror("cupidconf_load: fopen");
        return NULL;
    }

    if (ret == 0) {
        wordfree(&p);
    }

    cupidconf_t *conf = malloc(sizeof(cupidconf_t));
    if (!conf) {
        fclose(fp);
        return NULL;
    }
    conf->entries = NULL;

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        /* Remove newline if present */
        line[strcspn(line, "\n")] = '\0';

        /* Trim whitespace */
        char *trimmed = trim_whitespace(line);

        /* Skip empty lines and comments (lines starting with '#' or ';') */
        if (trimmed[0] == '\0' || trimmed[0] == '#' || trimmed[0] == ';')
            continue;

        /* Find the '=' separator */
        char *equal_sign = strchr(trimmed, '=');
        if (!equal_sign) {
            /* No '=' on this line; skip or handle error */
            continue;
        }

        /* Split key and value. */
        *equal_sign = '\0';
        char *key = trim_whitespace(trimmed);
        char *value = trim_whitespace(equal_sign + 1);

        /* Handle inline comments (starting with # or ;) */
        char *comment_start = value;
        while (*comment_start) {
            if (*comment_start == '#' || *comment_start == ';') {
                *comment_start = '\0';  // Terminate the value at the comment start
                value = trim_whitespace(value);  // Re-trim in case there was space before comment
                break;
            }
            comment_start++;
        }

        /* Expand ~ to home directory */
        char *expanded_value = expand_tilde(value);
        if (!expanded_value) {
            // On allocation failure, free everything and return NULL.
            cupidconf_free(conf);
            fclose(fp);
            return NULL;
        }

        if (add_entry(conf, key, expanded_value) != 0) {
            // On allocation failure, free everything and return NULL.
            free(expanded_value);
            cupidconf_free(conf);
            fclose(fp);
            return NULL;
        }

        free(expanded_value);
    }

    fclose(fp);
    return conf;
}

const char *cupidconf_get(cupidconf_t *conf, const char *key) {
    if (!conf || !key)
        return NULL;

    for (cupidconf_entry *entry = conf->entries; entry != NULL; entry = entry->next) {
        if (match_wildcard(key, entry->key))
            return entry->value;
    }
    return NULL;
}

char **cupidconf_get_list(cupidconf_t *conf, const char *key, int *count) {
    if (!conf || !key || !count)
        return NULL;

    int cnt = 0;
    for (cupidconf_entry *entry = conf->entries; entry != NULL; entry = entry->next) {
        if (match_wildcard(key, entry->key))
            cnt++;
    }

    if (cnt == 0) {
        *count = 0;
        return NULL;
    }

    char **values = malloc(sizeof(char *) * cnt);
    if (!values) {
        *count = 0;
        return NULL;
    }

    int idx = 0;
    for (cupidconf_entry *entry = conf->entries; entry != NULL; entry = entry->next) {
        if (match_wildcard(key, entry->key))
            values[idx++] = entry->value;
    }

    *count = cnt;
    return values;
}

bool cupidconf_value_in_list(cupidconf_t *conf, const char *key, const char *value) {
    if (!conf || !key || !value)
        return false;

    // Iterate over all entries in the config
    for (cupidconf_entry *entry = conf->entries; entry != NULL; entry = entry->next) {
        // If the entry's key exactly matches the requested key
        if (strcmp(entry->key, key) == 0) {
            // Now check if the 'value' you passed in matches
            // this entry's pattern (stored as entry->value).
            //
            // Example: entry->value == "*.txt"
            //          value        == "among.txt"
            if (fnmatch(entry->value, value, 0) == 0) {
                return true;  // We found a match, return immediately
            }
        }
    }

    // No match found
    return false;
}

void cupidconf_free(cupidconf_t *conf) {
    if (!conf)
        return;
    cupidconf_entry *entry = conf->entries;
    while (entry) {
        cupidconf_entry *next = entry->next;
        free(entry->key);
        free(entry->value);
        free(entry);
        entry = next;
    }
    free(conf);
}
