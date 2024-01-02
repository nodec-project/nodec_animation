#ifndef NODEC_ANIMATION__SYSTEMS__ANIMATOR_SYSTEM_HPP_
#define NODEC_ANIMATION__SYSTEMS__ANIMATOR_SYSTEM_HPP_

#include <nodec_scene/scene_registry.hpp>
#include <nodec_scene_serialization/scene_serialization.hpp>

#include "../component_registry.hpp"
#include "../components/animator.hpp"
#include "../components/impl/animated_data.hpp"

namespace nodec_animation {
namespace systems {

class AnimatorSystem {
public:
    AnimatorSystem(ComponentRegistry &registry)
        : component_registry_(registry) {}

    void update(nodec_scene::SceneRegistry &registry, float delta_time) {
        using namespace nodec_scene;
        using namespace components;
        using namespace components::impl;

        {
            auto view = registry.view<Animator, AnimatorBegin>();

            view.each([&](SceneEntity entity, Animator &animator, AnimatorBegin &) {
                bind(animator, registry, entity);
            });

            registry.remove_component<AnimatorBegin>(view.begin(), view.end());
        }

        registry.view<AnimatedData>().each([&](SceneEntity entity, AnimatedData &animated_data) {
            for (auto &component : animated_data.animated_entity()->components) {
                auto &animated_component = component.second;
                auto &type_info = component.first;

                auto *handler = component_registry_.get_handler(type_info);
                if (!handler) continue;

                auto &state = animated_data.component_animation_states[type_info];

                handler->write_properties(registry, entity, animated_component, animated_data.time,
                                          &state);
                animated_data.time += delta_time;
            }
        });
    }

private:
    void bind(components::Animator &animator, nodec_scene::SceneRegistry &registry, const nodec_scene::SceneEntity &entity) {
        if (!animator.clip) return;

        bind_each(registry, entity, animator.clip->root_entity(), animator.clip);
    }

    void bind_each(nodec_scene::SceneRegistry &registry, const nodec_scene::SceneEntity &entity, const resources::AnimatedEntity &animated_entity,
                   std::shared_ptr<resources::AnimationClip> &clip) {
        using namespace nodec::entities;
        using namespace nodec_scene::components;
        using namespace nodec_animation::components::impl;

        {
            auto &animated_data = registry.emplace_component<AnimatedData>(entity).first;
            animated_data.reset(clip, &animated_entity);
        }

        auto &hierarchy = registry.emplace_component<Hierarchy>(entity).first;

        auto child_entity = hierarchy.first;
        while (child_entity != null_entity) {
            auto &child_hierarchy = registry.emplace_component<Hierarchy>(child_entity).first;
            auto child_name = registry.try_get_component<Name>(child_entity);
            if (!child_name) {
                child_entity = child_hierarchy.next;
                continue;
            }

            auto iter = animated_entity.children.find(child_name->value);
            if (iter == animated_entity.children.end()) {
                child_entity = child_hierarchy.next;
                continue;
            }

            bind_each(registry, child_entity, iter->second, clip);

            child_entity = child_hierarchy.next;
        }
    }

private:
    ComponentRegistry &component_registry_;
};
} // namespace systems
} // namespace nodec_animation
#endif