#include "ClapModuleRegistry.hpp"

#ifndef WBS_SINGLE_MODULE_INDEX
#define WBS_SINGLE_MODULE_INDEX 0
#endif

namespace {

uint32_t clap_single_get_plugin_count(const clap_plugin_factory* /*factory*/) {
    return 1;
}

const clap_plugin_descriptor_t* clap_single_get_plugin_descriptor(
    const clap_plugin_factory* /*factory*/,
    uint32_t index)
{
    return (index == 0) ? wbs::clap::getDescriptor(WBS_SINGLE_MODULE_INDEX) : nullptr;
}

const clap_plugin_t* clap_single_create_plugin(
    const clap_plugin_factory* /*factory*/,
    const clap_host_t* host,
    const char* plugin_id)
{
    const auto* desc = wbs::clap::getDescriptor(WBS_SINGLE_MODULE_INDEX);
    if (!desc) return nullptr;
    if (plugin_id && std::strcmp(plugin_id, desc->id) != 0) {
        return nullptr;
    }
    auto* instance = wbs::clap::createPluginInstance(WBS_SINGLE_MODULE_INDEX, host);
    return instance ? instance->getClapPlugin() : nullptr;
}

static const clap_plugin_factory_t s_single_factory = {
    clap_single_get_plugin_count,
    clap_single_get_plugin_descriptor,
    clap_single_create_plugin
};

bool clap_single_entry_init(const char* /*plugin_path*/) {
    return true;
}

void clap_single_entry_deinit() {
}

const void* clap_single_entry_get_factory(const char* factory_id) {
    if (factory_id && std::strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0) {
        return &s_single_factory;
    }
    return nullptr;
}

} // anonymous namespace

CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION,
    clap_single_entry_init,
    clap_single_entry_deinit,
    clap_single_entry_get_factory
};
