# utflite

A lightweight, zero-dependency UTF-8 library for C.

## Features

- UTF-8 encoding and decoding with full validation
- Unicode 17.0 character width tables (wcwidth alternative)
- String navigation (next/prev character)
- Utility functions: validate, count, width, truncate
- Single-header option for easy integration
- C17 compliant, no external dependencies

## Use Cases

**Terminal/CLI applications**
- Text editors that need cursor positioning over Unicode text
- Progress bars and status lines with proper column alignment
- Tree views and tables that must align with CJK/emoji content

**Text processing tools**
- Validating UTF-8 input from files or network
- Counting characters (not bytes) for length limits
- Truncating strings to fit display widths without breaking characters

**Embedded systems**
- Lightweight alternative to ICU or libunistring
- Single-header makes it easy to integrate into firmware
- No heap allocation, no dependencies

**Game development**
- Chat systems handling international text
- UI text measurement for layout
- Input validation for player names

**When NOT to use utflite**
- Full Unicode normalization (NFC/NFD) - use ICU
- Complex text shaping (Arabic, Indic scripts) - use HarfBuzz
- Locale-aware sorting/comparison - use ICU

## Project Structure

```
utflite/
├── include/utflite/      # Public headers
│   └── utflite.h
├── single_include/       # Single-header distribution
│   └── utflite.h
├── src/                  # Implementation
│   └── utflite.c
├── test/
│   └── test_utflite.c
└── build/                # Build artifacts (gitignored)
```

## Installation

### Single Header (Easiest)

Copy `single_include/utflite.h` to your project. In **one** .c file:

```c
#define UTFLITE_IMPLEMENTATION
#include "utflite.h"
```

In all other files, just include normally:

```c
#include "utflite.h"
```

### Static Library

```bash
make
make install  # installs to /usr/local
```

Then in your code:

```c
#include <utflite/utflite.h>
```

And compile with `-lutflite`.

## Tutorial

### Decoding UTF-8

UTF-8 encodes Unicode codepoints as 1-4 bytes. Use `utflite_decode()` to convert bytes to a codepoint:

```c
#define UTFLITE_IMPLEMENTATION
#include "utflite.h"
#include <stdio.h>

int main(void) {
    const char *text = "A";           // ASCII: 1 byte
    const char *text2 = "\xC3\xA9";   // e-acute (U+00E9): 2 bytes
    const char *text3 = "\xE4\xB8\xAD"; // Chinese character (U+4E2D): 3 bytes

    uint32_t codepoint;
    int bytes;

    bytes = utflite_decode(text, 1, &codepoint);
    printf("'A' = U+%04X (%d byte)\n", codepoint, bytes);
    // Output: 'A' = U+0041 (1 byte)

    bytes = utflite_decode(text2, 2, &codepoint);
    printf("e-acute = U+%04X (%d bytes)\n", codepoint, bytes);
    // Output: e-acute = U+00E9 (2 bytes)

    bytes = utflite_decode(text3, 3, &codepoint);
    printf("Chinese = U+%04X (%d bytes)\n", codepoint, bytes);
    // Output: Chinese = U+4E2D (3 bytes)

    return 0;
}
```

### Encoding to UTF-8

Convert a codepoint back to UTF-8 bytes:

```c
#include "utflite.h"
#include <stdio.h>

int main(void) {
    char buffer[UTFLITE_MAX_BYTES];  // Always 4 bytes max
    int len;

    // Encode emoji (U+1F600 = grinning face)
    len = utflite_encode(0x1F600, buffer);

    printf("Emoji is %d bytes: ", len);
    for (int i = 0; i < len; i++) {
        printf("%02X ", (unsigned char)buffer[i]);
    }
    printf("\n");
    // Output: Emoji is 4 bytes: F0 9F 98 80

    return 0;
}
```

### Iterating Over Characters

Use `utflite_next_char()` to walk through a UTF-8 string character by character:

```c
#include "utflite.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    // Mixed string: ASCII + Chinese + emoji
    const char *text = "Hi\xE4\xB8\xAD\xF0\x9F\x98\x80";
    int len = strlen(text);  // 10 bytes, but only 4 characters

    int offset = 0;
    int char_num = 0;

    while (offset < len) {
        uint32_t codepoint;
        int bytes = utflite_decode(text + offset, len - offset, &codepoint);

        printf("Char %d: U+%04X at byte %d (%d bytes)\n",
               char_num, codepoint, offset, bytes);

        offset = utflite_next_char(text, len, offset);
        char_num++;
    }
    // Output:
    // Char 0: U+0048 at byte 0 (1 bytes)   'H'
    // Char 1: U+0069 at byte 1 (1 bytes)   'i'
    // Char 2: U+4E2D at byte 2 (3 bytes)   Chinese
    // Char 3: U+1F600 at byte 5 (4 bytes)  Emoji

    return 0;
}
```

### Character Display Width

Terminals display characters with different widths. ASCII is 1 column, CJK and emoji are 2 columns, combining marks are 0 columns:

```c
#include "utflite.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    // Check individual codepoints
    printf("'A' width: %d\n", utflite_codepoint_width(0x0041));      // 1
    printf("Chinese width: %d\n", utflite_codepoint_width(0x4E2D));  // 2
    printf("Emoji width: %d\n", utflite_codepoint_width(0x1F600));   // 2
    printf("Combining accent width: %d\n", utflite_codepoint_width(0x0301)); // 0

    // Calculate total display width of a string
    const char *text = "Hello\xE4\xB8\xAD";  // "Hello" + Chinese char
    int width = utflite_string_width(text, strlen(text));
    printf("'Hello中' display width: %d columns\n", width);  // 7 (5 + 2)

    return 0;
}
```

### Validating UTF-8

Check if a string contains valid UTF-8:

```c
#include "utflite.h"
#include <stdio.h>

int main(void) {
    const char *valid = "Hello \xE4\xB8\xAD";
    const char *invalid = "Bad \x80\x81 data";  // Invalid continuation bytes

    int error_pos;

    if (utflite_validate(valid, 9, NULL)) {
        printf("String 1: valid UTF-8\n");
    }

    if (!utflite_validate(invalid, 11, &error_pos)) {
        printf("String 2: invalid at byte %d\n", error_pos);
    }
    // Output:
    // String 1: valid UTF-8
    // String 2: invalid at byte 4

    return 0;
}
```

### Truncating for Display

When you need to fit text in a fixed-width column, use `utflite_truncate()` to find the correct byte offset:

```c
#include "utflite.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    // String with mixed widths
    const char *text = "AB\xE4\xB8\xAD\xE4\xB8\xAD" "CD";
    // A(1) B(1) 中(2) 中(2) C(1) D(1) = 8 columns total

    int len = strlen(text);

    // Truncate to fit in 5 columns
    int cut = utflite_truncate(text, len, 5);

    printf("Original (%d cols): %s\n", utflite_string_width(text, len), text);
    printf("Truncated to 5 cols: %.*s\n", cut, text);
    // Output:
    // Original (8 cols): AB中中CD
    // Truncated to 5 cols: AB中

    return 0;
}
```

### Counting Characters

Count codepoints (not bytes) in a string:

```c
#include "utflite.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *text = "Hello\xE4\xB8\xAD\xF0\x9F\x98\x80";
    int len = strlen(text);

    printf("Bytes: %d\n", len);
    printf("Characters: %d\n", utflite_codepoint_count(text, len));
    printf("Display width: %d columns\n", utflite_string_width(text, len));
    // Output:
    // Bytes: 12
    // Characters: 7
    // Display width: 9

    return 0;
}
```

### Handling Invalid Input

`utflite_decode()` never crashes on invalid input. It returns the replacement character (U+FFFD) and advances by 1 byte:

```c
#include "utflite.h"
#include <stdio.h>

int main(void) {
    // Invalid UTF-8: bare continuation byte
    const char *bad = "\x80\x81\x82";

    uint32_t cp;
    int offset = 0;

    while (offset < 3) {
        int bytes = utflite_decode(bad + offset, 3 - offset, &cp);

        if (cp == UTFLITE_REPLACEMENT_CHAR) {
            printf("Invalid byte at %d\n", offset);
        }
        offset += bytes;
    }
    // Output:
    // Invalid byte at 0
    // Invalid byte at 1
    // Invalid byte at 2

    return 0;
}
```

## API Reference

### Constants

```c
#define UTFLITE_REPLACEMENT_CHAR 0xFFFD  // Returned on decode errors
#define UTFLITE_MAX_BYTES 4              // Max bytes per codepoint
```

### Core Functions

```c
// Decode UTF-8 to codepoint. Returns bytes consumed (1-4).
int utflite_decode(const char *bytes, int length, uint32_t *codepoint);

// Encode codepoint to UTF-8. Returns bytes written (1-4), 0 on error.
int utflite_encode(uint32_t codepoint, char *buffer);
```

### Character Width

```c
// Returns display width: -1 (control), 0 (combining), 1 (normal), 2 (wide)
int utflite_codepoint_width(uint32_t codepoint);

// Width of character at byte offset
int utflite_char_width(const char *text, int length, int offset);
```

### Navigation

```c
// Get byte offset of next/previous character
int utflite_next_char(const char *text, int length, int offset);
int utflite_prev_char(const char *text, int offset);

// Grapheme cluster navigation (currently same as char, future: UAX #29)
int utflite_next_grapheme(const char *text, int length, int offset);
int utflite_prev_grapheme(const char *text, int offset);
```

### Utilities

```c
// Validate UTF-8 string. Returns 1 if valid, 0 if invalid.
int utflite_validate(const char *text, int length, int *error_offset);

// Count codepoints in string
int utflite_codepoint_count(const char *text, int length);

// Calculate display width of string
int utflite_string_width(const char *text, int length);

// Check character properties
int utflite_is_zero_width(uint32_t codepoint);  // Zero-width (combining, ZWJ, etc)?
int utflite_is_wide(uint32_t codepoint);        // Double-width (CJK, emoji)?

// Find truncation point for max display columns
int utflite_truncate(const char *text, int length, int max_cols);
```

## Building

```bash
make              # Build static library (output in build/)
make test         # Run tests with static library
make test-single  # Run tests with single-header version
make install      # Install to /usr/local
make clean        # Clean build artifacts
```

## License

MIT License. See LICENSE file.
