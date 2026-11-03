#include "display.hpp"
#include "test_helpers.hpp"

using chip8::Display;

namespace {
const std::uint8_t kRow4 = 0xF0;  // ####....
}

TEST(Display, StartsBlank) {
    Display d;
    for (int y = 0; y < Display::kHeight; ++y)
        for (int x = 0; x < Display::kWidth; ++x) ASSERT_FALSE(d.pixel(x, y));
}

TEST(Display, DrawSetsOnlyTheBitsOfTheSprite) {
    Display d;
    EXPECT_FALSE(d.draw_sprite(0, 0, &kRow4, 1));
    for (int x = 0; x < 4; ++x) EXPECT_TRUE(d.pixel(x, 0));
    for (int x = 4; x < 8; ++x) EXPECT_FALSE(d.pixel(x, 0));
    EXPECT_FALSE(d.pixel(0, 1));
}

TEST(Display, DrawingTwiceErasesAndReportsCollision) {
    Display d;
    d.draw_sprite(10, 5, &kRow4, 1);
    EXPECT_TRUE(d.draw_sprite(10, 5, &kRow4, 1));
    for (int x = 10; x < 14; ++x) EXPECT_FALSE(d.pixel(x, 5));
}

TEST(Display, OverlapWithoutErasingLitPixelIsNotACollision) {
    Display d;
    const std::uint8_t right_half = 0x0F;
    d.draw_sprite(0, 0, &kRow4, 1);
    EXPECT_FALSE(d.draw_sprite(0, 0, &right_half, 1));
    for (int x = 0; x < 8; ++x) EXPECT_TRUE(d.pixel(x, 0));
}

TEST(Display, StartCoordinateWrapsAround) {
    Display d;
    const std::uint8_t dot = 0x80;
    d.draw_sprite(Display::kWidth + 2, Display::kHeight + 1, &dot, 1);
    EXPECT_TRUE(d.pixel(2, 1));
}

TEST(Display, SpriteIsClippedAtTheRightEdge) {
    Display d;
    const std::uint8_t full = 0xFF;
    d.draw_sprite(60, 0, &full, 1);
    for (int x = 60; x < 64; ++x) EXPECT_TRUE(d.pixel(x, 0));
    EXPECT_FALSE(d.pixel(0, 0));  // não reaparece do outro lado
}

TEST(Display, SpriteIsClippedAtTheBottomEdge) {
    Display d;
    const std::uint8_t rows[2] = {0x80, 0x80};
    d.draw_sprite(0, 31, rows, 2);
    EXPECT_TRUE(d.pixel(0, 31));
    EXPECT_FALSE(d.pixel(0, 0));
}

TEST(Display, ClearTurnsEverythingOff) {
    Display d;
    d.draw_sprite(0, 0, &kRow4, 1);
    d.clear();
    EXPECT_FALSE(d.pixel(0, 0));
}

TEST(Display, PixelOutsideTheScreenIsOff) {
    Display d;
    EXPECT_FALSE(d.pixel(-1, 0));
    EXPECT_FALSE(d.pixel(0, Display::kHeight));
}

TEST(Display, DirtyFlagFollowsChanges) {
    Display d;
    d.clear_dirty();
    EXPECT_FALSE(d.dirty());
    d.draw_sprite(0, 0, &kRow4, 1);
    EXPECT_TRUE(d.dirty());
}

TEST(Display, AsciiHasOneLinePerRow) {
    Display d;
    d.draw_sprite(0, 0, &kRow4, 1);
    const std::string ascii = d.to_ascii();
    EXPECT_EQ(ascii.size(), static_cast<std::size_t>((Display::kWidth + 1) * Display::kHeight));
    EXPECT_EQ(ascii.substr(0, 6), "####..");
}
