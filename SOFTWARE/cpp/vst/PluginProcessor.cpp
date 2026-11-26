#include "PluginProcessor.hpp"
#include <cmath>
#include <algorithm>

namespace wbs::m405d {

M405DPluginProcessor::M405DPluginProcessor()
    : engine::SharedPluginFrame("Ward-Beck M405D Extended Range VU & PPM Meter", "M405D") {
    defaultOversampling_ = engine::OversamplingRate::None;
}

void M405DPluginProcessor::prepareModule(double hostRate, double /*oversampledRate*/, size_t /*maxBlockSize*/, size_t /*numChannels*/) {
    vuFilter_.prepare(hostRate, engine::MeterMovementStyle::TraditionalSimpson);
    ppmEngine_.prepare(hostRate);
    ppmEngine_.setMode(engine::PpmBallisticsMode::Type2_BBC);

    hpfPole1_.prepare(hostRate);
    hpfPole1_.update(250.0, 0.5411961, 0.0, engine::FilterType::HighPass18dB);

    hpfPole2_.prepare(hostRate);
    hpfPole2_.update(250.0, 1.306563, 0.0, engine::FilterType::HighPass18dB);

    currentVuDbm_ = -50.0f;
    currentPpmDbm_ = -50.0f;
    vuNorm_ = 0.0f;
    ppmNorm_ = 0.0f;
}

void M405DPluginProcessor::releaseModule() {
    vuFilter_.reset();
    ppmEngine_.reset();
    hpfPole1_.reset();
    hpfPole2_.reset();
}

void M405DPluginProcessor::updateParametersFromHost() noexcept {
    // Coefficients stay fixed for 250 Hz 24 dB/oct roll-off
}

void M405DPluginProcessor::processModuleSlice(engine::AudioBlock& slice) noexcept {
    if (slice.numChannels == 0 || slice.numSamples == 0) return;

    // Direct input or Channel A / Channel B
    const float* inL = slice.getChannelPointer(0);
    const float* inR = (slice.numChannels > 1) ? slice.getChannelPointer(1) : inL;

    // Impedance loading attenuation
    float zLossLinear = 1.0f;
    if (impedance_ == M405dImpedance::Term600) {
        zLossLinear = 0.977f; // -0.2 dB 600 ohm loading
    } else if (impedance_ == M405dImpedance::Term150) {
        zLossLinear = 0.933f; // -0.6 dB 150 ohm loading
    }

    // Range attenuator sensitivity:
    // When rangeDbm_ is e.g. +20 dBm, meter sensitivity is attenuated by 20 dB so +20 dBm reads 0 dB on meter.
    const float detGain = std::pow(10.0f, -rangeDbm_ / 20.0f) * zLossLinear;

    // Headset output monitoring gain
    const float monGain = std::pow(10.0f, headsetGainDb_ / 20.0f);

    for (size_t i = 0; i < slice.numSamples; ++i) {
        // Mono sum or selected input channel
        float sample = 0.5f * (inL[i] + inR[i]);

        // Active High-Pass Filter (24 dB/oct @ 250 Hz)
        if (hpFilterActive_) {
            sample = static_cast<float>(hpfPole1_.processSample(sample));
            sample = static_cast<float>(hpfPole2_.processSample(sample));
        }

        // Full wave rectified detector for ballistics
        const float detSig = std::abs(sample) * detGain;

        vuFilter_.processSample(detSig);
        ppmEngine_.processSample(detSig, 0.707f, 0.95f);

        // Monitoring output pass-through
        for (size_t ch = 0; ch < slice.numChannels; ++ch) {
            float* chPtr = slice.getChannelPointer(ch);
            chPtr[i] *= monGain;
        }
    }
}

void M405DPluginProcessor::updateMeters(const engine::AudioBlock& /*audioBlock*/) noexcept {
    const float rawVu = vuFilter_.getCurrentVu();
    const float rawPpm = ppmEngine_.getCurrentPpm();

    // 0 dBm reference: 0.1 linear = -20 dBFS = 0 dB on scale
    const float vuDb = (rawVu > 1e-5f) ? (20.0f * std::log10(rawVu) + 20.0f) : -40.0f;
    const float ppmDb = (rawPpm > 1e-5f) ? (20.0f * std::log10(rawPpm) + 20.0f) : -40.0f;

    // Normalized deflections (0.0 to 1.0)
    // Left meter: ANSI C16.5 VU: -20 VU to +3 VU (23 dB range)
    vuNorm_ = std::clamp((vuDb + 20.0f) / 23.0f, 0.0f, 1.0f);

    // Right meter: BS 5428 / BBC PPM: -12 dB to +12 dB (24 dB range, center 0 dB is 0.5)
    ppmNorm_ = std::clamp((ppmDb + 12.0f) / 24.0f, 0.0f, 1.0f);

    // Actual absolute signal levels in dBm
    currentVuDbm_ = rangeDbm_ + vuDb;
    currentPpmDbm_ = rangeDbm_ + ppmDb;
}

void M405DPluginProcessor::setRangeDbm(float rangeDbm) noexcept {
    // Stepped in 2 dB increments: -50 dBm to +30 dBm
    const float stepped = std::round(rangeDbm / 2.0f) * 2.0f;
    rangeDbm_ = std::clamp(stepped, -50.0f, 30.0f);
}

void M405DPluginProcessor::setInputSource(M405dInputSource src) noexcept {
    inputSource_ = src;
}

void M405DPluginProcessor::setImpedance(M405dImpedance z) noexcept {
    impedance_ = z;
}

void M405DPluginProcessor::setHpFilter(bool enabled) noexcept {
    hpFilterActive_ = enabled;
}

void M405DPluginProcessor::setHeadsetGainDb(float gainDb) noexcept {
    headsetGainDb_ = std::clamp(gainDb, -60.0f, 12.0f);
}

} // namespace wbs::m405d
