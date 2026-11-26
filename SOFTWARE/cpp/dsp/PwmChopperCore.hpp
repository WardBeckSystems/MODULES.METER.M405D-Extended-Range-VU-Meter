#pragma once
#include "EngineTypes.hpp"
#include "StateVariableCore.hpp"
#include <cmath>
#include <algorithm>

namespace wbs::engine {

    /**
     * @brief Rodger Beck's M406 Pulse-Width Modulation (PWM) Chopper Dynamics Engine
     * Emulates the variable duty cycle (3% to 97%) shunt gate attenuation and active low-pass reconstruction.
     */
    class PwmChopperDynamicsCore {
    public:
        PwmChopperDynamicsCore() = default;

        void prepare(double sampleRate) noexcept {
            sampleRate_ = sampleRate;
            rampPhase_ = 0.0f;
            envelopeLevel_ = 0.0f;
            controlVoltage_ = 0.0f;

            // Reconstruct filter at 40 kHz (strips out virtual 96 kHz PWM carrier)
            reconstructionFilter_.prepare(sampleRate);
            reconstructionFilter_.update(40000.0, 0.707, 0.0, FilterType::LowPass18dB);
            reset();
        }

        void reset() noexcept {
            rampPhase_ = 0.0f;
            envelopeLevel_ = 0.0f;
            controlVoltage_ = 0.0f;
            reconstructionFilter_.reset();
        }

        void setParameters(Sample thresholdDb, Sample ratio, Sample attackSeconds, Sample releaseSeconds, PwmEngineMode mode) noexcept {
            thresholdDb_ = thresholdDb;
            ratio_ = std::max(1.0f, ratio);
            mode_ = mode;

            // Ballistic time coefficients
            attackCoeff_ = static_cast<Sample>(1.0 - std::exp(-1.0 / (std::max(0.0001, static_cast<double>(attackSeconds)) * sampleRate_)));
            releaseCoeff_ = static_cast<Sample>(1.0 - std::exp(-1.0 / (std::max(0.001, static_cast<double>(releaseSeconds)) * sampleRate_)));

            // PWM carrier frequency (typically 96 kHz inside 8x oversampled domain)
            rampIncrement_ = static_cast<Sample>(96000.0 / sampleRate_);
        }

        [[nodiscard]] inline Sample processSample(Sample input, Sample& gainReductionDb) noexcept {
            // 1. Full-wave rectification & Sidechain Ballistics Detection
            const Sample absInput = std::abs(input);
            const Sample coeff = (absInput > envelopeLevel_) ? attackCoeff_ : releaseCoeff_;
            envelopeLevel_ += coeff * (absInput - envelopeLevel_);

            // 2. Convert detected peak to dB
            const Sample levelDb = (envelopeLevel_ > 1e-5f) ? (20.0f * std::log10(envelopeLevel_)) : -100.0f;

            // 3. Compute Gain Reduction Curve
            Sample targetGrDb = 0.0f;
            if (levelDb > thresholdDb_) {
                targetGrDb = (thresholdDb_ - levelDb) * (1.0f - (1.0f / ratio_));
            }
            gainReductionDb = std::abs(targetGrDb);

            // 4. Derive Rodger Beck DC Control Voltage (0.0 to 1.0)
            // 0 dB GR -> 0.03 duty cycle (3% shunt); -30 dB GR -> 0.97 duty cycle (97% shunt)
            const Sample normGr = std::clamp(gainReductionDb / 30.0f, 0.0f, 1.0f);
            const Sample dutyCycle = 0.03f + normGr * 0.94f; // 3% to 97% range

            // 5. Dual Engine Evaluation
            if (mode_ == PwmEngineMode::EcoVcaPristine) {
                // Pristine VCA Mode: direct linear attenuation (zero aliasing, zero latency)
                const Sample linearGain = std::pow(10.0f, targetGrDb / 20.0f);
                return input * linearGain;
            }

            // Authentic PWM Chopper Mode (Hardware Exact):
            // Generate virtual 96 kHz linear ramp (0.0 to 1.0)
            rampPhase_ += rampIncrement_;
            if (rampPhase_ >= 1.0f) {
                rampPhase_ -= 1.0f;
            }

            // Differential Comparator: When ramp < dutyCycle, shunt gate is closed (signal shunted to ground)
            const bool shuntGateActive = (rampPhase_ < dutyCycle);
            const Sample choppedSample = shuntGateActive ? (input * 0.015f) : input;

            // Active low-pass reconstruction filter removes 96 kHz switching pulses
            return reconstructionFilter_.processSample(choppedSample);
        }

    private:
        double sampleRate_{ 384000.0 }; // Typically evaluated in 8x domain
        Sample thresholdDb_{ -18.0f };
        Sample ratio_{ 3.0f };
        PwmEngineMode mode_{ PwmEngineMode::AuthenticPwmChopper };

        Sample attackCoeff_{ 0.1f };
        Sample releaseCoeff_{ 0.01f };
        Sample envelopeLevel_{ 0.0f };
        Sample controlVoltage_{ 0.0f };

        Sample rampPhase_{ 0.0f };
        Sample rampIncrement_{ 0.25f };

        StateVariableFilterCore reconstructionFilter_;
    };

} // namespace wbs::engine
