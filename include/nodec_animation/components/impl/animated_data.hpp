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
    }

    std::shared_ptr<resources::AnimationClip> clip() const noexcept {
        return clip_;
    }

    const resources::AnimatedEntity *animated_entity() const noexcept {
        return animated_entity_;
    }

    std::unordered_map<nodec::type_info, AnimatedComponentWriter::ComponentAnimationState>
        component_animation_states;

    std::uint16_t current_ticks{0};

private:
    std::shared_ptr<resources::AnimationClip> clip_;
    const resources::AnimatedEntity *animated_entity_{nullptr};
};
} // namespace impl
} // namespace components
} // namespace nodec_animation
#endif