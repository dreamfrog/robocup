#ifndef VISIONLINE_H
#define VISIONLINE_H

#include "Tools/Math/Line.h"
#include "Vision/VisionTypes/Interfaces/ransacmodel.h"

class VisionLine : public Line
{
public:
    VisionLine();

    bool regenerate(const vector<Point> &pts);

    unsigned int minPointsForFit() const {return 2;}

    double calculateError(Point p) const;
};

#endif // VISIONLINE_H