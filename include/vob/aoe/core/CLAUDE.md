# core

Legacy scaffolding, mid-migration (per map in `common/input/KeyboardUtil.h:415-423`). Mostly superseded, don't build new code on it without checking usage first.

- `data/` (`vob::aoe::data`) — old generic database (`ADatabase`, `Cache`, `Id`, `_Database`/`_DatabaseLoader`/`_Handle`). Superseded by top-level `data/` (`vob::aoedt`).
- `resource/` (`vob::aoe::res`) — old resource holder/handle (`AHolder`, `DefaultHolder`, `Handle`). Marked for removal in the migration map.
- `type/` (`vob::aoe::type`) — `ADynamicType` (still referenced by `src/DataHolder.h` in NeonHorizon). Migration target: `sta/type`.
- `visitor/` (`vob::misvi`/`vob::aoe`) — old json/visitor scaffolding (`_Applicator`, `_Factory`, `_Standard`, `_StringId`, `_Traits`, `_Utils`, `_Variant`, `JsonWriter`). Superseded by `Miscellaneous`'s `vob::misvi`/`vob::misty` visitor & type libs. Migration target: `sta/visitor`.
- `utils/SimpleProfiler.h` — macro-based manual timer (`PROFILER_START/LAP`). Marked for removal; actual profiling uses Tracy (`engine/TracyFrameSystem.h`).
