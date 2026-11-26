#pragma once
#include "EngineTypes.hpp"
#include <cmath>

namespace wbs::engine {

    /**
     * @brief Models Ward-Beck 80-series custom iron transformers and discrete transistor stages
     * Implements non-linear saturation curves with asymmetric quadratic bias (2nd harmonic)
     * and low-frequency magnetic core flux saturation (3rd & 5th harmonics).
     */
    class NonLinearTransformerCore {
    public:
        NonLinearTransformerCore() = default;

        void prepare(double sampleRate) noexcept {
            sampleRate_ = sampleRate;
            dcBlockStateX_ = 0.0f;
            dcBlockStateY_ = 0.0f;
            lowFluxDetector_ = 0.0f;
        }

        void reset() noexcept {
            dcBlockStateX_ = 0.0f;
            dcBlockStateY_ = 0.0f;
            lowFluxDetector_ = 0.0f;
        }

        /**
         * @brief Process single sample through transformer & transistor model
         * @param in Input audio sample
         * @param drive Normalized drive amount (0.0 to 1.0 -> maps to 1x to 4x gain)
         * @param ironHysteresis Amount of magnetic core flux saturation (0.0 to 1.0)
         */
        [[nodiscard]] inline Sample processSample(Sample in, Sample drive, Sample ironHysteresis) noexcept {
            // 1. Scale input drive (broadcast nominal +4 dBu to +24 dBu clipping margin)
            const Sample alpha = 1.0f + (drive * 3.5f);
            const Sample beta = drive * 0.18f;    // 2nd harmonic transistor warmth
            const Sample gamma = ironHysteresis * 0.35f; // Odd-order core hysteresis

            // 2. Track low-frequency energy for frequency-dependent magnetic hysteresis
            // Low frequencies saturate the iron core earlier than high frequencies
            const Sample absIn = std::abs(in);
            lowFluxDetector_ += 0.005f * (absIn - lowFluxDetector_);
            const Sample dynamicGamma = gamma * (1.0f + 2.0f * lowFluxDetector_);

            // 3. Evaluate non-linear transfer function
            // V_out = tanh(alpha*x + beta*x^2) / sqrt(1 + dynamicGamma*x^2)
            const Sample x = in;
            const Sample polyNumerator = alpha * x + beta * (x * x);
            const Sample sat = std::tanh(polyNumerator);
            const Sample out = sat / std::sqrt(1.0f + dynamicGamma * (x * x));

            // 4. DC Blocker to prevent transistor DC bias offset drift
            // y[n] = x[n] - x[n-1] + 0.9995 * y[n-1]
            const Sample dcBlocked = out - dcBlockStateX_ + 0.9995f * dcBlockStateY_;
            dcBlockStateX_ = out;
            dcBlockStateY_ = dcBlocked;

            return dcBlocked;
        }

        void processBlock(Sample* data, size_t numSamples, Sample drive, Sample ironHysteresis) noexcept {
            for (size_t i = 0; i < numSamples; ++i) {
                data[i] = processSample(data[i], drive, ironHysteresis);
            }
        }

    private:
        double sampleRate_{ 48000.0 };
        Sample dcBlockStateX_{ 0.0f };
        Sample dcBlockStateY_{ 0.0f };
        Sample lowFluxDetector_{ 0.0f };
    };

    using NonLinearCore = NonLinearTransformerCore;

} // namespace wbs::engine
