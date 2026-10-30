# Credits

PokeFinder-GPU stands on a lot of other people's work. This page lists every project, data source and tool used to build it, with its owner. License names come from each project's repository. Each feature that lands in this fork adds its own rows here.

## The program itself

| Project | Owner | Used for |
| --- | --- | --- |
| [PokeFinder](https://github.com/Admiral-Fish/PokeFinder) (GPL-3.0) | Admiral-Fish (Admiral_Fish), bumba, EzPzStreamz and contributors | The whole application this fork is built on: Gen 3–5 and Gen 8 tools, UI, RNG classes and the [EncounterTableGenerator](https://github.com/Admiral-Fish/EncounterTableGenerator) data |

The upstream PokeFinder credits (RNG Reporter, PPRNG, 3DSRNGTool, the PokemonRNG team and others) are kept in the README.

## GPU search (Gen 5)

| Project | Owner | Used for |
| --- | --- | --- |
| pk5-gpu-search (`gen5search`) | Javier (Xenon-Hacks) | The OpenCL boot-seed, IV-scan and PID-scan kernels and the two-stage search shape the Gen 5 GPU searcher is a port of, with the CPU re-derivation of every hit |

## Gen 6 (X/Y, OR/AS) tab

| Project | Owner | Used for |
| --- | --- | --- |
| [3DSRNGTool](https://github.com/wwwwwwzx/3DSRNGTool) (MIT) | wwwwwwzx, with zaksabeast, Zep715, Real96, Admiral Fish and Shiny_Sylveon (per its credits) | Reference for the Gen 6 generation order, Wonder Card, egg and horde logic, the legend/gift/fossil list and the delays not measured live (X/Y fossils, Kalos starters 280/330, Kanto starters 4, Pidgey 14; OR/AS Hoopa-ring and soaring legends 16/20/88, Treecko 186, Voltorb 120, Electrode 122) |
| [PKHeX](https://github.com/kwsch/PKHeX) (GPL-3.0) | Kurt (kwsch / Kaphotics) and contributors | The X/Y and OR/AS personal tables (`personal_xy`, `personal_ao`: base stats, gender ratios, abilities, forms), the wild encounter tables (`encounter_x.pkl`, `encounter_y.pkl`, `encounter_or.pkl`, `encounter_as.pkl`), location and item names, the OR/AS static/gift and in-game trade lists, the `.wc6` Wonder Card layout and PK6 encryption details |
| [pk3DS](https://github.com/kwsch/pk3DS) | Kurt (kwsch / Kaphotics) | Part of the research toolset for X/Y romfs data |
| [gen6-gpu-tools](https://github.com/lincoln-lm/gen6-gpu-tools) (GPL-3.0) | Lincoln-LM | Reference for the OpenCL MT19937 initial-seed search that the Seed Finder kernel is modelled on |
| [poke6-seed-finder](https://github.com/ukikagi/poke6-seed-finder) (MIT) | ukikagi | Reference for the "base seed" (基準seed) search and the frame + 63 IV offset |
| Gen 6 RNG research (`Gen6-RNG-and-CIA-Extraction-Complete-Reference` with its ORAS addendum and the 2026-10-30 implementation spec) | Javier (Xenon-Hacks) | The live-verified seeding formula (86 frames on X/Y, 236/237 on OR/AS), draw orders, delays, shiny locks, Mirage spot rule, TinyMT overworld model and save offsets the tab is built on, and the RAM logs the tests replay |

## Gen 7 (Sun/Moon, Ultra Sun/Ultra Moon) tab

| Project | Owner | Used for |
| --- | --- | --- |
| [3DSRNGTool](https://github.com/wwwwwwzx/3DSRNGTool) (MIT) | wwwwwwzx and contributors | Baseline for the Gen 7 generation blocks, Wonder Card fields and SOS chain tables that the research re-verified |
| [PKHeX](https://github.com/kwsch/PKHeX) (GPL-3.0) | Kurt (kwsch / Kaphotics) and contributors | SM/USUM save layout, personal tables, encounter and Wonder Card formats |
| Gen 7 RNG research (`PK7 Generation` project, 2026-10-20 to 2026-10-30: Moon reference, Sun and USUM references, hardware timing on a New 3DS) | Javier (Xenon-Hacks) | The field-timeline model (blink engine, Continue rule, fidget, Honey D = 93/63), needle seed search, every static/gift/fishing/trade/egg/Pelago rule, the Honey and fishing spot presets, the timer constants, and the Python reference models the tests replay |

## Research tools

These were used to produce the Gen 6 and Gen 7 research. None of them ship with PokeFinder-GPU.

| Tool | Owner | Used for |
| --- | --- | --- |
| [Azahar](https://github.com/azahar-emu/azahar) (GPL-3.0) | The Azahar project, based on Citra | Running X 1.0, Y 1.5, OR/AS, Sun/Moon and USUM with a debugger and memory access to verify every recipe live |
| [Project_CTR](https://github.com/3DSGuy/Project_CTR) (ctrtool, makerom) | 3DSGuy (jakcron) | Decrypting the game images and extracting `code.bin` |
| [Capstone](https://github.com/capstone-engine/capstone) | Nguyen Anh Quynh and the Capstone contributors | Disassembling the game code |
| [NTR-HR](https://github.com/Nanquitas/BootNTR) lineage, Luma3DS | Their respective authors | Live RAM reads on the New 3DS for the Gen 7 hardware runs |

## Libraries

| Library | Owner | License |
| --- | --- | --- |
| [Qt 6](https://www.qt.io/) | The Qt Company and contributors | LGPL-3.0 |
| [JSON for Modern C++](https://github.com/nlohmann/json) | Niels Lohmann | MIT |
| [Zstandard](https://github.com/facebook/zstd) | Meta (Facebook) | BSD / GPL-2.0 dual |
| [gf2](https://github.com/nessan/gf2) | nessan | MIT |
| [fph-table](https://github.com/renzibei/fph-table) (`meta_fph_table.h`) | renzibei | Apache-2.0 |

PokeFinder-GPU is licensed under GPL-3.0. See [LICENSE](LICENSE).
