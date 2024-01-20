#ifndef NODEC_ANIMATION__SERIALIZATION__RESOURCES__ANIMATION_CLIP_HPP_
#define NODEC_ANIMATION__SERIALIZATION__RESOURCES__ANIMATION_CLIP_HPP_

#include <memory>

#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/unordered_map.hpp>
#include <nodec_scene_serialization/archive_context.hpp>
#include <nodec_scene_serialization/serializable_component.hpp>

#include <nodec_animation/resources/animation_clip.hpp>

#include "../animation_curve.hpp"

namespace nodec_animation {
namespace resources {

template<class Archive>
void serialize(Archive &archive, AnimatedProperty &property) {
    archive(cereal::make_nvp("curve", property.curve));
}

template<class Archive>
void serialize(Archive &archive, AnimatedComponent &component) {
    archive(cereal::make_nvp("properties", component.properties));
}

struct SerializableAnimatedComponentForSave {
    SerializableAnimatedComponentForSave(
        std::unique_ptr<nodec_scene_serialization::BaseSerializableComponent> &&placeholder,
        const std::unordered_map<std::string, AnimatedProperty> &ref_properties)
        : placeholder(std::move(placeholder)),
          ref_properties(ref_properties) {}

    std::unique_ptr<nodec_scene_serialization::BaseSerializableComponent> placeholder;
    const std::unordered_map<std::string, AnimatedProperty> &ref_properties;

    template<class Archive>
    void serialize(Archive &archive) {
        archive(cereal::make_nvp("placeholder", placeholder));
        archive(cereal::make_nvp("properties", ref_properties));
    }
};

struct SerializableAnimatedComponentForLoad {
    std::unique_ptr<nodec_scene_serialization::BaseSerializableComponent> placeholder;
    std::unordered_map<std::string, AnimatedProperty> properties;

    template<class Archive>
    void serialize(Archive &archive) {
        archive(cereal::make_nvp("placeholder", placeholder));
        archive(cereal::make_nvp("properties", properties));
    }
};

template<class Archive>
void save(Archive &archive, const AnimatedEntity &entity) {
    using namespace nodec_scene_serialization;
    ArchiveContext &context = cereal::get_user_data<ArchiveContext>(archive);
    auto &scene_serialization = context.scene_serialization();

    {
        std::vector<SerializableAnimatedComponentForSave> components;
        components.reserve(entity.components.size());
        for (const auto &component : entity.components) {
            components.emplace_back(scene_serialization.make_serializable_component(component.first),
                                    component.second.properties);
        }
        archive(cereal::make_nvp("components", components));
    }
    archive(cereal::make_nvp("children", entity.children));
}

template<class Archive>
void load(Archive &archive, AnimatedEntity &entity) {
    using namespace nodec_scene_serialization;
    ArchiveContext &context = cereal::get_user_data<ArchiveContext>(archive);
    auto &scene_serialization = context.scene_serialization();

    {
        std::vector<SerializableAnimatedComponentForLoad> components;
        archive(cereal::make_nvp("components", components));
        for (auto &component : components) {
            const auto type_info = scene_serialization.get_component_type_info(component.placeholder->type_info());
            if (type_info == nodec::type_id<nullptr_t>()) continue;
            entity.components[type_info] = AnimatedComponent{
                std::move(component.properties)};
        }
    }
    archive(cereal::make_nvp("children", entity.children));
}

template<class Archive>
void save(Archive &archive, const AnimationClip &clip) {
    archive(cereal::make_nvp("root_entity", clip.root_entity()));
}

template<class Archive>
void load(Archive &archive, AnimationClip &clip) {
    AnimatedEntity root_entity;
    archive(cereal::make_nvp("root_entity", root_entity));
    clip.set_root_entity(std::move(root_entity));
}

} // namespace resources
} // namespace nodec_animation

#endif