

/*
This demo is best viewed using the FastLED compiler.
Install: pip install fastled
Run: fastled <this sketch directory>
This will compile and preview the sketch in the browser, and enable
all the UI elements you see below.
*/

#include <Arduino.h>
#include <FastLED.h>

#include "fl/math_macros.h"
#include "fl/ui.h"
#include "fx/2d/wave.h"
#include "fx/2d/blend.h"



using namespace fl;

#define HEIGHT 100
#define WIDTH 100
#define NUM_LEDS ((WIDTH) * (HEIGHT))
#define IS_SERPINTINE true

CRGB leds[NUM_LEDS];

UITitle title("FxWave2D Demo");
UIDescription description("Advanced layered and blended wave effects.");

UIButton button("Trigger");
UICheckbox autoTrigger("Auto Trigger", true);
UISlider triggerSpeed("Trigger Speed", .5f, 0.0f, 1.0f, 0.01f);
UICheckbox easeModeSqrt("Ease Mode Sqrt", false);
UISlider blurAmount("Global Blur Amount", 0, 0, 172, 1);
UISlider blurPasses("Global Blur Passes", 1, 1, 10, 1);
UISlider superSample("SuperSampleExponent", 1.f, 0.f, 3.f, 1.f);


UISlider speedUpper("Wave Upper: Speed", 0.12f, 0.0f, 1.0f);
UISlider dampeningUpper("Wave Upper: Dampening", 8.9f, 0.0f, 20.0f, 0.1f);
UICheckbox halfDuplexUpper("Wave Upper: Half Duplex", true);
UISlider blurAmountUpper("Wave Upper: Blur Amount", 95, 0, 172, 1);
UISlider blurPassesUpper("Wave Upper: Blur Passes", 1, 1, 10, 1);

UISlider speedLower("Wave Lower: Speed", 0.26f, 0.0f, 1.0f);
UISlider dampeningLower("Wave Lower: Dampening", 9.0f, 0.0f, 20.0f, 0.1f);
UICheckbox halfDuplexLower("Wave Lower: Half Duplex", true);
UISlider blurAmountLower("Wave Lower: Blur Amount", 0, 0, 172, 1);
UISlider blurPassesLower("Wave Lower: Blur Passes", 1, 1, 10, 1);


DEFINE_GRADIENT_PALETTE(electricBlueFirePal){
    0,   0,   0,   0,   // Black
    32,  0,   0,   70,  // Dark blue
    128, 20,  57,  255, // Electric blue
    255, 255, 255, 255  // White
};

DEFINE_GRADIENT_PALETTE(electricGreenFirePal){
    0,   0,   0,   0,  // black
    8,  128, 64, 64, // green
    16, 255, 222, 222, // red
    64, 255, 255, 255, // white
    255, 255, 255, 255 // white
};

XYMap xyMap(WIDTH, HEIGHT, IS_SERPINTINE);
XYMap xyRect(WIDTH, HEIGHT, false);
WaveFx waveFxLower(xyRect, WaveFx::Args{
    .factor = SUPER_SAMPLE_4X,
    .half_duplex = true,
    .speed = 0.18f,
    .dampening = 9.0f,
    .crgbMap = WaveCrgbGradientMapPtr::New(electricBlueFirePal),
});

WaveFx waveFxUpper(xyRect, WaveFx::Args{
    .factor = SUPER_SAMPLE_4X,
    .half_duplex = true,
    .speed = 0.25f,
    .dampening = 3.0f,
    .crgbMap = WaveCrgbGradientMapPtr::New(electricGreenFirePal),
});

Blend2d fxBlend(xyMap);

void setup() {
    Serial.begin(115200);
    FastLED.addLeds<NEOPIXEL, 2>(leds, NUM_LEDS).setScreenMap(xyMap);
    fxBlend.add(waveFxLower);
    fxBlend.add(waveFxUpper);
}

SuperSample getSuperSample() {
    switch (int(superSample)) {
    case 0:
        return SuperSample::SUPER_SAMPLE_NONE;
    case 1:
        return SuperSample::SUPER_SAMPLE_2X;
    case 2:
        return SuperSample::SUPER_SAMPLE_4X;
    case 3:
        return SuperSample::SUPER_SAMPLE_8X;
    default:
        return SuperSample::SUPER_SAMPLE_NONE;
    }
}

void triggerRipple() {
    int x = random() % WIDTH;
    int y = random() % HEIGHT;
    waveFxLower.setf(x, y, 1);
    waveFxUpper.setf(x, y, 1);
}

bool ui() {
    U8EasingFunction easeMode = easeModeSqrt ? U8EasingFunction::WAVE_U8_MODE_SQRT : U8EasingFunction::WAVE_U8_MODE_LINEAR;
    waveFxLower.setSpeed(speedLower);
    waveFxLower.setDampening(dampeningLower);
    waveFxLower.setHalfDuplex(halfDuplexLower);
    waveFxLower.setSuperSample(getSuperSample());
    waveFxLower.setEasingMode(easeMode);

    waveFxUpper.setSpeed(speedUpper);
    waveFxUpper.setDampening(dampeningUpper);
    waveFxUpper.setHalfDuplex(halfDuplexUpper);
    waveFxUpper.setSuperSample(getSuperSample());
    waveFxUpper.setEasingMode(easeMode);
    fxBlend.setGlobalBlurAmount(blurAmount);
    fxBlend.setGlobalBlurPasses(blurPasses);

    Blend2dParams lower_params = {
        .blur_amount = blurAmountLower,
        .blur_passes = blurPassesLower,
    };

    Blend2dParams upper_params = {
        .blur_amount = blurAmountUpper,
        .blur_passes = blurPassesUpper,
    };

    fxBlend.setParams(waveFxLower, lower_params);
    fxBlend.setParams(waveFxUpper, upper_params);
    return button;
}


void processAutoTrigger(uint32_t now) {
    static uint32_t nextTrigger = 0;
    uint32_t trigger_delta = nextTrigger - now;
    if (trigger_delta > 10000) {
        // rolled over!
        trigger_delta = 0;
    }
    if (autoTrigger) {
        if (now >= nextTrigger) {
            triggerRipple();
            float speed = 1.0f - triggerSpeed.value();
            uint32_t min_rand = 400 * speed;
            uint32_t max_rand = 2000 * speed;

            uint32_t min = MIN(min_rand, max_rand);
            uint32_t max = MAX(min_rand, max_rand);
            if (min == max) {
                max += 1;
            }
            nextTrigger = now + random(min, max);
        }
    }
}

void loop() {
    // Your code here
    bool triggered = ui();
    if (triggered) {
        triggerRipple();
    }
    uint32_t now = millis();
    processAutoTrigger(now);
    Fx::DrawContext ctx(now, leds);
    fxBlend.draw(ctx);
    FastLED.show();
}