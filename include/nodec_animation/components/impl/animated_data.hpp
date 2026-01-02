#ifndef NODEC_ANIMATION__COMPONENTS__IMPL__ANIMATED_DATA_HPP_
#define NODEC_ANIMATION__COMPONENTS__IMPL__ANIMATED_DATA_HPP_

#include "../../animated_component_writer.hpp"
#include "../../resources/animation_clip.hpp"

namespace nodec_animation {
namespace components {
namespace impl {

struct AnimatedData {
    void reset(std::shared_ptr<resources::AnimationClip> clip,
               const resources::AnimatedEntity *animated_entity) {
        clip_ = clip;
        animated_entity_ = animated_entity;
        cached_clip_version_ = clip ? clip->version() : 0;
    }

    std::shared_ptr<resources::AnimationClip> clip() const noexcept {
        return clip_;
    }

    /// Returns the cached animated entity pointer, or nullptr if invalid.
    /// The pointer becomes invalid when the clip's version has changed
    /// (e.g., when the clip is updated via the editor API).
    const resources::AnimatedEntity *animated_entity() const noexcept {
        if (!is_valid()) {
            return nullptr;
        }
        return animated_entity_;
    }

    /// Check if the cached animated entity pointer is still valid.
    bool is_valid() const noexcept {
        return clip_ && clip_->version() == cached_clip_version_ && animated_entity_ != nullptr;
    }

    std::unordered_map<nodec::type_info, AnimatedComponentWriter::ComponentAnimationState>
        component_animation_states;

    float time{0.f};

private:
    std::shared_ptr<resources::AnimationClip> clip_;
    const resources::AnimatedEntity *animated_entity_{nullptr};
    std::uint32_t cached_clip_version_{0};
};
} // namespace impl
} // namespace components
} // namespace nodec_animation
#endif