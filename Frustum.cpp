#include "Frustum.h"

Frustum::Frustum() {}

void Frustum::update(const glm::mat4& viewProjection, float margin) {
    this->margin = margin;
    // Extract frustum planes from the view-projection matrix
    // Left plane
    planes[0] = glm::vec4(viewProjection[0][3] + viewProjection[0][0],
        viewProjection[1][3] + viewProjection[1][0],
        viewProjection[2][3] + viewProjection[2][0],
        viewProjection[3][3] + viewProjection[3][0]);

    // Right plane
    planes[1] = glm::vec4(viewProjection[0][3] - viewProjection[0][0],
        viewProjection[1][3] - viewProjection[1][0],
        viewProjection[2][3] - viewProjection[2][0],
        viewProjection[3][3] - viewProjection[3][0]);

    // Bottom plane
    planes[2] = glm::vec4(viewProjection[0][3] + viewProjection[0][1],
        viewProjection[1][3] + viewProjection[1][1],
        viewProjection[2][3] + viewProjection[2][1],
        viewProjection[3][3] + viewProjection[3][1]);

    // Top plane
    planes[3] = glm::vec4(viewProjection[0][3] - viewProjection[0][1],
        viewProjection[1][3] - viewProjection[1][1],
        viewProjection[2][3] - viewProjection[2][1],
        viewProjection[3][3] - viewProjection[3][1]);

    // Near plane
    planes[4] = glm::vec4(viewProjection[0][3] + viewProjection[0][2],
        viewProjection[1][3] + viewProjection[1][2],
        viewProjection[2][3] + viewProjection[2][2],
        viewProjection[3][3] + viewProjection[3][2]);

    // Far plane
    planes[5] = glm::vec4(viewProjection[0][3] - viewProjection[0][2],
        viewProjection[1][3] - viewProjection[1][2],
        viewProjection[2][3] - viewProjection[2][2],
        viewProjection[3][3] - viewProjection[3][2]);

    // Normalize the planes
    for (auto& plane : planes) {
        plane /= glm::length(glm::vec3(plane));
        plane.w += margin; // Move the plane outward by the margin
    }
}

bool Frustum::isPointInFrustum(const glm::vec3& point) const {
    for (const auto& plane : planes) {
        if (glm::dot(glm::vec3(plane), point) + plane.w < 0) {
            return false;
        }
    }
    return true;
}

bool Frustum::isAABBInFrustum(const glm::vec3& minPoint, const glm::vec3& maxPoint) const {
    for (const auto& plane : planes) {
        glm::vec3 p = minPoint;
        if (plane.x >= 0) p.x = maxPoint.x;
        if (plane.y >= 0) p.y = maxPoint.y;
        if (plane.z >= 0) p.z = maxPoint.z;

        if (glm::dot(glm::vec3(plane), p) + plane.w < 0) {
            return false;
        }
     }
    return true;
}