#ifndef NODEC_ANIMATION__KEYFRAME_HPP_
#define NODEC_ANIMATION__KEYFRAME_HPP_

#include <cstdint>

namespace nodec_animation {

struct Keyframe {
    float time;
    float value;

    constexpr bool operator<(const Keyframe &other) const noexcept {
        return time < other.time;
    }

    constexpr bool operator>(const Keyframe &other) const noexcept {
        return time > other.time;
    }
};
} // namespace nodec_animation

#endif