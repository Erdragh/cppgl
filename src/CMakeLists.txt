# glob for source files
file(GLOB_RECURSE SOURCES "*.cpp")

# define target
add_library(cppgl SHARED ${SOURCES})
generate_export_header(cppgl)

# public interface (cppgl.h is an available header file for all dependant projects)
target_include_directories(cppgl PUBLIC .)
if (NOT GLM_FOUND)
    target_include_directories(cppgl PUBLIC ../subtrees/glm)
else()
    target_link_libraries(cppgl PUBLIC glm::glm)
endif()
if (NOT GLEW_FOUND)
    target_include_directories(cppgl PUBLIC ../subtrees/glew/include)
    target_link_libraries(cppgl libglew_shared)
else()
    target_include_directories(cppgl PUBLIC ${GLEW_INCLUDE_DIRS})
    target_link_libraries(cppgl ${GLEW_LIBRARIES})
endif()
if (NOT glfw3_FOUND)
    target_include_directories(cppgl PRIVATE ../subtrees/glfw/include)
else()
    target_include_directories(cppgl PRIVATE ${GLFW_INCLUDE_DIRS})
endif()
if (NOT assimp_FOUND)
    target_include_directories(cppgl PRIVATE ../subtrees/assimp/include)
else()
    target_include_directories(cppgl PRIVATE ${assimp_INCLUDE_DIR})
endif()

# ----------------------------------------------------------
# dependencies

# opengl
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
target_include_directories(cppgl PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(cppgl ${OPENGL_LIBRARIES})

if(UNIX)
    target_link_libraries(cppgl stdc++fs) # required for std::filesystem
else()
    target_compile_definitions(cppgl PRIVATE -DBUILD_CPPGL_DLL)
    target_compile_definitions(cppgl PRIVATE -DBUILD_SHARED_LIBS)
endif()

target_link_libraries(cppgl glfw assimp)
