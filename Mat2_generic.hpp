#pragma once
#include <cmath>
#include "Vec2_generic.hpp"

namespace cge
{
    template<typename T>
    class Mat2_generic
    {
        T mat[2 * 2];
    public:

        Mat2_generic()
        {
            mat[0] = 1; mat[1] = 0;            
            mat[2] = 0; mat[3] = 1;
        }
        Mat2_generic(T c0, T c1, T c2, T c3)
        {
            mat[0] = c0; mat[1] = c1;
            mat[2] = c2; mat[3] = c3;
        }        

        Vec2_generic<T> operator*(const Vec2_generic<T>& v)
        {
            return Vec2_generic<T>(mat[0] * v.x + mat[1] * v.y, mat[2] * v.x + mat[3] * v.y);
        }
        T operator[](size_t i)
        {
            return mat[i];
        }
        Mat2_generic operator*(const Mat2_generic& rhs)
        {
            return Mat2_generic(
                mat[0]*rhs.mat[0] + mat[1]*rhs.mat[2], mat[0]*rhs.mat[1] + mat[1]*rhs.mat[3],
                mat[2]*rhs.mat[0] + mat[3]*rhs.mat[2], mat[2]*rhs.mat[1] + mat[3]*rhs.mat[3]
            );
        }
        Mat2_generic& operator*=(const Mat2_generic& rhs)
        {
            mat[0] = mat[0]*rhs.mat[0] + mat[1]*rhs.mat[2];
            mat[1] = mat[0]*rhs.mat[1] + mat[1]*rhs.mat[3];
            mat[2] = mat[2]*rhs.mat[0] + mat[3]*rhs.mat[2];
            mat[3] = mat[2]*rhs.mat[1] + mat[3]*rhs.mat[3];
            return *this;
        }        
    };

    template<typename T>
    Mat2_generic<T> Rotate(Mat2_generic<T> mat, const float& angle)
    {
        float s = std::sin(angle);
        float c = std::cos(angle);
        return mat * Mat2_generic<T>( c, -s, s, c);
    }
    template<typename T>
    Mat2_generic<T> Scale(Mat2_generic<T> mat, const Vec2_generic<T>& v)
    {
        return mat * Mat2_generic<T>( v.x, 0, 0, v.y);
    }
}