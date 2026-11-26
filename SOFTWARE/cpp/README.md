# M405D: the C++ plugin (2030 suite)

M405D's part of the earlier C++ plugin suite (`2030.env/vst` in WBSps.ca, CLAP): everything this project needs to
make the module, copied here so it lives with the module. The suite itself still builds from `2030.env/vst`
(one CMake project for all modules); this folder is the module's own code and data, not a separate build.

The Rust VST3 / CLAP plugin is in [`../plugin`](../plugin).

| | What |
|---|---|
| [`M405D.json`](M405D.json) | the catalog entry: title, description, circuit summary, signal path (nodes, wires), CLAP id and name, suite index None |
| `vst/` | the module's processor (DSP chain, parameters) and editor (GUI), its CMakeLists and README |
| `dsp/` | the DSP cores it includes (filters, saturation, ballistics …) and the shared plugin frame |
| `shell/` | the CLAP shell: `ClapSingleEntry.cpp` (compiled with `WBS_SINGLE_MODULE_INDEX=None`) and `registry.cpp`, the module's descriptor, parameter table and instance |
| `gui/` | `layout.hpp` (control positions on the 176 × 770 faceplate), `circuit.hpp` (the signal-path diagram), and the web console's panel art (`module_None_real/cad/signal.svg`) |
| `notes/` | the build plan |

Copyright (c) 2026 Ward-Beck Systems (Ward-Beck.com - Ward-Beck.ca). All rights reserved.
