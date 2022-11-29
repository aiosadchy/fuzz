#ifndef FUZZ_INCLUDE_FUZZ_FUZZ_H
#define FUZZ_INCLUDE_FUZZ_FUZZ_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct FuzzConfig {
    struct {
        size_t initial_buffer_size;
        double buffer_growth_rate;
    } text_data;

    struct {
        size_t initial_buffer_size;
        double buffer_growth_rate;
    } string_array;

} FuzzConfig;

typedef struct FuzzTextData {
    char *data;
    size_t size;
    size_t allocated_size;
} FuzzTextData;

typedef struct FuzzStringView {
    const char *begin;
    const char *end;
} FuzzStringView;

typedef struct FuzzStringArray {
    FuzzStringView *data;
    size_t size;
    size_t allocated_size;
} FuzzStringArray;

void fuzz_init_config(FuzzConfig *config);
void fuzz_get_config(FuzzConfig *config);
void fuzz_set_config(FuzzConfig *config);
FuzzConfig *fuzz_get_config_location(void);

void fuzz_init_text_data(FuzzTextData *text_data);
int fuzz_read_text_data(FuzzTextData *text_data, FILE *file);
void fuzz_clear_text_data(FuzzTextData *text_data);

void fuzz_init_string_array(FuzzStringArray *string_array);
int fuzz_split_into_string_array(
    FuzzStringArray *string_array,
    const FuzzTextData *text_data,
    const char *separator
);
void fuzz_clear_string_array(FuzzStringArray *string_array);


#ifdef FUZZ_IMPLEMENTATION

inline static int fuzz_detail_grow_array(
    void **array,
    size_t *allocated_size,
    size_t object_size,
    size_t initial_buffer_size,
    double growth_rate
) {
    if (array == NULL || allocated_size == NULL || object_size == 0) {
        return 0;
    }

    if (initial_buffer_size < 1) {
        initial_buffer_size = 1;
    }

    if (growth_rate < 1.0) {
        growth_rate = 1.0;
    }

    size_t new_allocated_size = initial_buffer_size;
    if (*allocated_size != 0) {
        new_allocated_size = (size_t)((double)(*allocated_size) * growth_rate);
    }
    if (new_allocated_size < *allocated_size + 1) {
        new_allocated_size = *allocated_size + 1;
    }

    void *buffer = realloc(*array, new_allocated_size * object_size);
    if (buffer == NULL) {
        return -1;
    }

    *array          = buffer;
    *allocated_size = new_allocated_size;

    return 0;
}

inline static void fuzz_detail_prefix_function(
    const char *string,
    size_t *prefix_function_values
) {
    if (string == NULL || prefix_function_values == NULL) {
        return;
    }

    const size_t length = strlen(string);
    if (length == 0) {
        return;
    }

    prefix_function_values[0] = 0;
    for (size_t i = 1; i < length; ++i) {
        size_t prefix_length = prefix_function_values[i - 1];
        while (string[i] != string[prefix_length] && prefix_length != 0) {
            prefix_length = prefix_function_values[prefix_length - 1];
        }
        if (string[i] == string[prefix_length]) {
            ++prefix_length;
        }
        prefix_function_values[i] = prefix_length;
    }
}

inline static const char *fuzz_detail_search_substring(
    const FuzzStringView string,
    const char *pattern,
    const size_t *prefix_function_values
) {
    if (pattern == NULL || prefix_function_values == NULL) {
        return string.end;
    }

    const size_t pattern_length = strlen(pattern);
    if (pattern_length == 0) {
        return string.end;
    }

    size_t prefix_length = 0;
    for (const char *c = string.begin; c != string.end; ++c) {
        while (*c != pattern[prefix_length] && prefix_length != 0) {
            prefix_length = prefix_function_values[prefix_length - 1];
        }
        if (*c == pattern[prefix_length]) {
            ++prefix_length;
        }
        if (prefix_length == pattern_length) {
            return c - pattern_length + 1;
        }
    }

    return string.end;
}

void fuzz_init_config(FuzzConfig *config) {
    if (config == NULL) {
        return;
    }
    const FuzzConfig default_config = {
        .text_data = {
            .initial_buffer_size = 16,
            .buffer_growth_rate  = 2.0,
        },
        .string_array = {
            .initial_buffer_size = 16,
            .buffer_growth_rate  = 2.0,
        },
    };
    *config = default_config;
}

void fuzz_get_config(FuzzConfig *config) {
    if (config == NULL) {
        return;
    }
    *config = *fuzz_get_config_location();
}

void fuzz_set_config(FuzzConfig *config) {
    if (config == NULL) {
        fuzz_init_config(fuzz_get_config_location());
        return;
    }
    *fuzz_get_config_location() = *config;
}

FuzzConfig *fuzz_get_config_location(void) {
    static int initialized = 0;
    static FuzzConfig config;
    if (!initialized) {
        fuzz_init_config(&config);
        initialized = 1;
    }
    return &config;
}

void fuzz_init_text_data(FuzzTextData *text_data) {
    if (text_data == NULL) {
        return;
    }
    const FuzzTextData initial_text_data = {
        .data           = NULL,
        .size           = 0,
        .allocated_size = 0,
    };
    *text_data = initial_text_data;
}

int fuzz_read_text_data(FuzzTextData *text_data, FILE *file) {
    if (text_data == NULL || file == NULL) {
        return 0;
    }

    const FuzzConfig *config = fuzz_get_config_location();

    while (!feof(file)) {
        size_t free_space = text_data->allocated_size - text_data->size;

        if (free_space == 0) {
            int grow_array_error = fuzz_detail_grow_array(
                (void **)(&text_data->data),
                &text_data->allocated_size,
                sizeof(*text_data->data),
                config->text_data.initial_buffer_size,
                config->text_data.buffer_growth_rate
            );

            if (grow_array_error) {
                return -1;
            }

            free_space = text_data->allocated_size - text_data->size;
        }

        const size_t characters_read = fread(
            (void *)(text_data->data + text_data->size),
            sizeof(*text_data->data),
            free_space,
            file
        );
        if (characters_read != free_space) {
            if (ferror(file)) {
                return -1;
            }
        }
        text_data->size += characters_read;
    }

    return 0;
}

void fuzz_clear_text_data(FuzzTextData *text_data) {
    if (text_data != NULL) {
        free((void *)(text_data->data));
        text_data->data           = NULL;
        text_data->size           = 0;
        text_data->allocated_size = 0;
    }
}

void fuzz_init_string_array(FuzzStringArray *string_array) {
    if (string_array == NULL) {
        return;
    }
    const FuzzStringArray initial_string_array = {
        .data           = NULL,
        .size           = 0,
        .allocated_size = 0,
    };
    *string_array = initial_string_array;
}

int fuzz_split_into_string_array(
    FuzzStringArray *string_array,
    const FuzzTextData *text_data,
    const char *separator
) {
    if (string_array == NULL || text_data == NULL || separator == NULL) {
        return 0;
    }

    const size_t separator_length = strlen(separator);
    if (separator_length == 0) {
        return 0;
    }

    size_t prefix_function[separator_length];
    fuzz_detail_prefix_function(separator, prefix_function);

    FuzzStringView remaining_data;
    remaining_data.begin = text_data->data;
    remaining_data.end   = text_data->data + text_data->size;

    const FuzzConfig *config = fuzz_get_config_location();

    while (remaining_data.begin != remaining_data.end) {
        const char *current_match = fuzz_detail_search_substring(
            remaining_data,
            separator,
            prefix_function
        );

        if (string_array->allocated_size == string_array->size) {
            int grow_array_error = fuzz_detail_grow_array(
                (void **)(&string_array->data),
                &string_array->allocated_size,
                sizeof(*string_array->data),
                config->string_array.initial_buffer_size,
                config->string_array.buffer_growth_rate
            );

            if (grow_array_error) {
                return -1;
            }
        }

        FuzzStringView *new_string = string_array->data + string_array->size;
        new_string->begin          = remaining_data.begin;
        new_string->end            = current_match;
        ++string_array->size;

        // TODO: add option to include separator to resulting string view
        remaining_data.begin = current_match;
        if (remaining_data.begin != remaining_data.end) {
            remaining_data.begin += separator_length;
        }
    }

    return 0;
}

void fuzz_clear_string_array(FuzzStringArray *string_array) {
    if (string_array != NULL) {
        free((void *)(string_array->data));
        string_array->data           = NULL;
        string_array->size           = 0;
        string_array->allocated_size = 0;
    }
}

#endif

#endif // FUZZ_INCLUDE_FUZZ_FUZZ_H
