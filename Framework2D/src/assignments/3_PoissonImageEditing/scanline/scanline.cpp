#include "scanline.h"

#include <vector>
#include <algorithm>
#include <imgui.h>

namespace USTC_CG
{
Scanline::Scanline(const std::vector<ImVec2>& vertices, int num)
    :vertices(vertices), num(num)
{
    find_range();
    build_edge_tabel();
}

Scanline::Edge::Edge(int start_x, int start_y, int end_x, int end_y)
{
    if (start_y > end_y){
        y_max = start_y;
        y_min = end_y;
        x = end_x;
    }
    else{
        y_max = end_y;
        y_min = start_y;
        x = start_x;
    }

    dx = (end_x - start_x) / (double)(end_y - start_y);
}

bool Scanline::Edge::operator<(const Edge& other) const
{
    return x < other.x;
}

void Scanline::update_edge_list()
{
    edge_list.erase(
        std::remove_if(edge_list.begin(), edge_list.end(), 
            [this](const Edge& edge){
                return this->current_y >= edge.y_max;
            }), edge_list.end());

    for (auto& edge : edge_list){
        edge.x += edge.dx;
    }

    int index = current_y - polygon_y_min;
    for (const auto& edge : edge_table[index]){
        edge_list.push_back(edge);
    }

    std::sort(edge_list.begin(), edge_list.end());
}

void Scanline::update_interior_point()
{
    int n = edge_list.size();

    for (int i = 0; i < n; i += 2){
        int start = (int)edge_list[i].x;
        int end = (int)edge_list[i + 1].x;
        for (int x = start; x <= end; x++){
            interior_points.push_back(std::make_pair(x, current_y));
        }
    }
}

void Scanline::find_range()
{
    polygon_y_max = static_cast<int>(vertices[0].y);
    polygon_y_min = static_cast<int>(vertices[0].y);

    for (const auto& point : vertices){
        int y = static_cast<int>(point.y);
        polygon_y_min = std::min(polygon_y_min, y);
        polygon_y_max = std::max(polygon_y_max, y);
    }
    current_y = polygon_y_min;
}

void Scanline::build_edge_tabel()
{
    edge_table.resize(polygon_y_max - polygon_y_min + 1);

    for (int i = 0; i < num; i++){
        int j = (i + 1) % num;

        if (vertices[i].y == vertices[j].y)
            continue;
        Edge edge(vertices[i].x, vertices[i].y, 
            vertices[j].x, vertices[j].y);

        int index = edge.y_min - polygon_y_min;
        edge_table[index].push_back(edge);
    }
}
}  // namespace USTC_CG