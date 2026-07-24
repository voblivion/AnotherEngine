# spacetime (`vob::aoest`)

Space (position/rotation/velocity) and time (wall-clock, fixed tick, interpolation) management.

- Transform: `PositionComponent`, `RotationComponent`, `LinearVelocityComponent`, `AngularVelocityLocalComponent`, `TransformUtils.h`.
- Attachment/follow: `AttachmentComponent`/`AttachmentSystem` (rigid), `SoftFollowComponent`/`SoftFollowSystem` (camera-style soft follow).
- Time: `TimeContext`/`TimeSystem` (wall clock), `FixedRateTimeContext`/`FixedRateTimeSystem`/`FixedRateLimitingSystem` (fixed simulation tick).
- Interpolation, smoothing presentation between simulation ticks: `InterpolatedTransform.h`, `InterpolationTimeComponent`, `InterpolationContext`, `InterpolationExchangeContext`, `TransformInterpolationSystem`.
- Debug: frame-time tracking/history for both worlds.
