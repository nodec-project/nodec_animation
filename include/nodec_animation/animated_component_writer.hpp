#ifndef NODEC_ANIMATION__ANIMATED_COMPONENT_WRITER_HPP_
#define NODEC_ANIMATION__ANIMATED_COMPONENT_WRITER_HPP_

#include <nodec_animation/resources/animation_clip.hpp>
#include <nodec_scene_serialization/scene_serialization.hpp>

namespace nodec_animation {

/**
 * @brief The AnimatedComponentWriter class provides the functionality to write animated component properties to a serializable component.
 *
 */
class AnimatedComponentWriter {
    struct InternalTag {};

public:
    class PropertyWriter : public cereal::InputArchive<PropertyWriter> {
    public:
        PropertyWriter(const nodec_animation::resources::AnimatedComponent &source,
                       std::uint16_t ticks,
                       AnimatedComponentWriter &owner, InternalTag)
            : InputArchive(this),
              source_(source), ticks_(ticks), owner_(owner) {}

        void load_value(std::string &value) {
            // Ignore.
        }

        template<class T, cereal::traits::EnableIf<std::is_arithmetic<T>::value> = cereal::traits::sfinae>
        void load_value(T &value) {
            auto iter = source_.properties.find(current_property_name_);
            if (iter == source_.properties.end()) return;

            const auto &property = iter->second;
            const auto &curve = property.curve;
            auto sample = curve.evaluate(ticks_);

            value = static_cast<T>(sample.second);
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
        const nodec_animation::resources::AnimatedComponent &source_;
        const std::uint16_t ticks_;
        AnimatedComponentWriter &owner_;
        std::vector<const char *> name_stack_;
        std::string current_property_name_;
    };

    AnimatedComponentWriter() {
    }

    /**
     * @brief Writes properties of source on the specific time to the dest.
     *
     * @param source
     * @param dest
     */
    template<typename Component>
    void write(const nodec_animation::resources::AnimatedComponent &source,
               std::uint16_t ticks,
               Component &dest) {
        // TODO: pass the context of previous frame.

        PropertyWriter writer(source, ticks, *this, InternalTag{});

        writer(dest);
    }
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
inline void CEREAL_LOAD_FUNCTION_NAME(AnimatedComponentWriter::PropertyWriter &ar, std::shared_ptr<T> &) {
    // Ignore.
}

template<class T>
inline void CEREAL_LOAD_FUNCTION_NAME(AnimatedComponentWriter::PropertyWriter &ar, std::unique_ptr<T> &) {
    // Ignore.
}

} // namespace nodec_animation

CEREAL_REGISTER_ARCHIVE(nodec_animation::AnimatedComponentWriter::PropertyWriter)

#endif