cmake_minimum_required (VERSION 3.8)

function(msg level)
    message(${level} "[fuzz] " ${ARGN})
endfunction()

file(READ .version FUZZ_VERSION)
string(STRIP "${FUZZ_VERSION}" FUZZ_VERSION)

project(fuzz
    VERSION         "${FUZZ_VERSION}"
    DESCRIPTION     "A simple fuzzy search library implemented in C."
    HOMEPAGE_URL    "https://github.com/aiosadchy/fuzz"
)


add_library(fuzz src/fuzz/fuzz.c)
target_include_directories(fuzz PUBLIC include)
# TODO: maybe 99 is enough
set_property(TARGET fuzz PROPERTY C_STANDARD 11)


add_executable(fuzz_test
    test/src/main.c
)
target_link_libraries(fuzz_test
    PRIVATE fuzz
)
target_compile_options(fuzz_test
    PRIVATE
        -Wall
        -Werror
        -Wextra
        -Wmissing-prototypes
        -pedantic
)
