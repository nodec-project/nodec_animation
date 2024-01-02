#ifndef NODEC_ANIMATION__COMPONENTS__ANIMATOR_HPP_
#define NODEC_ANIMATION__COMPONENTS__ANIMATOR_HPP_

#include <memory>

#include "../resources/animation_clip.hpp"

namespace nodec_animation {
namespace components {

struct Animator {
    std::shared_ptr<resources::AnimationClip> clip;

};

struct AnimatorStart {};

struct AnimatorStop {};

}
} // namespace nodec_animation

#endif