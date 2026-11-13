#if FONT_ROBOTOMONO

#include "fonts/RobotoMonoFont.h"

#include "config/constants.h" // NOLINT(misc-include-cleaner)

static_assert(GRID_ROWS >= 8U, __STRING(FONT_ROBOTOMONO) " is not compatible with this device's display size.");

/**
 * @brief Maps a Unicode character to its Roboto Mono symbol.
 *
 * @param character Unicode character to convert.
 * @return FontModule::Symbol The corresponding symbol, or an empty symbol for unsupported characters.
 */
FontModule::Symbol RobotoMonoFont::getChar(char32_t character) const
{
    switch (character)
    {
    case ' ': // U+0020 SPACE
        return whitespace(4U);
    case '!': // U+0021 !
        return toSymbol(u0021, 1U, 0);
    case '"': // U+0022 "
        return toSymbol(u0022, 1U, 4);
    case '#': // U+0023 #
        return toSymbol(u0023);
    case '$': // U+0024 $
        return toSymbol(u0024);
    case '%': // U+0025 %
        return toSymbol(u0025);
    case '&': // U+0026 &
        return toSymbol(u0026);
    case '\'': // U+0027 '
        return toSymbol(u0027, 1U, 4);
    case '(': // U+0028 (
        return toSymbol(u0028, 1U, -1);
    case ')': // U+0029 )
        return toSymbol(u0029, 1U, -1);
    case '*': // U+002A *
        return toSymbol(u002A);
    case '+': // U+002B +
        return toSymbol(u002B, 1);
    case ',': // U+002C ,
        return toSymbol(u002C, 1U, -1);
    case '-': // U+002D -
        return toSymbol(u002D, 1U, 2);
    case '.': // U+002E .
        return toSymbol(u002E, 1U, 0);
    case '/': // U+002F /
        return toSymbol(u002F, 1U, 0);
    case '0': // U+0030 0
        return toSymbol(u0030);
    case '1': // U+0031 1
        return toSymbol(u0031, 1U, 0);
    case '2': // U+0032 2
        return toSymbol(u0032);
    case '3': // U+0033 3
        return toSymbol(u0033);
    case '4': // U+0034 4
        return toSymbol(u0034);
    case '5': // U+0035 5
        return toSymbol(u0035);
    case '6': // U+0036 6
        return toSymbol(u0036);
    case '7': // U+0037 7
        return toSymbol(u0037);
    case '8': // U+0038 8
        return toSymbol(u0038);
    case '9': // U+0039 9
        return toSymbol(u0039);
    case ':': // U+003A :
        return toSymbol(u003A, 1U, 0);
    case ';': // U+003B ;
        return toSymbol(u003B, 1U, -1);
    case '<': // U+003C <
        return toSymbol(u003C);
    case '=': // U+003D =
        return toSymbol(u003D, 1);
    case '>': // U+003E >
        return toSymbol(u003E);
    case '?': // U+003F ?
        return toSymbol(u003F);
    case '@': // U+0040 @
        return toSymbol(u0040);
    case 'A': // U+0041 A
        return toSymbol(u0041);
    case 'B': // U+0042 B
        return toSymbol(u0042);
    case 'C': // U+0043 C
        return toSymbol(u0043);
    case 'D': // U+0044 D
        return toSymbol(u0044);
    case 'E': // U+0045 E
        return toSymbol(u0045);
    case 'F': // U+0046 F
        return toSymbol(u0046);
    case 'G': // U+0047 G
        return toSymbol(u0047);
    case 'H': // U+0048 H
        return toSymbol(u0048);
    case 'I': // U+0049 I
        return toSymbol(u0049, 1U, 0);
    case 'J': // U+004A J
        return toSymbol(u004A);
    case 'K': // U+004B K
        return toSymbol(u004B);
    case 'L': // U+004C L
        return toSymbol(u004C);
    case 'M': // U+004D M
        return toSymbol(u004D);
    case 'N': // U+004E N
        return toSymbol(u004E);
    case 'O': // U+004F O
        return toSymbol(u004F);
    case 'P': // U+0050 P
        return toSymbol(u0050);
    case 'Q': // U+0051 Q
        return toSymbol(u0051, -1);
    case 'R': // U+0052 R
        return toSymbol(u0052);
    case 'S': // U+0053 S
        return toSymbol(u0053);
    case 'T': // U+0054 T
        return toSymbol(u0054);
    case 'U': // U+0055 U
        return toSymbol(u0055);
    case 'V': // U+0056 V
        return toSymbol(u0056);
    case 'W': // U+0057 W
        return toSymbol(u0057);
    case 'X': // U+0058 X
        return toSymbol(u0058);
    case 'Y': // U+0059 Y
        return toSymbol(u0059);
    case 'Z': // U+005A Z
        return toSymbol(u005A);
    case '[': // U+005B [
        return toSymbol(u005B, 1U, -1);
    case '\\': // U+005C  
        return toSymbol(u005C, 1U, 0);
    case ']': // U+005D ]
        return toSymbol(u005D, 1U, -1);
    case '^': // U+005E ^
        return toSymbol(u005E, 1U, 2);
    case '_': // U+005F _
        return toSymbol(u005F, -1);
    case '`': // U+0060 `
        return toSymbol(u0060, 1U, 4);
    case 'a': // U+0061 a
        return toSymbol(u0061);
    case 'b': // U+0062 b
        return toSymbol(u0062);
    case 'c': // U+0063 c
        return toSymbol(u0063);
    case 'd': // U+0064 d
        return toSymbol(u0064);
    case 'e': // U+0065 e
        return toSymbol(u0065);
    case 'f': // U+0066 f
        return toSymbol(u0066);
    case 'g': // U+0067 g
        return toSymbol(u0067, -1);
    case 'h': // U+0068 h
        return toSymbol(u0068);
    case 'i': // U+0069 i
        return toSymbol(u0069, 1U, 0);
    case 'j': // U+006A j
        return toSymbol(u006A, 1U, -1);
    case 'k': // U+006B k
        return toSymbol(u006B);
    case 'l': // U+006C l
        return toSymbol(u006C, 1U, 0);
    case 'm': // U+006D m
        return toSymbol(u006D);
    case 'n': // U+006E n
        return toSymbol(u006E);
    case 'o': // U+006F o
        return toSymbol(u006F);
    case 'p': // U+0070 p
        return toSymbol(u0070, -1);
    case 'q': // U+0071 q
        return toSymbol(u0071, -1);
    case 'r': // U+0072 r
        return toSymbol(u0072, 1U, 0);
    case 's': // U+0073 s
        return toSymbol(u0073);
    case 't': // U+0074 t
        return toSymbol(u0074);
    case 'u': // U+0075 u
        return toSymbol(u0075);
    case 'v': // U+0076 v
        return toSymbol(u0076);
    case 'w': // U+0077 w
        return toSymbol(u0077);
    case 'x': // U+0078 x
        return toSymbol(u0078);
    case 'y': // U+0079 y
        return toSymbol(u0079, -1);
    case 'z': // U+007A z
        return toSymbol(u007A);
    case '{': // U+007B {
        return toSymbol(u007B, 1U, -1);
    case '|': // U+007C |
        return toSymbol(u007C, 1U, -1);
    case '}': // U+007D }
        return toSymbol(u007D, 1U, -1);
    default:
        return {};
    }
}

#endif // FONT_ROBOTOMONO
