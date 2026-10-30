#ifndef SCREEN0VIEW_HPP
#define SCREEN0VIEW_HPP

#include <gui_generated/screen0_screen/screen0ViewBase.hpp>
#include <gui/screen0_screen/screen0Presenter.hpp>
#include "sensor_data.h"
#include <touchgfx/Unicode.hpp>

class screen0View : public screen0ViewBase
{
public:
    screen0View();
    virtual ~screen0View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void displaySensorData(const SensorData_t& data);
    virtual void displayFatigueAnalysis(uint8_t progress, uint8_t countdown,
                                         uint8_t fatigue_result, uint8_t confidence);
protected:
    // Wildcard buffers for dynamic text display
    static const uint16_t TEXTAREA1_SIZE = 10;
    touchgfx::Unicode::UnicodeChar textArea1Buffer[TEXTAREA1_SIZE];

    static const uint16_t TEXTAREA2_SIZE1 = 10;
    touchgfx::Unicode::UnicodeChar textArea2Buffer1[TEXTAREA2_SIZE1];

    static const uint16_t TEXTAREA2_SIZE2 = 5;
    touchgfx::Unicode::UnicodeChar textArea2Buffer2[TEXTAREA2_SIZE2];

    // 疲劳分析结果和倒计时buffers
    static const uint16_t TEXTAREA_RESULT_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textAreaResultBuffer[TEXTAREA_RESULT_SIZE];

    static const uint16_t TEXTAREA_COUNTDOWN_SIZE = 5;
    touchgfx::Unicode::UnicodeChar textAreaCountdownBuffer[TEXTAREA_COUNTDOWN_SIZE];
};

#endif // SCREEN0VIEW_HPP
