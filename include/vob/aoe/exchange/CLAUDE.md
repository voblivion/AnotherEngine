# exchange (`vob::aoexc`)

One-way async data handoff between two `EcsWorld`s, so producer and consumer never block each other.

- `EcsExchangeData.h/.cpp` — triple-buffered registry, swapped between producer and consumer without blocking either.
- `EventPool.h` — type-erased multi-type event queue, accumulated between consumer reads.

One instance per direction; a bidirectional link between two worlds needs two.
