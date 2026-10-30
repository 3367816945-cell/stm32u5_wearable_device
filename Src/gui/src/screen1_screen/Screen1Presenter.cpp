#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

Screen1Presenter::Screen1Presenter(Screen1View& v)
    : view(v)
{

}

void Screen1Presenter::activate()
{

}

void Screen1Presenter::deactivate()
{

}

void Screen1Presenter::sensorDataUpdated(const SensorData_t& data)
{
    // Screen1不显示实时传感器数据,忽略
}

void Screen1Presenter::fatigueAnalysisUpdated(uint8_t progress, uint8_t countdown,
                                              uint8_t fatigue_result, uint8_t confidence)
{
    /* 直接调用View的更新方法 */
    view.updateProgress(progress, countdown);

    /* 分析完成后显示结果 */
    if (countdown == 0) {
        view.displayResult(fatigue_result, confidence);
    }
}