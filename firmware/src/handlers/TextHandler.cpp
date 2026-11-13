#include "handlers/TextHandler.h"

#include "config/constants.h"        // NOLINT(misc-include-cleaner)
#include "services/DisplayService.h" // NOLINT(misc-include-cleaner)
#include "services/FontsService.h"

#include <cctype>

/**
 * @brief Initializes a text handler and calculates the text dimensions.
 *
 * @param text UTF-8 text to measure and render, optionally with `{font:Name}` markup.
 * @param font Default font, used until the text switches font.
 */
TextHandler::TextHandler(std::string text, const FontModule &font)
{
    parse(text, font);
    if (glyphs.empty())
    {
        return;
    }
    int8_t yMax{0};
    int8_t yMin{0};
    for (const auto &[codepoint, glyphFont] : glyphs)
    {
        const FontModule::Symbol character{glyphFont->getChar(codepoint)};
        std::visit(
            [&](auto &&bitmap)
            {
                if (!bitmap.empty())
                {
                    yMax = max<int8_t>(yMax, bitmap.size() + character.offsetY);
                }
            },
            character.bitmap);
        yMin = min<int8_t>(yMin, character.offsetY);
    }
    height = yMax - yMin;
    tracking = static_cast<uint8_t>(ceilf(height / Display.getRatio() / 10.0F));
    width = 0U;
    for (const auto &[codepoint, glyphFont] : glyphs)
    {
        const FontModule::Symbol character{glyphFont->getChar(codepoint)};
        std::visit(
            [&](auto &&bitmap)
            {
                if (!bitmap.empty())
                {
                    width += calcMsbMax(bitmap) + 1U + character.offsetX + tracking;
                }
                else if (character.offsetX != 0U)
                {
                    width += character.offsetX + tracking;
                }
                else
                {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
                    ESP_LOGV(glyphFont->name.data(), "missing symbol 0x%X %s", codepoint, encode(codepoint).data());
                }
            },
            character.bitmap);
    }
    if (width > tracking)
    {
        width -= tracking;
    }
}

void TextHandler::draw(uint8_t brightness) const
{
    draw(max(0U, (GRID_COLUMNS - width) / 2U), (GRID_ROWS - height) / 2U, brightness);
}

/**
 * @brief Renders the text at the specified position.
 *
 * @param x Horizontal starting position.
 * @param y Top of the text box; glyphs of every font share the same baseline.
 * @param brightness Pixel brightness.
 */
void TextHandler::draw(int16_t x, int8_t y, uint8_t brightness) const
{
    for (const auto &[codepoint, glyphFont] : glyphs)
    {
        const FontModule::Symbol character{glyphFont->getChar(codepoint)};
        std::visit(
            [&](auto &&bitmap)
            {
                const uint8_t _height{static_cast<uint8_t>(bitmap.size())};
                if (_height != 0U)
                {
                    const uint8_t msbMax{calcMsbMax(bitmap)};
                    for (uint16_t _x{0U}; _x <= msbMax; ++_x)
                    {
                        for (uint8_t _y{0U}; _y < _height; ++_y)
                        {
                            const int pixelX{static_cast<int>(x) + static_cast<int>(character.offsetX) +
                                             static_cast<int>(_x)};
                            const int pixelY{static_cast<int>(y) + static_cast<int>(height) -
                                             static_cast<int>(_height) - static_cast<int>(character.offsetY) +
                                             static_cast<int>(_y)};
                            if (pixelX >= 0 && pixelX < GRID_COLUMNS && pixelY >= 0 && pixelY < GRID_ROWS &&
                                (((bitmap[_y] >> (msbMax - _x)) & 0b1U) != 0U))
                            {
                                Display.setPixel(
                                    static_cast<uint8_t>(pixelX), static_cast<uint8_t>(pixelY), brightness);
                            }
                        }
                    }
                    x += msbMax + 1 + character.offsetX + tracking;
                }
                else if (character.offsetX != 0U)
                {
                    x += character.offsetX + tracking;
                }
            },
            character.bitmap);
    }
}

uint8_t TextHandler::getHeight() const { return height; }

/**
 * @brief Retrieves the measured text width.
 *
 * @return uint8_t Text width in pixels.
 */
uint8_t TextHandler::getWidth() const { return width; }

/**
 * @brief Splits the text into glyphs, each bound to the font active at that point.
 *
 * `{font:Name}` switches to the named font and `{font:}` back to the default one.
 * Markup naming an unknown or disabled font is dropped and the current font is kept.
 *
 * @param text UTF-8 text with optional markup.
 * @param font Default font.
 */
void TextHandler::parse(const std::string &text, const FontModule &font)
{
    const FontModule *current{&font};
    size_t index{0U};
    while (index < text.length())
    {
        if (text.compare(index, markupOpen.size(), markupOpen) == 0)
        {
            const size_t close{text.find('}', index + markupOpen.size())};
            if (close != std::string::npos)
            {
                const std::string_view fontName{
                    std::string_view{text}.substr(index + markupOpen.size(), close - index - markupOpen.size())};
                if (fontName.empty())
                {
                    current = &font;
                }
                else if (const FontModule *resolved{resolveFont(fontName)})
                {
                    current = resolved;
                }
                index = close + 1U;
                continue;
            }
        }
        char32_t codepoint{0U};
        if (nextCodepoint(text, index, codepoint))
        {
            // Fall back to the default font for characters the inline font lacks.
            const FontModule *glyphFont{current};
            if (current != &font)
            {
                const FontModule::Symbol symbol{current->getChar(codepoint)};
                const bool missing{std::visit([](auto &&bitmap) { return bitmap.empty(); }, symbol.bitmap) &&
                                   symbol.offsetX == 0U};
                if (missing)
                {
                    glyphFont = &font;
                }
            }
            glyphs.emplace_back(codepoint, glyphFont);
        }
    }
}

/**
 * @brief Finds an enabled font by a loosely written name.
 *
 * @param fontName Font name; case, spaces, dashes and underscores are ignored.
 * @return const FontModule* Font owned by this handler, or nullptr when not found.
 */
const FontModule *TextHandler::resolveFont(std::string_view fontName)
{
    const auto normalize{[](std::string_view value)
                         {
                             std::string out{};
                             for (const char c : value)
                             {
                                 if (c != ' ' && c != '-' && c != '_')
                                 {
                                     out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                                 }
                             }
                             return out;
                         }};
    const std::string wanted{normalize(fontName)};
    for (const std::string_view name : FontsService::names)
    {
        if (normalize(name) != wanted)
        {
            continue;
        }
        for (const std::unique_ptr<const FontModule> &owned : inlineFonts)
        {
            if (owned->name == name)
            {
                return owned.get();
            }
        }
        std::unique_ptr<const FontModule> loaded{Fonts.get(name)};
        if (loaded)
        {
            inlineFonts.push_back(std::move(loaded));
            return inlineFonts.back().get();
        }
    }
    return nullptr;
}

/**
 * @brief Decodes the next UTF-8 code point.
 *
 * Invalid, incomplete, overlong, surrogate, and out-of-range sequences are
 * replaced with U+FFFD.
 *
 * @param text UTF-8 text.
 * @param index Index of the next byte to inspect; advanced past the consumed bytes.
 * @param buffer Receives the decoded code point or U+FFFD for an invalid sequence.
 * @return true if input bytes were available, false if the end of the text was reached.
 */
bool TextHandler::nextCodepoint(const std::string &text, size_t &index, char32_t &buffer)
{
    if (index >= text.length())
    {
        return false;
    }
    const uint8_t first{static_cast<uint8_t>(text[index])};
    ++index;
    if (first < 0x80U)
    {
        buffer = first;
        return true;
    }
    char32_t codepoint{};
    char32_t minimum{};
    uint8_t remaining{};
    if ((first & 0xE0U) == 0xC0U)
    {
        codepoint = first & 0x1FU;
        minimum = 0x80U;
        remaining = 1U;
    }
    else if ((first & 0xF0U) == 0xE0U)
    {
        codepoint = first & 0x0FU;
        minimum = 0x800U;
        remaining = 2U;
    }
    else if ((first & 0xF8U) == 0xF0U)
    {
        codepoint = first & 0x07U;
        minimum = 0x10000U;
        remaining = 3U;
    }
    else
    {
        buffer = U'�';
        return true;
    }
    if (index + remaining > text.length())
    {
        index = text.length();
        buffer = U'�';
        return true;
    }
    while (remaining != 0U)
    {
        const uint8_t next{static_cast<uint8_t>(text[index])};
        if ((next & 0xC0U) != 0x80U)
        {
            buffer = U'�';
            return true;
        }
        ++index;
        codepoint = (codepoint << 6U) | (next & 0x3FU);
        --remaining;
    }
    if (codepoint < minimum || codepoint > 0x10FFFFU || (codepoint >= 0xD800U && codepoint <= 0xDFFFU))
    {
        buffer = U'�';
        return true;
    }
    buffer = codepoint;
    return true;
}

template <typename T>
    requires std::is_unsigned_v<T>
/**
 * @brief Finds the highest set bit in a bitmap.
 *
 * @param bitmap Bitmap elements to inspect.
 * @return uint8_t Index of the highest set bit, or zero if the bitmap is empty or contains no set bits.
 */
uint8_t TextHandler::calcMsbMax(std::span<const T> bitmap) const
{
    uint8_t msbMax{0U};
    for (const T bitset : bitmap)
    {
        if (bitset != 0U)
        {
            msbMax = max(msbMax, static_cast<uint8_t>(std::bit_width(bitset) - 1));
        }
    }
    return msbMax;
}

/**
 * @brief Encodes a Unicode code point as a null-terminated UTF-8 sequence.
 *
 * @param codepoint Unicode code point to encode.
 * @return A five-byte array containing the UTF-8 sequence, or an empty string for an invalid or surrogate code point.
 */
std::array<char, 5U> TextHandler::encode(char32_t codepoint)
{
    if (codepoint >= 0x110000U || (codepoint >= 0xD800U && codepoint <= 0xDFFFU))
    {
        return std::array<char, 5U>{'\0'};
    }
    if (codepoint < 0x80U)
    {
        return std::array<char, 5U>{
            static_cast<char>(codepoint),
            '\0',
            '\0',
            '\0',
            '\0',
        };
    }
    if (codepoint < 0x800U)
    {
        return std::array<char, 5U>{
            static_cast<char>(0xC0U | (codepoint >> 6U)),
            static_cast<char>(0x80U | (codepoint & 0x3FU)),
            '\0',
            '\0',
            '\0',
        };
    }
    if (codepoint < 0x10000U)
    {
        return std::array<char, 5U>{
            static_cast<char>(0xE0U | (codepoint >> 12U)),
            static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)),
            static_cast<char>(0x80U | (codepoint & 0x3FU)),
            '\0',
            '\0',
        };
    }
    return std::array<char, 5U>{
        static_cast<char>(0xF0U | (codepoint >> 18U)),
        static_cast<char>(0x80U | ((codepoint >> 12U) & 0x3FU)),
        static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)),
        static_cast<char>(0x80U | (codepoint & 0x3FU)),
        '\0',
    };
}
