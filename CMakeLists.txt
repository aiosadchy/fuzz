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
