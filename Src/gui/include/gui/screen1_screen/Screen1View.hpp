#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

    /* 疲劳分析显示方法 */
    void updateProgress(uint8_t progress, uint8_t countdown);
    void displayResult(uint8_t fatigue_result, uint8_t confidence);

protected:
    /* TextArea wildcard buffers */
    static const uint16_t TEXTAREA2_SIZE = 20;
    touchgfx::Unicode::UnicodeChar textArea2Buffer[TEXTAREA2_SIZE];
};

#endif // SCREEN1VIEW_HPP