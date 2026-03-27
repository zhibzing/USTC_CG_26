#pragma once

#include <vector>
#include <imgui.h>

namespace USTC_CG
{
class Scanline
{
   public:
    Scanline(const std::vector<ImVec2>& vertices, int num);
    ~Scanline() = default;

    void update_edge_list();
    void update_interior_point();

   public:
    // Interior points
    std::vector<std::pair<int, int>> interior_points;
    // Current y
    int current_y = 0;
    // Top and botton of the polygon
    int polygon_y_max = 0, polygon_y_min = 0;

   private:
    struct Edge
    {
        int y_max, y_min;
        double x;
        double dx;

        Edge(int start_x, int start_y, int end_x, int end_y);

        bool operator<(const Edge& other) const; 
    };

    std::vector<ImVec2> vertices;
    int num;
    // Edge table of all of the edges
    std::vector<std::vector<Edge>> edge_table;
    // Edge list of current line
    std::vector<Edge> edge_list;

   private:
    void find_range();
    void build_edge_tabel();
};
}  // namespace USTC_CG