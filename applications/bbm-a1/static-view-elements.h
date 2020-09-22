#pragma once

#include <cppgl.h>

void init_static_prototypes();
void draw_gui();

class Floor {
public:
	Floor(int w, int h);

	void draw();

    // data
    glm::mat4 trafo;
    static Drawelement prototype;
};

class Skybox {
public:
	Skybox(int w, int h, int d);

	void draw();

    // data
    glm::mat4 trafo;
    static std::vector<Drawelement> prototype;
};
