#ifndef NODEC_ANIMATION__SERIALIZATION__ANIMATION_CURVE_HPP_
#define NODEC_ANIMATION__SERIALIZATION__ANIMATION_CURVE_HPP_

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

#include <nodec_animation/animation_curve.hpp>

#include "keyframe.hpp"

namespace nodec_animation {

template<class Archive>
void save(Archive &archive, const AnimationCurve &curve) {
    archive(cereal::make_nvp("wrap_mode", curve.wrap_mode()));
    archive(cereal::make_nvp("keyframes", curve.keyframes()));
}

template<class Archive>
void load(Archive &archive, AnimationCurve &curve) {
    std::vector<Keyframe> keyframes;
    archive(cereal::make_nvp("keyframes", keyframes));
    curve.set_keyframes(std::move(keyframes));

    WrapMode wrap_mode;
    archive(cereal::make_nvp("wrap_mode", wrap_mode));
    curve.set_wrap_mode(wrap_mode);
}

} // namespace nodec_animation

#endif