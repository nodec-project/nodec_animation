#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <vector>

#include <nodec/math/math.hpp>
#include <nodec/serialization/vector3.hpp>
#include <nodec_animation/animated_component_writer.hpp>
#include <nodec_animation/resources/animation_clip.hpp>
#include <nodec_scene/scene.hpp>
#include <nodec_scene_serialization/scene_serialization.hpp>
#include <nodec_scene_serialization/serializable_component.hpp>

#include <cereal/types/string.hpp>

struct Resource {
    template<class Archive>
    void serialize(Archive &archive) {
    }

    int field;
};

// CEREAL_REGISTER_TYPE(Resource)

struct TestComponent : nodec_scene_serialization::BaseSerializableComponent {
    TestComponent()
        : BaseSerializableComponent(this) {}

    template<class Archive>
    void serialize(Archive &archive) {
        archive(cereal::make_nvp("field", field));
        archive(cereal::make_nvp("resource", resource));
        archive(cereal::make_nvp("position", position));
    }

    float field;
    std::shared_ptr<Resource> resource;
    nodec::Vector3f position;
};

NODEC_SCENE_REGISTER_SERIALIZABLE_COMPONENT(TestComponent)

TEST_CASE("Testing to write properties to a component") {
    using namespace nodec;
    using namespace nodec_scene_serialization;
    using namespace nodec_scene;
    using namespace nodec_animation;
    using namespace nodec_animation::resources;

    SceneSerialization serialization;

    serialization.register_component<TestComponent>();

    Scene scene;

    auto root_entity = scene.create_entity("root");

    auto &test_component = scene.registry().emplace_component<TestComponent>(root_entity).first;
    
    auto test_resource = std::make_shared<Resource>();
    {
        test_component.field = 1.f;
        test_component.position.set(1.0f, 2.0f, 3.0f);
        test_component.resource = test_resource;
        test_component.resource->field = 100;
    }

    AnimationClip clip;
    {
        AnimationCurve curve;
        curve.add_keyframe({0, 0.0f});
        curve.add_keyframe({1000, 1.0f});
        clip.set_curve<TestComponent>("", "position.x", curve);
    }

    auto *animated_entity = &clip.root_entity();
    {
        const auto &pair = *animated_entity->components.begin();
        auto &component_type_info = pair.first;
        auto &animated_component = pair.second;

        AnimatedComponentWriter writer;

        CHECK(component_type_info == nodec::type_id<TestComponent>());
        writer.write(animated_component, 500, test_component);

        CHECK(math::approx_equal(test_component.position.x, 0.5f));
        CHECK(test_component.position.y == 2.f);
        CHECK(test_component.position.z == 3.f);
        CHECK(test_component.field == 1.f);
        CHECK(test_component.resource == test_resource);
    }
}