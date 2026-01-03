#ifndef NODEC_ANIMATION__RESOURCES__ANIMATION_CLIP_HPP_
#define NODEC_ANIMATION__RESOURCES__ANIMATION_CLIP_HPP_

#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>

#include <nodec/ranges.hpp>
#include <nodec/type_info.hpp>

#include "../animation_curve.hpp"

namespace nodec_animation {
namespace resources {

struct AnimatedProperty {
    AnimationCurve curve;
};

struct AnimatedComponent {
    // Instead of using a map, I think it would be better to use an index-based approach,
    // and have a map to retrieve the index from a string.
    std::unordered_map<std::string, AnimatedProperty> properties;
};

struct AnimatedEntity {
    // Using std::map instead of std::unordered_map for pointer stability.
    // std::map guarantees that pointers/references to elements remain valid
    // after insert/erase operations (node-based container), which allows
    // AnimatedData to safely cache pointers to AnimatedEntity nodes.
    std::map<std::string, AnimatedEntity> children;
    std::unordered_map<nodec::type_info, AnimatedComponent> components;
};

class AnimationClip {
public:
    AnimationClip() {
    }

    template<class Component>
    void set_curve(const std::string &relative_path, const std::string &property_name, const AnimationCurve &curve) {
        auto split_string = [](const std::string &s, const char *delim) {
            // https://gist.github.com/ScottHutchinson/6b699c997a33c33130821922c11d25c3
            std::vector<std::string> elems;
            size_t start{};
            size_t end{};

            do {
                end = s.find_first_of(delim, start);
                elems.emplace_back(s.substr(start, end - start));
                start = end + 1;
            } while (end != std::string::npos);
            return elems;
        };

        auto &entity = [&]() -> AnimatedEntity & {
            const auto parts = split_string(relative_path, "/");
            if (parts[0].empty()) return root_entity_;

            AnimatedEntity *current = &root_entity_;

            for (const auto &part : parts) {
                current = &(current->children[part]);
            }
            return *current;
        }();

        if (property_name.empty()) return;

        entity.components[nodec::type_id<Component>()].properties[property_name].curve = curve;
        ++version_;
    }

    const AnimatedEntity &root_entity() const {
        return root_entity_;
    }

    void set_root_entity(AnimatedEntity &&entity) {
        root_entity_ = std::move(entity);
        ++version_;
    }

    std::uint32_t version() const noexcept {
        return version_;
    }

private:
    AnimatedEntity root_entity_;
    std::uint32_t version_{0};
};

} // namespace resources
} // namespace nodec_animation

#endif