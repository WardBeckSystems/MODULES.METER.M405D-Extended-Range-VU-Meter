#pragma once
#ifdef None
#undef None
#endif
#ifdef Success
#undef Success
#endif
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <array>
#include <span>

#if defined(__x86_64__) || defined(_M_X64)
#include <xmmintrin.h>
#endif

namespace wbs::engine {

    // Floating-point precision selection (32-bit float for SIMD vectorization, 64-bit double for filter states)
    using Sample = float;
    using Sample64 = double;

    // Standard alignment for AVX2 / AVX-512 / ARM NEON vector instructions
    constexpr size_t kSimdAlignment = 64;
    constexpr size_t kMaxBlockSize = 2048;
    constexpr size_t kMaxChannels = 8;
    constexpr size_t kInnerSliceSize = 64; // Block slicing for sample-accurate parameter interpolation

    // Architectural Oversampling Factor
    enum class OversamplingRate : uint8_t {
        None = 1,
        TwoX = 2,
        FourX = 4,
        EightX = 8
    };

    // Filter Topology Type
    enum class FilterType : uint8_t {
        LowPass18dB,      // M404 / M464
        HighPass18dB,     // M404 / M464
        LowShelf,         // M462 / M522
        HighShelf,        // M462 / M522
        PeakingBell,      // M462 / M522
        Notch60Hz         // M402
    };

    // Dynamics Operating Mode
    enum class PwmEngineMode : uint8_t {
        EcoVcaPristine = 0,   // Continuous mathematical transfer function (zero-latency, ultra-low CPU)
        AuthenticPwmChopper = 1 // 8x oversampled 100 kHz linear ramp & comparator shunt modulation
    };

    // Console Summing Size
    enum class ConsoleBusSize : uint8_t {
        Small8 = 8,
        Standard16 = 16,
        Production24 = 24,
        LargeFormat32 = 32
    };

    // Memory buffer slice spanning non-interleaved channel buffers
    struct AudioBlock {
        union {
            Sample* const* channels{ nullptr };
            Sample* const* channelData;
        };
        size_t numChannels{ 0 };
        size_t numSamples{ 0 };

        Sample* getChannelPointer(size_t ch) const { return channels[ch]; }
        std::span<Sample> getChannelSpan(size_t ch) const {
            return std::span<Sample>(channels[ch], numSamples);
        }
    };

    // Denormal protection scope for real-time audio threads
    struct ScopedDenormalFlags {
#if defined(__x86_64__) || defined(_M_X64)
        uint32_t oldMxcsr{ 0 };
        ScopedDenormalFlags() noexcept {
            oldMxcsr = _mm_getcsr();
            _mm_setcsr(oldMxcsr | 0x8040); // Enable DAZ (Denormals Are Zero) & FTZ (Flush To Zero)
        }
        ~ScopedDenormalFlags() noexcept {
            _mm_setcsr(oldMxcsr);
        }
#else
        ScopedDenormalFlags() noexcept = default;
        ~ScopedDenormalFlags() noexcept = default;
#endif
    };

} // namespace wbs::engine
