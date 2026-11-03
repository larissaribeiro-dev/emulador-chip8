#include "test_helpers.hpp"
#include "timers.hpp"

using chip8::Timers;

TEST(Timers, BothCountDownByOnePerTick) {
    Timers t;
    t.delay = 5;
    t.sound = 3;
    t.tick();
    EXPECT_EQ(t.delay, 4);
    EXPECT_EQ(t.sound, 2);
}

TEST(Timers, StopAtZero) {
    Timers t;
    t.delay = 1;
    t.tick();
    t.tick();
    EXPECT_EQ(t.delay, 0);
    EXPECT_EQ(t.sound, 0);
}

TEST(Timers, BeepsOnlyWhileSoundTimerIsRunning) {
    Timers t;
    EXPECT_FALSE(t.beeping());
    t.sound = 2;
    EXPECT_TRUE(t.beeping());
    t.tick();
    t.tick();
    EXPECT_FALSE(t.beeping());
}
