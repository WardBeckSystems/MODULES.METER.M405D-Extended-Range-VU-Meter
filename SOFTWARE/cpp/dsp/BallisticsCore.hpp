#pragma once
#include "EngineTypes.hpp"
#include <cmath>
#include <algorithm>

namespace wbs::engine {

    /**
     * @brief The three classic physical meter movement & aesthetic styles
     */
    enum class MeterMovementStyle : uint8_t {
        TraditionalSimpson = 0, // Classic American broadcast moving coil (ANSI C16.5, 300ms, zeta ~ 0.65, cream dial)
        PpmPeak = 1,            // European Peak Program Meter (DIN 45406 / BBC 3205-E, fast attack, log decay)
        ModernSifam = 2         // Modern British precision taut-band movement (Director/Clarity series, 200ms, zeta ~ 0.85, crisp white dial)
    };

    /**
     * @brief Ballistics mode for Peak Program Meter (PPM)
     */
    enum class PpmBallisticsMode : uint8_t {
        Type1_Nordic = 0, // 5 ms attack, 1.5 s / 24 dB decay (16 dB/s)
        Type2_BBC = 1     // 10 ms attack, 3.0 s / 24 dB decay (8 dB/s)
    };

    /**
     * @brief Meter calibration reference level
     */
    enum class MeterCalibration : uint8_t {
        Minus20dBFS = 0, // Default: 0 VU = -20 dBFS (+4 dBu)
        Minus18dBFS = 1, // EBU: 0 VU = -18 dBFS (+4 dBu)
        Minus16dBFS = 2, // 0 VU = -16 dBFS (+4 dBu)
        Minus14dBFS = 3  // 0 VU = -14 dBFS (+4 dBu)
    };

    /**
     * @brief Input signal domain
     */
    enum class MeterDomain : uint8_t {
        Digital = 0, // 0 dBFS ceiling, -20 dBFS nominal
        Analog = 1   // +24 dBu ceiling, +4 dBu nominal
    };

    /**
     * @brief Vertical edgewise meter operating modes (M466, M466M, M467)
     */
    enum class VerticalMeterMode : uint8_t {
        GainReduction = 0,    // Needle biased at 0 VU at top, drops downwards with compression
        GateAttenuation = 1,  // Noise gate attenuation (0 dB down to -50 dB)
        InputLevel = 2,       // Normal level monitoring (+3 to -inf)
        OutputLevel = 3
    };

    /**
     * @brief Standard 2nd-order VU Ballistic Filter Core supporting Traditional Simpson & Modern Sifam styles
     * Accurately simulates the physical moving coil needle inertia, spring tension, and magnetic damping.
     */
    class VuBallisticsFilter {
    public:
        VuBallisticsFilter() = default;

        void prepare(double sampleRate, MeterMovementStyle style = MeterMovementStyle::TraditionalSimpson) noexcept {
            sampleRate_ = sampleRate;
            style_ = style;
            reset();
            updateCoefficients();
        }

        void reset() noexcept {
            x1_ = x2_ = y1_ = y2_ = 0.0f;
            currentVu_ = 0.0f;
        }

        void setStyle(MeterMovementStyle style) noexcept {
            style_ = style;
            updateCoefficients();
        }

        inline float processSample(float rectifiedAbsSample) noexcept {
            // Full-wave rectified input drives 2nd-order ballistic filter
            const float yn = b0_ * rectifiedAbsSample + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;

            x2_ = x1_;
            x1_ = rectifiedAbsSample;
            y2_ = y1_;
            y1_ = yn;

            currentVu_ = std::max(0.0f, yn);
            return currentVu_;
        }

        [[nodiscard]] float getCurrentVu() const noexcept { return currentVu_; }
        [[nodiscard]] MeterMovementStyle getStyle() const noexcept { return style_; }

    private:
        void updateCoefficients() noexcept {
            // Simpson: Natural frequency ~2.1 Hz, damping ratio zeta ~0.65 (300 ms integration, 1-1.5% overshoot)
            // Sifam: Taut-band movement ~3.2 Hz, damping ratio zeta ~0.85 (faster 200 ms settling, minimal overshoot)
            const double f0 = (style_ == MeterMovementStyle::ModernSifam) ? 3.2 : 2.1;
            const double zeta = (style_ == MeterMovementStyle::ModernSifam) ? 0.85 : 0.65;
            const double omega0 = 2.0 * 3.14159265358979323846 * f0 / sampleRate_;

            // Bilinear transform approximation of continuous 2nd order mass-spring-damper
            const double alpha = omega0 * omega0;
            const double beta = 2.0 * zeta * omega0;
            const double a0 = 1.0 + beta + alpha;

            b0_ = static_cast<float>(alpha / a0);
            b1_ = static_cast<float>(2.0 * alpha / a0);
            b2_ = static_cast<float>(alpha / a0);
            a1_ = static_cast<float>(2.0 * (alpha - 1.0) / a0);
            a2_ = static_cast<float>((1.0 - beta + alpha) / a0);
        }

        double sampleRate_{ 48000.0 };
        MeterMovementStyle style_{ MeterMovementStyle::TraditionalSimpson };
        float b0_{ 0.0f }, b1_{ 0.0f }, b2_{ 0.0f };
        float a1_{ 0.0f }, a2_{ 0.0f };
        float x1_{ 0.0f }, x2_{ 0.0f };
        float y1_{ 0.0f }, y2_{ 0.0f };
        float currentVu_{ 0.0f };
    };

    /**
     * @brief Peak Program Meter (PPM) Ballistics with configurable Type 1 / Type 2 decay
     * and 3-second Peak Hold functionality.
     */
    class PpmBallisticsEngine {
    public:
        PpmBallisticsEngine() = default;

        void prepare(double sampleRate) noexcept {
            sampleRate_ = sampleRate;
            reset();
            updateCoefficients();
        }

        void reset() noexcept {
            currentPpm_ = 0.0f;
            heldPeak_ = 0.0f;
            holdSamplesRemaining_ = 0;
            stickyPeakLed_ = false;
            stickySamplesRemaining_ = 0;
        }

        void setMode(PpmBallisticsMode mode) noexcept {
            mode_ = mode;
            updateCoefficients();
        }

        void setPeakHoldEnabled(bool enabled) noexcept {
            peakHoldEnabled_ = enabled;
            if (!enabled) {
                heldPeak_ = 0.0f;
                holdSamplesRemaining_ = 0;
            }
        }

        inline void processSample(float absSample, float nominalLinearThreshold, float stickyLinearThreshold) noexcept {
            // 1. Attack / Decay envelope
            if (absSample > currentPpm_) {
                currentPpm_ += attackCoeff_ * (absSample - currentPpm_);
            } else {
                currentPpm_ *= decayCoeff_;
            }

            // 2. Peak Hold logic (holds PPM peaks above nominal for 3 seconds)
            if (peakHoldEnabled_ && currentPpm_ >= nominalLinearThreshold) {
                if (currentPpm_ >= heldPeak_) {
                    heldPeak_ = currentPpm_;
                    holdSamplesRemaining_ = static_cast<size_t>(sampleRate_ * 3.0); // 3 seconds
                }
            }

            if (holdSamplesRemaining_ > 0) {
                --holdSamplesRemaining_;
            } else {
                heldPeak_ *= decayCoeff_;
            }

            // 3. Sticky™ Peak LED logic (instantaneous peak exceeding -3 dBFS / +21 dBu held for 3 sec)
            if (absSample >= stickyLinearThreshold) {
                stickyPeakLed_ = true;
                stickySamplesRemaining_ = static_cast<size_t>(sampleRate_ * 3.0);
            } else if (stickySamplesRemaining_ > 0) {
                --stickySamplesRemaining_;
                if (stickySamplesRemaining_ == 0) {
                    stickyPeakLed_ = false;
                }
            }
        }

        [[nodiscard]] float getCurrentPpm() const noexcept { return currentPpm_; }
        [[nodiscard]] float getHeldPeak() const noexcept { return heldPeak_; }
        [[nodiscard]] bool getStickyPeak() const noexcept { return stickyPeakLed_; }

    private:
        void updateCoefficients() noexcept {
            const float attackSec = (mode_ == PpmBallisticsMode::Type1_Nordic) ? 0.005f : 0.010f; // 5ms or 10ms
            attackCoeff_ = 1.0f - std::exp(-1.0f / (attackSec * static_cast<float>(sampleRate_)));

            // 24 dB decay time: Type 1 = 1.5s, Type 2 = 3.0s
            const float decayTimeSec = (mode_ == PpmBallisticsMode::Type1_Nordic) ? 1.5f : 3.0f;
            const float tauDecay = decayTimeSec / 2.763102f; // ln(10^(24/20)) = 2.763102
            decayCoeff_ = std::exp(-1.0f / (tauDecay * static_cast<float>(sampleRate_)));
        }

        double sampleRate_{ 48000.0 };
        PpmBallisticsMode mode_{ PpmBallisticsMode::Type1_Nordic };
        bool peakHoldEnabled_{ true };

        float attackCoeff_{ 0.1f };
        float decayCoeff_{ 0.999f };

        float currentPpm_{ 0.0f };
        float heldPeak_{ 0.0f };
        size_t holdSamplesRemaining_{ 0 };

        bool stickyPeakLed_{ false };
        size_t stickySamplesRemaining_{ 0 };
    };

    /**
     * @brief Vertical Edgewise Meter Core for Ward-Beck Compressors & Noise Gates (M466, M466M, M467)
     * Models the physical upright edgewise meter movement where the needle drops downwards on compression.
     */
    class VerticalEdgewiseMeterCore {
    public:
        VerticalEdgewiseMeterCore() = default;

        void prepare(double sampleRate, VerticalMeterMode mode = VerticalMeterMode::GainReduction) noexcept {
            sampleRate_ = sampleRate;
            mode_ = mode;
            ballisticFilter_.prepare(sampleRate, MeterMovementStyle::TraditionalSimpson);
            reset();
        }

        void reset() noexcept {
            ballisticFilter_.reset();
            needlePosition_ = 1.0f; // 1.0 = top (0 dB GR / no compression)
        }

        void setMode(VerticalMeterMode mode) noexcept {
            mode_ = mode;
        }

        /**
         * @param gainReductionDb Positive decibel value of attenuation (e.g. 0.0 to 30.0 dB)
         */
        inline void processGainReduction(float gainReductionDb) noexcept {
            // Meter is mechanically biased to top (0 VU / 0 dB GR).
            // Needle deflects downward proportional to compression decibels:
            // 0 dB GR -> needle = 1.0 (top)
            // 20 dB GR -> needle = 0.0 (bottom / -20 dB mark)
            const float targetNorm = std::clamp(1.0f - (gainReductionDb / 20.0f), 0.0f, 1.0f);
            needlePosition_ = ballisticFilter_.processSample(targetNorm);
        }

        /**
         * @param attenuationDb Noise gate attenuation (0.0 to 50.0 dB)
         */
        inline void processGateAttenuation(float attenuationDb) noexcept {
            const float targetNorm = std::clamp(1.0f - (attenuationDb / 50.0f), 0.0f, 1.0f);
            needlePosition_ = ballisticFilter_.processSample(targetNorm);
        }

        [[nodiscard]] float getNeedlePosition() const noexcept { return needlePosition_; }
        [[nodiscard]] VerticalMeterMode getMode() const noexcept { return mode_; }

    private:
        double sampleRate_{ 48000.0 };
        VerticalMeterMode mode_{ VerticalMeterMode::GainReduction };
        VuBallisticsFilter ballisticFilter_;
        float needlePosition_{ 1.0f };
    };

    /**
     * @brief Normalized Pearson Phase Correlation Engine (-1.0 to +1.0)
     * Integration time ~300 ms for steady stereo correlation readout.
     */
    class PhaseCorrelationEngine {
    public:
        PhaseCorrelationEngine() = default;

        void prepare(double sampleRate) noexcept {
            sampleRate_ = sampleRate;
            reset();
            alpha_ = 1.0f - std::exp(-1.0f / (0.300f * static_cast<float>(sampleRate_)));
        }

        void reset() noexcept {
            sL2_ = 0.0f;
            sR2_ = 0.0f;
            sLR_ = 0.0f;
            correlation_ = 1.0f;
        }

        inline void processSample(float left, float right) noexcept {
            sL2_ += alpha_ * (left * left - sL2_);
            sR2_ += alpha_ * (right * right - sR2_);
            sLR_ += alpha_ * (left * right - sLR_);

            const float denom = std::sqrt(sL2_ * sR2_) + 1e-9f;
            correlation_ = std::clamp(sLR_ / denom, -1.0f, 1.0f);
        }

        [[nodiscard]] float getCorrelation() const noexcept { return correlation_; }

        [[nodiscard]] int getSevenSegmentIndex() const noexcept {
            if (correlation_ > 0.80f) return 3;
            if (correlation_ > 0.45f) return 2;
            if (correlation_ > 0.15f) return 1;
            if (correlation_ > -0.15f) return 0;
            if (correlation_ > -0.45f) return -1;
            if (correlation_ > -0.80f) return -2;
            return -3;
        }

    private:
        double sampleRate_{ 48000.0 };
        float alpha_{ 0.0001f };
        float sL2_{ 0.0f };
        float sR2_{ 0.0f };
        float sLR_{ 0.0f };
        float correlation_{ 1.0f };
    };

} // namespace wbs::engine
