#pragma once

#include "modules/FontModule.h"
#include "modules/HandlerModule.h"

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/**
 * @brief Measures and renders UTF-8 text.
 *
 * The text may switch font inline with the markup `{font:Name}`, where `Name`
 * is any enabled font (case, spaces, dashes and underscores are ignored).
 * `{font:}` switches back to the font passed to the constructor. Glyphs of
 * different fonts share the same baseline. Characters missing from an inline
 * font are drawn with the default font.
 */
class TextHandler final : public HandlerModule
{
public:
    explicit TextHandler(std::string text, const FontModule &font);

    void draw(uint8_t brightness = UINT8_MAX) const;
    void draw(int16_t x, int8_t y, uint8_t brightness = UINT8_MAX) const;

    [[nodiscard]] uint8_t getHeight() const;
    [[nodiscard]] uint8_t getWidth() const;

    static constexpr std::string_view markupOpen{"{font:"};

private:
    uint8_t height{0U};
    uint8_t tracking{1U};
    uint8_t width{0U};

    std::vector<std::pair<char32_t, const FontModule *>> glyphs{};
    std::vector<std::unique_ptr<const FontModule>> inlineFonts{};

    void parse(const std::string &text, const FontModule &font);

    [[nodiscard]] const FontModule *resolveFont(std::string_view fontName);

    template <typename T>
        requires std::is_unsigned_v<T>
    [[nodiscard]] uint8_t calcMsbMax(std::span<const T> bitmap) const;

    static bool nextCodepoint(const std::string &text, size_t &index, char32_t &out);

    static std::array<char, 5U> encode(char32_t codepoint);
};
