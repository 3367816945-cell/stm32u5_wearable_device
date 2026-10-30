#include <gui/common/FrontendApplication.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <gui/screen0_screen/screen0View.hpp>
#include <gui/screen0_screen/screen0Presenter.hpp>
#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <touchgfx/transitions/NoTransition.hpp>

extern "C" {
    extern volatile uint8_t g_scrId;
    extern volatile uint8_t g_scrReq;
}

FrontendApplication::FrontendApplication(Model& m, FrontendHeap& heap)
    : FrontendApplicationBase(m, heap)
{
}

void FrontendApplication::handleTickEvent()
{
    model.tick();

    /* 屏幕切换请求（三屏循环） */
    if (g_scrReq) {
        g_scrReq = 0;
        switch (g_scrId) {
            case 0: gotoMainScreenNoTransition(); break;
            case 1: gotoscreen0ScreenNoTransition(); break;
            case 2: gotoScreen1ScreenNoTransition(); break;
        }
    }

    FrontendApplicationBase::handleTickEvent();
}

/* screen0屏幕切换 */
void FrontendApplication::gotoscreen0ScreenNoTransition()
{
    transitionCallback = touchgfx::Callback<FrontendApplicationBase>(this,
        static_cast<void (FrontendApplicationBase::*)()>(&FrontendApplication::gotoscreen0ScreenNoTransitionImpl));
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoscreen0ScreenNoTransitionImpl()
{
    touchgfx::makeTransition<screen0View, screen0Presenter, touchgfx::NoTransition, Model>(
        &currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

/* Screen1屏幕切换 */
void FrontendApplication::gotoScreen1ScreenNoTransition()
{
    transitionCallback = touchgfx::Callback<FrontendApplicationBase>(this,
        static_cast<void (FrontendApplicationBase::*)()>(&FrontendApplication::gotoScreen1ScreenNoTransitionImpl));
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoScreen1ScreenNoTransitionImpl()
{
    touchgfx::makeTransition<Screen1View, Screen1Presenter, touchgfx::NoTransition, Model>(
        &currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}