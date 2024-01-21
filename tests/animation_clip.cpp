#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <algorithm>
#include <sstream>
#include <vector>

#include <cereal/archives/json.hpp>
#include <nodec/ranges.hpp>
#include <nodec_animation/resources/animation_clip.hpp>
#include <nodec_animation/serialization/resources/animation_clip.hpp>
#include <nodec_scene_serialization/archive_context.hpp>

struct ComponentA {
    int prop;
};
struct ComponentB {
    int prop;
};

TEST_CASE("Testing set_curve") {
    using namespace nodec_animation::resources;
    using namespace nodec_animation;

    SUBCASE("root path") {
        AnimationClip clip;
        clip.set_curve<ComponentA>("", "prop", AnimationCurve{});
        clip.set_curve<ComponentB>("/", "prop", AnimationCurve{});

        CHECK(clip.root_entity().children.size() == 0);
        CHECK(clip.root_entity().components.size() == 2);
    }

    SUBCASE("child paths") {
        AnimationClip clip;
        clip.set_curve<ComponentA>("a", "prop", AnimationCurve{});
        clip.set_curve<ComponentA>("b", "prop", AnimationCurve{});
        clip.set_curve<ComponentA>("a/a", "prop", AnimationCurve{});

        CHECK(clip.root_entity().children.size() == 2);
        CHECK(clip.root_entity().components.size() == 0);

        CHECK(clip.root_entity().children.at("a").children.at("a").components.size() == 1);
    }
}

struct SerializableComponentA : public nodec_scene_serialization::BaseSerializableComponent {
    int prop{0};

    SerializableComponentA()
        : BaseSerializableComponent(this) {}

    SerializableComponentA(const ComponentA &other)
        : BaseSerializableComponent(this),
          prop(other.prop) {}

    operator ComponentA() const noexcept {
        ComponentA value;
        value.prop = prop;
        return value;
    }

    template<class Archive>
    void serialize(Archive &archive) {
        archive(cereal::make_nvp("prop", prop));
    }
};
NODEC_SCENE_REGISTER_SERIALIZABLE_COMPONENT(SerializableComponentA)

struct SerializableComponentC : public nodec_scene_serialization::BaseSerializableComponent {
    int prop{0};

    SerializableComponentC()
        : BaseSerializableComponent(this) {
    }

    template<class Archive>
    void serialize(Archive &archive) {
        archive(cereal::make_nvp("prop", prop));
    }
};
NODEC_SCENE_REGISTER_SERIALIZABLE_COMPONENT(SerializableComponentC)

TEST_CASE("Testing serialization") {
    using namespace nodec_scene_serialization;
    using namespace nodec_animation;
    using namespace nodec_animation::resources;
    using namespace nodec::resource_management;

    ResourceRegistry resource_registry;

    SceneSerialization serialization;
    serialization.register_component<ComponentA, SerializableComponentA>();
    serialization.register_component<SerializableComponentC>();

    AnimationClip clip;

    {
        AnimationCurve curve;
        curve.add_keyframe({0.f, 0.0f});
        clip.set_curve<ComponentA>("", "prop", curve);
    }
    {
        AnimationCurve curve;
        curve.add_keyframe({0.f, 1.0f});
        curve.add_keyframe({0.5f, 0.5f});
        clip.set_curve<ComponentA>("a", "prop", curve);
    }
    {
        AnimationCurve curve;
        curve.add_keyframe({0.f, 0.0f});
        curve.add_keyframe({0.5f, 0.5f});
        curve.add_keyframe({1.0f, 1.0f});
        clip.set_curve<SerializableComponentC>("b/a", "prop", curve);
    }

    std::stringstream ss;
    {
        cereal::UserDataAdapter<ArchiveContext, cereal::JSONOutputArchive> archive(
            ArchiveContext{serialization, resource_registry}, ss);
        archive(cereal::make_nvp("clip", clip));
    }
    // std::cout << ss.str() << "\n";
    {
        cereal::UserDataAdapter<ArchiveContext, cereal::JSONInputArchive> archive(
            ArchiveContext{serialization, resource_registry}, ss);
        AnimationClip clip;
        archive(cereal::make_nvp("clip", clip));
        CHECK(clip.root_entity().components.at(nodec::type_id<ComponentA>()).properties.at("prop").curve.keyframes().size() == 1);
        CHECK(clip.root_entity().children.at("a").components.at(nodec::type_id<ComponentA>()).properties.at("prop").curve.keyframes().size() == 2);
        CHECK(clip.root_entity().children.at("b").children.at("a").components.at(nodec::type_id<SerializableComponentC>()).properties.at("prop").curve.keyframes().size() == 3);
    }
}