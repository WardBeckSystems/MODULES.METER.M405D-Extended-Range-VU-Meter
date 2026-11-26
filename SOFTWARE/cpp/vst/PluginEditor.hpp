#pragma once
#include "PluginProcessor.hpp"

namespace wbs::m405d {

class M405DPluginEditor {
public:
    explicit M405DPluginEditor(M405DPluginProcessor& processor);
    ~M405DPluginEditor() = default;

private:
    M405DPluginProcessor& processor_;
};

} // namespace wbs::m405d
