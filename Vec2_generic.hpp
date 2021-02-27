#pragma once
#include <cmath>

namespace cge
{
    template<class T>
    struct Vec2_generic
    {
        T x = 0;
        T y = 0;        

        Vec2_generic() {}
        Vec2_generic(const Vec2_generic& v) : x(v.x), y(v.y) {}
        Vec2_generic(const T& x, const T& y) : x(x), y(y) {}
        Vec2_generic(const T& val) : x(val), y(val) {}
        //Vec2f(float x, float y) : x(x), y(y) {}

        T Mag() { return std::sqrt(x * x + y * y); }
        Vec2_generic Round() { return Vec2_generic(std::round(this->x), std::round(this->y)); }
        Vec2_generic Normalized() {
            T mag = Mag();
            if (mag == 0)
                return Vec2_generic(0, 0);
            return Vec2_generic(x / mag, y / mag);
        }

        Vec2_generic operator+(const Vec2_generic& v)
        {
            return Vec2_generic(this->x + v.x, this->y + v.y);
        }
        Vec2_generic& operator+=(const Vec2_generic& rhs)
        {
            this->x += rhs.x;
            this->y += rhs.y;
            return *this;
        }
        T operator*(const Vec2_generic& rhs)
        {
            return this->x * rhs.x + this->y + rhs.y;
        }
        Vec2_generic operator*(const T& rhs)
        {
            return Vec2_generic(this->x * rhs, this->y * rhs);
        }        
        Vec2_generic& operator*=(const T& rhs)
        {
            this->x *= rhs;
            this->y *= rhs;
            return *this;
        }
        Vec2_generic operator-(const Vec2_generic& rhs)
        {
            return Vec2_generic(this->x - rhs.x, this->y - rhs.y);
        }
        Vec2_generic& operator-=(const Vec2_generic& rhs)
        {
            this->x -= rhs.x;
            this->y -= rhs.y;
            return *this;
        }
        T& operator[](std::size_t i)
        {
            return *((float*)this + i);
        }
        std::string to_string()
        {
            return "[" + std::to_string(x) + ";" + std::to_string(y) + "]";
        }
        std::wstring to_wstring()
        {
            return L"[" + std::to_wstring(x) + ";" + std::to_wstring(y) + "]";
        }    

    };
    template<class T>
    Vec2_generic<T> operator*(const T& lhs, const Vec2_generic<T>& rhs)
    {
        return Vec2_generic<T>(lhs * rhs.x, lhs * rhs.y);
    }
}