#include "ColorInterpolator.h"

void ColorInterpolator::addColorPoint(double level, const Vect3d& color) {
    colorPoints[level] = color;
}

Vect3d ColorInterpolator::interpolate(double value) {
    // Clamp the value between 0 and 1
    value = std::max(0.0, std::min(value, 1.0));

    // If the value is exactly at a color point, return that color
    if (colorPoints.count(value)) return colorPoints[value];

    // Find the two adjacent color points
    auto it = colorPoints.lower_bound(value);
    if (it == colorPoints.begin()) {
        return it->second;
    }
    if (it == colorPoints.end()) {
        return (--it)->second;
    }

    auto itLow = std::prev(it);
    auto itHigh = it;

    // Interpolate between the two colors
    double range = itHigh->first - itLow->first;
    double normalizedValue = (value - itLow->first) / range;
    Vect3d colorDiff = itHigh->second - itLow->second;
    return itLow->second + colorDiff * normalizedValue;
}