#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
#include "sensor_data.h"

class ModelListener
{
public:
    ModelListener() : model(0) {}

    virtual ~ModelListener() {}

    void bind(Model* m)
    {
        model = m;
    }

    virtual void sensorDataUpdated(const SensorData_t& data) {}

    virtual void fatigueAnalysisUpdated(uint8_t progress, uint8_t countdown,
                                        uint8_t fatigue_result, uint8_t confidence) {}

protected:
    Model* model;
};
#endif /* MODELLISTENER_HPP */
