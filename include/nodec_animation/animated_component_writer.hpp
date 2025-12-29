#ifndef NODEC_ANIMATION__ANIMATED_COMPONENT_WRITER_HPP_
#define NODEC_ANIMATION__ANIMATED_COMPONENT_WRITER_HPP_

#include <cereal/cereal.hpp>

#include <nodec_animation/resources/animation_clip.hpp>

namespace nodec_animation {

/**
 * @brief The AnimatedComponentWriter class provides the functionality to write animated component properties to a serializable component.
 *
 */
class AnimatedComponentWriter {
    struct InternalTag {};

public:
    struct PropertyAnimationState {
        int current_index;
    };

    struct ComponentAnimationState {
        std::unordered_map<std::string, PropertyAnimationState> properties;
    };

    /**
     * @brief Custom cereal InputArchive for applying animation curve values to component properties.
     *
     * Unlike standard cereal archives (JSONInputArchive, BinaryInputArchive, etc.),
     * this class can be reused via reset() because it does NOT use cereal's internal state:
     *   - itsPolymorphicTypeMap: Not used (no polymorphic types deserialized)
     *   - itsSharedPointerMap: Not used (shared_ptr is ignored, see CEREAL_LOAD_FUNCTION_NAME overloads)
     *   - itsVersionedTypes: Not used (no versioning)
     *
     * Standard cereal archives accumulate these states during deserialization,
     * causing ID collisions and invalid references on reuse.
     * PropertyWriter only maintains traversal state (name_stack_, current_property_name_)
     * which is cleared in reset().
     */
    class PropertyWriter : public cereal::InputArchive<PropertyWriter> {
    public:
        // Constructor for reusable instance (call reset() before each use)
        PropertyWriter(AnimatedComponentWriter &owner, InternalTag)
            : InputArchive(this),
              source_(nullptr), time_(0), owner_(owner), state_(nullptr) {}

        // Reset state for reuse with new parameters
        void reset(const nodec_animation::resources::AnimatedComponent &source,
                   float time,
                   ComponentAnimationState *state) {
            source_ = &source;
            time_ = time;
            state_ = state;
            name_stack_.clear();
            current_property_name_.clear();
        }

        void load_value(std::string &) {
            // Ignore.
        }

        template<class T, cereal::traits::EnableIf<std::is_arithmetic<T>::value> = cereal::traits::sfinae>
        void load_value(T &value) {
            if (!source_) return;

            auto iter = source_->properties.find(current_property_name_);
            if (iter == source_->properties.end()) return;

            auto *property_animation_state = [&]() -> PropertyAnimationState * {
                if (!state_) return nullptr;
                return &state_->properties[current_property_name_];
            }();

            const auto &property = iter->second;
            const auto &curve = property.curve;
            auto sample = curve.evaluate(time_,
                                         property_animation_state
                                             ? property_animation_state->current_index
                                             : -1);

            value = static_cast<T>(sample.second);
            if (property_animation_state) {
                property_animation_state->current_index = sample.first;
            }
        }

        void start_node(const char *name) {
            name_stack_.emplace_back(name);
            if (name == nullptr) return;

            if (current_property_name_.size() > 0) {
                current_property_name_ += ".";
            }
            current_property_name_ += name;
        }

        void end_node() {
            auto last = name_stack_.back();
            name_stack_.pop_back();

            if (last == nullptr) return;

            current_property_name_.erase(current_property_name_.size() - std::strlen(last));
            if (current_property_name_.size() > 0) {
                current_property_name_.erase(current_property_name_.size() - 1);
            }
        }

    private:
        const nodec_animation::resources::AnimatedComponent *source_;
        float time_;
        AnimatedComponentWriter &owner_;
        std::vector<const char *> name_stack_;
        std::string current_property_name_;
        ComponentAnimationState *state_;
    };

    AnimatedComponentWriter()
        : writer_(*this, InternalTag{}) {
    }

    /**
     * @brief Writes properties of source on the specific time to the dest.
     *
     * @param source
     * @param dest
     */
    template<typename Component>
    void write(const nodec_animation::resources::AnimatedComponent &source,
               float time,
               Component &dest, ComponentAnimationState *state = nullptr) {
        writer_.reset(source, time, state);
        writer_(dest);
    }

private:
    PropertyWriter writer_;
};

// --- PropertyWriter ---

template<class T>
inline void prologue(AnimatedComponentWriter::PropertyWriter &ar,
                     const cereal::NameValuePair<T> &pair) {
    ar.start_node(pair.name);
}

template<class T>
inline void prologue(AnimatedComponentWriter::PropertyWriter &ar,
                     const T &value) {
    ar.start_node(nullptr);
}

template<class T>
inline void epilogue(AnimatedComponentWriter::PropertyWriter &ar,
                     const T &value) {
    ar.end_node();
}

template<class T>
inline void CEREAL_LOAD_FUNCTION_NAME(AnimatedComponentWriter::PropertyWriter &ar,
                                      cereal::NameValuePair<T> &pair) {
    ar(pair.value);
}

template<class T, cereal::traits::EnableIf<std::is_arithmetic<T>::value> = cereal::traits::sfinae>
inline void CEREAL_LOAD_FUNCTION_NAME(AnimatedComponentWriter::PropertyWriter &ar,
                                      T &value) {
    ar.load_value(value);
}

template<class CharT, class Traits, class Alloc>
inline void CEREAL_LOAD_FUNCTION_NAME(AnimatedComponentWriter::PropertyWriter &ar,
                                      std::basic_string<CharT, Traits, Alloc> &value) {
    ar.load_value(value);
}

template<class T>
inline void CEREAL_LOAD_FUNCTION_NAME(AnimatedComponentWriter::PropertyWriter &, std::shared_ptr<T> &) {
    // Ignore.
}

template<class T>
inline void CEREAL_LOAD_FUNCTION_NAME(AnimatedComponentWriter::PropertyWriter &, std::unique_ptr<T> &) {
    // Ignore.
}

} // namespace nodec_animation

CEREAL_REGISTER_ARCHIVE(nodec_animation::AnimatedComponentWriter::PropertyWriter)

#endif