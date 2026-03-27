#pragma once

#include "shape.h"
#include "../scanline/scanline.h"

#include <vector>
#include <imgui.h>

namespace USTC_CG
{
class Freehand : public Shape
{
   public:
    Freehand() = default;
    
    // Initialize a freehand with start and end points
    Freehand(float start_point_x, float start_point_y);

    virtual ~Freehand() = default;

    // Draws the freehand on the screen
    // Overrides draw function to implement freehand-specific drawing logic
    void draw(const Config& config) const override;

    // Overrides Shape's update function to adjust the freehand size during
    // interaction
    // As freehand is continuous, add control points at all position
    void update(float x, float y) override;

    // Get the interior rasterized pixels of the rectangle
    // Returns the array of pixel coordinates that are inside the rectangle
    std::vector<std::pair<int, int>> get_interior_pixels() const override;

   private:
    // number of the control points
    int num_points = 0;
    // Coordinates of the control points of the polyogn fitting freehand
    std::vector<ImVec2> control_points;
};
} // namespace USTC_CG
