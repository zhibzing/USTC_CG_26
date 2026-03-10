#include "warper.h"
#include "Eigen/Dense"

#include <vector>
#include <iostream>

namespace USTC_CG
{
    void Warper::update(const std::vector<ImVec2> start_points, const std::vector<ImVec2> end_points)
    {
        int size = start_points.size();
        for (int i = 0; i < size; i++){
            Eigen::Vector2d p(start_points[i].x, start_points[i].y);
            Eigen::Vector2d q(end_points[i].x, end_points[i].y);
            m_point_p.push_back(p);
            m_point_q.push_back(q);
            point_num++;
        }
    }
}  // namespace USTC_CG