#ifndef FUZZ_INCLUDE_FUZZ_FUZZ_H
#define FUZZ_INCLUDE_FUZZ_FUZZ_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct FuzzConfig {
    struct {
        size_t initial_buffer_size;
        double buffer_growth_rate;
    } file_input;
} FuzzConfig;

typedef struct FuzzString {
    const char *begin;
    const char *end;
} FuzzString;

typedef struct FuzzRawData {
    const char *data;
    size_t size;
    size_t allocation_size;
} FuzzRawData;

typedef struct FuzzData {
    FuzzString *data;
    size_t size;
    size_t allocated_size;
} FuzzData;

FuzzConfig fuzz_make_config(void);
FuzzConfig fuzz_get_config(void);
FuzzConfig *fuzz_get_config_location(void);
void fuzz_set_config(FuzzConfig config);

int fuzz_read_from_file(FuzzRawData *data, FILE *file);
void fuzz_clear_raw_data(FuzzRawData *data);


#ifdef FUZZ_IMPLEMENTATION

FuzzConfig fuzz_make_config(void) {
    // clang-format off
    FuzzConfig result = {
        .file_input = {
            .initial_buffer_size = 4096,
            .buffer_growth_rate = 2.0,
        },
    };
    // clang-format on
    return result;
}

FuzzConfig fuzz_get_config(void) {
    return *fuzz_get_config_location();
}

FuzzConfig *fuzz_get_config_location(void) {
    static int initialized = 0;
    static FuzzConfig config;
    if (!initialized) {
        config      = fuzz_make_config();
        initialized = 1;
    }
    return &config;
}

void fuzz_set_config(FuzzConfig config) {
    *fuzz_get_config_location() = config;
}

int fuzz_read_from_file(FuzzRawData *data, FILE *file) {
    if (data == NULL || file == NULL) {
        return 0;
    }

    const FuzzConfig config = fuzz_get_config();

    while (!feof(file)) {
        size_t free_space = data->allocation_size - data->size;

        if (free_space == 0) {
            size_t new_size = (data->allocation_size == 0)
                ? config.file_input.initial_buffer_size
                : (size_t)((double)data->size * config.file_input.buffer_growth_rate);

            const char *buffer = realloc((void *)(data->data), new_size);
            if (buffer == NULL) {
                return -1;
            }
            data->data            = buffer;
            data->allocation_size = new_size;
            free_space            = data->allocation_size - data->size;
        }

        size_t characters_read = fread(
            (void *)(data->data + data->size),
            sizeof(*data->data),
            free_space,
            file
        );
        if (characters_read != free_space) {
            if (ferror(file)) {
                return -1;
            }
        }
        data->size += characters_read;
    }

    return 0;
}

void fuzz_clear_raw_data(FuzzRawData *data) {
    if (data != NULL) {
        free((void *)(data->data));
        data->data            = NULL;
        data->size            = 0;
        data->allocation_size = 0;
    }
}

#endif

#endif // FUZZ_INCLUDE_FUZZ_FUZZ_H
