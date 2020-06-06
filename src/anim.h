#pragma once

#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "named_map.h"

// TODO import/export (json?)

class Animation : public NamedMap<Animation> {
public:
    Animation(const std::string& name);
    virtual ~Animation();

    // step animation and apply to Camera::current() if running
    void update(float dt_ms);

    void clear();
    size_t length() const;

    void play();
    void pause();
    void toggle_pause();
    void stop();
    void reset();

    // add/modify camera path nodes
    size_t push_node(const glm::vec3& pos, const glm::quat& rot);
    void put_node(size_t i, const glm::vec3& pos, const glm::quat& rot);
    // insert data at given node along camera path
    void put_data(const std::string& name, size_t i, int val);
    void put_data(const std::string& name, size_t i, float val);
    void put_data(const std::string& name, size_t i, const glm::vec2& val);
    void put_data(const std::string& name, size_t i, const glm::vec3& val);
    void put_data(const std::string& name, size_t i, const glm::vec4& val);

    // evaluate
    glm::vec3 eval_pos() const;
    glm::quat eval_rot() const;
    int eval_int(const std::string& name) const;
    float eval_float(const std::string& name) const;
    glm::vec2 eval_vec2(const std::string& name) const;
    glm::vec3 eval_vec3(const std::string& name) const;
    glm::vec4 eval_vec4(const std::string& name) const;

    // data
    float time;
    float ms_between_nodes;
    bool running;
    std::vector<std::pair<glm::vec3, glm::quat>> camera_path;
    std::map<std::string, std::vector<int>> data_int;
    std::map<std::string, std::vector<float>> data_float;
    std::map<std::string, std::vector<glm::vec2>> data_vec2;
    std::map<std::string, std::vector<glm::vec3>> data_vec3;
    std::map<std::string, std::vector<glm::vec4>> data_vec4;
};

// variadic alias for std::make_shared<>(...)
template <class... Args> std::shared_ptr<Animation> make_animation(Args&&... args) {
    return std::make_shared<Animation>(args...);
}