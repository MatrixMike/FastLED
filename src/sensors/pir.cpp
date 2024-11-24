#define FASTLED_INTERNAL
#include "FastLED.h"

#include "sensors/pir.h"
#include "fastpin.h"
#include "fl/warn.h"


namespace fl {

namespace {
  int g_counter = 0;
  Str buttonName() {
    int count = g_counter++;
    if (count == 0) {
      return Str("PIR");
    }
    return Str("Pir ") << g_counter++;
  }
}


Pir::Pir(int pin): mButton(buttonName().c_str()), mPin(pin) {
    mPin.setPinMode(DigitalPin::kInput);
}

bool Pir::detect() {
    return mPin.high() || mButton.clicked();
}

PirLatching::PirLatching(int pin, uint32_t latchMs, uint32_t risingTime, uint32_t fallingTime) 
    : mPir(pin)
    , mLatchMs(latchMs)
    , mRisingTime(risingTime)
    , mFallingTime(fallingTime)
    , mLastTrigger(0)
    , mLastState(false) {

    if (mRisingTime + mFallingTime > mLatchMs) {
        FASTLED_WARN("PirLatching: risingTime + fallingTime must be less than latchMs");
        mRisingTime = mLatchMs / 2;
        mFallingTime = mLatchMs / 2;
    }
}

bool PirLatching::detect(uint32_t now) {
    bool currentState = mPir.detect();
    
    if (currentState && !mLastState) {
        mLastTrigger = now;
    }
    
    mLastState = currentState;
    return (now - mLastTrigger) < mLatchMs;
}

uint8_t PirLatching::transition(uint32_t now) {
    detect(now);
    uint32_t elapsed = now - mLastTrigger;
    
    if (elapsed < mRisingTime) {
        // Rising phase - alpha goes from 0 to 255
        return (elapsed * 255) / mRisingTime;
    } 
    else if (elapsed >= mLatchMs - mFallingTime && elapsed < mLatchMs) {
        // Falling phase - alpha goes from 255 to 0
        uint32_t fallingElapsed = elapsed - (mLatchMs - mFallingTime);
        return 255 - ((fallingElapsed * 255) / mFallingTime);
    }
    else if (elapsed >= mRisingTime && elapsed < mLatchMs - mFallingTime) {
        // Fully on
        return 255;
    }
    
    // Outside latch period
    return 0;
}

}  // namespace fl