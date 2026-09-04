# rendering (`vob::aoegl`)

OpenGL 4.6 forward+ renderer: light clustering, CSM sun shadows + spot shadows, depth pre-pass, opaque g-buffer (normal/surface/geometric-normal/depth), SSAO, SSR, opaque composition, translucent, skybox, anti-aliasing, present, debug geometry, hud, ImGui.

- `GraphicTypes.h` — GL type aliases (`GraphicId`=GLuint, etc), `k_invalidId`.
- `resources/` — `GpuResource<>` RAII wrappers owning one GL name each (`GpuBuffer`, `GpuTexture`, `GpuFramebuffer`, `GpuVertexArray`, `GpuProgram`, `GpuQuery`); deletion is deferred through `GpuDeleteQueue`, drained once per frame. Composites: `GpuMesh`, `GpuMaterial`, `GpuShader` (shared via `shared_ptr`), `GpuRenderTarget`/`GpuRenderSceneTargets`.
- `GpuState.h` — tracked GL state wrapper (avoids redundant `glEnable`/`glBindFramebuffer`/etc calls), templated on expected-change likelihood.
- `Model.h`, `ShadedMesh.h`, `StaticModelTemplate.h` — in-memory model/mesh representations (shared mesh + material + shading pass), template = mesh+material+lights bundle for spawning instances.
- `data/` — `ImageData`/`ImageLoader` (image → pixels), `ModelData`/`ModelLoader` (assimp-backed static/rigged mesh+bone loading).
- `components/` — `CameraComponent`, `LightComponent`, `StaticModelComponent`/`RiggedModelComponent`, `InstancedModelsComponent`, `ModelTransformComponent` (per-entity model UBO).
- `contexts/` — `CameraDirectorContext`/`DebugCameraDirectorContext` (active/debug camera), `RenderSceneContext` (all UBOs/framebuffers/textures/programs for the pipeline above), `GpuDeleteQueueContext`, `DebugMeshContext` (line/tri debug draw builder: addLine/addObb/addSphere/etc), `DebugProgramContext` (hot-reloadable shader sources).
- `systems/` — `RenderSceneSystem` (main draw), `DebugCameraDirectorSystem`, `DebugRenderLightsSystem`, `ReleaseUnusedGpuResourcesSystem` (drains `GpuDeleteQueue`), `PrepareImGuiFrameSystem`/`RenderImGuiFrameSystem`, `SwapBuffersSystem`.
- `ModelUtils.h/.cpp`, `ProgramUtils.h/.cpp` — build `Mesh`/`Model` from loaded data; compile/link the various pipeline shader programs.
- `CameraUtils.h` — camera property extraction, view-frustum plane culling.
- `Color.h` — `Rgb`/`Rgba` + named color constants.
- `ImGuiUtils.h/.cpp` — ImGui init/teardown for GLFW+GL.
- `shaders/` — GLSL sources for the pipeline above + `bindings.glsl`/`defines.h` (shared UBO/SSBO/texture binding indices, capacity constants).
