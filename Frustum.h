//
// Created by Salva on 24/09/2024.
//

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <glm/glm.hpp>
#include <array>

class Frustum {
public:
    Frustum();
    void update(const glm::mat4 &viewProjection, float margin = 0.1f);
    bool isPointInFrustum(const glm::vec3& point) const;
    bool isAABBInFrustum(const glm::vec3& minPoint, const glm::vec3& maxPoint) const;

private:
    std::array<glm::vec4, 6> planes;
    float margin;
};



#endif //FRUSTUM_H
