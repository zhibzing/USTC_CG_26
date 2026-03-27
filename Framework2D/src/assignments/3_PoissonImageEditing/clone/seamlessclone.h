#pragma once

#include "common/image_widget.h"
#include "../source_image_widget.h"
#include "clone.h"
#include "Eigen/Sparse"

#include <vector>

namespace USTC_CG
{
class SeamlessClone : public Clone
{
   public:
    SeamlessClone() = default;
    ~SeamlessClone() = default;

    std::shared_ptr<Image> solve() override;

   private:
    // sparse matrix of Poisson equation
    Eigen::SparseMatrix<double> A;
    // constent term for each channel
    std::vector<Eigen::VectorXd> B = std::vector<Eigen::VectorXd>(3);
    // result of Poisson equation
    std::vector<Eigen::VectorXd> r = std::vector<Eigen::VectorXd>(3);
    // use triplet list to set A
    std::vector<Eigen::Triplet<double>> triplet_list;

   private:
    void set_A();
    void set_B();
};
}  // namespace USTC_CG
