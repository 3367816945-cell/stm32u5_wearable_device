#include <gui/screen0_screen/screen0View.hpp>
#include <stdio.h>

screen0View::screen0View()
{

}

void screen0View::setupScreen()
{
    screen0ViewBase::setupScreen();
    dynamicGraph1.setGraphRangeY(200, 1000);  // 设置Y轴范围200-1000
    dynamicGraph1.setGraphRangeX(0, 30);      // 设置X轴范围：只显示最近30个点（约1.7秒数据）

    // 配置TextArea wildcard buffers
    textArea1.setWildcard(textArea1Buffer);
    textArea2.setWildcard1(textArea2Buffer1);  // textArea2有两个wildcard
    textArea2.setWildcard2(textArea2Buffer2);

    // 配置疲劳分析相关TextArea（如果存在）
#ifdef textAreaResult
    textAreaResult.setWildcard(textAreaResultBuffer);
    textAreaResult.setVisible(false);
    textAreaResult.invalidate();
#endif

#ifdef textAreaCountdown
    textAreaCountdown.setWildcard(textAreaCountdownBuffer);
    textAreaCountdown.setVisible(false);
    textAreaCountdown.invalidate();
#endif

#ifdef circleProgress1
    circleProgress1.setVisible(false);
    circleProgress1.invalidate();
#endif
}

void screen0View::tearDownScreen()
{
    screen0ViewBase::tearDownScreen();
}

void screen0View::displaySensorData(const SensorData_t& data)
{
    static uint32_t graphPointCount = 0;
    graphPointCount++;

    if (graphPointCount % 10 == 1) {
        printf("Display#%u: BPM=%u, Signal=%u, Fatigue=%u, Confidence=%u\r\n",
               graphPointCount, data.bpm, data.signal, data.fatigue_level, data.confidence);
    }

    dynamicGraph1.addDataPoint(data.signal);  // 显示原始波形值而非BPM

    // 更新textArea1：显示BPM
    touchgfx::Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%u", data.bpm);
    textArea1.invalidate();

    // 更新textArea2：显示Fatigue和Confidence组合信息（两个wildcard）
    const char* fatigueText;
    switch (data.fatigue_level) {
        case 0: fatigueText = "Low"; break;
        case 1: fatigueText = "Med"; break;
        case 2: fatigueText = "High"; break;
        default: fatigueText = "N/A"; break;
    }
    // 使用Unicode::strncpy转换ASCII字符串到Unicode
    touchgfx::Unicode::strncpy(textArea2Buffer1, fatigueText, TEXTAREA2_SIZE1);
    touchgfx::Unicode::snprintf(textArea2Buffer2, TEXTAREA2_SIZE2, "%u%%", data.confidence);
    textArea2.invalidate();
}

void screen0View::displayFatigueAnalysis(uint8_t progress, uint8_t countdown,
                                         uint8_t fatigue_result, uint8_t confidence)
{
    // 更新环形进度条（如果存在）
#ifdef circleProgress1
    circleProgress1.setVisible(true);
    circleProgress1.setValue(progress);  // 设置进度值（0-100）
    circleProgress1.invalidate();
#endif

    // 更新倒计时文本（如果存在）
#ifdef textAreaCountdown
    touchgfx::Unicode::snprintf(textAreaCountdownBuffer, TEXTAREA_COUNTDOWN_SIZE, "%us", countdown);
    textAreaCountdown.setVisible(true);
    textAreaCountdown.invalidate();
#endif

    // 分析完成后显示结果（如果存在）
#ifdef textAreaResult
    if (countdown == 0) {
        const char* fatigueText;
        switch (fatigue_result) {
            case 0: fatigueText = "Low (Recovery Fast)"; break;
            case 1: fatigueText = "Medium"; break;
            case 2: fatigueText = "High (Recovery Slow)"; break;
            default: fatigueText = "N/A"; break;
        }
        touchgfx::Unicode::snprintf(textAreaResultBuffer, TEXTAREA_RESULT_SIZE,
                                    "Fatigue: %s (%u%%)", fatigueText, confidence);
        textAreaResult.setVisible(true);
        textAreaResult.invalidate();
    }
#endif

    // 输出调试信息
    printf("UI: Progress=%u%%, Countdown=%us, Result=%u, Confidence=%u%%\r\n",
           progress, countdown, fatigue_result, confidence);
}
