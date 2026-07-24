# engine (`vob::aoeng`)

Core game loop, world abstraction, ECS world, frame scheduling.

- `Game.h` — `IWorld`/`IGameController`: the run loop, swaps worlds on request.
- `FrameJob.h`, `SynchronizedFrameJobsRepeater`, `UnsynchronizedFrameJobsRepeater` — running work across threads, in lockstep (within one world) or free-running (one per world).
- `EcsWorldDataAccess.h` — the accessor types (`EcsWorldContextRef`, `EcsWorldViewRef`, etc) systems use to declare what data they touch.
- `EcsWorld.h/.cpp` — `IWorld` impl: registry + systems scheduled across threads.
- `MultiWorld.h` — `IWorld` impl composing several `IWorld`s, each on its own thread.
- `EcsEntitySyncContext.h` — bidirectional entity-id mapping between two registries, used alongside `exchange/`.
- `TracyFrameSystem.h` — marks a Tracy frame boundary each tick.
