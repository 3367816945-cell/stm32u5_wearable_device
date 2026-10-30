#ifndef FRONTENDAPPLICATION_HPP
#define FRONTENDAPPLICATION_HPP

#include <gui_generated/common/FrontendApplicationBase.hpp>

class FrontendHeap;

using namespace touchgfx;

class FrontendApplication : public FrontendApplicationBase
{
public:
    FrontendApplication(Model& m, FrontendHeap& heap);
    virtual ~FrontendApplication() { }

    virtual void handleTickEvent();

    /* 屏幕切换方法（基类已含Main） */
    void gotoscreen0ScreenNoTransition();
    void gotoScreen1ScreenNoTransition();

private:
    void gotoscreen0ScreenNoTransitionImpl();
    void gotoScreen1ScreenNoTransitionImpl();
};

#endif // FRONTENDAPPLICATION_HPP