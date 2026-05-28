// App/Inc/logic/pid_controller.hpp
#pragma once

namespace Hephaestus {

    class PidController {
    private:
        float _kp, _ki, _kd;
        float _integral;
        float _prevError;
        
        float _outMin, _outMax;

    public:
        PidController(float kp, float ki, float kd, float outMin, float outMax)
            : _kp(kp), _ki(ki), _kd(kd), _integral(0.0f), _prevError(0.0f), 
              _outMin(outMin), _outMax(outMax) {}

        void setTunings(float kp, float ki, float kd) {
            _kp = kp; _ki = ki; _kd = kd;
        }

        void reset() {
            _integral = 0.0f;
            _prevError = 0.0f;
        }

        float compute(float setpoint, float measured, float dt) {
            if (dt <= 0.0001f) return 0.0f; 

            float error = setpoint - measured;

            float pOut = _kp * error;

            float derivative = (error - _prevError) / dt;
            float dOut = _kd * derivative;

            float provisionalOut = pOut + (_ki * _integral) + dOut;

            bool isSaturatedTop = (provisionalOut >= _outMax) && (error > 0.0f);
            bool isSaturatedBot = (provisionalOut <= _outMin) && (error < 0.0f);

            if (!isSaturatedTop && !isSaturatedBot) {
                _integral += error * dt; 
            }

            float output = pOut + (_ki * _integral) + dOut;

            if (output > _outMax) output = _outMax;
            else if (output < _outMin) output = _outMin;

            _prevError = error;
            return output;
        }
    };

} // namespace Hephaestus