#define FUZZ_IMPLEMENTATION
#include <fuzz/fuzz.h>

int main(void) {
    FuzzTextData text_data;
    fuzz_init_text_data(&text_data);

    FILE *f1 = fopen("/home/aiosadchy/1.txt", "r");
    FILE *f2 = fopen("/home/aiosadchy/2.txt", "r");

    fuzz_read_text_data(&text_data, f1);
    fuzz_read_text_data(&text_data, f2);

    fclose(f1);
    fclose(f2);

    FuzzStringArray string_array;
    fuzz_init_string_array(&string_array);

    fuzz_split_into_string_array(
        &string_array,
        &text_data,
        "\n"
    );

    for (size_t i = 0; i < string_array.size; ++i) {
        const FuzzStringView *string = string_array.data + i;
        printf("[%.*s]\n", (int)(string->end - string->begin), string->begin);
    }

    fuzz_clear_string_array(&string_array);
    fuzz_clear_text_data(&text_data);
}
