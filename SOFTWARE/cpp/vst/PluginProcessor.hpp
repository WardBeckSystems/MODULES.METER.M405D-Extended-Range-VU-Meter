#pragma once
#include "../core/SharedPluginFrame.hpp"
#include "../core/BallisticsCore.hpp"
#include "../core/StateVariableCore.hpp"
#include <string>

namespace wbs::m405d {

enum class M405dInputSource : uint8_t {
    InputA = 0,
    InputB = 1,
    DirectFront = 2
};

enum class M405dImpedance : uint8_t {
    Bridge20k = 0,
    Term600 = 1,
    Term150 = 2
};

class M405DPluginProcessor : public engine::SharedPluginFrame {
public:
    M405DPluginProcessor();
    ~M405DPluginProcessor() override = default;

    void prepareModule(double hostRate, double oversampledRate, size_t maxBlockSize, size_t numChannels) override;
    void releaseModule() override;
    void updateParametersFromHost() noexcept override;
    void processModuleSlice(engine::AudioBlock& slice) noexcept override;
    void updateMeters(const engine::AudioBlock& audioBlock) noexcept override;

    // Parameter Setters
    void setRangeDbm(float rangeDbm) noexcept;
    void setInputSource(M405dInputSource src) noexcept;
    void setImpedance(M405dImpedance z) noexcept;
    void setHpFilter(bool enabled) noexcept;
    void setHeadsetGainDb(float gainDb) noexcept;

    // Getters for UI display
    [[nodiscard]] float getVuNorm() const noexcept { return vuNorm_; }
    [[nodiscard]] float getPpmNorm() const noexcept { return ppmNorm_; }
    [[nodiscard]] float getRangeDbm() const noexcept { return rangeDbm_; }
    [[nodiscard]] M405dInputSource getInputSource() const noexcept { return inputSource_; }
    [[nodiscard]] M405dImpedance getImpedance() const noexcept { return impedance_; }
    [[nodiscard]] bool getHpFilter() const noexcept { return hpFilterActive_; }
    [[nodiscard]] float getHeadsetGainDb() const noexcept { return headsetGainDb_; }
    [[nodiscard]] float getVuDbm() const noexcept { return currentVuDbm_; }
    [[nodiscard]] float getPpmDbm() const noexcept { return currentPpmDbm_; }

private:
    float rangeDbm_{ 0.0f }; // -50 dBm to +30 dBm in 2 dB steps
    M405dInputSource inputSource_{ M405dInputSource::InputA };
    M405dImpedance impedance_{ M405dImpedance::Bridge20k };
    bool hpFilterActive_{ false };
    float headsetGainDb_{ 0.0f };

    // Active 4-pole (24 dB/oct) High-Pass Filter @ 250 Hz
    engine::StateVariableFilterCore hpfPole1_;
    engine::StateVariableFilterCore hpfPole2_;

    // Ballistics
    engine::VuBallisticsFilter vuFilter_;
    engine::PpmBallisticsEngine ppmEngine_;

    // Meter readings
    float vuNorm_{ 0.0f };
    float ppmNorm_{ 0.0f };
    float currentVuDbm_{ -50.0f };
    float currentPpmDbm_{ -50.0f };
};

} // namespace wbs::m405d
