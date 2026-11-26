#pragma once
#include "EngineTypes.hpp"
#include <cmath>
#include <algorithm>

namespace wbs::engine {

    /**
     * @brief Ward-Beck 4-141 Virtual-Earth Power Summing & Bus Dynamics Core
     * Derived from Schematic B 4-141-02-001 (Summing Amplifier, Nov 1974)
     * Emulates finite virtual-earth open loop saturation, power supply rail sag, and bus crosstalk.
     */
    class PowerSummingEngine {
    public:
        PowerSummingEngine() = default;

        void prepare(double sampleRate) noexcept {
            sampleRate_ = sampleRate;
            railSagEnvelope_ = 0.0f;
            currentRailSagGain_ = 1.0f;
            crosstalkFilterStateL_ = 0.0f;
            crosstalkFilterStateR_ = 0.0f;
            reset();
        }

        void reset() noexcept {
            railSagEnvelope_ = 0.0f;
            currentRailSagGain_ = 1.0f;
            crosstalkFilterStateL_ = 0.0f;
            crosstalkFilterStateR_ = 0.0f;
        }

        void setParameters(ConsoleBusSize consoleSize, Sample railSagAmount, Sample crosstalkAmount) noexcept {
            consoleSize_ = consoleSize;
            railSagAmount_ = std::clamp(railSagAmount, 0.0f, 1.0f);
            crosstalkAmount_ = std::clamp(crosstalkAmount, 0.0f, 1.0f);

            // Channel count scaling factor (more channels increase summing bus impedance load)
            channelLoadFactor_ = static_cast<Sample>(consoleSize) / 16.0f;

            // Attack (15 ms) and discharge (200 ms) time constants for central DC power supply capacitor banks
            sagAttackCoeff_ = static_cast<Sample>(1.0 - std::exp(-1.0 / (0.015 * sampleRate_)));
            sagReleaseCoeff_ = static_cast<Sample>(1.0 - std::exp(-1.0 / (0.200 * sampleRate_)));
        }

        /**
         * @brief Process stereo summing bus through 4-141 virtual-earth and rail sag model
         */
        void processStereoBlock(Sample* leftChannel, Sample* rightChannel, size_t numSamples) noexcept {
            const Sample xtalkCoeff = crosstalkAmount_ * 0.0035f; // -49 dB bleed

            for (size_t i = 0; i < numSamples; ++i) {
                const Sample inL = leftChannel[i];
                const Sample inR = rightChannel[i];

                // 1. Measure composite mix bus energy
                const Sample busEnergy = 0.5f * (std::abs(inL) + std::abs(inR)) * channelLoadFactor_;

                // 2. Dynamic power supply rail sag integrator (PSU capacitor reservoir discharge)
                const Sample coeff = (busEnergy > railSagEnvelope_) ? sagAttackCoeff_ : sagReleaseCoeff_;
                railSagEnvelope_ += coeff * (busEnergy - railSagEnvelope_);

                // 3. Dynamic Headroom / Sag Gain modulation
                // Heavy transient bursts momentarily pull down central DC supply rails by up to 2.5 dB
                const Sample targetSagGain = 1.0f - (railSagEnvelope_ * 0.32f * railSagAmount_);
                currentRailSagGain_ += 0.15f * (targetSagGain - currentRailSagGain_);

                // 4. Virtual-earth finite open-loop soft saturation:
                // V_bus = in * [1 - epsilon * (in/I_max)^2]
                const Sample satL = inL * (1.0f - 0.05f * inL * inL) * currentRailSagGain_;
                const Sample satR = inR * (1.0f - 0.05f * inR * inR) * currentRailSagGain_;

                // 5. Inter-channel Motherboard Crosstalk (Capacitive trace coupling leaks HF out-of-phase)
                // d/dt high-pass bleed
                const Sample dR = satR - crosstalkFilterStateR_;
                crosstalkFilterStateR_ = satR;
                const Sample dL = satL - crosstalkFilterStateL_;
                crosstalkFilterStateL_ = satL;

                leftChannel[i] = satL - (xtalkCoeff * dR);
                rightChannel[i] = satR - (xtalkCoeff * dL);
            }
        }

        [[nodiscard]] Sample getCurrentRailSagDb() const noexcept {
            return (currentRailSagGain_ > 1e-4f) ? (20.0f * std::log10(currentRailSagGain_)) : 0.0f;
        }

    private:
        double sampleRate_{ 48000.0 };
        ConsoleBusSize consoleSize_{ ConsoleBusSize::Production24 };
        Sample railSagAmount_{ 0.4f };
        Sample crosstalkAmount_{ 0.5f };
        Sample channelLoadFactor_{ 1.5f };

        Sample sagAttackCoeff_{ 0.05f };
        Sample sagReleaseCoeff_{ 0.005f };
        Sample railSagEnvelope_{ 0.0f };
        Sample currentRailSagGain_{ 1.0f };

        Sample crosstalkFilterStateL_{ 0.0f };
        Sample crosstalkFilterStateR_{ 0.0f };
    };

} // namespace wbs::engine
