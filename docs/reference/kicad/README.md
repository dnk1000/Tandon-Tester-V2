# TM100 Tester KiCad Project

This is a **KiCad legacy-format schematic project** generated from the TM100 ESP32 Hardware Design Specification. KiCad 6/7/8/9 can open/import the `.sch` file and then save it as a modern `.kicad_sch`.

## Files
- `TM100_Tester.sch` — schematic
- `TM100_Tester.lib` / `TM100_Tester-cache.lib` — project-local symbols
- `sym-lib-table` — project symbol library mapping
- `TM100_Tester.pro` — legacy project stub
- `PRELIMINARY_BOM.csv` — design-level BOM

## Important
This is an engineering draft, not a fabrication release. Exact footprints, LMR51430 package pin numbering, reverse-polarity/eFuse choices, and the physical TM-100 power connector must be finalized and reviewed before PCB layout/fabrication.

The LMR51430 passives use TI's published 12V-to-5V, 3A, 500kHz example as the starting point (6.8uH, 100k/13.7k feedback divider, 2x22uF/25V output, >=2x4.7uF/50V input ceramic).
