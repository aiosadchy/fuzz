#define FUZZ_IMPLEMENTATION
#include <fuzz/fuzz.h>

int main(void) {
    FuzzRawData raw_data;
    fuzz_init_raw_data(&raw_data);

    FILE *f1 = fopen("/home/aiosadchy/1.txt", "r");
    FILE *f2 = fopen("/home/aiosadchy/2.txt", "r");

    fuzz_read_raw_data(&raw_data, f1);
    fuzz_read_raw_data(&raw_data, f2);

    fclose(f1);
    fclose(f2);

    fuzz_get_config_location()->raw_data_reading.initial_buffer_size = 10;

    printf("size            = [%zu]\n", raw_data.size);
    printf("allocation_size = [%zu]\n", raw_data.allocation_size);
    printf("data            = [%.*s]\n", (int)(raw_data.size), raw_data.data);

    fuzz_clear_raw_data(&raw_data);
}
