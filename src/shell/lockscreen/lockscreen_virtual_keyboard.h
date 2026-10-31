#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

class Button;
class Flex;
class Node;
class Renderer;

namespace lockscreen_virtual_keyboard {

  /// On-screen keyboard for the session lock screen. The keyboard owns its scene
  /// nodes and routes keys through the supplied callbacks; it never touches the
  /// password model directly.
  class VirtualKeyboard {
  public:
    struct Callbacks {
      std::function<void(std::string_view)> onText;
      std::function<void()> onBackspace;
      std::function<void()> onEnter;
      /// Runs before every key action so the host can restore password-field focus
      /// (clicking a key briefly gives it pointer focus).
      std::function<void()> onInteraction;
    };

    VirtualKeyboard();
    ~VirtualKeyboard();

    VirtualKeyboard(const VirtualKeyboard&) = delete;
    VirtualKeyboard& operator=(const VirtualKeyboard&) = delete;

    void create(Node& parent, Callbacks callbacks);
    void setVisible(bool visible);
    [[nodiscard]] bool visible() const noexcept;
    void setEnabled(bool enabled);
    void resetState();

    /// Keyboard width for a screen, capped so keys stay a comfortable size on
    /// tablets and desktops. Height follows from that width.
    [[nodiscard]] static float resolveWidth(float screenWidth);
    [[nodiscard]] static float resolveHeight(float width);

    void arrange(Renderer& renderer, float x, float y, float width, float height);

  private:
    enum class KeyAction : std::uint8_t {
      Text,
      Backspace,
      Enter,
      Shift,
      Page,
      Space,
    };

    struct KeyButton {
      Button* button = nullptr;
      std::string lower;
      std::string upper;
      KeyAction action = KeyAction::Text;
      bool alpha = false;
    };

    void buildPages();
    void buildLetterPage();
    void buildSymbolPage();
    void addKey(Flex& row, std::vector<KeyButton>& keys, KeyButton key, float weight, std::string_view glyph = {});
    void activate(const std::vector<KeyButton>& keys, std::size_t index);
    void updateKeyLabels();
    void updateChrome(float width);
    [[nodiscard]] Flex& addRow(Flex& page);

    Callbacks m_callbacks;
    Flex* m_panel = nullptr;
    Flex* m_letterPage = nullptr;
    Flex* m_symbolPage = nullptr;
    Button* m_shiftButton = nullptr;
    std::vector<Flex*> m_rows;
    std::vector<Button*> m_buttons;
    std::vector<KeyButton> m_letterKeys;
    std::vector<KeyButton> m_symbolKeys;
    bool m_visible = false;
    bool m_symbols = false;
    bool m_shift = false;
  };

} // namespace lockscreen_virtual_keyboard
