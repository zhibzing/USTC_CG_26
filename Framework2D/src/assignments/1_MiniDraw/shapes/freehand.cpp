#include "freehand.h"

#include <vector>
#include <algorithm>
#include <imgui.h>

namespace USTC_CG
{
Freehand::Freehand(float start_point_x,
        float start_point_y,
        float end_point_x,
        float end_point_y)
{
    control_points.push_back(ImVec2(start_point_x, start_point_y));
    control_points.push_back(ImVec2(end_point_x, end_point_y));
    num_points = 2;
}

// Draw the freehand in the type of polygon using ImGui
void Freehand::draw(const Config& config) const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    std::vector<ImVec2> polygon_points(control_points.size());
    std::transform(control_points.begin(), control_points.end(), polygon_points.begin(), 
                    [config](const ImVec2& p){
                        return ImVec2(p.x + config.bias[0], p.y + config.bias[1]);
                    }); // add the bias
    draw_list->AddPolyline(
        polygon_points.data(),
        num_points,
        IM_COL32(
            config.line_color[0],
            config.line_color[1],
            config.line_color[2],
            config.line_color[3]),
        ImDrawFlags_None,
        config.line_thickness);
}

// Add control points of polygon fitting freehand
void Freehand::update(float x, float y)
{
    if(control_points[num_points - 1].x != x || control_points[num_points - 1].y != y)
    {
        control_points.push_back(ImVec2(x, y));
        num_points++;
    }
}

}  // namespace USTC_CG
