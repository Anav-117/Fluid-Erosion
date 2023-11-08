#pragma once
#include <map>
#include "math/vect3d.h"

class ColorInterpolator {
private:
    std::map<double, Vect3d> colorPoints;

public:
    void addColorPoint(double level, const Vect3d& color);
    Vect3d interpolate(double value);
};
