#include "MarioLevelEdit.hpp"
#include "level-edits/index.hpp"
#include "EditorState.hpp"         // also defines ENABLE_LEVEL_EDITOR

#ifdef ENABLE_LEVEL_EDITOR
#include "LevelEditWindows.hpp"
#include "spark/RenderBuses.hpp"
#include "spark/hook/Hooks.hpp"
#include "spark/mod/ImGuiBridge.hpp"
#endif

namespace Mod::Mario::LevelEdit {

// ── Data layer (always compiled) ──────────────────────────────────────────────

LevelEdits* getLevelEdits(LevelEditContext& context) {
    return Index::lookup(context.bspSignature);
}

// ── Editor state ──────────────────────────────────────────────────────────────

#ifdef ENABLE_LEVEL_EDITOR

static EditorState s_state;

#endif

// ── Public API ────────────────────────────────────────────────────────────────

void setContext(LevelEditContext context) {
#ifdef ENABLE_LEVEL_EDITOR
    s_state.currentContext = std::move(context);
    s_state.currentEdits   = Index::lookup(s_state.currentContext.bspSignature);
    s_state.selectedIdx    = -1;
#else
    (void)context;
#endif
}

void initHandlers(Spark::ModId modId) {
#ifdef ENABLE_LEVEL_EDITOR
    s_state.currentEdits = Index::lookup(s_state.currentContext.bspSignature);

    using Bus = Spark::EventBus<void>;
    Spark::RenderBSPAlbedo::addHandler(modId, +[](void* ctx, Spark::RenderBSPAlbedo::Cursor next) {
        next();
        Spark::Mod::syncImGuiContext();
        renderWorld(*static_cast<EditorState*>(ctx));
    }, &s_state);
    Spark::onRenderDebugUI.addHandler(modId, +[](void* ctx, Bus::Cursor next) {
        Spark::Mod::syncImGuiContext();
        renderUI(*static_cast<EditorState*>(ctx));
        next();
    }, &s_state);
    Spark::UpdatePlayerControlsAndLook::addHandler(modId, +[](void* ctx, auto next, float param_1, uint32_t param_2) {
        auto& state = *static_cast<EditorState*>(ctx);
        if (!(state.editorOpen && state.editorInputEnabled)) {
            next(param_1, param_2);
        }
    }, &s_state, 10);
#else
    (void)modId;
#endif
}

bool isInputSuppressed() {
#ifdef ENABLE_LEVEL_EDITOR
    return s_state.editorOpen && s_state.editorInputEnabled;
#else
    return false;
#endif
}

} // namespace Mod::Mario::LevelEdit
