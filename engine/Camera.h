#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FROCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

namespace engine::component {
    class  Camera {
    private:
        glm::mat4 m_ProjectionMatrix{1.0f};
        glm::mat4 m_ViewMatrix{1.0f};

    public:
        void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

        void SetPerspectiveProjection(float fovy, float ascpect, float near, float far);

        void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0, -1, 0));

        void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0, -1, 0));

        void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

        [[nodiscard]] const glm::mat4 &Projection() const { return m_ProjectionMatrix; }
        [[nodiscard]] const glm::mat4 &View() const { return m_ViewMatrix; }
    };

} // namespace engine