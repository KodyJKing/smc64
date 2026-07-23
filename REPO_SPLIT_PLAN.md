## Goal

Finish splitting Mario mod (SMC64) and Spark mod loader into separate repositories.

- Strip spark code form smc64.
- Strip smc64 code from spark.
- Add spark as a git submodule to smc64.
- Add multiple mod directory support to spark.
    - Detect a SPARK_MODS_PATH environment variable.
- Setup build script for smc64 preserving current workflow: On change to spark OR smc64, uninject, rebuild and reinject loader and mod.


## Context

- I have already cloned smc64 into a new repository "spark" and added it to our workspace.
- I have already created a new branch in smc64 called "repo-split". The stripped down Mario mod should land here.
- Current actual state (verified 2026-07-21):
  - `smc64` repo (`origin/KodyJKing/smc64.git`) is on branch `mod-repo-split` (not `repo-split` -
    looks like it was renamed/recreated). Working tree has only trivial diffs (this plan file +
    workspace file).
  - `spark` repo (`origin/KodyJKing/spark.git`) is on `main`, clean, and is a straight fork of the
    `dll-split` history - it still contains a full copy of everything, including `smc64/` (the
    Mario mod), `data/` (Mario model conversion scripts), and `tags/` (compiled Mario/jackal-shield
    tags). Nothing has actually been stripped from either repo yet - both are still complete,
    duplicate snapshots of the monorepo. This plan starts from that state.
  - Neither repo currently references the other as a submodule; both have identical top-level
    `premake5.lua` (single `workspace "spark"` including `spark/`, `smc64/`, `spark-launcher/`,
    `spark-dll-test/`, `spark-unicorn-test/`), and identical `.gitmodules`
    (`vendor/asmjit`, `vendor/imgui`, `vendor/Detours`, `vendor/libsm64`, `vendor/unicorn`).
    `vendor/minhook` and `vendor/zydis` are committed source, not submodules.

## Repo boundaries

| Path | Keep in `spark` (engine) | Keep in `smc64` (mod) |
|---|---|---|
| `spark/` (engine project) | ✅ | ❌ (via submodule) |
| `smc64/` (mod project) | ❌ | ✅ |
| `spark-launcher/` (injector exe) | ✅ | ❌ (built via submodule, see below) |
| `spark-dll-test/`, `spark-unicorn-test/` | ✅ (engine-only tests) | ❌ |
| `data/` (Mario model/armature conversion) | ❌ | ✅ (Mario-specific) |
| `tags/` (`mario`, `jackal_shield`, `mario_cyborg`) | ❌ | ✅ (all 3 subfolders are mod content) |
| `vendor/libsm64` | ❌ | ✅ (mod-only dependency) |
| `vendor/imgui` | ✅ | ❌ (mod references `../spark/vendor/imgui` instead of its own copy) |
| `vendor/minhook`, `vendor/zydis` (committed source) | ✅ | ❌ (referenced via submodule path) |
| `vendor/asmjit`, `vendor/Detours`, `vendor/unicorn` (submodules) | ✅ | ❌ |
| `reversing/` (Halo engine RE notes) | ✅ | ❌ (nothing in there is Mario-specific today) |
| top-level `scripts/` (build/watch/inject/package) | ✅ (engine-focused subset) | ✅ (own copies, mod-focused subset - see Phase B) |

Confirmed: `reversing/notes/` (and the rest of `reversing/`) belongs entirely to `spark` - none of
it is Mario-specific.

## Key design decision: submodule placement

`smc64/premake5.lua` currently references the engine via relative paths: `../spark/src`,
`../vendor/minhook/include`, `../vendor/imgui`, and links `"spark"`. Per correction during
execution, the `spark` repo is added as a submodule at `vendor/spark/` (not repo-root `spark/` -
engine deps belong under `vendor/`). Since the `spark` repo's own root is itself an "engine repo"
(containing `spark/`, `spark-launcher/`, `vendor/`, its own `premake5.lua`, etc.), this produces a
nested `vendor/spark/spark/src` layout. `smc64/premake5.lua`'s relative paths were updated:

- `../spark/src` → `../vendor/spark/spark/src`
- `../vendor/imgui` → `../vendor/spark/vendor/imgui` (drop `smc64`'s own imgui copy entirely - it's
  the same source, just compiled into a separate TU set for `smc64.dll`)
- `../vendor/libsm64/...` stays as-is (still smc64's own submodule)
- `../vendor/minhook/include` → `../vendor/spark/vendor/minhook/include` (kept, NOT dropped -
  turned out to still be required: `smc64/src` itself has zero direct MinHook/Zydis API calls,
  but spark's own `spark/src/spark/hook/Hook.hpp` header does `#include "MinHook.h"`, and smc64
  transitively includes that header. Discovered via a C1083 `MinHook.h` build failure after
  initially dropping the includedir - grep-confirming "no direct usage" isn't sufficient to prove
  an includedir is safe to remove when the dependency comes in transitively through engine
  headers).

For the top-level `smc64/premake5.lua` workspace file, the original plan was to
`include "spark/premake5.lua"` wholesale (reusing the submodule's own top-level script). This
turned out to be wrong: premake writes the generated `.sln` file into the directory of whichever
script calls `workspace(...)`, so including the submodule's own top-level script put `spark.sln`
under `vendor/spark/` instead of the repo root (breaking `build.ps1`'s assumption that `spark.sln`
lives at repo root). The working solution instead declares `workspace "spark"` directly in the
outer repo's own top-level `premake5.lua`, and includes the individual project scripts from
inside the submodule:

```lua
group "Dependencies"
   include "vendor/spark/vendor/minhook"
   include "vendor/spark/vendor/zydis"
group ""

include "vendor/spark/spark/premake5.lua"
include "vendor/spark/spark-launcher/premake5.lua"
include "smc64/premake5.lua"
```

This was the one step that needed a build-and-fix iteration, as anticipated.

## Phase A — Strip `spark` repo down to engine-only — ✅ DONE

- [x] Created working branch `strip-mod` off `main` in the `spark` repo.
- [x] `git rm -r smc64/ data/ tags/` (all confirmed mod-only content).
- [x] `git submodule deinit -f vendor/libsm64 && git rm -r vendor/libsm64` (spark itself doesn't
      need libsm64).
- [x] Edited top-level `premake5.lua`: dropped `include "smc64/premake5.lua"`.
- [x] Removed `scripts/copy_mods.ps1` entirely (and its call from `build.ps1`) - it hardcoded
      `smc64.dll` staging, so it's a mod-repo concern, not spark's. `package.ps1`/
      `install_package.ps1` were trimmed to only stage/remove `spark.dll` + the xaudio2 detour -
      spark's own end-user package (`spark-$Config.zip`) is loader-only now. A mod repo is
      expected to layer its own packaging on top of spark's package (build it, drop its own dll
      into `mods/`, re-zip) rather than spark knowing about any specific mod.
- [x] Added `SPARK_MODS_PATH` support to `ModLoader::loadAll()` (see Phase C - done here since it's
      engine-only code).
- [x] Replaced mod-specific `spark/shipfiles/SMC64_REAMDE.md` with a generic
      `spark/shipfiles/README.md` covering just the loader.
- [x] Updated `README.md`/`MOD_LOADER_PLAN.md` to describe spark as a standalone engine/mod-loader
      repo, with mods (like smc64) living in their own repos and being staged into `mods/`.
- [x] Full rebuild (`spark.sln`, all projects) - `spark.dll`, `spark-launcher.exe`,
      `spark-dll-test.exe` all build with 0 errors (only pre-existing `LNK4217` noise). Note:
      submodules must be initialized (`git submodule update --init --recursive`) before running
      `premake5.exe vs2022`, or `vendor/imgui`'s glob silently matches 0 files - see
      `/memories/repo/smc64-dll-split.md`. `spark-unicorn-test` needs a separately pre-built
      `unicorn.lib` (`scripts/build_unicorn.ps1`) - unrelated pre-existing condition, not caused
      by the split.
- [x] Committed locally on `strip-mod` (`8b29638`). **Not yet pushed/merged** - holding off until
      Phase B confirms the submodule wiring works end-to-end, in case anything here needs to
      change.

## Phase B — Strip `smc64` repo down to mod-only + wire submodule — ✅ DONE

- [x] On the `mod-repo-split` branch:
- [x] `git rm -r spark/ spark-launcher/ spark-dll-test/ spark-unicorn-test/` and
      `git rm -r vendor/Detours vendor/unicorn vendor/minhook vendor/zydis vendor/imgui`
      (`vendor/asmjit` was already gone). Kept `vendor/libsm64`.
- [x] `git submodule add https://github.com/KodyJKing/spark.git vendor/spark` (NOT root `spark/` -
      corrected mid-execution; nested submodules initialized via
      `git submodule update --init --recursive vendor/spark`).
- [x] Updated `smc64/premake5.lua` per the (revised) path rewrites above.
- [x] Rewrote top-level `premake5.lua`: declares `workspace "spark"` itself and includes the
      individual project scripts from inside the submodule (see design decision above for why
      reusing the submodule's whole top-level script doesn't work).
- [x] Updated scripts that assumed in-repo `spark/` paths:
  - `run_launcher.ps1`, `build.ps1`, `kill_injected_instances.ps1`, `watch.ps1`,
    `watch_launcher.ps1` - confirmed no path assumptions needed changes (still reference
    `bin/$Config-Win64/...` and `spark.sln`, both unchanged).
  - `copy_mods.ps1` was already present in the smc64 repo (never removed here) - only its doc
    comment referencing `spark/src/...` was updated to `vendor/spark/spark/src/...`.
  - `package.ps1`: `spark\shipfiles\*` → `vendor\spark\spark\shipfiles\*`;
    `bin\$Config-Win64\spark\spark.dll` → `vendor\spark\bin\$Config-Win64\spark\spark.dll`;
    setdll.exe path → `..\..\..\vendor\spark\vendor\Detours\bin.X64\setdll.exe`.
  - `build_setdll.bat`: `cd vendor\Detours\samples\setdll` →
    `cd vendor\spark\vendor\Detours\samples\setdll` (and matching extra `cd ..` at the end).
- [x] Updated `README.md`: clone instructions now mention `vendor/spark`; added
      `git submodule update --init --recursive` note for existing clones; fixed stale
      `SMC64_REAMDE.md` reference (that file no longer exists - shipfiles now ship a generic
      `README.md` from the spark submodule).
- [x] Full rebuild (`premake5.exe vs2022` + `MSBuild spark.sln /t:Rebuild`) - all 5 projects
      (`MinHook`, `Zydis`, `spark`, `smc64`, `spark-launcher`) build and link with 0 errors.
      Needed one fix iteration: initially dropped the `vendor/minhook/include` includedir from
      `smc64/premake5.lua` (assumed unused per grep), which broke with C1083 `MinHook.h` not
      found - had to restore it (rewritten to the `vendor/spark/...` path) since spark's own
      `Hook.hpp` header needs it transitively. See design-decision section above.
- [ ] Commit on `mod-repo-split`, verify, merge to `main` - not yet done (pending user
      confirmation before pushing).
- Note: `build_setdll.bat`'s new path resolves correctly, but actually building `setdll.exe` in
      this fresh `vendor/spark/vendor/Detours` checkout hit a separate, pre-existing Detours
      build-order issue (`NMAKE fatal error U1073: don't know how to make '..\..\lib.X64\syelog.lib'`)
      unrelated to the repo split - not investigated further here.

## Phase C — Multi-mod directory support in `ModLoader`

- [ ] `spark/src/spark/mod/ModLoader.cpp`: in addition to `Utils::getModsDirectory()`
      (`<exe-dir>/mods/`), read `SPARK_MODS_PATH` env var (semicolon-delimited, like `PATH`) and
      scan each listed directory the same way, skipping ones that don't exist. Keep dedup by
      resolved absolute path in case the same dir is listed twice or overlaps the default.
- [ ] Document `SPARK_MODS_PATH` in `spark`'s README as an option for pointing at a repo's own
      `bin/<Config>-Win64/<mod>/` output directly during dev, instead of requiring a copy step -
      though `copy_mods.ps1`-style staging remains the default/recommended path for parity with
      the current workflow.

## Phase D — Verify dev workflow end-to-end

- [ ] From a fresh clone of `smc64` (recursive), run `scripts/build.ps1` and confirm it builds
      `spark.dll`, `spark-launcher.exe`, and `smc64.dll` with 0 errors.
- [ ] Run `scripts/watch_launcher.ps1`, edit a file in `smc64/src/**`, confirm uninject → rebuild →
      reinject loop still works.
- [ ] Edit a file in `spark/src/**` (inside the submodule) with the same watcher running, confirm
      the change is picked up and triggers the same rebuild/reinject loop (this is the "on change
      to spark OR smc64" requirement from the goal).
- [ ] Confirm packaging still produces a working end-user install containing both `spark.dll`
      (+ xaudio2 detour) and `smc64.dll`. Since spark's own `package.ps1`/`install_package.ps1`
      are loader-only now (Phase A), `smc64` needs its own packaging step that builds on top of
      spark's package output (installs/copies it, then adds `smc64.dll` to `mods/`, re-zips) -
      this still needs to be designed/written in Phase B, it wasn't carried over automatically.

## Notes / risks

- This is a lot of destructive-looking git surgery (`git rm -r` across two repos, submodule
  add/remove). Each phase's `git rm`/submodule changes are easily reversible via git history, but
  I'll confirm with you before running the actual removal commands in each repo, rather than doing
  it all in one pass.
- Expect the premake wiring (nested `include`, path rewrites) to need at least one build-and-fix
  iteration, consistent with how the original DLL-split's ABI surface was discovered by iterating
  on linker errors rather than upfront auditing (see `/memories/repo/smc64-dll-split.md`).
