// HW2_TODO: Please implement the abstract class Warper
// 1. The Warper class should abstract the **mathematical mapping** involved in
// the warping problem, **independent of image**.
// 2. The Warper class should have a virtual function warp(...) to be called in
// our image warping application.
//    - You should design the inputs and outputs of warp(...) according to the
//    mathematical abstraction discussed in class.
//    - Generally, the warping map should map one input point to another place.
// 3. Subclasses of Warper, IDWWarper and RBFWarper, should implement the
// warp(...) function to perform the actual warping.
#pragma once

#include "common/image_widget.h"
#include "Eigen/Dense"

#include <vector>
#include <iostream>

namespace USTC_CG
{
class Warper
{
   public:
    virtual ~Warper() = default;

    // HW2_TODO: A virtual function warp(...)
    virtual std::pair<int, int> warp(int x, int y) const = 0;
    
    // HW2_TODO: other functions or variables if you need
    // Add a pair of start point and end point to m_point_p and m_point_q
    void update(const std::vector<ImVec2> start_points, const std::vector<ImVec2> end_points, const Image& image);

   protected:
    int point_num = 0;
    std::vector<Eigen::Vector2d> m_point_p, m_point_q;
};
}  // namespace USTC_CG