#ifndef NODEC_ANIMATION__SERIALIZATION__COMPONENTS__ANIMATOR_HPP_
#define NODEC_ANIMATION__SERIALIZATION__COMPONENTS__ANIMATOR_HPP_

#include <nodec_scene_serialization/serializable_component.hpp>
#include <nodec_scene_serialization/archive_context.hpp>

#include <nodec_animation/components/animator.hpp>

namespace nodec_animation {
namespace components {

struct SerializableAnimator : nodec_scene_serialization::BaseSerializableComponent {
    SerializableAnimator()
        : BaseSerializableComponent(this) {
    }
    SerializableAnimator(const Animator &animator)
        : BaseSerializableComponent(this), clip(animator.clip) {
    }

    operator Animator() const noexcept {
        Animator value;
        value.clip = clip;
        return value;
    }

    std::shared_ptr<resources::AnimationClip> clip;

    template<class Archive>
    void save(Archive &archive) const {
        using namespace nodec_scene_serialization;
        ArchiveContext &context = cereal::get_user_data<ArchiveContext>(archive);

        archive(cereal::make_nvp("clip", context.resource_registry().lookup_name<resources::AnimationClip>(clip).first));
    }

    template<class Archive>
    void load(Archive &archive) {
        using namespace nodec_scene_serialization;
        using namespace nodec::resource_management;

        auto &context = cereal::get_user_data<ArchiveContext>(archive);

        {
            std::string name;
            archive(cereal::make_nvp("clip", name));
            clip = context.resource_registry().get_resource_direct<resources::AnimationClip>(name);
        }
    }
};

struct SerializableAnimatorStart : nodec_scene_serialization::BaseSerializableComponent {
    SerializableAnimatorStart()
        : BaseSerializableComponent(this) {
    }
    SerializableAnimatorStart(const AnimatorStart &)
        : BaseSerializableComponent(this) {
    }
    operator AnimatorStart() const noexcept {
        return AnimatorStart();
    }

    template<class Archive>
    void serialize(Archive &) {
    }
};

struct SerializableAnimatorStop : nodec_scene_serialization::BaseSerializableComponent {
    SerializableAnimatorStop()
        : BaseSerializableComponent(this) {
    }
    SerializableAnimatorStop(const AnimatorStop &)
        : BaseSerializableComponent(this) {
    }
    operator AnimatorStop() const noexcept {
        return AnimatorStop();
    }

    template<class Archive>
    void serialize(Archive &) {
    }
};

} // namespace components
} // namespace nodec_animation

NODEC_SCENE_REGISTER_SERIALIZABLE_COMPONENT(nodec_animation::components::SerializableAnimator)
NODEC_SCENE_REGISTER_SERIALIZABLE_COMPONENT(nodec_animation::components::SerializableAnimatorStart)
NODEC_SCENE_REGISTER_SERIALIZABLE_COMPONENT(nodec_animation::components::SerializableAnimatorStop)

#endif