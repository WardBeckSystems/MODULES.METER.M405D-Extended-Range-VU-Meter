# M405D Extended Range VU Meter · Meter

Ward-Beck Systems M405D, 1984|17. Wiki page: [M405D on WBSps.ca](https://wardbecksystems.github.io/WBSps.ca/PAGES/M405D.html)

| Folder | What is in it | Files |
|---|---|---|
| [SCHEMATICS/](SCHEMATICS/) | electrical drawings: schematics, functional diagrams, component layouts, wiring | 1 |
| [MECHANICAL/](MECHANICAL/) | front panels, cover plates, metalwork, assemblies; the front panel as vectors | 6 |
| [PINOUTS/](PINOUTS/) | the RTO-144 connector: connection sheets, connector layouts, rear views | 1 |
| [MANUALS/](MANUALS/) | manuals, manual pages, price lists, brochures, documents | 8 |
| [PHOTOS/](PHOTOS/) | photographs of the module | 2 |
| [SOFTWARE/](SOFTWARE/) | program listings, firmware, the plugin | 2 |
| [KICAD/](KICAD/) | KiCad schematics traced from the drawings, the symbol, PCB projects | 2 |
| [OCR/](OCR/) | the note of every file: date, origin, title block, OCR text | 9 |

<!-- module data: tools/module_specs.py, tools/forum_quotes.py -->

## Specifications

From the technical data sheet: [1984 - M405D - 013.jpg](MANUALS/1984%20-%20M405D%20-%20013.jpg) page 1 (read by OCR; check against the page). As data: [`Specifications.json`](Specifications.json).

| | |
|---|---|
| **Range** | 50 dBm to +30 dBm in or PPM readings dB steps for |
| **Input selection** | Pushbutton selects between two sources ADC PJ339A type jack on front panel provides direct input. |
| **Source impedance** | 600 ohms or 150 ohms. |
| **Input impedance** | 600 ohms, 150 ohms or 20k ohms. |
| **Attenuator Tracking** | +/ -0.25 dB per step (non accumulative) |
| **Meter ballistics** | Complies with ANSI C16.5 Complies with' BSI B54297. |
| **High pass filter** | -3 dB at 250 Hz. -24 dB at 120 Hz. |
| **Frequency response** |  |
| &nbsp;&nbsp;Metering | +/ -0.25 dB from 30 Hz to 15 kHz |
| **Monitoring outputs** | +/-0.5 dB from 50 Hz to 10 kHz. |
| **Total harmonic distortion** |  |
| &nbsp;&nbsp;Headset output | Less than 1%, 50 Hz to 10 kHz (measured at any input level and at +24 dBm output level) |
| **Speaker output** | Less than 1%, 50 Hz to 10 kHz (measured at any input level and at watts, ohms output level |
| **Noise** | 40 dB below vu or PPM reference. |
| **Output impedance** |  |
| &nbsp;&nbsp;Headset output | 150 ohms |
| **Speaker output** | ohms |

<!-- /module data -->

## Photos

_Photographs of the module itself: front, back, inside, racked._

<table>
<tr><td align="center" valign="top" width="33%"><a href="https://drive.google.com/file/d/0B0yR0b5T78SKYkV6VmVPTE0zSVE/view?resourcekey=0-jOLGTL67mwA_eOIcX7Ezyg" title="Open on Google Drive"><img src="https://drive.google.com/thumbnail?sz=w480&id=0B0yR0b5T78SKYkV6VmVPTE0zSVE&resourcekey=0-jOLGTL67mwA_eOIcX7Ezyg" width="240" alt="1984 - M405D - 011 - Picture.jpg"></a><br><sub>1984|17 · 011 - Picture</sub></td></tr>
</table>

## Mechanical

_Mechanical drawings: front panels, cover plates, metalwork and hardware (-03- drawings), AutoCAD files, each with its `(EXTRACTED).svg` vectors; `<module> - front panel.svg` is the front panel as vectors._

<table>
<tr><td align="center" valign="top" width="33%"><a href="https://drive.google.com/file/d/1HonwVIzxwPl6E0iV3k2PCGoGgtzNHSv1/view" title="Open on Google Drive"><img src="https://drive.google.com/thumbnail?sz=w480&id=1HonwVIzxwPl6E0iV3k2PCGoGgtzNHSv1" width="240" alt="19xx - M405D - DOC - 405-03-048 - NOTES REMOVE ALL BURRS.jpg"></a><br><sub>19xx - M405D - DOC - 405-03-048 - NOTES REMOVE ALL BURRS · <a href="MECHANICAL/19xx%20-%20M405D%20-%20DOC%20-%20405-03-048%20-%20NOTES%20REMOVE%20ALL%20BURRS%20(EXTRACTED).svg">vectors</a></sub></td></tr>
</table>

## 3-D model

[`MECHANICAL/M405D.stl`](MECHANICAL/M405D.stl) (GitHub turns it in 3-D) · [view, change and generate it on WBSps.ca](https://wardbecksystems.github.io/WBSps.ca/2026/stl.html?m=M405D)

- **Size:** 41 × 178 mm panel, 165 mm deep, 3.2 mm plate (assumed: 1.6" x 7" x 6.5", the common module)
- **From the drawing:** 0 holes and cut-outs, 0 knobs (front panel: `MECHANICAL/M405D - front panel.svg`)
- no drawing of the front panel: the panel is blank
- Made by `tools/module_models.py` (→ `MECHANICAL/M405D.model.json`) and `tools/stl/make_stl.mjs` in [WBSps.ca](https://github.com/WardBeckSystems/WBSps.ca); the site makes the same STL from the same JSON.

## Manuals and documents

_Manuals and manual pages, price lists, RTO-144 connection sheets, brochures, ads and other document scans._

<table>
<tr><td align="center" valign="top" width="33%"><a href="https://drive.google.com/file/d/0B0yR0b5T78SKZWlDYkMtWHQ1NEk/view?resourcekey=0-N8TZSdxjt6H57h4dgCXfUQ" title="Open on Google Drive"><img src="https://drive.google.com/thumbnail?sz=w480&id=0B0yR0b5T78SKZWlDYkMtWHQ1NEk&resourcekey=0-N8TZSdxjt6H57h4dgCXfUQ" width="240" alt="1984 - M405D - 010.jpg"></a><br><sub>1984|17 · 010</sub></td><td align="center" valign="top" width="33%"><a href="https://drive.google.com/file/d/0B0yR0b5T78SKcVF5aUpwUVNpY1U/view?resourcekey=0-y6RhS3NHUXuARmyPUGM_dg" title="Open on Google Drive"><img src="https://drive.google.com/thumbnail?sz=w480&id=0B0yR0b5T78SKcVF5aUpwUVNpY1U&resourcekey=0-y6RhS3NHUXuARmyPUGM_dg" width="240" alt="1984 - M405D - 012.jpg"></a><br><sub>1984|17 · 012</sub></td><td align="center" valign="top" width="33%"><a href="https://drive.google.com/file/d/0B0yR0b5T78SKNXZOOWc0TF9kQ0U/view?resourcekey=0-Ifg5ofLkBGD8rhod06L9KQ" title="Open on Google Drive"><img src="https://drive.google.com/thumbnail?sz=w480&id=0B0yR0b5T78SKNXZOOWc0TF9kQ0U&resourcekey=0-Ifg5ofLkBGD8rhod06L9KQ" width="240" alt="1984 - M405D - 013.jpg"></a><br><sub>1984|17 · 013</sub></td></tr>
<tr><td align="center" valign="top" width="33%"><a href="https://drive.google.com/file/d/0B0yR0b5T78SKVmZUV0laSURTTmM/view?resourcekey=0-1oeXAT4-thJmR8rW6KsMtA" title="Open on Google Drive"><img src="https://drive.google.com/thumbnail?sz=w480&id=0B0yR0b5T78SKVmZUV0laSURTTmM&resourcekey=0-1oeXAT4-thJmR8rW6KsMtA" width="240" alt="1984 - M405D - 014.jpg"></a><br><sub>1984|17 · 014</sub></td><td align="center" valign="top" width="33%"><a href="https://drive.google.com/file/d/0B0yR0b5T78SKNEg1YlhDcFpUWFU/view?resourcekey=0-ELRW2AFmEhc9uqj2iAS2cA" title="Open on Google Drive"><img src="https://drive.google.com/thumbnail?sz=w480&id=0B0yR0b5T78SKNEg1YlhDcFpUWFU&resourcekey=0-ELRW2AFmEhc9uqj2iAS2cA" width="240" alt="1984 - M405D - 015.jpg"></a><br><sub>1984|17 · 015</sub></td><td align="center" valign="top" width="33%"><a href="https://drive.google.com/file/d/0B0yR0b5T78SKQm1EQVA5Wl9LSUk/view?resourcekey=0-2FR2oTd0ogwILP_LtEwAbw" title="Open on Google Drive"><img src="https://drive.google.com/thumbnail?sz=w480&id=0B0yR0b5T78SKQm1EQVA5Wl9LSUk&resourcekey=0-2FR2oTd0ogwILP_LtEwAbw" width="240" alt="1984 - M405D - 016.jpg"></a><br><sub>1984|17 · 016</sub></td></tr>
<tr><td align="center" valign="top" width="33%"><a href="https://drive.google.com/file/d/0B0yR0b5T78SKbEZkZGpKcDg4MjA/view?resourcekey=0-Squd-AdpwxiYQmqB6ph10Q" title="Open on Google Drive"><img src="https://drive.google.com/thumbnail?sz=w480&id=0B0yR0b5T78SKbEZkZGpKcDg4MjA&resourcekey=0-Squd-AdpwxiYQmqB6ph10Q" width="240" alt="1984 - M405D - 017.jpg"></a><br><sub>1984|17 · 017</sub></td></tr>
</table>

## KiCad

_KiCad: a schematic (`.kicad_sch`) traced from each electrical drawing, the module symbol (`.kicad_sym`) and its PCB projects. Traced schematics are the line work only: redraw with symbols over them._

- [M405D.kicad_sym](KICAD/M405D.kicad_sym)
- [index.html](KICAD/index.html)

## Software

- [cpp](SOFTWARE/cpp)
- [index.html](SOFTWARE/index.html)

---
Archive: Anthony Peter Kuzub · [WBSps.ca](https://WBSps.ca) · [WBSps.Kuzub.com](https://WBSps.Kuzub.com)
