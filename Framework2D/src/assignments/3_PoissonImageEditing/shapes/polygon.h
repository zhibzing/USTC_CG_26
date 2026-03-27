#pragma once

#include "shape.h"
#include "../scanline/scanline.h"

#include <vector>
#include <imgui.h>

namespace USTC_CG
{
class Polygon : public Shape
{
   public:
    Polygon() = default;
    
    // Initialize a polygon with start and end points
    Polygon(float start_point_x,
        float start_point_y,
        float end_point_x,
        float end_point_y);

    virtual ~Polygon() = default;

    // Draws the polygon on the screen
    // Overrides draw function to implement polygon-specific drawing logic
    void draw(const Config& config) const override;

    // Overrides Shape's update function to adjust the polygon size during
    // interaction
    void update(float x, float y) override;

    // Get the interior rasterized pixels of the rectangle
    // Returns the array of pixel coordinates that are inside the rectangle
    std::vector<std::pair<int, int>> get_interior_pixels() const override;

    // Add control point(polygon vertex) of the polygon
    void add_control_point(float x, float y) override;

   private:
    // number of the control points
    int num_points = 0;
    // Coordinates of the control points of the polyogn
    std::vector<ImVec2> control_points;
};
}  // namespace USTC_CG
