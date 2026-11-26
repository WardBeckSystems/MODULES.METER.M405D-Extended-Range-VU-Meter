#pragma once
#include "EngineTypes.hpp"
#include "BallisticsCore.hpp"
#include "OversamplingEngine.hpp"
#include "ParameterEngine.hpp"
#include "NonLinearCore.hpp"
#include "StateVariableCore.hpp"
#include "PwmChopperCore.hpp"
#include "PowerSummingEngine.hpp"

#include <string>
#include <vector>
#include <memory>

namespace wbs::engine {

    /**
     * @brief Abstract Base Plugin Framework shared across all 9 Ward-Beck VST3/AU/AAX plugins
     * Enforces real-time memory guarantees, block slicing, oversampling, and host latency reporting.
     */
    class SharedPluginFrame {
    public:
        SharedPluginFrame(const std::string& pluginName, const std::string& moduleCode)
            : pluginName_(pluginName), moduleCode_(moduleCode) {}
        
        virtual ~SharedPluginFrame() = default;

        // Life-Cycle Methods called by VST3/AU wrapper
        void prepareToPlay(double sampleRate, size_t maxBlockSize, size_t numChannels) {
            sampleRate_ = sampleRate;
            maxBlockSize_ = maxBlockSize;
            numChannels_ = numChannels;

            // Initialize oversampler (default to 8x for PWM chopper accuracy)
            oversampler_.prepare(sampleRate, maxBlockSize, numChannels, defaultOversampling_);

            // Allocate static slice memory for inner block slicing (zero allocation on audio thread)
            sliceBuffer_.resize(numChannels * kInnerSliceSize);
            slicePointers_.resize(numChannels);
            for (size_t ch = 0; ch < numChannels; ++ch) {
                slicePointers_[ch] = sliceBuffer_.data() + (ch * kInnerSliceSize);
            }

            // Call derived module preparation hook
            prepareModule(sampleRate, oversampler_.getOversampledSampleRate(), maxBlockSize, numChannels);
        }

        void releaseResources() {
            releaseModule();
        }

        /**
         * @brief Master processBlock loop executed by the DAW audio render thread
         */
        void processBlock(AudioBlock& audioBlock) noexcept {
            // 1. Enforce FTZ/DAZ to avoid denormal CPU performance penalties
            const ScopedDenormalFlags denormalGuard;

            if (audioBlock.numSamples == 0 || audioBlock.numChannels == 0) {
                return;
            }

            // 2. Poll thread-safe parameter FIFO and update smoothed targets
            updateParametersFromHost();

            // 3. Upsample block if oversampling is active
            AudioBlock processingBlock = oversampler_.upsample(audioBlock);

            // 4. Slice processing block into small 64-sample inner slices for sample-accurate smoothing
            const size_t totalSamples = processingBlock.numSamples;
            size_t sampleOffset = 0;

            while (sampleOffset < totalSamples) {
                const size_t sliceSize = std::min(kInnerSliceSize, totalSamples - sampleOffset);

                // Populate slice view
                for (size_t ch = 0; ch < processingBlock.numChannels; ++ch) {
                    slicePointers_[ch] = processingBlock.getChannelPointer(ch) + sampleOffset;
                }
                AudioBlock sliceBlock{ slicePointers_.data(), processingBlock.numChannels, sliceSize };

                // Execute derived module DSP processing
                processModuleSlice(sliceBlock);

                sampleOffset += sliceSize;
            }

            // 5. Downsample back into host buffer
            oversampler_.downsample(audioBlock);

            // 6. Metering peak detection hook
            updateMeters(audioBlock);
        }

        [[nodiscard]] size_t getLatencySamples() const noexcept {
            return oversampler_.getLatencyInSamples();
        }

        [[nodiscard]] const std::string& getPluginName() const noexcept { return pluginName_; }
        [[nodiscard]] const std::string& getModuleCode() const noexcept { return moduleCode_; }

    protected:
        // Pure virtual hooks implemented by each specific hardware module target
        virtual void prepareModule(double hostRate, double oversampledRate, size_t maxBlockSize, size_t numChannels) = 0;
        virtual void releaseModule() = 0;
        virtual void updateParametersFromHost() noexcept = 0;
        virtual void processModuleSlice(AudioBlock& slice) noexcept = 0;
        virtual void updateMeters(const AudioBlock& audioBlock) noexcept = 0;

        std::string pluginName_;
        std::string moduleCode_;
        double sampleRate_{ 48000.0 };
        size_t maxBlockSize_{ 512 };
        size_t numChannels_{ 2 };

        OversamplingEngine oversampler_;
        OversamplingRate defaultOversampling_{ OversamplingRate::EightX };

    private:
        std::vector<Sample> sliceBuffer_;
        std::vector<Sample*> slicePointers_;
    };

} // namespace wbs::engine
