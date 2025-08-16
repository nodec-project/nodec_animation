# Property Reflection Design in nodec_animation

## Overview

The nodec_animation module uses a clever approach to animate component properties without requiring explicit reflection metadata. It leverages the Cereal serialization library's traversal mechanism to discover and update properties at runtime.

## Key Design Principle

Instead of building a separate reflection system, nodec_animation **reuses the existing serialization infrastructure** to:
1. Discover component structure
2. Build property paths dynamically
3. Update property values through the serialization interface

## The PropertyWriter Archive

### Concept
`PropertyWriter` is a custom Cereal InputArchive that:
- **Reads** animation data from curves
- **Writes** interpolated values to component properties
- **Tracks** property paths during traversal

### Implementation Details

```cpp
class PropertyWriter : public cereal::InputArchive<PropertyWriter> {
    // Current property path (e.g., "transform.position.x")
    std::string current_property_name_;
    
    // Stack to track nested property names
    std::vector<const char*> name_stack_;
    
    // Animation data source
    const AnimatedComponent& source_;
    
    // Current animation time
    float time_;
};
```

## Property Path Construction

### How Paths Are Built

1. **Node Entry** (`start_node`):
   ```cpp
   void start_node(const char* name) {
       name_stack_.push_back(name);
       if (name != nullptr) {
           if (!current_property_name_.empty()) {
               current_property_name_ += ".";
           }
           current_property_name_ += name;
       }
   }
   ```

2. **Node Exit** (`end_node`):
   ```cpp
   void end_node() {
       auto last = name_stack_.back();
       name_stack_.pop_back();
       if (last != nullptr) {
           // Remove the property name from path
           current_property_name_.erase(...);
       }
   }
   ```

### Example Traversal

For a component with nested structure:
```cpp
struct Transform {
    struct Position {
        float x, y, z;
        template<class Archive>
        void serialize(Archive& ar) {
            ar(CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z));
        }
    } position;
    
    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(position));
    }
};
```

Traversal produces paths:
- `"position"` → enters Position struct
- `"position.x"` → x property
- `"position.y"` → y property  
- `"position.z"` → z property

## Value Animation Process

### 1. Curve Lookup
When PropertyWriter encounters an arithmetic property:
```cpp
template<class T, EnableIf<std::is_arithmetic<T>::value>>
void load_value(T& value) {
    // Find animation curve for current property path
    auto iter = source_.properties.find(current_property_name_);
    if (iter == source_.properties.end()) return;
    
    const auto& curve = iter->second.curve;
    // ...
}
```

### 2. Curve Evaluation
```cpp
// Evaluate curve at current time with optimization hint
auto sample = curve.evaluate(time_, hint_index);

// Cast to target type and assign
value = static_cast<T>(sample.second);
```

### 3. State Tracking
For performance, tracks last keyframe index:
```cpp
if (property_animation_state) {
    property_animation_state->current_index = sample.first;
}
```

## Cereal Integration Points

### Archive Registration
```cpp
CEREAL_REGISTER_ARCHIVE(nodec_animation::AnimatedComponentWriter::PropertyWriter)
```

### Custom Load Functions
```cpp
// Handle arithmetic types (animatable)
template<class T, EnableIf<std::is_arithmetic<T>::value>>
inline void load(PropertyWriter& ar, T& value) {
    ar.load_value(value);
}

// Ignore smart pointers (non-animatable)
template<class T>
inline void load(PropertyWriter& ar, std::shared_ptr<T>&) {
    // No-op
}
```

### Name-Value Pair Support
```cpp
template<class T>
inline void prologue(PropertyWriter& ar, const NameValuePair<T>& pair) {
    ar.start_node(pair.name);
}

template<class T>
inline void epilogue(PropertyWriter& ar, const T& value) {
    ar.end_node();
}
```

## Type Support Matrix

| Type Category | Supported | Behavior |
|--------------|-----------|----------|
| Arithmetic (int, float, etc.) | ✅ | Animated via curves |
| Strings | ✅ | Can be set but not interpolated |
| Structs/Classes | ✅ | Traversed recursively |
| Arrays/Vectors | ✅ | Each element addressed by index |
| Smart Pointers | ❌ | Ignored during traversal |
| Raw Pointers | ❌ | Not supported |

## Advantages of This Design

### 1. **Zero Boilerplate**
Components need no special attributes or registration beyond existing serialization:
```cpp
// This is all that's needed!
template<class Archive>
void serialize(Archive& ar) {
    ar(CEREAL_NVP(property1), CEREAL_NVP(property2));
}
```

### 2. **Type Safety**
Compile-time type checking through templates ensures type mismatches are caught early.

### 3. **Flexibility**
- Any serializable component is automatically animatable
- Nested structures supported out-of-the-box
- Easy to add new component types

### 4. **Performance**
- No runtime type information overhead
- Efficient curve evaluation with hints
- Minimal memory overhead per animated component

### 5. **Maintainability**
- Single source of truth for component structure (serialize method)
- Changes to component structure automatically reflected in animation system

## Limitations and Considerations

### 1. **Property Naming**
Property paths depend on CEREAL_NVP names:
```cpp
// Good - provides clear property path
ar(CEREAL_NVP(position));

// Bad - no property name available
ar(position);  // Results in unnamed node
```

### 2. **Performance Overhead**
- Serialization traversal has some overhead
- Mitigated by caching and hints
- Consider grouping frequently animated properties

### 3. **Type Restrictions**
- Only arithmetic types can be smoothly interpolated
- Complex types need custom handling
- Reference types not supported

## Best Practices

### 1. Component Design
```cpp
struct AnimatableComponent {
    // Group related properties
    struct Transform {
        float x, y, z;
        // Serialize method here
    } transform;
    
    // Use meaningful names
    float opacity;  // Good
    float o;       // Bad - unclear in animation editor
    
    // Serialize with named values
    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(transform),
           CEREAL_NVP(opacity));
    }
};
```

### 2. Animation Setup
```cpp
// Use clear property paths
clip->set_curve<Transform>("entity", "position.x", curve);

// Consider animation groups
clip->set_curve<Transform>("", "scale.x", scale_curve);
clip->set_curve<Transform>("", "scale.y", scale_curve);
clip->set_curve<Transform>("", "scale.z", scale_curve);
```

### 3. Performance Optimization
```cpp
// Register only animatable components
registry.register_component<Transform>();
registry.register_component<Color>();
// Don't register data-only components
```

## Future Enhancements

### Potential Improvements
1. **Property Metadata**: Additional attributes for min/max ranges, interpolation modes
2. **Custom Interpolators**: Support for quaternion slerp, color space interpolation
3. **Animation Blending**: Multiple clips affecting same property
4. **Runtime Property Discovery**: Editor UI for browsing animatable properties
5. **Optimization**: Compile-time property path resolution for known types