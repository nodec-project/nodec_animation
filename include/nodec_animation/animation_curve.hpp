#ifndef NODEC_ANIMATION__ANIMATION_CURVE_HPP_
#define NODEC_ANIMATION__ANIMATION_CURVE_HPP_

#include "keyframe.hpp"
#include "wrap_mode.hpp"

#include <nodec/algorithm.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <vector>

namespace nodec_animation {

class AnimationCurve {
public:
    using iterator = std::vector<Keyframe>::const_iterator;

    AnimationCurve() {}

    AnimationCurve(const AnimationCurve &other)
        : keyframes_(other.keyframes_),
          wrap_mode_(other.wrap_mode_) {
    }

    AnimationCurve &operator=(const AnimationCurve &other) {
        keyframes_ = other.keyframes_;
        wrap_mode_ = other.wrap_mode_;
        return *this;
    }

    AnimationCurve(AnimationCurve &&other) noexcept
        : keyframes_(std::move(other.keyframes_)),
          wrap_mode_(other.wrap_mode_) {
    }

    AnimationCurve &operator=(AnimationCurve &&other) noexcept {
        keyframes_ = std::move(other.keyframes_);
        wrap_mode_ = other.wrap_mode_;
        return *this;
    }

    const std::vector<Keyframe> &keyframes() const {
        return keyframes_;
    }

    void set_keyframes(std::vector<Keyframe> &&keyframes) {
        keyframes_ = std::move(keyframes);
    }

    void set_wrap_mode(const WrapMode &mode) {
        wrap_mode_ = mode;
    }

    WrapMode wrap_mode() const {
        return wrap_mode_;
    }

    int add_keyframe(const Keyframe &keyframe) {
        auto iter = std::lower_bound(keyframes_.begin(), keyframes_.end(), keyframe);
        iter = keyframes_.insert(iter, keyframe);
        return static_cast<int>(std::distance(keyframes_.begin(), iter));
    }

    /**
     * @brief
     *
     * @param ticks
     * @param hint
     * @return std::pair<int, float>
     */
    std::pair<int, float> evaluate(float time, int hint = -1) const {
        if (keyframes_.size() == 0) return std::make_pair(-1, 0.0f);

        Keyframe current;
        current.time = [&]() {
            switch (wrap_mode_) {
            case WrapMode::Once:
            default:
                return nodec::clamp(time, 0.f, keyframes_.back().time);

            case WrapMode::Loop:
                return std::fmod(time, keyframes_.back().time);
            }
        }();

        assert(0 <= current.time && current.time <= keyframes_.back().time);

        auto iter = [&]() {
            // The hint index should be in the range [0, last - 1]
            //
            // |<--   hint   -->|
            // o   o    o       o   o
            //                      ^last
            if (hint < 0 || keyframes_.size() - 1 <= hint) {
                return std::upper_bound(keyframes_.begin(), keyframes_.end(), current);
            }

            assert(0 <= hint && hint < keyframes_.size() - 1);

            if (current.time < keyframes_[hint].time) {
                if (keyframes_[hint - 1].time <= current.time) {
                    return keyframes_.begin() + hint;
                }

                return std::upper_bound(keyframes_.begin(), keyframes_.begin() + hint - 1, current);
            }

            if (current.time < keyframes_[hint + 1].time) {
                return keyframes_.begin() + hint + 1;
            }

            if (keyframes_.size() <= hint + 2) {
                return keyframes_.end();
            }

            if (current.time < keyframes_[hint + 2].time) {
                return keyframes_.begin() + hint + 2;
            }

            return std::upper_bound(keyframes_.begin() + hint + 2, keyframes_.end(), current);
        }();

        assert(keyframes_.size() > 0);

        // o  x         o
        //    ^current  ^iter
        const int index = static_cast<int>(std::distance(keyframes_.begin(), iter));

        if (iter == keyframes_.end()) return {index - 1, std::prev(iter)->value};

        if (iter == keyframes_.begin()) return {index, iter->value};

        auto prev = std::prev(iter);
        float value = prev->value + (iter->value - prev->value) / (iter->time - prev->time) * (current.time - prev->time);

        return {index - 1, value};
    }

private:
    std::vector<Keyframe> keyframes_;
    WrapMode wrap_mode_{WrapMode::Once};
};

} // namespace nodec_animation

#endif