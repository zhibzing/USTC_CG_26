#include "ellipse.h"

#include <imgui.h>
#include <cmath>

namespace USTC_CG
{
// Draw the ellipse using ImGui
void Ellipse::draw(const Config& config) const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddEllipse(
        ImVec2(
            config.bias[0] + start_point_x_, config.bias[1] + start_point_y_),
        ImVec2(end_point_x_ - start_point_x_, end_point_y_ - start_point_y_),
        IM_COL32(
            config.line_color[0],
            config.line_color[1],
            config.line_color[2],
            config.line_color[3]),
        0.f,  // No rounding of corners
        ImDrawFlags_None,
        config.line_thickness);
}

void Ellipse::update(float x, float y)
{
    end_point_x_ = x;
    end_point_y_ = y;
}

std::vector<std::pair<int, int>> Ellipse::get_interior_pixels() const
{
    int start_pixel_x = static_cast<int>(start_point_x_);
    int start_pixel_y = static_cast<int>(start_point_y_);
    int end_pixel_x = static_cast<int>(end_point_x_);
    int end_pixel_y = static_cast<int>(end_point_y_);
    
    if (start_pixel_x > end_pixel_x)
    {
        std::swap(start_pixel_x, end_pixel_x);
    }
    if (start_pixel_y > end_pixel_y)
    {
        std::swap(start_pixel_y, end_pixel_y);
    }
    
    double a = end_pixel_x - start_pixel_x;
    double b = end_pixel_y - start_pixel_y;
    double center_x = start_pixel_x;
    double center_y = start_pixel_y;
    // Pick the pixels in the rectangle (including the boundary)
    std::vector<std::pair<int, int>> int_pixels;
    int estimated_pixels = static_cast<int>(3.14159 * a * b * 1.2);
    int_pixels.reserve(estimated_pixels);
    if (a > b)
    {
        for (int i = center_x - a; i <= center_x + a; ++i)
        {
            double dx = (i - center_x) / a;
            
            double dy = std::sqrt(1 - dx * dx);
            int y_start = static_cast<int>(std::ceil(center_y - b * dy));
            int y_end = static_cast<int>(std::floor(center_y + b * dy));
            
            for (int j = y_start; j <= y_end; ++j) 
            {
                int_pixels.push_back(std::make_pair(i, j));
            }
        }
    }
    else
    {
        for (int j = center_y - b; j <= center_y + b; ++j)
        {
            double dy = (j - center_y) / b;
            
            double dx = std::sqrt(1 - dy * dy);
            int x_start = static_cast<int>(std::ceil(center_x - a * dx));
            int x_end = static_cast<int>(std::floor(center_x + a * dx));
            
            for (int i = x_start; i <= x_end; ++i) 
            {
                int_pixels.push_back(std::make_pair(i, j));
            }
        }
    }
    return int_pixels;
}
}  // namespace USTC_CG
