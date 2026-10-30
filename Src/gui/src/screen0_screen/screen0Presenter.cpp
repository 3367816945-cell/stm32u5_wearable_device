#include <gui/screen0_screen/screen0View.hpp>
#include <gui/screen0_screen/screen0Presenter.hpp>
#include <stdio.h>

screen0Presenter::screen0Presenter(screen0View& v)
    : view(v)
{

}

void screen0Presenter::activate()
{

}

void screen0Presenter::deactivate()
{

}

void screen0Presenter::sensorDataUpdated(const SensorData_t& data)
{
    static uint32_t updateCount = 0;
    updateCount++;

    if (updateCount % 10 == 1) {
        printf("Presenter#%u: BPM=%u, Fatigue=%u\r\n",
               updateCount, data.bpm, data.fatigue_level);
    }

    view.displaySensorData(data);
}

void screen0Presenter::fatigueAnalysisUpdated(uint8_t progress, uint8_t countdown,
                                              uint8_t fatigue_result, uint8_t confidence)
{
    view.displayFatigueAnalysis(progress, countdown, fatigue_result, confidence);
}
