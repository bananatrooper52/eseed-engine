#pragma once

#include <eseed/math/ops.hpp>

namespace esdm {

template <typename T>
Mat4<T> translate(const Vec3<T> &translation) {
    Mat4<T> out = *this;
    out[0][3] = translation.x;
    out[1][3] = translation.y;
    out[2][3] = translation.z;
    return out;
}

template <typename T>
Mat4<T> rotateX(const T &xAngle) {
    return Mat4<T> {
        1, 0, 0, 0,
        0, cos(xAngle), -sin(xAngle), 0,
        0, sin(xAngle), cos(xAngle), 0,
        0, 0, 0, 1
    };
}

template <typename T>
Mat4<T> rotateY(const T &yAngle) {
    return Mat4<T> {
        cos(yAngle), 0, sin(yAngle), 0,
        0, 1, 0, 0,
        -sin(yAngle), 0, cos(yAngle), 0,
        0, 0, 0, 1
    };
}

template <typename T>
Mat4<T> rotateZ(const T &zAngle) {
    return Mat4<T> {
        cos(zAngle), -sin(zAngle), 0, 0,
        sin(zAngle), cos(zAngle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

};