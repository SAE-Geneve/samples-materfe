//
// Created by Mat on 1/23/2025.
//

#ifndef SAMPLES_OPENGL_UTILITY_TOOLS_H
#define SAMPLES_OPENGL_UTILITY_TOOLS_H
#include <random>

namespace tools
{
    template<typename T>
    T GenerateRandomNumber(T min_number, T max_number)
    {
#ifdef TRACY_ENABLE
        ZoneScoped;
#endif
        static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "function requires a number"); //compiler error if not number

        static std::random_device r;
        static std::default_random_engine e1(r()); //static -> one instance

        if constexpr (std::is_integral_v<T>)
        {
            std::uniform_int_distribution<T> uniform_dist(min_number, max_number);
            return uniform_dist(e1);
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            std::uniform_real_distribution<T> dist(min_number, max_number);
            return dist(e1);
        }
    }
}



#endif //SAMPLES_OPENGL_UTILITY_TOOLS_H
