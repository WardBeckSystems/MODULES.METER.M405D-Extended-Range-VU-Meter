#pragma once
#include "EngineTypes.hpp"
#include <atomic>
#include <cmath>

namespace wbs::engine {

    /**
     * @brief Thread-safe, sample-accurate smoothed parameter
     * Uses single-pole low-pass filtering to guarantee click-free parameter modulation
     */
    class SmoothedParameter {
    public:
        SmoothedParameter() = default;

        void reset(double sampleRate, double smoothingTimeSeconds, Sample initialValue = 0.0f) noexcept {
            sampleRate_ = sampleRate;
            targetValue_.store(initialValue, std::memory_order_relaxed);
            currentValue_ = initialValue;
            setTime(smoothingTimeSeconds);
        }

        void setTime(double smoothingTimeSeconds) noexcept {
            if (smoothingTimeSeconds <= 0.0) {
                coefficient_ = 0.0;
            } else {
                coefficient_ = std::exp(-1.0 / (smoothingTimeSeconds * sampleRate_));
            }
        }

        void setTarget(Sample newTarget) noexcept {
            targetValue_.store(newTarget, std::memory_order_release);
        }

        [[nodiscard]] Sample getTarget() const noexcept {
            return targetValue_.load(std::memory_order_acquire);
        }

        [[nodiscard]] Sample getNextValue() noexcept {
            const Sample target = targetValue_.load(std::memory_order_relaxed);
            currentValue_ = target + static_cast<Sample>(coefficient_) * (currentValue_ - target);
            return currentValue_;
        }

        [[nodiscard]] Sample getCurrentValue() const noexcept {
            return currentValue_;
        }

        [[nodiscard]] bool isSmoothing() const noexcept {
            return std::abs(currentValue_ - targetValue_.load(std::memory_order_relaxed)) > 1e-5f;
        }

    private:
        double sampleRate_{ 48000.0 };
        double coefficient_{ 0.99 };
        std::atomic<Sample> targetValue_{ 0.0f };
        Sample currentValue_{ 0.0f };
    };

    /**
     * @brief Parameter buffer manager sliced across inner block sub-segments
     */
    class ParameterBlockInterpolator {
    public:
        static void processLinearRamp(SmoothedParameter& param, Sample* outputBuffer, size_t numSamples) noexcept {
            for (size_t i = 0; i < numSamples; ++i) {
                outputBuffer[i] = param.getNextValue();
            }
        }
    };

} // namespace wbs::engine
