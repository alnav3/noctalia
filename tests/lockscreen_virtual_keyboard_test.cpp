#include "shell/lockscreen/lockscreen_virtual_keyboard.h"
#include "tests/test_check.h"

#include <algorithm>

int main() {
  using lockscreen_virtual_keyboard::VirtualKeyboard;

  for (const float screenWidth : {0.0F, 100.0F, 240.0F, 320.0F, 640.0F, 960.0F, 1920.0F, 4000.0F}) {
    const float width = VirtualKeyboard::resolveWidth(screenWidth);
    TEST_CHECK(width >= 0.0F);
    TEST_CHECK(width <= std::min(screenWidth, 760.0F));
    if (width > 0.0F) {
      TEST_CHECK(VirtualKeyboard::resolveHeight(width) >= VirtualKeyboard::resolveMinHeight(width));
      TEST_CHECK(VirtualKeyboard::resolveHeight(width, 120.0F) <= 120.0F);
    }
  }

  TEST_CHECK(VirtualKeyboard::resolveWidth(4000.0F) == 760.0F);
  TEST_CHECK(VirtualKeyboard::resolveMinHeight(320.0F) < VirtualKeyboard::resolveMinHeight(760.0F));

  return 0;
}
