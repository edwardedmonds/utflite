/*
 * utflite.h - Lightweight UTF-8 handling library (Single-Header Version)
 *
 * Unicode 15.0 compliant UTF-8 encoding/decoding with character width support.
 * Zero external dependencies beyond standard C.
 *
 * USAGE:
 *   In ONE .c file, before including this header:
 *     #define UTFLITE_IMPLEMENTATION
 *     #include "utflite.h"
 *
 *   In all other files, just include normally:
 *     #include "utflite.h"
 *
 * EXAMPLE:
 *     #define UTFLITE_IMPLEMENTATION
 *     #include "utflite.h"
 *
 *     int main(void) {
 *         uint32_t cp;
 *         utflite_decode("\xE4\xB8\xAD", 3, &cp);  // Decode Chinese char
 *         int width = utflite_codepoint_width(cp); // Returns 2 (double-width)
 *         return 0;
 *     }
 */

#ifndef UTFLITE_H
#define UTFLITE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version information */
#define UTFLITE_VERSION_MAJOR 1
#define UTFLITE_VERSION_MINOR 1
#define UTFLITE_VERSION_PATCH 0
#define UTFLITE_VERSION_STRING "1.1.0"

/* Unicode replacement character (returned on decode errors) */
#define UTFLITE_REPLACEMENT_CHAR 0xFFFD

/* Maximum bytes needed to encode any Unicode codepoint */
#define UTFLITE_MAX_BYTES 4

/* ============================================================================
 * Core Encoding/Decoding
 * ============================================================================ */

/*
 * Decodes a UTF-8 byte sequence into a Unicode codepoint.
 *
 * Parameters:
 *   bytes     - Pointer to UTF-8 encoded bytes
 *   length    - Number of bytes available in buffer
 *   codepoint - Output: decoded codepoint (U+FFFD on error)
 *
 * Returns:
 *   Number of bytes consumed (1-4), always at least 1 for forward progress.
 *
 * Error handling:
 *   - Invalid sequences set *codepoint to U+FFFD
 *   - Overlong encodings are rejected
 *   - Surrogate pairs (U+D800-U+DFFF) are rejected
 *   - Values > U+10FFFF are rejected
 */
int utflite_decode(const char *bytes, int length, uint32_t *codepoint);

/*
 * Encodes a Unicode codepoint as UTF-8.
 *
 * Parameters:
 *   codepoint - Unicode codepoint to encode (0x0000-0x10FFFF)
 *   buffer    - Output buffer (must have at least UTFLITE_MAX_BYTES space)
 *
 * Returns:
 *   Number of bytes written (1-4), or 0 for invalid codepoints.
 *
 * Note: Does not null-terminate. Caller manages buffer.
 */
int utflite_encode(uint32_t codepoint, char *buffer);

/* ============================================================================
 * Character Width (Display Columns)
 * ============================================================================ */

/*
 * Returns the display width of a Unicode codepoint.
 *
 * Returns:
 *   -1  Non-printable control character (C0/C1 except NUL)
 *    0  Zero-width (combining marks, format chars, ZWJ, etc.)
 *    1  Normal width (ASCII, Latin, most scripts)
 *    2  Double-width (CJK ideographs, fullwidth forms, emoji)
 */
int utflite_codepoint_width(uint32_t codepoint);

/*
 * Returns display width of UTF-8 character at given offset.
 * Convenience function combining decode + width lookup.
 */
int utflite_char_width(const char *text, int length, int offset);

/* ============================================================================
 * String Navigation
 * ============================================================================ */

/*
 * Returns byte offset of the next character.
 *
 * Parameters:
 *   text   - UTF-8 string
 *   length - Total bytes in string
 *   offset - Current byte position
 *
 * Returns:
 *   Byte offset of next character, or 'length' if at end.
 */
int utflite_next_char(const char *text, int length, int offset);

/*
 * Returns byte offset of the previous character.
 *
 * Parameters:
 *   text   - UTF-8 string
 *   offset - Current byte position
 *
 * Returns:
 *   Byte offset of previous character start, or 0 if at beginning.
 */
int utflite_prev_char(const char *text, int offset);

/* ============================================================================
 * Grapheme Cluster Support (Future/Stub)
 * ============================================================================ */

/*
 * Returns byte offset of next grapheme cluster boundary.
 * Currently equivalent to utflite_next_char().
 * Future: UAX #29 compliant grapheme segmentation.
 */
int utflite_next_grapheme(const char *text, int length, int offset);

/*
 * Returns byte offset of previous grapheme cluster boundary.
 * Currently equivalent to utflite_prev_char().
 * Future: UAX #29 compliant grapheme segmentation.
 */
int utflite_prev_grapheme(const char *text, int offset);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/*
 * Validates a UTF-8 string.
 *
 * Parameters:
 *   text         - UTF-8 string to validate
 *   length       - Number of bytes in string
 *   error_offset - Optional: set to byte offset of first invalid sequence
 *
 * Returns:
 *   1 if valid UTF-8, 0 if invalid.
 */
int utflite_validate(const char *text, int length, int *error_offset);

/*
 * Counts the number of Unicode codepoints in a UTF-8 string.
 *
 * Returns:
 *   Number of codepoints (not bytes, not graphemes).
 */
int utflite_codepoint_count(const char *text, int length);

/*
 * Calculates display width of a UTF-8 string.
 *
 * Returns:
 *   Total display columns needed. Control characters count as 0.
 */
int utflite_string_width(const char *text, int length);

/*
 * Checks if a codepoint is zero-width (combining marks, format chars, ZWJ, etc).
 *
 * Returns:
 *   1 if codepoint is zero-width, 0 otherwise.
 */
int utflite_is_zero_width(uint32_t codepoint);

/*
 * Checks if a codepoint is double-width (CJK, emoji, etc).
 *
 * Returns:
 *   1 if codepoint is double-width, 0 otherwise.
 */
int utflite_is_wide(uint32_t codepoint);

/*
 * Finds byte offset to truncate string at a maximum display width.
 *
 * Parameters:
 *   text     - UTF-8 string
 *   length   - Number of bytes in string
 *   max_cols - Maximum display columns
 *
 * Returns:
 *   Byte offset where string should be truncated to fit within max_cols.
 *   Returns length if string already fits.
 */
int utflite_truncate(const char *text, int length, int max_cols);

#ifdef __cplusplus
}
#endif

#endif /* UTFLITE_H */

/* ============================================================================
 * IMPLEMENTATION
 * ============================================================================ */

#ifdef UTFLITE_IMPLEMENTATION

typedef struct {
    uint32_t start;
    uint32_t end;
} utflite__unicode_range;

/*
 * Zero-width character ranges.
 * These characters take 0 display columns (combining marks, control chars).
 * Sorted by start codepoint for binary search.
 */
static const utflite__unicode_range UTFLITE__ZERO_WIDTH_RANGES[] = {
    {0x0300, 0x036F},   /* Combining Diacritical Marks */
    {0x0483, 0x0489},   /* Cyrillic combining marks */
    {0x0591, 0x05BD},   /* Hebrew accents */
    {0x05BF, 0x05BF},   /* Hebrew point rafe */
    {0x05C1, 0x05C2},   /* Hebrew points */
    {0x05C4, 0x05C5},   /* Hebrew marks */
    {0x05C7, 0x05C7},   /* Hebrew point qamats qatan */
    {0x0610, 0x061A},   /* Arabic signs */
    {0x064B, 0x065F},   /* Arabic fathatan and marks */
    {0x0670, 0x0670},   /* Arabic superscript alef */
    {0x06D6, 0x06DC},   /* Arabic small marks */
    {0x06DF, 0x06E4},   /* Arabic small marks */
    {0x06E7, 0x06E8},   /* Arabic small marks */
    {0x06EA, 0x06ED},   /* Arabic small marks */
    {0x0711, 0x0711},   /* Syriac superscript alaph */
    {0x0730, 0x074A},   /* Syriac combining marks */
    {0x07A6, 0x07B0},   /* Thaana combining marks */
    {0x07EB, 0x07F3},   /* NKo combining marks */
    {0x07FD, 0x07FD},   /* NKo dantayalan */
    {0x0816, 0x0819},   /* Samaritan marks */
    {0x081B, 0x0823},   /* Samaritan marks */
    {0x0825, 0x0827},   /* Samaritan marks */
    {0x0829, 0x082D},   /* Samaritan marks */
    {0x0859, 0x085B},   /* Mandaic marks */
    {0x08D3, 0x08E1},   /* Arabic extended marks */
    {0x08E3, 0x0902},   /* Arabic extended marks, Devanagari */
    {0x093A, 0x093A},   /* Devanagari vowel sign oe */
    {0x093C, 0x093C},   /* Devanagari sign nukta */
    {0x0941, 0x0948},   /* Devanagari vowel signs */
    {0x094D, 0x094D},   /* Devanagari sign virama */
    {0x0951, 0x0957},   /* Devanagari stress signs */
    {0x0962, 0x0963},   /* Devanagari vowel signs */
    {0x0981, 0x0981},   /* Bengali sign candrabindu */
    {0x09BC, 0x09BC},   /* Bengali sign nukta */
    {0x09C1, 0x09C4},   /* Bengali vowel signs */
    {0x09CD, 0x09CD},   /* Bengali sign virama */
    {0x09E2, 0x09E3},   /* Bengali vowel signs */
    {0x09FE, 0x09FE},   /* Bengali sandhi mark */
    {0x0A01, 0x0A02},   /* Gurmukhi signs */
    {0x0A3C, 0x0A3C},   /* Gurmukhi sign nukta */
    {0x0A41, 0x0A42},   /* Gurmukhi vowel signs */
    {0x0A47, 0x0A48},   /* Gurmukhi vowel signs */
    {0x0A4B, 0x0A4D},   /* Gurmukhi vowel signs and virama */
    {0x0A51, 0x0A51},   /* Gurmukhi sign udaat */
    {0x0A70, 0x0A71},   /* Gurmukhi tippi and addak */
    {0x0A75, 0x0A75},   /* Gurmukhi sign yakash */
    {0x0A81, 0x0A82},   /* Gujarati signs */
    {0x0ABC, 0x0ABC},   /* Gujarati sign nukta */
    {0x0AC1, 0x0AC5},   /* Gujarati vowel signs */
    {0x0AC7, 0x0AC8},   /* Gujarati vowel signs */
    {0x0ACD, 0x0ACD},   /* Gujarati sign virama */
    {0x0AE2, 0x0AE3},   /* Gujarati vowel signs */
    {0x0AFA, 0x0AFF},   /* Gujarati signs */
    {0x0B01, 0x0B01},   /* Oriya sign candrabindu */
    {0x0B3C, 0x0B3C},   /* Oriya sign nukta */
    {0x0B3F, 0x0B3F},   /* Oriya vowel sign i */
    {0x0B41, 0x0B44},   /* Oriya vowel signs */
    {0x0B4D, 0x0B4D},   /* Oriya sign virama */
    {0x0B55, 0x0B56},   /* Oriya signs */
    {0x0B62, 0x0B63},   /* Oriya vowel signs */
    {0x0B82, 0x0B82},   /* Tamil sign anusvara */
    {0x0BC0, 0x0BC0},   /* Tamil vowel sign ii */
    {0x0BCD, 0x0BCD},   /* Tamil sign virama */
    {0x0C00, 0x0C00},   /* Telugu sign combining candrabindu */
    {0x0C04, 0x0C04},   /* Telugu sign combining anusvara */
    {0x0C3E, 0x0C40},   /* Telugu vowel signs */
    {0x0C46, 0x0C48},   /* Telugu vowel signs */
    {0x0C4A, 0x0C4D},   /* Telugu vowel signs and virama */
    {0x0C55, 0x0C56},   /* Telugu length marks */
    {0x0C62, 0x0C63},   /* Telugu vowel signs */
    {0x0C81, 0x0C81},   /* Kannada sign candrabindu */
    {0x0CBC, 0x0CBC},   /* Kannada sign nukta */
    {0x0CBF, 0x0CBF},   /* Kannada vowel sign i */
    {0x0CC6, 0x0CC6},   /* Kannada vowel sign e */
    {0x0CCC, 0x0CCD},   /* Kannada vowel sign au and virama */
    {0x0CE2, 0x0CE3},   /* Kannada vowel signs */
    {0x0D00, 0x0D01},   /* Malayalam signs */
    {0x0D3B, 0x0D3C},   /* Malayalam signs */
    {0x0D41, 0x0D44},   /* Malayalam vowel signs */
    {0x0D4D, 0x0D4D},   /* Malayalam sign virama */
    {0x0D62, 0x0D63},   /* Malayalam vowel signs */
    {0x0D81, 0x0D81},   /* Sinhala sign candrabindu */
    {0x0DCA, 0x0DCA},   /* Sinhala sign al-lakuna */
    {0x0DD2, 0x0DD4},   /* Sinhala vowel signs */
    {0x0DD6, 0x0DD6},   /* Sinhala vowel sign diga gaetta-pilla */
    {0x0E31, 0x0E31},   /* Thai vowel sign mai han-akat */
    {0x0E34, 0x0E3A},   /* Thai vowel signs and marks */
    {0x0E47, 0x0E4E},   /* Thai marks */
    {0x0EB1, 0x0EB1},   /* Lao vowel sign mai kan */
    {0x0EB4, 0x0EBC},   /* Lao vowel signs and marks */
    {0x0EC8, 0x0ECD},   /* Lao tone marks */
    {0x0F18, 0x0F19},   /* Tibetan astrological signs */
    {0x0F35, 0x0F35},   /* Tibetan mark ngas bzung nyi zla */
    {0x0F37, 0x0F37},   /* Tibetan mark ngas bzung sgor rtags */
    {0x0F39, 0x0F39},   /* Tibetan mark tsa -phru */
    {0x0F71, 0x0F7E},   /* Tibetan vowel signs */
    {0x0F80, 0x0F84},   /* Tibetan vowel signs and marks */
    {0x0F86, 0x0F87},   /* Tibetan signs */
    {0x0F8D, 0x0F97},   /* Tibetan subjoined consonants */
    {0x0F99, 0x0FBC},   /* Tibetan subjoined consonants */
    {0x0FC6, 0x0FC6},   /* Tibetan symbol padma gdan */
    {0x102D, 0x1030},   /* Myanmar vowel signs */
    {0x1032, 0x1037},   /* Myanmar vowel signs and marks */
    {0x1039, 0x103A},   /* Myanmar signs */
    {0x103D, 0x103E},   /* Myanmar consonant signs */
    {0x1058, 0x1059},   /* Myanmar vowel signs */
    {0x105E, 0x1060},   /* Myanmar consonant signs */
    {0x1071, 0x1074},   /* Myanmar vowel signs */
    {0x1082, 0x1082},   /* Myanmar consonant sign shan medial wa */
    {0x1085, 0x1086},   /* Myanmar vowel signs */
    {0x108D, 0x108D},   /* Myanmar sign shan council emphatic tone */
    {0x109D, 0x109D},   /* Myanmar vowel sign aiton ai */
    {0x135D, 0x135F},   /* Ethiopic combining marks */
    {0x1712, 0x1714},   /* Tagalog vowel signs and virama */
    {0x1732, 0x1734},   /* Hanunoo vowel signs and pamudpod */
    {0x1752, 0x1753},   /* Buhid vowel signs */
    {0x1772, 0x1773},   /* Tagbanwa vowel signs */
    {0x17B4, 0x17B5},   /* Khmer vowel inherent signs */
    {0x17B7, 0x17BD},   /* Khmer vowel signs */
    {0x17C6, 0x17C6},   /* Khmer sign nikahit */
    {0x17C9, 0x17D3},   /* Khmer signs */
    {0x17DD, 0x17DD},   /* Khmer sign atthacan */
    {0x180B, 0x180D},   /* Mongolian free variation selectors */
    {0x1885, 0x1886},   /* Mongolian letters */
    {0x18A9, 0x18A9},   /* Mongolian letter ali gali dagalga */
    {0x1920, 0x1922},   /* Limbu vowel signs */
    {0x1927, 0x1928},   /* Limbu vowel signs */
    {0x1932, 0x1932},   /* Limbu small letter anusvara */
    {0x1939, 0x193B},   /* Limbu signs */
    {0x1A17, 0x1A18},   /* Buginese vowel signs */
    {0x1A1B, 0x1A1B},   /* Buginese vowel sign ae */
    {0x1A56, 0x1A56},   /* Tai Tham consonant sign medial la */
    {0x1A58, 0x1A5E},   /* Tai Tham consonant signs */
    {0x1A60, 0x1A60},   /* Tai Tham sign sakot */
    {0x1A62, 0x1A62},   /* Tai Tham vowel sign mai sat */
    {0x1A65, 0x1A6C},   /* Tai Tham vowel signs */
    {0x1A73, 0x1A7C},   /* Tai Tham vowel signs and tone marks */
    {0x1A7F, 0x1A7F},   /* Tai Tham combining cryptogrammic dot */
    {0x1AB0, 0x1ABE},   /* Combining Diacritical Marks Extended */
    {0x1ABF, 0x1AC0},   /* Combining marks */
    {0x1B00, 0x1B03},   /* Balinese signs */
    {0x1B34, 0x1B34},   /* Balinese sign rerekan */
    {0x1B36, 0x1B3A},   /* Balinese vowel signs */
    {0x1B3C, 0x1B3C},   /* Balinese vowel sign la lenga */
    {0x1B42, 0x1B42},   /* Balinese vowel sign pepet */
    {0x1B6B, 0x1B73},   /* Balinese musical symbols */
    {0x1B80, 0x1B81},   /* Sundanese signs */
    {0x1BA2, 0x1BA5},   /* Sundanese consonant signs */
    {0x1BA8, 0x1BA9},   /* Sundanese vowel signs */
    {0x1BAB, 0x1BAD},   /* Sundanese signs */
    {0x1BE6, 0x1BE6},   /* Batak sign tompi */
    {0x1BE8, 0x1BE9},   /* Batak vowel signs */
    {0x1BED, 0x1BED},   /* Batak vowel sign karo o */
    {0x1BEF, 0x1BF1},   /* Batak vowel signs and pangolat */
    {0x1C2C, 0x1C33},   /* Lepcha vowel signs */
    {0x1C36, 0x1C37},   /* Lepcha signs */
    {0x1CD0, 0x1CD2},   /* Vedic tone marks */
    {0x1CD4, 0x1CE0},   /* Vedic signs */
    {0x1CE2, 0x1CE8},   /* Vedic signs */
    {0x1CED, 0x1CED},   /* Vedic sign tiryak */
    {0x1CF4, 0x1CF4},   /* Vedic tone candra above */
    {0x1CF8, 0x1CF9},   /* Vedic tones */
    {0x1DC0, 0x1DF9},   /* Combining Diacritical Marks Supplement */
    {0x1DFB, 0x1DFF},   /* Combining marks */
    {0x200B, 0x200F},   /* Zero-width space, joiners, direction marks */
    {0x202A, 0x202E},   /* Bidi formatting characters */
    {0x2060, 0x2064},   /* Word joiner, invisible operators */
    {0x2066, 0x206F},   /* Bidi isolates */
    {0x20D0, 0x20F0},   /* Combining Diacritical Marks for Symbols */
    {0x2CEF, 0x2CF1},   /* Coptic combining marks */
    {0x2D7F, 0x2D7F},   /* Tifinagh consonant joiner */
    {0x2DE0, 0x2DFF},   /* Cyrillic Extended-A combining */
    {0x302A, 0x302D},   /* Ideographic tone marks */
    {0x3099, 0x309A},   /* Combining katakana-hiragana marks */
    {0xA66F, 0xA672},   /* Combining Cyrillic marks */
    {0xA674, 0xA67D},   /* Combining Cyrillic marks */
    {0xA69E, 0xA69F},   /* Cyrillic combining marks */
    {0xA6F0, 0xA6F1},   /* Bamum combining marks */
    {0xA802, 0xA802},   /* Syloti Nagri sign dvisvara */
    {0xA806, 0xA806},   /* Syloti Nagri sign hasanta */
    {0xA80B, 0xA80B},   /* Syloti Nagri sign anusvara */
    {0xA825, 0xA826},   /* Syloti Nagri vowel signs */
    {0xA82C, 0xA82C},   /* Syloti Nagri sign alternate hasanta */
    {0xA8C4, 0xA8C5},   /* Saurashtra signs */
    {0xA8E0, 0xA8F1},   /* Combining Devanagari marks */
    {0xA8FF, 0xA8FF},   /* Devanagari vowel sign ay */
    {0xA926, 0xA92D},   /* Kayah Li vowel signs and tone marks */
    {0xA947, 0xA951},   /* Rejang vowel signs and virama */
    {0xA980, 0xA982},   /* Javanese signs */
    {0xA9B3, 0xA9B3},   /* Javanese sign cecak telu */
    {0xA9B6, 0xA9B9},   /* Javanese vowel signs */
    {0xA9BC, 0xA9BD},   /* Javanese vowel signs */
    {0xA9E5, 0xA9E5},   /* Myanmar sign shan saw */
    {0xAA29, 0xAA2E},   /* Cham vowel signs */
    {0xAA31, 0xAA32},   /* Cham vowel signs */
    {0xAA35, 0xAA36},   /* Cham consonant signs */
    {0xAA43, 0xAA43},   /* Cham consonant sign final ng */
    {0xAA4C, 0xAA4C},   /* Cham consonant sign final m */
    {0xAA7C, 0xAA7C},   /* Myanmar sign tai laing tone-2 */
    {0xAAB0, 0xAAB0},   /* Tai Viet mai kang */
    {0xAAB2, 0xAAB4},   /* Tai Viet vowel signs */
    {0xAAB7, 0xAAB8},   /* Tai Viet vowel signs */
    {0xAABE, 0xAABF},   /* Tai Viet vowel signs */
    {0xAAC1, 0xAAC1},   /* Tai Viet tone mai tho */
    {0xAAEC, 0xAAED},   /* Meetei Mayek vowel signs */
    {0xAAF6, 0xAAF6},   /* Meetei Mayek virama */
    {0xABE5, 0xABE5},   /* Meetei Mayek vowel sign anap */
    {0xABE8, 0xABE8},   /* Meetei Mayek vowel sign unap */
    {0xABED, 0xABED},   /* Meetei Mayek apun iyek */
    {0xFB1E, 0xFB1E},   /* Hebrew point judeo-spanish varika */
    {0xFE00, 0xFE0F},   /* Variation Selectors */
    {0xFE20, 0xFE2F},   /* Combining Half Marks */
    {0xFEFF, 0xFEFF},   /* Zero Width No-Break Space (BOM) */
    {0xFFF9, 0xFFFB},   /* Interlinear annotation anchors */
    {0x101FD, 0x101FD}, /* Phaistos Disc sign combining oblique stroke */
    {0x102E0, 0x102E0}, /* Coptic combining mark */
    {0x10376, 0x1037A}, /* Combining Old Permic letters */
    {0x10A01, 0x10A03}, /* Kharoshthi vowel signs */
    {0x10A05, 0x10A06}, /* Kharoshthi vowel signs */
    {0x10A0C, 0x10A0F}, /* Kharoshthi signs */
    {0x10A38, 0x10A3A}, /* Kharoshthi signs */
    {0x10A3F, 0x10A3F}, /* Kharoshthi virama */
    {0x10AE5, 0x10AE6}, /* Manichaean abbreviation marks */
    {0x10D24, 0x10D27}, /* Hanifi Rohingya signs */
    {0x10EAB, 0x10EAC}, /* Yezidi combining marks */
    {0x10F46, 0x10F50}, /* Sogdian combining marks */
    {0x11001, 0x11001}, /* Brahmi sign anusvara */
    {0x11038, 0x11046}, /* Brahmi vowel signs and virama */
    {0x1107F, 0x11081}, /* Brahmi/Kaithi signs */
    {0x110B3, 0x110B6}, /* Kaithi vowel signs */
    {0x110B9, 0x110BA}, /* Kaithi signs */
    {0x11100, 0x11102}, /* Chakma signs */
    {0x11127, 0x1112B}, /* Chakma vowel signs */
    {0x1112D, 0x11134}, /* Chakma vowel signs and virama */
    {0x11173, 0x11173}, /* Mahajani sign nukta */
    {0x11180, 0x11181}, /* Sharada signs */
    {0x111B6, 0x111BE}, /* Sharada vowel signs */
    {0x111C9, 0x111CC}, /* Sharada signs */
    {0x111CF, 0x111CF}, /* Sharada sign inverted candrabindu */
    {0x1122F, 0x11231}, /* Khojki vowel signs */
    {0x11234, 0x11234}, /* Khojki sign anusvara */
    {0x11236, 0x11237}, /* Khojki signs */
    {0x1123E, 0x1123E}, /* Khojki sign sukun */
    {0x112DF, 0x112DF}, /* Khudawadi sign anusvara */
    {0x112E3, 0x112EA}, /* Khudawadi vowel signs */
    {0x11300, 0x11301}, /* Grantha signs */
    {0x1133B, 0x1133C}, /* Grantha combining marks */
    {0x11340, 0x11340}, /* Grantha vowel sign ii */
    {0x11366, 0x1136C}, /* Combining Grantha marks */
    {0x11370, 0x11374}, /* Combining Grantha marks */
    {0x11438, 0x1143F}, /* Newa vowel signs */
    {0x11442, 0x11444}, /* Newa signs */
    {0x11446, 0x11446}, /* Newa sign nukta */
    {0x1145E, 0x1145E}, /* Newa sandhi mark */
    {0x114B3, 0x114B8}, /* Tirhuta vowel signs */
    {0x114BA, 0x114BA}, /* Tirhuta vowel sign short e */
    {0x114BF, 0x114C0}, /* Tirhuta signs */
    {0x114C2, 0x114C3}, /* Tirhuta signs */
    {0x115B2, 0x115B5}, /* Siddham vowel signs */
    {0x115BC, 0x115BD}, /* Siddham signs */
    {0x115BF, 0x115C0}, /* Siddham signs */
    {0x115DC, 0x115DD}, /* Siddham vowel signs */
    {0x11633, 0x1163A}, /* Modi vowel signs */
    {0x1163D, 0x1163D}, /* Modi sign anusvara */
    {0x1163F, 0x11640}, /* Modi signs */
    {0x116AB, 0x116AB}, /* Takri sign anusvara */
    {0x116AD, 0x116AD}, /* Takri vowel sign aa */
    {0x116B0, 0x116B5}, /* Takri vowel signs */
    {0x116B7, 0x116B7}, /* Takri sign nukta */
    {0x1171D, 0x1171F}, /* Ahom consonant signs */
    {0x11722, 0x11725}, /* Ahom vowel signs */
    {0x11727, 0x1172B}, /* Ahom vowel signs and virama */
    {0x1182F, 0x11837}, /* Dogra vowel signs */
    {0x11839, 0x1183A}, /* Dogra signs */
    {0x1193B, 0x1193C}, /* Dives Akuru signs */
    {0x1193E, 0x1193E}, /* Dives Akuru virama */
    {0x11943, 0x11943}, /* Dives Akuru sign nukta */
    {0x119D4, 0x119D7}, /* Nandinagari vowel signs */
    {0x119DA, 0x119DB}, /* Nandinagari vowel signs */
    {0x119E0, 0x119E0}, /* Nandinagari sign virama */
    {0x11A01, 0x11A0A}, /* Zanabazar Square vowel signs */
    {0x11A33, 0x11A38}, /* Zanabazar Square finals and signs */
    {0x11A3B, 0x11A3E}, /* Zanabazar Square cluster signs */
    {0x11A47, 0x11A47}, /* Zanabazar Square subjoiner */
    {0x11A51, 0x11A56}, /* Soyombo vowel signs */
    {0x11A59, 0x11A5B}, /* Soyombo consonant signs */
    {0x11A8A, 0x11A96}, /* Soyombo finals and signs */
    {0x11A98, 0x11A99}, /* Soyombo gemination and subjoiner */
    {0x11C30, 0x11C36}, /* Bhaiksuki vowel signs */
    {0x11C38, 0x11C3D}, /* Bhaiksuki vowel signs */
    {0x11C3F, 0x11C3F}, /* Bhaiksuki sign virama */
    {0x11C92, 0x11CA7}, /* Marchen subjoined letters */
    {0x11CAA, 0x11CB0}, /* Marchen signs */
    {0x11CB2, 0x11CB3}, /* Marchen signs */
    {0x11CB5, 0x11CB6}, /* Marchen signs */
    {0x11D31, 0x11D36}, /* Masaram Gondi vowel signs */
    {0x11D3A, 0x11D3A}, /* Masaram Gondi vowel sign e */
    {0x11D3C, 0x11D3D}, /* Masaram Gondi vowel signs */
    {0x11D3F, 0x11D45}, /* Masaram Gondi vowel signs and virama */
    {0x11D47, 0x11D47}, /* Masaram Gondi ra-kara */
    {0x11D90, 0x11D91}, /* Gunjala Gondi vowel signs */
    {0x11D95, 0x11D95}, /* Gunjala Gondi sign anusvara */
    {0x11D97, 0x11D97}, /* Gunjala Gondi virama */
    {0x11EF3, 0x11EF4}, /* Makasar vowel signs */
    {0x16AF0, 0x16AF4}, /* Bassa Vah combining marks */
    {0x16B30, 0x16B36}, /* Pahawh Hmong marks */
    {0x16F4F, 0x16F4F}, /* Miao sign consonant modifier bar */
    {0x16F8F, 0x16F92}, /* Miao tone marks */
    {0x16FE4, 0x16FE4}, /* Khitan Small Script filler */
    {0x1BC9D, 0x1BC9E}, /* Duployan thick letter selector */
    {0x1D167, 0x1D169}, /* Musical combining marks */
    {0x1D17B, 0x1D182}, /* Musical combining marks */
    {0x1D185, 0x1D18B}, /* Musical combining marks */
    {0x1D1AA, 0x1D1AD}, /* Musical combining marks */
    {0x1D242, 0x1D244}, /* Combining Greek musical marks */
    {0x1DA00, 0x1DA36}, /* Signwriting head/face modifiers */
    {0x1DA3B, 0x1DA6C}, /* Signwriting modifiers */
    {0x1DA75, 0x1DA75}, /* Signwriting upper body locator */
    {0x1DA84, 0x1DA84}, /* Signwriting location head neck */
    {0x1DA9B, 0x1DA9F}, /* Signwriting modifiers */
    {0x1DAA1, 0x1DAAF}, /* Signwriting modifiers */
    {0x1E000, 0x1E006}, /* Combining Glagolitic letters */
    {0x1E008, 0x1E018}, /* Combining Glagolitic letters */
    {0x1E01B, 0x1E021}, /* Combining Glagolitic letters */
    {0x1E023, 0x1E024}, /* Combining Glagolitic letters */
    {0x1E026, 0x1E02A}, /* Combining Glagolitic letters */
    {0x1E130, 0x1E136}, /* Nyiakeng Puachue Hmong tone marks */
    {0x1E2EC, 0x1E2EF}, /* Wancho tone marks */
    {0x1E8D0, 0x1E8D6}, /* Mende Kikakui combining marks */
    {0x1E944, 0x1E94A}, /* Adlam modifiers */
    {0xE0100, 0xE01EF}, /* Variation Selectors Supplement */
};
#define UTFLITE__ZERO_WIDTH_COUNT (sizeof(UTFLITE__ZERO_WIDTH_RANGES) / sizeof(UTFLITE__ZERO_WIDTH_RANGES[0]))

/*
 * Double-width character ranges.
 * These characters take 2 display columns (CJK, fullwidth forms).
 * Sorted by start codepoint for binary search.
 */
static const utflite__unicode_range UTFLITE__DOUBLE_WIDTH_RANGES[] = {
    {0x1100, 0x115F},   /* Hangul Jamo leading consonants */
    {0x231A, 0x231B},   /* Watch, hourglass */
    {0x2329, 0x232A},   /* Left/right angle brackets */
    {0x23E9, 0x23F3},   /* Various symbols */
    {0x23F8, 0x23FA},   /* Various symbols */
    {0x25FD, 0x25FE},   /* Medium squares */
    {0x2614, 0x2615},   /* Umbrella, hot beverage */
    {0x2648, 0x2653},   /* Zodiac signs */
    {0x267F, 0x267F},   /* Wheelchair symbol */
    {0x2693, 0x2693},   /* Anchor */
    {0x26A1, 0x26A1},   /* High voltage */
    {0x26AA, 0x26AB},   /* Medium circles */
    {0x26BD, 0x26BE},   /* Soccer ball, baseball */
    {0x26C4, 0x26C5},   /* Snowman, sun behind cloud */
    {0x26CE, 0x26CE},   /* Ophiuchus */
    {0x26D4, 0x26D4},   /* No entry */
    {0x26EA, 0x26EA},   /* Church */
    {0x26F2, 0x26F3},   /* Fountain, golf */
    {0x26F5, 0x26F5},   /* Sailboat */
    {0x26FA, 0x26FA},   /* Tent */
    {0x26FD, 0x26FD},   /* Fuel pump */
    {0x2702, 0x2702},   /* Scissors */
    {0x2705, 0x2705},   /* Check mark */
    {0x2708, 0x270D},   /* Airplane through writing hand */
    {0x270F, 0x270F},   /* Pencil */
    {0x2712, 0x2712},   /* Black nib */
    {0x2714, 0x2714},   /* Check mark */
    {0x2716, 0x2716},   /* X mark */
    {0x271D, 0x271D},   /* Latin cross */
    {0x2721, 0x2721},   /* Star of David */
    {0x2728, 0x2728},   /* Sparkles */
    {0x2733, 0x2734},   /* Eight spoked asterisk, eight pointed star */
    {0x2744, 0x2744},   /* Snowflake */
    {0x2747, 0x2747},   /* Sparkle */
    {0x274C, 0x274C},   /* Cross mark */
    {0x274E, 0x274E},   /* Cross mark with box */
    {0x2753, 0x2755},   /* Question marks and exclamation */
    {0x2757, 0x2757},   /* Exclamation mark */
    {0x2763, 0x2764},   /* Heart exclamation, heart */
    {0x2795, 0x2797},   /* Plus, minus, division */
    {0x27A1, 0x27A1},   /* Right arrow */
    {0x27B0, 0x27B0},   /* Curly loop */
    {0x27BF, 0x27BF},   /* Double curly loop */
    {0x2934, 0x2935},   /* Arrows */
    {0x2B05, 0x2B07},   /* Arrows */
    {0x2B1B, 0x2B1C},   /* Squares */
    {0x2B50, 0x2B50},   /* Star */
    {0x2B55, 0x2B55},   /* Heavy large circle */
    {0x2E80, 0x2E99},   /* CJK Radicals Supplement */
    {0x2E9B, 0x2EF3},   /* CJK Radicals Supplement */
    {0x2F00, 0x2FD5},   /* Kangxi Radicals */
    {0x2FF0, 0x2FFF},   /* Ideographic Description Characters */
    {0x3000, 0x303E},   /* CJK Symbols and Punctuation */
    {0x3041, 0x3096},   /* Hiragana */
    {0x309B, 0x30FF},   /* Katakana */
    {0x3105, 0x312F},   /* Bopomofo */
    {0x3131, 0x318E},   /* Hangul Compatibility Jamo */
    {0x3190, 0x31E3},   /* Kanbun, Bopomofo Extended */
    {0x31F0, 0x321E},   /* Katakana Phonetic Extensions */
    {0x3220, 0x3247},   /* Enclosed CJK Letters and Months */
    {0x3250, 0x4DBF},   /* CJK, including Extension A */
    {0x4E00, 0x9FFF},   /* CJK Unified Ideographs */
    {0xA000, 0xA48C},   /* Yi Syllables */
    {0xA490, 0xA4C6},   /* Yi Radicals */
    {0xA960, 0xA97F},   /* Hangul Jamo Extended-A */
    {0xAC00, 0xD7A3},   /* Hangul Syllables */
    {0xF900, 0xFAFF},   /* CJK Compatibility Ideographs */
    {0xFE10, 0xFE19},   /* Vertical Forms */
    {0xFE30, 0xFE6F},   /* CJK Compatibility Forms */
    {0xFF00, 0xFF60},   /* Fullwidth ASCII and punctuation */
    {0xFFE0, 0xFFE6},   /* Fullwidth currency symbols */
    {0x16FE0, 0x16FE4}, /* Ideographic symbols */
    {0x17000, 0x187F7}, /* Tangut */
    {0x18800, 0x18CD5}, /* Tangut Components */
    {0x18D00, 0x18D08}, /* Tangut Supplement */
    {0x1AFF0, 0x1AFFF}, /* Kana Extended-B */
    {0x1B000, 0x1B11E}, /* Kana Supplement and Extended */
    {0x1B150, 0x1B152}, /* Hiragana Extended */
    {0x1B164, 0x1B167}, /* Katakana Extended */
    {0x1B170, 0x1B2FB}, /* Nushu */
    {0x1F004, 0x1F004}, /* Mahjong tile */
    {0x1F0CF, 0x1F0CF}, /* Playing card */
    {0x1F18E, 0x1F18E}, /* AB button */
    {0x1F191, 0x1F19A}, /* Squared symbols */
    {0x1F200, 0x1F202}, /* Enclosed ideographic supplement */
    {0x1F210, 0x1F23B}, /* Enclosed ideographic supplement */
    {0x1F240, 0x1F248}, /* Enclosed ideographic supplement */
    {0x1F250, 0x1F251}, /* Enclosed ideographic supplement */
    {0x1F260, 0x1F265}, /* Rounded symbols */
    {0x1F300, 0x1F320}, /* Miscellaneous Symbols and Pictographs */
    {0x1F32D, 0x1F335}, /* Food and drink */
    {0x1F337, 0x1F37C}, /* Plants and food */
    {0x1F37E, 0x1F393}, /* Food, celebration, education */
    {0x1F3A0, 0x1F3CA}, /* Entertainment */
    {0x1F3CF, 0x1F3D3}, /* Sports */
    {0x1F3E0, 0x1F3F0}, /* Buildings */
    {0x1F3F4, 0x1F3F4}, /* Waving black flag */
    {0x1F3F8, 0x1F43E}, /* Sports, animals */
    {0x1F440, 0x1F440}, /* Eyes */
    {0x1F442, 0x1F4FC}, /* Body parts, symbols */
    {0x1F4FF, 0x1F53D}, /* Objects */
    {0x1F54B, 0x1F54E}, /* Religious symbols */
    {0x1F550, 0x1F567}, /* Clock faces */
    {0x1F57A, 0x1F57A}, /* Man dancing */
    {0x1F595, 0x1F596}, /* Hand gestures */
    {0x1F5A4, 0x1F5A4}, /* Black heart */
    {0x1F5FB, 0x1F64F}, /* Places, faces, gestures */
    {0x1F680, 0x1F6C5}, /* Transport and map symbols */
    {0x1F6CC, 0x1F6CC}, /* Sleeping accommodation */
    {0x1F6D0, 0x1F6D2}, /* Symbols */
    {0x1F6D5, 0x1F6D7}, /* Symbols */
    {0x1F6DD, 0x1F6DF}, /* Symbols */
    {0x1F6EB, 0x1F6EC}, /* Airplane symbols */
    {0x1F6F4, 0x1F6FC}, /* Transport symbols */
    {0x1F7E0, 0x1F7EB}, /* Colored circles and squares */
    {0x1F7F0, 0x1F7F0}, /* Heavy equals sign */
    {0x1F90C, 0x1F93A}, /* Emoji */
    {0x1F93C, 0x1F945}, /* Sports emoji */
    {0x1F947, 0x1F9FF}, /* Various emoji */
    {0x1FA00, 0x1FA53}, /* Chess symbols */
    {0x1FA60, 0x1FA6D}, /* Xiangqi pieces */
    {0x1FA70, 0x1FA74}, /* Emoji */
    {0x1FA78, 0x1FA7C}, /* Medical symbols */
    {0x1FA80, 0x1FA86}, /* Emoji */
    {0x1FA90, 0x1FAAC}, /* Emoji */
    {0x1FAB0, 0x1FABA}, /* Emoji */
    {0x1FAC0, 0x1FAC5}, /* Emoji */
    {0x1FAD0, 0x1FAD9}, /* Food emoji */
    {0x1FAE0, 0x1FAE7}, /* Face emoji */
    {0x1FAF0, 0x1FAF6}, /* Hand emoji */
    {0x20000, 0x2FFFD}, /* CJK Extension B, C, D, E, F */
    {0x30000, 0x3FFFD}, /* CJK Extension G, H */
};
#define UTFLITE__DOUBLE_WIDTH_COUNT (sizeof(UTFLITE__DOUBLE_WIDTH_RANGES) / sizeof(UTFLITE__DOUBLE_WIDTH_RANGES[0]))

static inline int utflite__unicode_range_contains(uint32_t codepoint,
                                                   const utflite__unicode_range *ranges,
                                                   int count) {
    int low = 0;
    int high = count - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (codepoint < ranges[mid].start) {
            high = mid - 1;
        } else if (codepoint > ranges[mid].end) {
            low = mid + 1;
        } else {
            return 1;
        }
    }
    return 0;
}

/*
 * Error handling strategy: consume minimal bytes for structural errors,
 * consume full sequence for semantic errors.
 */
int utflite_decode(const char *bytes, int length, uint32_t *codepoint) {
    if (length <= 0 || !bytes) {
        *codepoint = UTFLITE_REPLACEMENT_CHAR;
        return 1;
    }
    unsigned char first = (unsigned char)bytes[0];
    if (first < 0x80) {
        *codepoint = first;
        return 1;
    }
    int sequence_length;
    uint32_t cp;
    if ((first & 0xE0) == 0xC0) {
        sequence_length = 2;
        cp = first & 0x1F;
    } else if ((first & 0xF0) == 0xE0) {
        sequence_length = 3;
        cp = first & 0x0F;
    } else if ((first & 0xF8) == 0xF0) {
        sequence_length = 4;
        cp = first & 0x07;
    } else {
        *codepoint = UTFLITE_REPLACEMENT_CHAR;
        return 1;
    }
    if (length < sequence_length) {
        *codepoint = UTFLITE_REPLACEMENT_CHAR;
        return 1;
    }
    for (int i = 1; i < sequence_length; i++) {
        unsigned char byte = (unsigned char)bytes[i];
        if ((byte & 0xC0) != 0x80) {
            *codepoint = UTFLITE_REPLACEMENT_CHAR;
            return 1;
        }
        cp = (cp << 6) | (byte & 0x3F);
    }
    if ((sequence_length == 2 && cp < 0x80) ||
        (sequence_length == 3 && cp < 0x800) ||
        (sequence_length == 4 && cp < 0x10000)) {
        *codepoint = UTFLITE_REPLACEMENT_CHAR;
        return sequence_length;
    }
    if (cp >= 0xD800 && cp <= 0xDFFF) {
        *codepoint = UTFLITE_REPLACEMENT_CHAR;
        return sequence_length;
    }
    if (cp > 0x10FFFF) {
        *codepoint = UTFLITE_REPLACEMENT_CHAR;
        return sequence_length;
    }
    *codepoint = cp;
    return sequence_length;
}

int utflite_encode(uint32_t codepoint, char *buffer) {
    if (codepoint < 0x80) {
        buffer[0] = (char)codepoint;
        return 1;
    } else if (codepoint < 0x800) {
        buffer[0] = (char)(0xC0 | (codepoint >> 6));
        buffer[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    } else if (codepoint < 0x10000) {
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
            return 0;
        }
        buffer[0] = (char)(0xE0 | (codepoint >> 12));
        buffer[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    } else if (codepoint <= 0x10FFFF) {
        buffer[0] = (char)(0xF0 | (codepoint >> 18));
        buffer[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        buffer[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    }
    return 0;
}

int utflite_codepoint_width(uint32_t codepoint) {
    if (codepoint < 0x20) {
        if (codepoint == 0x00) return 0;
        return -1;
    }
    if (codepoint < 0x7F) {
        return 1;
    }
    if (codepoint == 0x7F) {
        return -1;
    }
    if (codepoint < 0xA0) {
        return -1;
    }
    if (codepoint == 0x00AD) {
        return 1;
    }
    if (utflite__unicode_range_contains(codepoint, UTFLITE__ZERO_WIDTH_RANGES, UTFLITE__ZERO_WIDTH_COUNT)) {
        return 0;
    }
    if (utflite__unicode_range_contains(codepoint, UTFLITE__DOUBLE_WIDTH_RANGES, UTFLITE__DOUBLE_WIDTH_COUNT)) {
        return 2;
    }
    return 1;
}

int utflite_char_width(const char *text, int length, int offset) {
    if (offset >= length) {
        return 0;
    }
    uint32_t codepoint;
    utflite_decode(text + offset, length - offset, &codepoint);
    return utflite_codepoint_width(codepoint);
}

int utflite_next_char(const char *text, int length, int offset) {
    if (offset >= length) {
        return length;
    }
    uint32_t codepoint;
    int char_bytes = utflite_decode(text + offset, length - offset, &codepoint);
    int next_offset = offset + char_bytes;
    return (next_offset > length) ? length : next_offset;
}

int utflite_prev_char(const char *text, int offset) {
    if (offset <= 0) {
        return 0;
    }
    int pos = offset - 1;
    int limit = (offset > 4) ? offset - 4 : 0;
    while (pos > limit && ((unsigned char)text[pos] & 0xC0) == 0x80) {
        pos--;
    }
    return pos;
}

/*
 * WARNING: Grapheme cluster segmentation is NOT implemented.
 * This function currently just advances by one codepoint.
 * Multi-codepoint graphemes (emoji sequences, combining characters)
 * will not be handled correctly.
 */
int utflite_next_grapheme(const char *text, int length, int offset) {
    return utflite_next_char(text, length, offset);
}

/*
 * WARNING: Grapheme cluster segmentation is NOT implemented.
 * This function currently just advances by one codepoint.
 * Multi-codepoint graphemes (emoji sequences, combining characters)
 * will not be handled correctly.
 */
int utflite_prev_grapheme(const char *text, int offset) {
    return utflite_prev_char(text, offset);
}

int utflite_validate(const char *text, int length, int *error_offset) {
    int offset = 0;
    while (offset < length) {
        uint32_t codepoint;
        int bytes = utflite_decode(text + offset, length - offset, &codepoint);
        if (codepoint == UTFLITE_REPLACEMENT_CHAR) {
            unsigned char first = (unsigned char)text[offset];
            if (first != 0xEF || length - offset < 3 ||
                (unsigned char)text[offset + 1] != 0xBF ||
                (unsigned char)text[offset + 2] != 0xBD) {
                if (error_offset) {
                    *error_offset = offset;
                }
                return 0;
            }
        }
        offset += bytes;
    }
    return 1;
}

int utflite_codepoint_count(const char *text, int length) {
    int count = 0;
    int offset = 0;
    while (offset < length) {
        offset = utflite_next_char(text, length, offset);
        count++;
    }
    return count;
}

int utflite_string_width(const char *text, int length) {
    int width = 0;
    int offset = 0;
    while (offset < length) {
        int char_width = utflite_char_width(text, length, offset);
        if (char_width > 0) {
            width += char_width;
        }
        offset = utflite_next_char(text, length, offset);
    }
    return width;
}

int utflite_is_zero_width(uint32_t codepoint) {
    return utflite__unicode_range_contains(codepoint, UTFLITE__ZERO_WIDTH_RANGES, UTFLITE__ZERO_WIDTH_COUNT);
}

int utflite_is_wide(uint32_t codepoint) {
    return utflite__unicode_range_contains(codepoint, UTFLITE__DOUBLE_WIDTH_RANGES, UTFLITE__DOUBLE_WIDTH_COUNT);
}

int utflite_truncate(const char *text, int length, int max_cols) {
    int width = 0;
    int offset = 0;

    while (offset < length) {
        int char_width = utflite_char_width(text, length, offset);
        if (char_width > 0) {
            if (width + char_width > max_cols) {
                return offset;
            }
            width += char_width;
        }
        offset = utflite_next_char(text, length, offset);
    }
    return length;
}

#endif /* UTFLITE_IMPLEMENTATION */
