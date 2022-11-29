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

void fuzz_init_config(FuzzConfig *config);
void fuzz_get_config(FuzzConfig *config);
void fuzz_set_config(FuzzConfig *config);
FuzzConfig *fuzz_get_config_location(void);

void fuzz_init_raw_data(FuzzRawData *raw_data);
int fuzz_read_raw_data(FuzzRawData *raw_data, FILE *file);
void fuzz_clear_raw_data(FuzzRawData *raw_data);

void fuzz_init_string_array(FuzzStringArray *string_array);
int fuzz_build_string_array(
    FuzzStringArray *string_array,
    const FuzzRawData *raw_data,
    const char *separator
);
void fuzz_clear_string_array(FuzzStringArray *string_array);


#ifdef FUZZ_IMPLEMENTATION

void fuzz_init_config(FuzzConfig *config) {
    if (config == NULL) {
        return;
    }
    const FuzzConfig default_config = {
        .raw_data_reading = {
            .initial_buffer_size = 4096,
            .buffer_growth_rate  = 2.0,
        },
        .string_array_building = {
            .initial_buffer_size = 4096,
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

void fuzz_init_raw_data(FuzzRawData *raw_data) {
    if (raw_data == NULL) {
        return;
    }
    const FuzzRawData initial_raw_data = {
        .data            = NULL,
        .size            = 0,
        .allocation_size = 0,
    };
    *raw_data = initial_raw_data;
}

int fuzz_read_raw_data(FuzzRawData *raw_data, FILE *file) {
    if (raw_data == NULL || file == NULL) {
        return 0;
    }

    FuzzConfig config;
    fuzz_get_config(&config);
    const size_t buffer_size = config.raw_data_reading.initial_buffer_size;
    const double growth_rate = config.raw_data_reading.buffer_growth_rate;

    while (!feof(file)) {
        size_t free_space = raw_data->allocation_size - raw_data->size;

        if (free_space == 0) {
            // clang-format off
            const size_t new_size = (raw_data->allocation_size == 0)
                ? buffer_size
                : (size_t)((double)raw_data->size * growth_rate);
            // clang-format on

            char *buffer = realloc((void *)(raw_data->data), new_size);
            if (buffer == NULL) {
                return -1;
            }
            raw_data->data            = buffer;
            raw_data->allocation_size = new_size;

            free_space = raw_data->allocation_size - raw_data->size;
        }

        const size_t characters_read = fread(
            (void *)(raw_data->data + raw_data->size),
            sizeof(*raw_data->data),
            free_space,
            file
        );
        if (characters_read != free_space) {
            if (ferror(file)) {
                return -1;
            }
        }
        raw_data->size += characters_read;
    }

    return 0;
}

void fuzz_clear_raw_data(FuzzRawData *raw_data) {
    if (raw_data != NULL) {
        free((void *)(raw_data->data));
        raw_data->data            = NULL;
        raw_data->size            = 0;
        raw_data->allocation_size = 0;
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

int fuzz_build_string_array(
    FuzzStringArray *string_array,
    const FuzzRawData *raw_data,
    const char *separator
) {
    if (string_array == NULL || raw_data == NULL || separator == NULL) {
        return 0;
    }

    const size_t separator_length = strlen(separator);
    if (separator_length == 0) {
        return 0;
    }

    // size_t prefix_function[separator_length];


    // TODO: implementation

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
