#ifndef SCREEN0PRESENTER_HPP
#define SCREEN0PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include "sensor_data.h"

using namespace touchgfx;

class screen0View;

class screen0Presenter : public touchgfx::Presenter, public ModelListener
{
public:
    screen0Presenter(screen0View& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual void sensorDataUpdated(const SensorData_t& data) override;

    virtual void fatigueAnalysisUpdated(uint8_t progress, uint8_t countdown,
                                        uint8_t fatigue_result, uint8_t confidence) override;

    virtual ~screen0Presenter() {}

private:
    screen0Presenter();

    screen0View& view;
};

#endif // SCREEN0PRESENTER_HPP
