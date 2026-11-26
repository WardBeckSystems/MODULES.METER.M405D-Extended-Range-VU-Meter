// Copyright (c) 2026 Ward-Beck Systems (Ward-Beck.com - Ward-Beck.ca). All rights reserved.
// M405D: its part of 2030.env/vst/clap_wrapper/ClapModuleRegistry.cpp (suite index None).

// --- descriptor ---
static const clap_plugin_descriptor_t s_desc_m405d = {
    CLAP_VERSION,
    "ca.wbsps.m405d_extended_range_meter",
    "Ward-Beck M405D Extended Range Meter",
    "Ward-Beck Systems Preservation Society",
    "https://wbsps.ca",
    "",
    "https://wbsps.ca/contact",
    "1.0.0",
    "Extended range dual-movement audio test meter with ANSI C16.5 VU and BS 5428 PPM movements and stepped 2 dB attenuator",
    kAnalyzerFeatures
};

// --- parameters ---
std::vector<ParameterInfo> buildM405DParams() {
    std::vector<ParameterInfo> p;

    // 0: Range Attenuator (-50 to +30 dBm in 2 dB steps)
    p.push_back({
        0, "Range Attenuator", "Metering", -50.0, 30.0, 0.0,
        CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED,
        [](engine::SharedPluginFrame* f, double val) {
            static_cast<m405d::M405DPluginProcessor*>(f)->setRangeDbm(static_cast<float>(val));
        },
        [](engine::SharedPluginFrame* f) {
            return static_cast<double>(static_cast<m405d::M405DPluginProcessor*>(f)->getRangeDbm());
        },
        [](double v) { char buf[32]; std::snprintf(buf, sizeof(buf), "%+.0f dBm", v); return std::string(buf); },
        nullptr
    });

    // 1: Input Select (0: Input A, 1: Input B)
    p.push_back({
        1, "Input Select", "Routing", 0.0, 1.0, 0.0,
        CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED,
        [](engine::SharedPluginFrame* f, double val) {
            static_cast<m405d::M405DPluginProcessor*>(f)->setInputSource(val >= 0.5 ? m405d::M405dInputSource::InputB : m405d::M405dInputSource::InputA);
        },
        [](engine::SharedPluginFrame* f) {
            return (static_cast<m405d::M405DPluginProcessor*>(f)->getInputSource() == m405d::M405dInputSource::InputB) ? 1.0 : 0.0;
        },
        [](double v) { return (v >= 0.5) ? "INPUT B" : "INPUT A"; },
        nullptr
    });

    // 2: Input Impedance (0: Bridge 20k, 1: 600 ohm, 2: 150 ohm)
    p.push_back({
        2, "Input Impedance", "Impedance", 0.0, 2.0, 0.0,
        CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED,
        [](engine::SharedPluginFrame* f, double val) {
            int z = static_cast<int>(std::round(val));
            static_cast<m405d::M405DPluginProcessor*>(f)->setImpedance(static_cast<m405d::M405dImpedance>(z));
        },
        [](engine::SharedPluginFrame* f) {
            return static_cast<double>(static_cast<int>(static_cast<m405d::M405DPluginProcessor*>(f)->getImpedance()));
        },
        [](double v) {
            int z = static_cast<int>(std::round(v));
            if (z == 1) return "600 OHM";
            if (z == 2) return "150 OHM";
            return "BRIDGE 20k";
        },
        nullptr
    });

    // 3: High Pass Filter (250 Hz 24 dB/oct)
    p.push_back({
        3, "High Pass Filter", "Filter", 0.0, 1.0, 0.0,
        CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED,
        [](engine::SharedPluginFrame* f, double val) {
            static_cast<m405d::M405DPluginProcessor*>(f)->setHpFilter(val >= 0.5);
        },
        [](engine::SharedPluginFrame* f) {
            return static_cast<m405d::M405DPluginProcessor*>(f)->getHpFilter() ? 1.0 : 0.0;
        },
        [](double v) { return (v >= 0.5) ? "HPF 250Hz IN" : "HPF BYPASS"; },
        nullptr
    });

    // 4: Headset Monitor Gain (-60 to +12 dB)
    p.push_back({
        4, "Headset Volume", "Monitor", -60.0, 12.0, 0.0,
        CLAP_PARAM_IS_AUTOMATABLE,
        [](engine::SharedPluginFrame* f, double val) {
            static_cast<m405d::M405DPluginProcessor*>(f)->setHeadsetGainDb(static_cast<float>(val));
        },
        [](engine::SharedPluginFrame* f) {
            return static_cast<double>(static_cast<m405d::M405DPluginProcessor*>(f)->getHeadsetGainDb());
        },
        [](double v) { char buf[32]; std::snprintf(buf, sizeof(buf), "%.1f dB", v); return std::string(buf); },
        nullptr
    });

    return p;
}

