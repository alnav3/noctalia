#include "shell/lockscreen/lockscreen_virtual_keyboard.h"

#include "render/core/renderer.h"
#include "ui/builders.h"
#include "ui/controls/button.h"
#include "ui/controls/flex.h"
#include "ui/palette.h"
#include "ui/style.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace lockscreen_virtual_keyboard {

  namespace {

    constexpr float kMaxWidth = 760.0F;
    constexpr float kMinWidth = 240.0F;
    constexpr float kKeyGap = 6.0F;
    constexpr float kPanelPadding = 10.0F;
    constexpr int kRowCount = 5;
    constexpr float kMinKeyHeight = 30.0F;
    constexpr float kMaxKeyHeight = 52.0F;
    constexpr float kKeyHeightRatio = 0.78F;

    [[nodiscard]] float keyHeightForWidth(float width) {
      const float usable = std::max(0.0F, width - kPanelPadding * 2.0F - kKeyGap * 9.0F);
      return std::clamp(usable / 10.0F * kKeyHeightRatio, kMinKeyHeight, kMaxKeyHeight);
    }

  } // namespace

  VirtualKeyboard::VirtualKeyboard() = default;
  VirtualKeyboard::~VirtualKeyboard() = default;

  void VirtualKeyboard::create(Node& parent, Callbacks callbacks) {
    m_callbacks = std::move(callbacks);
    m_rows.reserve(kRowCount * 2);
    m_buttons.reserve(80);
    m_letterKeys.reserve(40);
    m_symbolKeys.reserve(40);

    auto panel = ui::flex(
        FlexDirection::Vertical,
        {
            .out = &m_panel,
            .align = FlexAlign::Stretch,
            .justify = FlexJustify::Start,
            .gap = kKeyGap,
            .padding = kPanelPadding,
            .fill = colorSpecFromRole(ColorRole::SurfaceContainerHigh, 0.94F),
            .radius = Style::scaledRadius(14.0F),
            .border = colorSpecFromRole(ColorRole::OutlineVariant, 0.85F),
            .borderWidth = Style::borderWidth,
            .widthPolicy = FlexSizePolicy::Fill,
            .heightPolicy = FlexSizePolicy::Fill,
            .clipChildren = true,
            .visible = false,
            .configure = [](Flex& flex) { flex.setZIndex(5); },
        }
    );
    parent.addChild(std::move(panel));

    buildPages();
  }

  void VirtualKeyboard::buildPages() {
    auto letterPage = ui::flex(
        FlexDirection::Vertical,
        {
            .out = &m_letterPage,
            .align = FlexAlign::Stretch,
            .justify = FlexJustify::Start,
            .gap = kKeyGap,
            .widthPolicy = FlexSizePolicy::Fill,
            .heightPolicy = FlexSizePolicy::Fill,
            .flexGrow = 1.0F,
        }
    );
    m_panel->addChild(std::move(letterPage));

    auto symbolPage = ui::flex(
        FlexDirection::Vertical,
        {
            .out = &m_symbolPage,
            .align = FlexAlign::Stretch,
            .justify = FlexJustify::Start,
            .gap = kKeyGap,
            .widthPolicy = FlexSizePolicy::Fill,
            .heightPolicy = FlexSizePolicy::Fill,
            .flexGrow = 1.0F,
            .visible = false,
        }
    );
    m_panel->addChild(std::move(symbolPage));

    buildLetterPage();
    buildSymbolPage();
  }

  Flex& VirtualKeyboard::addRow(Flex& page) {
    auto row = ui::flex(
        FlexDirection::Horizontal,
        {
            .out = nullptr,
            .align = FlexAlign::Stretch,
            .justify = FlexJustify::Start,
            .gap = kKeyGap,
            .widthPolicy = FlexSizePolicy::Fill,
            .heightPolicy = FlexSizePolicy::Content,
        }
    );
    Flex* raw = row.get();
    page.addChild(std::move(row));
    m_rows.push_back(raw);
    return *raw;
  }

  void VirtualKeyboard::addKey(
      Flex& row, std::vector<KeyButton>& keys, KeyButton key, float weight, std::string_view glyph
  ) {
    std::optional<std::string> glyphName;
    if (!glyph.empty()) {
      glyphName = std::string(glyph);
    }
    auto button = ui::button({
        .out = &key.button,
        .text = glyph.empty() ? key.lower : std::string{},
        .glyph = std::move(glyphName),
        .variant = ButtonVariant::Default,
        .minHeight = 0.0F,
        .maxHeight = 0.0F,
        .padding = 2.0F,
        .flexGrow = weight,
        .configure = [](Button& control) { control.setMinWidth(0.0F); },
    });
    button->setOnClick([this, &keys, index = keys.size()] { activate(keys, index); });
    m_buttons.push_back(button.get());
    keys.push_back(std::move(key));
    row.addChild(std::move(button));
  }

  void VirtualKeyboard::buildLetterPage() {
    Flex& digits = addRow(*m_letterPage);
    for (const char* digit : {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"}) {
      addKey(digits, m_letterKeys, KeyButton{.lower = digit, .upper = digit}, 1.0F);
    }

    Flex& top = addRow(*m_letterPage);
    for (const char* letter : {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"}) {
      addKey(
          top, m_letterKeys,
          KeyButton{.lower = letter, .upper = std::string(1, static_cast<char>(letter[0] - 'a' + 'A')), .alpha = true},
          1.0F
      );
    }

    Flex& home = addRow(*m_letterPage);
    for (const char* letter : {"a", "s", "d", "f", "g", "h", "j", "k", "l"}) {
      addKey(
          home, m_letterKeys,
          KeyButton{.lower = letter, .upper = std::string(1, static_cast<char>(letter[0] - 'a' + 'A')), .alpha = true},
          1.0F
      );
    }

    Flex& bottom = addRow(*m_letterPage);
    {
      KeyButton shift{.action = KeyAction::Shift};
      addKey(bottom, m_letterKeys, std::move(shift), 1.6F, "letter-case");
      m_shiftButton = m_letterKeys.back().button;
    }
    for (const char* letter : {"z", "x", "c", "v", "b", "n", "m"}) {
      addKey(
          bottom, m_letterKeys,
          KeyButton{.lower = letter, .upper = std::string(1, static_cast<char>(letter[0] - 'a' + 'A')), .alpha = true},
          1.0F
      );
    }
    addKey(bottom, m_letterKeys, KeyButton{.action = KeyAction::Backspace}, 1.6F, "backspace");

    Flex& actions = addRow(*m_letterPage);
    addKey(actions, m_letterKeys, KeyButton{.lower = "?123", .action = KeyAction::Page}, 1.5F);
    addKey(actions, m_letterKeys, KeyButton{.lower = ",", .upper = ","}, 1.0F);
    addKey(actions, m_letterKeys, KeyButton{.action = KeyAction::Space}, 5.0F, "space");
    addKey(actions, m_letterKeys, KeyButton{.lower = ".", .upper = "."}, 1.0F);
    addKey(actions, m_letterKeys, KeyButton{.action = KeyAction::Enter}, 1.5F, "corner-down-left");
  }

  void VirtualKeyboard::buildSymbolPage() {
    Flex& digits = addRow(*m_symbolPage);
    for (const char* symbol : {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")"}) {
      addKey(digits, m_symbolKeys, KeyButton{.lower = symbol, .upper = symbol}, 1.0F);
    }

    Flex& top = addRow(*m_symbolPage);
    for (const char* symbol : {"-", "_", "=", "+", "[", "]", "{", "}", "\\", "|"}) {
      addKey(top, m_symbolKeys, KeyButton{.lower = symbol, .upper = symbol}, 1.0F);
    }

    Flex& home = addRow(*m_symbolPage);
    for (const char* symbol : {";", ":", "'", "\"", "/", "?", "~", "`", "<", ">"}) {
      addKey(home, m_symbolKeys, KeyButton{.lower = symbol, .upper = symbol}, 1.0F);
    }

    Flex& bottom = addRow(*m_symbolPage);
    addKey(bottom, m_symbolKeys, KeyButton{.lower = "ABC", .action = KeyAction::Page}, 1.6F);
    for (const char* symbol : {"€", "£", "¥", "¢", "©", "®", "™"}) {
      addKey(bottom, m_symbolKeys, KeyButton{.lower = symbol, .upper = symbol}, 1.0F);
    }
    addKey(bottom, m_symbolKeys, KeyButton{.action = KeyAction::Backspace}, 1.6F, "backspace");

    Flex& actions = addRow(*m_symbolPage);
    addKey(actions, m_symbolKeys, KeyButton{.lower = ",", .upper = ","}, 1.0F);
    addKey(actions, m_symbolKeys, KeyButton{.action = KeyAction::Space}, 5.0F, "space");
    addKey(actions, m_symbolKeys, KeyButton{.lower = ".", .upper = "."}, 1.0F);
    addKey(actions, m_symbolKeys, KeyButton{.action = KeyAction::Enter}, 1.5F, "corner-down-left");
  }

  void VirtualKeyboard::activate(const std::vector<KeyButton>& keys, std::size_t index) {
    if (!m_visible || index >= keys.size()) {
      return;
    }
    if (m_callbacks.onInteraction) {
      m_callbacks.onInteraction();
    }
    const KeyButton& key = keys[index];
    switch (key.action) {
    case KeyAction::Text: {
      const bool shifted = m_shift && key.alpha;
      const std::string_view text = shifted ? key.upper : key.lower;
      if (shifted) {
        m_shift = false;
        updateKeyLabels();
      }
      if (m_callbacks.onText) {
        m_callbacks.onText(text);
      }
      break;
    }
    case KeyAction::Space:
      if (m_callbacks.onText) {
        m_callbacks.onText(" ");
      }
      break;
    case KeyAction::Backspace:
      if (m_callbacks.onBackspace) {
        m_callbacks.onBackspace();
      }
      break;
    case KeyAction::Enter:
      if (m_callbacks.onEnter) {
        m_callbacks.onEnter();
      }
      break;
    case KeyAction::Shift:
      m_shift = !m_shift;
      updateKeyLabels();
      break;
    case KeyAction::Page:
      m_symbols = !m_symbols;
      m_shift = false;
      if (m_letterPage != nullptr) {
        m_letterPage->setVisible(!m_symbols);
      }
      if (m_symbolPage != nullptr) {
        m_symbolPage->setVisible(m_symbols);
      }
      updateKeyLabels();
      break;
    }
  }

  void VirtualKeyboard::updateKeyLabels() {
    for (const auto& key : m_letterKeys) {
      if (key.button == nullptr || key.action != KeyAction::Text) {
        continue;
      }
      if (key.alpha && m_shift) {
        key.button->setText(key.upper);
      } else {
        key.button->setText(key.lower);
      }
    }
    if (m_shiftButton != nullptr) {
      m_shiftButton->setSelected(m_shift);
    }
  }

  void VirtualKeyboard::setVisible(bool visible) {
    m_visible = visible;
    if (m_panel != nullptr) {
      m_panel->setVisible(visible);
    }
  }

  bool VirtualKeyboard::visible() const noexcept { return m_visible; }

  void VirtualKeyboard::setEnabled(bool enabled) {
    for (Button* button : m_buttons) {
      if (button != nullptr) {
        button->setEnabled(enabled);
      }
    }
  }

  void VirtualKeyboard::resetState() {
    m_shift = false;
    m_symbols = false;
    if (m_letterPage != nullptr) {
      m_letterPage->setVisible(true);
    }
    if (m_symbolPage != nullptr) {
      m_symbolPage->setVisible(false);
    }
    updateKeyLabels();
  }

  float VirtualKeyboard::resolveWidth(float screenWidth) {
    return std::max(kMinWidth, std::min(screenWidth - Style::spaceLg * 2.0F, kMaxWidth));
  }

  float VirtualKeyboard::resolveHeight(float width) {
    const float keyHeight = keyHeightForWidth(width);
    return keyHeight * static_cast<float>(kRowCount)
        + kKeyGap * static_cast<float>(kRowCount - 1)
        + kPanelPadding * 2.0F;
  }

  void VirtualKeyboard::arrange(Renderer& renderer, float x, float y, float width, float height) {
    if (m_panel == nullptr) {
      return;
    }

    const float keyHeight = std::clamp(
        (height - kPanelPadding * 2.0F - kKeyGap * static_cast<float>(kRowCount - 1)) / static_cast<float>(kRowCount),
        kMinKeyHeight, kMaxKeyHeight
    );
    const float fontSize = std::clamp(keyHeight * 0.36F, 13.0F, 18.0F);
    const float glyphSize = std::clamp(keyHeight * 0.44F, 15.0F, 20.0F);
    const float keyRadius = std::round(keyHeight * 0.18F);

    for (Flex* row : m_rows) {
      if (row == nullptr) {
        continue;
      }
      row->setMinHeight(keyHeight);
      row->setMaxHeight(keyHeight);
    }
    for (Button* button : m_buttons) {
      if (button == nullptr) {
        continue;
      }
      button->setControlHeight(keyHeight);
      button->setFontSize(fontSize);
      button->setGlyphSize(glyphSize);
      button->setRadius(keyRadius);
    }

    m_panel->arrange(renderer, LayoutRect{x, y, width, height});
  }

} // namespace lockscreen_virtual_keyboard
