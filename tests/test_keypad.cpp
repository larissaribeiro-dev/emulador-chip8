#include "keypad.hpp"
#include "test_helpers.hpp"

using chip8::Keypad;

TEST(Keypad, PressAndRelease) {
    Keypad k;
    EXPECT_FALSE(k.is_pressed(0xA));
    k.set_key(0xA, true);
    EXPECT_TRUE(k.is_pressed(0xA));
    k.set_key(0xA, false);
    EXPECT_FALSE(k.is_pressed(0xA));
}

TEST(Keypad, KeysOutsideRangeAreIgnored) {
    Keypad k;
    k.set_key(0x10, true);
    EXPECT_FALSE(k.is_pressed(0x10));
    EXPECT_FALSE(k.take_release().has_value());
}

TEST(Keypad, ReleaseIsLatchedUntilConsumed) {
    Keypad k;
    k.set_key(0x5, true);
    EXPECT_FALSE(k.take_release().has_value());  // pressionar não conta
    k.set_key(0x5, false);
    auto released = k.take_release();
    ASSERT_TRUE(released.has_value());
    EXPECT_EQ(*released, 0x5);
    EXPECT_FALSE(k.take_release().has_value());  // só pode ser consumida uma vez
}

TEST(Keypad, DiscardReleaseDropsPendingEvent) {
    Keypad k;
    k.set_key(0x1, true);
    k.set_key(0x1, false);
    k.discard_release();
    EXPECT_FALSE(k.take_release().has_value());
}

TEST(Keypad, ResetReleasesEverything) {
    Keypad k;
    k.set_key(0x3, true);
    k.reset();
    EXPECT_FALSE(k.is_pressed(0x3));
}
