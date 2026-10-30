#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/model/Model.hpp>
#include <touchgfx/Unicode.hpp>
#include <stdio.h>

extern "C" {
#include "fatigue_analysis.h"
}

Screen1View::Screen1View()
{
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    /* 初始化进度条为0%（覆盖构造函数默认值60%） */
    circleProgress1.setValue(0);
    circleProgress1.invalidate();

    /* 配置TextArea wildcard buffer */
    textArea2.setWildcard(textArea2Buffer);

    /* 初始显示:准备就绪 */
    touchgfx::Unicode::snprintf(textArea2Buffer, TEXTAREA2_SIZE, "Ready");
    textArea2.invalidate();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

/**
 * @brief 每帧调用的事件处理函数
 * @details 检查疲劳分析状态并更新UI
 */
void Screen1View::handleTickEvent()
{
    /* 检查疲劳分析是否激活 */
    if (FatigueAnalysis_IsActive()) {
        updateProgress(
            g_fatigueAnalysis.progress,
            g_fatigueAnalysis.countdown_sec
        );

        /* 分析完成后显示结果（仅触发一次） */
        static uint8_t resultShown = 0;
        if (g_fatigueAnalysis.countdown_sec == 0 && !resultShown) {
            resultShown = 1;
            displayResult(
                g_fatigueAnalysis.fatigue_result,
                g_fatigueAnalysis.confidence
            );
        }
    }
}

/**
 * @brief 更新进度显示
 * @param progress 进度百分比(100->0)
 * @param countdown 倒计时秒数(30->0)
 */
void Screen1View::updateProgress(uint8_t progress, uint8_t countdown)
{
    /* 更新Circle Progress进度条 */
    circleProgress1.setValue(progress);
    circleProgress1.invalidate();

    /* 更新倒计时文本 */
    if (countdown > 0) {
        touchgfx::Unicode::snprintf(textArea2Buffer, TEXTAREA2_SIZE, "%us", countdown);
    }
    textArea2.invalidate();

    /* 调试输出 */
    static uint32_t updateCount = 0;
    updateCount++;
    if (updateCount % 60 == 1) {
        printf("Screen1: Progress=%u%%, Countdown=%us\r\n", progress, countdown);
    }
}

/**
 * @brief 显示分析结果
 * @param fatigue_result 疲劳等级(0=Low, 1=Med, 2=High)
 * @param confidence 置信度百分比
 */
void Screen1View::displayResult(uint8_t fatigue_result, uint8_t confidence)
{
    const char* resultText;

    switch (fatigue_result) {
        case 0:
            resultText = "Low";
            break;
        case 1:
            resultText = "Med";
            break;
        case 2:
            resultText = "High";
            break;
        default:
            resultText = "N/A";
            break;
    }

    /* 使用strncpy将窄字符串转为Unicode（snprintf的%s不支持char*） */
    uint16_t len = Unicode::strncpy(textArea2Buffer, resultText, TEXTAREA2_SIZE);
    /* 追加 " XX%" */
    if (len < TEXTAREA2_SIZE) {
        Unicode::snprintf(textArea2Buffer + len, TEXTAREA2_SIZE - len, " %u%%", confidence);
    }
    textArea2.invalidate();

    printf("Screen1: Fatigue=%s, Confidence=%u%%\r\n", resultText, confidence);
}