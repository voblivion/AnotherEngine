# AnotherEngine

C++23 game engine: EnTT ECS, OpenGL 4.6 renderer. Namespaces `vob::aoe*`.

## Architecture
- `IWorld`: start/update/stop. `EcsWorld` = registry + systems scheduled across threads. `MultiWorld` composes several `IWorld`s, each running independently.
- `exchange/` — one-way async data handoff between two `EcsWorld`s.
- Systems are plain structs with `init`/`execute`, declaring the registry/context/view access they need as members.

## Modules
- `engine/` — world/game loop/scheduling. See `include/vob/aoe/engine/CLAUDE.md`.
- `exchange/` — async cross-world data handoff. See `include/vob/aoe/exchange/CLAUDE.md`.
- `spacetime/` — space & time management. See `include/vob/aoe/spacetime/CLAUDE.md`.
- `rendering/` — OpenGL renderer. See `include/vob/aoe/rendering/CLAUDE.md`.
- `input/` — device input to logical game input. See `include/vob/aoe/input/CLAUDE.md`.
- `core/` — legacy, mid-migration. See `include/vob/aoe/core/CLAUDE.md`.
- `ecs/` — component-set/archetype scaffolding for data-driven entity spawning.
- `data/` — filesystem-backed database/loader system (json, string, multi-database).
- `window/` — window/context abstraction (GLFW-backed).
- `debug/` — assert/check macros, debug naming, ghost/replay controller.
- `editor/` — in-viewport gizmos (translate/rotate).
- `common/` — low-level input device types (mouse/keyboard/gamepad).
- `hack/` — scratch/temporary gameplay hooks.
- `math/` — small standalone math helpers.
