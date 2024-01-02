#ifndef NODEC_ANIMATION__COMPONENT_REGISTRY_HPP_
#define NODEC_ANIMATION__COMPONENT_REGISTRY_HPP_

#include <memory>

#include <nodec/type_info.hpp>
#include <nodec_scene/scene_registry.hpp>

#include "animated_component_writer.hpp"
#include "resources/animation_clip.hpp"

namespace nodec_animation {

class ComponentRegistry {
public:
    class BaseAnimationHandler {
    public:
        virtual ~BaseAnimationHandler() {}

        virtual void write_properties(nodec_scene::SceneRegistry &registry,
                                      const nodec_scene::SceneEntity &entity,
                                      const nodec_animation::resources::AnimatedComponent &source,
                                      std::uint16_t ticks,
                                      AnimatedComponentWriter::ComponentAnimationState *state = nullptr) const = 0;
    };

    template<class Component>
    class AnimationHandler : public BaseAnimationHandler {
    public:
        void write_properties(nodec_scene::SceneRegistry &registry,
                              const nodec_scene::SceneEntity &entity,
                              const nodec_animation::resources::AnimatedComponent &source,
                              std::uint16_t ticks,
                              AnimatedComponentWriter::ComponentAnimationState *state = nullptr) const override {
            auto *component = registry.try_get_component<Component>(entity);
            if (!component) return;

            AnimatedComponentWriter writer;
            writer.write(source, ticks, *component, state);
        }
    };

public:
    template<class Component>
    void register_component() {
        if (handlers_.find(nodec::type_id<Component>()) != handlers_.end()) {
            return;
        }
        handlers_.emplace(nodec::type_id<Component>(), std::make_unique<AnimationHandler<Component>>());
    }

    BaseAnimationHandler *get_handler(const nodec::type_info &type_info) const {
        auto iter = handlers_.find(type_info);
        if (iter == handlers_.end()) {
            return nullptr;
        }
        return iter->second.get();
    }

private:
    std::unordered_map<nodec::type_info, std::unique_ptr<BaseAnimationHandler>> handlers_;
};

} // namespace nodec_animation

#endif