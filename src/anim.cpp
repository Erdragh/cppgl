#include "anim.h"
#include "camera.h"

// -------------------------------------------
// spline (centripedal catmull rom) for float types, i.e. float, glm::vec2, glm::vec3, glm::vec4

template <typename T> struct CubicPolynomial {
    void init(const T& x0, const T& x1, const T& t0, const T& t1) {
        c0 = x0;
        c1 = t0;
        c2 = -3.f*x0 + 3.f*x1 - 2.f*t0 - t1;
        c3 = 2.f*x0 - 2.f*x1 + t0 + t1;
    }

    T operator()(float t) const {
        const float t2 = t*t;
        const float t3 = t2*t;
        return c0 + c1*t + c2*t2 + c3*t3;
    }

    // data
    T c0, c1, c2, c3;
};

template <typename T> inline float dist_sqr(const T& x, const T& y) {
    const T d = x - y;
    return glm::dot(d, d);
}

template <typename T> struct CentripedalCR : public CubicPolynomial<T> {
    CentripedalCR(const T& p0, const T& p1, const T& p2, const T& p3) {
        float dt0 = powf(dist_sqr(p0, p1), 0.25f);
        float dt1 = powf(dist_sqr(p1, p2), 0.25f);
        float dt2 = powf(dist_sqr(p2, p3), 0.25f);
        // safety check for repeated points
        if (dt1 < 1e-4f)    dt1 = 1.0f;
        if (dt0 < 1e-4f)    dt0 = dt1;
        if (dt2 < 1e-4f)    dt2 = dt1;
        // compute tangents when parameterized in [t1,t2]
        T t1 = (p1 - p0) / dt0 - (p2 - p0) / (dt0 + dt1) + (p2 - p1) / dt1;
        T t2 = (p2 - p1) / dt1 - (p3 - p1) / (dt1 + dt2) + (p3 - p2) / dt2;
        // rescale tangents for parametrization in [0,1]
        t1 *= dt1;
        t2 *= dt1;
        // to cubic polynomial
        CubicPolynomial<T>::init(p1, p2, t1, t2);
    }
};

// -------------------------------------------
// Animation

Animation::Animation(const std::string& name) : NamedMap(name), time(0), ms_between_nodes(1000), running(false) {}

Animation::~Animation() {}

void Animation::update(float dt_ms) {
    if (running && time < camera_path.size() && !camera_path.empty()) {
        // compute spline
        const size_t at = glm::floor(time);
        const auto& p0 = camera_path[std::max(at - 1, size_t(0))];
        const auto& p1 = camera_path[std::min(at + 0, camera_path.size()) % camera_path.size()];
        const auto& p2 = camera_path[std::min(at + 1, camera_path.size()) % camera_path.size()];
        const auto& p3 = camera_path[std::min(at + 2, camera_path.size()) % camera_path.size()];
        const CentripedalCR spline(p0.first, p1.first, p2.first, p3.first);
        // update camera
        const glm::vec3& pos = spline(glm::fract(time));
        const glm::quat& rot = glm::slerp(p1.second, p2.second, glm::fract(time)); // TODO spherical spline
        Camera::current()->load(pos, rot);
        // advance
        time = glm::min(time + dt_ms / ms_between_nodes, float(camera_path.size()));
    } else
        running = false;
}

void Animation::clear() {
    camera_path.clear();
    data_int.clear();
    data_float.clear();
    data_vec2.clear();
    data_vec3.clear();
    data_vec4.clear();
}

size_t Animation::length() const {
    return camera_path.size();
}

void Animation::play() {
    time = 0;
    running = true;
}

void Animation::pause() {
    running = false;
}

void Animation::toggle_pause() {
    running = !running;
}

void Animation::stop() {
    time = 0;
    running = false;
}

void Animation::reset() {
    time = 0;
}

size_t Animation::push_node(const glm::vec3& pos, const glm::quat& rot) {
    const size_t i = camera_path.size();
    camera_path.push_back(std::make_pair(pos, rot));
    return i;
}

void Animation::put_node(size_t i, const glm::vec3& pos, const glm::quat& rot) {
    if (i < camera_path.size())
        camera_path[i] = std::make_pair(pos, rot);
}

void Animation::put_data(const std::string& name, size_t i, int val) {
    if (data_int[name].size() <= i)
        data_int[name].resize(i + 1);
    data_int[name][i] = val;
}

void Animation::put_data(const std::string& name, size_t i, float val) {
    if (data_float[name].size() <= i)
        data_float[name].resize(i + 1);
    data_float[name][i] = val;
}

void Animation::put_data(const std::string& name, size_t i, const glm::vec2& val) {
    if (data_vec2[name].size() <= i)
        data_vec2[name].resize(i + 1);
    data_vec2[name][i] = val;
}

void Animation::put_data(const std::string& name, size_t i, const glm::vec3& val) {
    if (data_vec3[name].size() <= i)
        data_vec3[name].resize(i + 1);
    data_vec3[name][i] = val;
}

void Animation::put_data(const std::string& name, size_t i, const glm::vec4& val) {
    if (data_vec4[name].size() <= i)
        data_vec4[name].resize(i + 1);
    data_vec4[name][i] = val;
}

glm::vec3 Animation::eval_pos() const {
    // eval centripedal catmull rom spline
    const size_t at = glm::floor(time);
    const auto& p0 = camera_path[std::max(size_t(0), at - 1)].first;
    const auto& p1 = camera_path[std::min(at, camera_path.size()) % camera_path.size()].first;
    const auto& p2 = camera_path[std::min(at + 1, camera_path.size()) % camera_path.size()].first;
    const auto& p3 = camera_path[std::min(at + 2, camera_path.size()) % camera_path.size()].first;
    CentripedalCR spline(p0, p1, p2, p3);
    return spline(glm::fract(time));
}

glm::quat Animation::eval_rot() const {
    // TODO spherical spline
    const size_t at = glm::floor(time);
    const auto& lower = camera_path[std::min(at, camera_path.size()) % camera_path.size()];
    const auto& upper = camera_path[std::min(at + 1, camera_path.size()) % camera_path.size()];
    return glm::slerp(lower.second, upper.second, time - at);
}

int Animation::eval_int(const std::string& name) const {
    const size_t i = std::min(size_t(glm::floor(time)), data_int.at(name).size() - 1);
    return data_int.at(name).at(i); // no lerp necessary
}

// lerp helper
template <typename T> inline T lerp_data(const std::vector<T>& data, float time) {
    const size_t at = glm::floor(time);
    const auto& lower = data[std::min(at, data.size()) % data.size()];
    const auto& upper = data[std::min(at + 1, data.size()) % data.size()];
    return glm::mix(lower, upper, time - at);
}

float Animation::eval_float(const std::string& name) const {
    return lerp_data<float>(data_float.at(name), time);
}

glm::vec2 Animation::eval_vec2(const std::string& name) const {
    return lerp_data<glm::vec2>(data_vec2.at(name), time);
}

glm::vec3 Animation::eval_vec3(const std::string& name) const {
    return lerp_data<glm::vec3>(data_vec3.at(name), time);
}

glm::vec4 Animation::eval_vec4(const std::string& name) const {
    return lerp_data<glm::vec4>(data_vec4.at(name), time);
}
