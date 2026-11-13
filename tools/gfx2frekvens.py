"""
Convert an Adafruit GFX font header into a Frekvens FontModule.

Usage:
    python tools/gfx2frekvens.py <gfx_font.h> <ClassName> "<Display name>" <FONT_FLAG> [--out firmware]

Writes firmware/include/fonts/<ClassName>.h and firmware/src/fonts/<ClassName>.cpp.
The generated font still has to be registered in FontsService and enabled in .env.

GFX glyphs are stored as a packed bit stream (row-major, MSB first) inside a
bounding box of w x h pixels placed at (xOffset, yOffset) from the cursor, with
yOffset measured from the baseline (negative = above). Frekvens glyphs are a list
of rows, each row right-aligned in an unsigned integer, drawn bottom-aligned on the
baseline and moved up by offsetY.
"""

import argparse
import pathlib
import re
import sys


def parse_gfx(source: str) -> tuple[list[int], list[tuple[int, int, int, int, int, int]], int, int]:
    """Return (bitmap bytes, glyphs, first codepoint, last codepoint) from a GFX header."""
    without_comments = re.sub(r"//[^\n]*|/\*.*?\*/", "", source, flags=re.S)
    bitmap_match = re.search(r"Bitmaps\s*\[\s*\]\s*PROGMEM\s*=\s*\{(.*?)\};", without_comments, re.S)
    glyph_match = re.search(r"Glyphs\s*\[\s*\]\s*PROGMEM\s*=\s*\{(.*?)\};", without_comments, re.S)
    font_match = re.search(r"GFXfont\s+\w+\s+PROGMEM\s*=\s*\{(.*?)\};", without_comments, re.S)
    if not bitmap_match or not glyph_match or not font_match:
        raise ValueError("not an Adafruit GFX font header")
    bitmap = [int(value, 16) for value in re.findall(r"0x[0-9A-Fa-f]+", bitmap_match.group(1))]
    glyphs = [
        tuple(int(value) for value in entry.split(","))
        for entry in re.findall(r"\{\s*(-?\d+\s*,\s*-?\d+\s*,\s*-?\d+\s*,\s*-?\d+\s*,\s*-?\d+\s*,\s*-?\d+)\s*\}", glyph_match.group(1))
    ]
    numbers = re.findall(r"0x[0-9A-Fa-f]+|\b\d+\b", font_match.group(1).split(")")[-1])
    first, last = int(numbers[0], 0), int(numbers[1], 0)
    if len(glyphs) > last - first + 1:
        raise ValueError(f"expected at most {last - first + 1} glyphs, found {len(glyphs)}")
    if len(glyphs) < last - first + 1:
        # Some generators declare a wider range than the glyph table holds.
        print(f"Warning: font declares U+{first:04X}-U+{last:04X} but has {len(glyphs)} glyphs", file=sys.stderr)
        last = first + len(glyphs) - 1
    return bitmap, glyphs, first, last


def glyph_rows(bitmap: list[int], offset: int, width: int, height: int) -> list[int]:
    """Unpack a GFX glyph into rows, each right-aligned in `width` bits."""
    rows = []
    bit = 0
    for _ in range(height):
        row = 0
        for _ in range(width):
            byte = bitmap[offset + bit // 8]
            row = (row << 1) | ((byte >> (7 - bit % 8)) & 1)
            bit += 1
        rows.append(row)
    return rows


def trim(rows: list[int], width: int, y_offset: int) -> tuple[list[int], int, int]:
    """
    Drop empty rows at the top and bottom and empty columns on the left.

    Returns (rows, left padding, bottom relative to the baseline). Frekvens measures a
    glyph width from its highest set bit, so empty left columns become offsetX.
    """
    while rows and rows[0] == 0:
        rows = rows[1:]
        y_offset += 1
    while rows and rows[-1] == 0:
        rows = rows[:-1]
    if not rows:
        return [], 0, 0
    used = max(row.bit_length() for row in rows)
    return rows, width - used, y_offset + len(rows)


def identifier(codepoint: int) -> str:
    return f"u{codepoint:04X}"


def describe(codepoint: int) -> str:
    char = chr(codepoint)
    shown = char if char.isprintable() and char not in "\\" else " "
    return f"U+{codepoint:04X} {shown}"


def generate(header: str, class_name: str, display_name: str, flag: str, origin: str) -> tuple[str, str]:
    bitmap, glyphs, first, _last = parse_gfx(header)
    declarations = []
    cases = []
    space_advance = None
    for index, (offset, width, height, x_advance, x_offset, y_offset) in enumerate(glyphs):
        codepoint = first + index
        if codepoint == 0x20:
            space_advance = x_advance
            continue
        rows, left, bottom = trim(glyph_rows(bitmap, offset, width, height), width, y_offset)
        if not rows:
            continue
        storage = "uint8_t" if max(row.bit_length() for row in rows) <= 8 else "uint16_t"
        bits = max(row.bit_length() for row in rows)
        body = "\n".join(f"        0b{row:0{bits}b}U," for row in rows)
        declarations.append(
            f"    // {describe(codepoint)}\n"
            f"    static constexpr std::array<{storage}, {len(rows)}U> {identifier(codepoint)}{{\n{body}\n    }};\n"
        )
        offset_x = max(0, x_offset + left)
        offset_y = -bottom
        char_literal = "'\\''" if codepoint == 0x27 else "'\\\\'" if codepoint == 0x5C else f"'{chr(codepoint)}'"
        if offset_x == 0 and offset_y == 0:
            call = f"toSymbol({identifier(codepoint)})"
        elif offset_x == 0:
            call = f"toSymbol({identifier(codepoint)}, {offset_y})"
        else:
            call = f"toSymbol({identifier(codepoint)}, {offset_x}U, {offset_y})"
        cases.append(f"    case {char_literal}: // {describe(codepoint)}\n        return {call};")
    if space_advance is not None:
        cases.insert(0, f"    case ' ': // U+0020 SPACE\n        return whitespace({max(1, space_advance - 1)}U);")

    include_guard_flag = flag
    header_out = f"""#pragma once

#if {include_guard_flag}

#include "modules/FontModule.h"

#include <array>

/**
 * {display_name}
 *
 * Generated by tools/gfx2frekvens.py from {origin}.
 */
class {class_name} final : public FontModule
{{
private:
{chr(10).join(declarations)}
public:
    static constexpr std::string_view name{{"{display_name}"}};

    explicit {class_name}() : FontModule(name) {{}};

    [[nodiscard]] Symbol getChar(char32_t character) const override;
}};

#endif // {include_guard_flag}
"""
    rows_max = 0
    for offset, width, height, *_ in glyphs:
        rows_max = max(rows_max, height)
    source_out = f"""#if {include_guard_flag}

#include "fonts/{class_name}.h"

#include "config/constants.h" // NOLINT(misc-include-cleaner)

static_assert(GRID_ROWS >= {rows_max}U, __STRING({include_guard_flag}) " is not compatible with this device's display size.");

/**
 * @brief Maps a Unicode character to its {display_name} symbol.
 *
 * @param character Unicode character to convert.
 * @return FontModule::Symbol The corresponding symbol, or an empty symbol for unsupported characters.
 */
FontModule::Symbol {class_name}::getChar(char32_t character) const
{{
    switch (character)
    {{
{chr(10).join(cases)}
    default:
        return {{}};
    }}
}}

#endif // {include_guard_flag}
"""
    return header_out, source_out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("source", type=pathlib.Path)
    parser.add_argument("class_name")
    parser.add_argument("display_name")
    parser.add_argument("flag")
    parser.add_argument("--out", type=pathlib.Path, default=pathlib.Path("firmware"))
    parser.add_argument("--origin", default=None, help="attribution written in the generated header")
    args = parser.parse_args()
    header, source = generate(
        args.source.read_text(encoding="utf-8", errors="ignore"),
        args.class_name,
        args.display_name,
        args.flag,
        args.origin or args.source.name,
    )
    (args.out / "include" / "fonts" / f"{args.class_name}.h").write_text(header, encoding="utf-8", newline="\n")
    (args.out / "src" / "fonts" / f"{args.class_name}.cpp").write_text(source, encoding="utf-8", newline="\n")
    print(f"{args.class_name}: written to {args.out}/include/fonts and {args.out}/src/fonts")
    return 0


if __name__ == "__main__":
    sys.exit(main())
