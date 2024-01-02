#ifndef NODEC_ANIMATION__COMPONENTS__IMPL__ANIMATOR_ACTIVITY_HPP_
#define NODEC_ANIMATION__COMPONENTS__IMPL__ANIMATOR_ACTIVITY_HPP_

#include <memory>

#include <nodec_scene/scene_entity.hpp>

#include "../../resources/animation_clip.hpp"

namespace nodec_animation {
namespace components {
namespace impl {

struct AnimatorActivity {
    std::shared_ptr<resources::AnimationClip> clip;
    std::vector<nodec_scene::SceneEntity> animated_entities;
};

} // namespace impl
} // namespace components
} // namespace nodec_animation
#endif