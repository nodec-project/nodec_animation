#ifndef NODEC_ANIMATION__SERIALIZATION__KEYFRAME_HPP_
#define NODEC_ANIMATION__SERIALIZATION__KEYFRAME_HPP_

#include <cereal/cereal.hpp>

#include <nodec_animation/keyframe.hpp>

namespace nodec_animation {

template<class Archive>
void serialize(Archive &archive, Keyframe &keyframe) {
    archive(cereal::make_nvp("time", keyframe.time));
    archive(cereal::make_nvp("value", keyframe.value));
}

} // namespace nodec_animation

#endif