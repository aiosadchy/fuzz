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
    } raw_data_reading;

    struct {
        size_t initial_buffer_size;
        double buffer_growth_rate;
    } string_array_building;

} FuzzConfig;

typedef struct FuzzRawData {
    char *data;
    size_t size;
    size_t allocation_size;
} FuzzRawData;

typedef struct FuzzString {
    const char *begin;
    const char *end;
} FuzzString;

typedef struct FuzzStringArray {
    FuzzString *data;
    size_t size;
    size_t allocated_size;
} FuzzStringArray;

FuzzConfig fuzz_make_config(void);
FuzzConfig fuzz_get_config(void);
FuzzConfig *fuzz_get_config_location(void);
void fuzz_set_config(FuzzConfig config);

FuzzRawData fuzz_make_raw_data(void);
int fuzz_read_raw_data(FuzzRawData *data, FILE *file);
void fuzz_clear_raw_data(FuzzRawData *data);

FuzzStringArray fuzz_make_string_array(void);
int fuzz_build_string_array(
    FuzzStringArray *data,
    const FuzzRawData *raw_data,
    const char *separator
);
void fuzz_clear_string_array(FuzzStringArray *data);


#ifdef FUZZ_IMPLEMENTATION

FuzzConfig fuzz_make_config(void) {
    // clang-format off
    FuzzConfig result = {
        .raw_data_reading = {
            .initial_buffer_size = 4096,
            .buffer_growth_rate = 2.0,
        },
        .string_array_building = {
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

FuzzRawData fuzz_make_raw_data(void) {
    FuzzRawData raw_data = {
        .data            = NULL,
        .size            = 0,
        .allocation_size = 0,
    };
    return raw_data;
}

int fuzz_read_raw_data(FuzzRawData *data, FILE *file) {
    if (data == NULL || file == NULL) {
        return 0;
    }

    const FuzzConfig config          = fuzz_get_config();
    const size_t initial_buffer_size = config.raw_data_reading.initial_buffer_size;
    const double buffer_growth_rate  = config.raw_data_reading.buffer_growth_rate;

    while (!feof(file)) {
        size_t free_space = data->allocation_size - data->size;

        if (free_space == 0) {
            // clang-format off
            const size_t new_size = (data->allocation_size == 0)
                ? initial_buffer_size
                : (size_t)((double)data->size * buffer_growth_rate);
            // clang-format on

            char *buffer = realloc((void *)(data->data), new_size);
            if (buffer == NULL) {
                return -1;
            }
            data->data            = buffer;
            data->allocation_size = new_size;
            free_space            = data->allocation_size - data->size;
        }

        const size_t characters_read = fread(
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

FuzzStringArray fuzz_make_string_array(void) {
    FuzzStringArray data = {
        .data           = NULL,
        .size           = 0,
        .allocated_size = 0,
    };
    return data;
}

int fuzz_build_string_array(
    FuzzStringArray *data,
    const FuzzRawData *raw_data,
    const char *separator
) {
    if (data == NULL || raw_data == NULL) {
        return 0;
    }

    const size_t separator_length = (separator == NULL) ? 0 : strlen(separator);
    if (separator_length == 0) {
        return 0;
    }

    // TODO: implementation

    return 0;
}

void fuzz_clear_string_array(FuzzStringArray *data) {
    if (data != NULL) {
        free((void *)(data->data));
        data->data           = NULL;
        data->size           = 0;
        data->allocated_size = 0;
    }
}

#endif

#endif // FUZZ_INCLUDE_FUZZ_FUZZ_H
