#ifndef NODEC_ANIMATION__SYSTEMS__ANIMATOR_SYSTEM_HPP_
#define NODEC_ANIMATION__SYSTEMS__ANIMATOR_SYSTEM_HPP_

#include <nodec_scene/scene_registry.hpp>
#include <nodec_scene_serialization/scene_serialization.hpp>

#include "../component_registry.hpp"
#include "../components/animator.hpp"
#include "../components/impl/animated_data.hpp"
#include "../components/impl/animator_activity.hpp"

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
            auto view = registry.view<Animator, AnimatorStart>();

            view.each([&](SceneEntity entity, Animator &animator, AnimatorStart &) {
                auto animator_activity_result = registry.emplace_component<AnimatorActivity>(entity);
                auto &animator_activity = animator_activity_result.first;
                auto animator_activity_created = animator_activity_result.second;

                if (animator_activity_created) {
                    // At first time to create animator activity.
                    bind(animator, registry, entity, animator_activity);
                    return;
                }

                if (animator_activity.clip != animator.clip) {
                    // Previously created animator activity is not matched with the new animator.
                    // So, we need to clear the previous AnimatedData and rebind.

                    registry.remove_component<AnimatedData>(animator_activity.animated_entities.begin(),
                                                            animator_activity.animated_entities.end());
                    animator_activity.animated_entities.clear();
                    bind(animator, registry, entity, animator_activity);

                    return;
                }

                // The animator activity is already created and matched with the new animator.
                // So, we don't need to rebind, but we need to reset the animation time.
                reset_animation_time(registry, animator_activity);
            });

            registry.remove_component<AnimatorStart>(view.begin(), view.end());
        }
        {
            auto view = registry.view<Animator, AnimatorStop>();

            view.each([&](SceneEntity entity, Animator &animator, AnimatorStop &) {
                auto animator_activity = registry.try_get_component<AnimatorActivity>(entity);
                if (!animator_activity) return;

                registry.remove_component<AnimatedData>(animator_activity->animated_entities.begin(),
                                                        animator_activity->animated_entities.end());
                registry.remove_component<AnimatorActivity>(entity);
            });

            registry.remove_component<AnimatorStop>(view.begin(), view.end());
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
    void reset_animation_time(nodec_scene::SceneRegistry &registry,
                              components::impl::AnimatorActivity &animator_activity) {
        using namespace nodec_scene;
        using namespace components;
        using namespace components::impl;

        for (auto &entity : animator_activity.animated_entities) {
            auto animated_data = registry.try_get_component<AnimatedData>(entity);
            if (!animated_data) continue;
            animated_data->time = 0.f;
        }
    }

    void bind(components::Animator &animator, nodec_scene::SceneRegistry &registry, const nodec_scene::SceneEntity &entity,
              components::impl::AnimatorActivity &animator_activity) {
        if (!animator.clip) return;

        bind_each(registry, entity, animator.clip->root_entity(), animator.clip, animator_activity);
    }

    void bind_each(nodec_scene::SceneRegistry &registry, const nodec_scene::SceneEntity &entity, const resources::AnimatedEntity &animated_entity,
                   std::shared_ptr<resources::AnimationClip> &clip,
                   components::impl::AnimatorActivity &animator_activity) {
        using namespace nodec::entities;
        using namespace nodec_scene::components;
        using namespace nodec_animation::components::impl;

        {
            auto &animated_data = registry.emplace_component<AnimatedData>(entity).first;
            animated_data.reset(clip, &animated_entity);
        }

        animator_activity.animated_entities.push_back(entity);

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

            bind_each(registry, child_entity, iter->second, clip, animator_activity);

            child_entity = child_hierarchy.next;
        }
    }

private:
    ComponentRegistry &component_registry_;
};
} // namespace systems
} // namespace nodec_animation
#endif