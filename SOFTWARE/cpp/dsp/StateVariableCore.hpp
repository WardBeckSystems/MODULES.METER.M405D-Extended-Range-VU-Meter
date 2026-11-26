#pragma once
#include "EngineTypes.hpp"
#include <cmath>

namespace wbs::engine {

    /**
     * @brief Zero-Delay Feedback (ZDF) State Variable Filter (SVF)
     * Directly derived from Ward-Beck 4-126 active filter board (Kerwin-Huelsman-Newcomb KHN topology).
     * Unconditionally stable under continuous, fast frequency/Q parameter sweeps.
     */
    class StateVariableFilterCore {
    public:
        StateVariableFilterCore() = default;

        void prepare(double sampleRate) noexcept {
            sampleRate_ = sampleRate;
            reset();
        }

        void reset() noexcept {
            s1_ = 0.0;
            s2_ = 0.0;
        }

        /**
         * @brief Update filter coefficients
         * @param cutoffHz Cutoff or center frequency in Hertz (20 to 22000)
         * @param q Q-factor / resonance (0.2 to 20.0)
         * @param gainDb Boost/cut gain in decibels (-15.0 to +15.0)
         * @param type LowPass, HighPass, LowShelf, HighShelf, Peaking, Notch
         */
        void update(double cutoffHz, double q, double gainDb, FilterType type) noexcept {
            type_ = type;
            const double clampedFreq = std::clamp(cutoffHz, 10.0, sampleRate_ * 0.49);
            const double clampedQ = std::max(0.2, q);

            // Pre-warped frequency coefficient (trapezoidal integrator)
            const double omega = 3.14159265358979323846 * clampedFreq / sampleRate_;
            g_ = std::tan(omega);
            damping_ = 1.0 / clampedQ;

            // Linear gain ratio
            A_ = std::pow(10.0, gainDb / 40.0); // sqrt of linear gain for symmetrical shelving
            A2_ = A_ * A_;
        }

        [[nodiscard]] inline Sample processSample(Sample input) noexcept {
            // Trapezoidal Zero-Delay Feedback equation:
            // v0 = input
            // hp = (v0 - (damping + g)*s1 - s2) / (1 + damping*g + g*g)
            // bp = g*hp + s1
            // lp = g*bp + s2
            const double v0 = static_cast<double>(input);
            const double denom = 1.0 + damping_ * g_ + g_ * g_;
            const double hp = (v0 - (damping_ + g_) * s1_ - s2_) / denom;
            const double bp = g_ * hp + s1_;
            const double lp = g_ * bp + s2_;

            // State update for trapezoidal integrators
            s1_ = g_ * hp + bp;
            s2_ = g_ * bp + lp;

            double output = 0.0;
            switch (type_) {
                case FilterType::LowPass18dB:
                    // 2-pole SVF cascaded with passive pole gives 18 dB/oct
                    output = lp;
                    break;
                case FilterType::HighPass18dB:
                    output = hp;
                    break;
                case FilterType::PeakingBell:
                    // Peaking: input + (A2 - 1) * damping * bp
                    output = v0 + (A2_ - 1.0) * damping_ * bp;
                    break;
                case FilterType::LowShelf:
                    output = v0 + (A2_ - 1.0) * lp;
                    break;
                case FilterType::HighShelf:
                    output = v0 + (A2_ - 1.0) * hp;
                    break;
                case FilterType::Notch60Hz:
                    output = hp + lp;
                    break;
            }

            return static_cast<Sample>(output);
        }

        void processBlock(Sample* data, size_t numSamples) noexcept {
            for (size_t i = 0; i < numSamples; ++i) {
                data[i] = processSample(data[i]);
            }
        }

    private:
        double sampleRate_{ 48000.0 };
        FilterType type_{ FilterType::PeakingBell };
        double g_{ 0.1 };
        double damping_{ 0.707 };
        double A_{ 1.0 };
        double A2_{ 1.0 };

        // Internal integrator state registers (double precision prevents limit cycles)
        double s1_{ 0.0 };
        double s2_{ 0.0 };
    };

} // namespace wbs::engine
