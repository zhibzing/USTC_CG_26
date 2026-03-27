#include "common/image_widget.h"
#include "../source_image_widget.h"
#include "mixinggradients.h"
#include "Eigen/Sparse"

#include <vector>

namespace USTC_CG
{
std::shared_ptr<Image> MixingGradients::solve()
{
    set_A();
    set_B();

    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
    solver.compute(A);

    if (solver.info() != Eigen::Success)
    {
        throw std::runtime_error("Linear system solving failed");
        return tar_img_;
    }

    for (int type = 0; type < 3; type++)
    {
        r[type].resize(mask_size);
        r[type] = solver.solve(B[type]);
    }

    for (int i = 0; i < mask_size; i++)
    {
        int x = interior_points[i].x;
        int y = interior_points[i].y;

        // clamp the result in [0, 255]
        auto clamp_to_uchar = [](double value) -> unsigned char
        {
            if (value < 0) return 0;
            if (value > 255) return 255;
            return static_cast<unsigned char>(value);
        };
        tar_img_->set_pixel(x, y, {clamp_to_uchar(r[0](i)), 
                                   clamp_to_uchar(r[1](i)), 
                                   clamp_to_uchar(r[2](i))});
    }

    return tar_img_;
}

void MixingGradients::set_A()
{
    A.resize(mask_size, mask_size);
    A.setZero();
    triplet_list.clear();
    triplet_list.reserve(5 * mask_size);

    std::vector<unsigned char> mask_data(width * height, 0);
    for (int i = 0; i < mask_size; i++)
    {
        int x = static_cast<int>(interior_points[i].x);
        int y = static_cast<int>(interior_points[i].y);
        int index = y * width + x;
        mask_data[index] = src_selected_mask_->get_pixel(x, y)[0];
    }

    for (int i = 0; i < mask_size; i++)
    {
        int x = static_cast<int>(interior_points[i].x);
        int y = static_cast<int>(interior_points[i].y);
        int index = y * width + x;

        triplet_list.push_back(Eigen::Triplet<double>(i, i, 4.0));

        if (x > 0 && mask_data[index - 1] > 0) {
            triplet_list.emplace_back(i, pos_to_index[index - 1], -1.0);
        }
        if (x + 1 < width && mask_data[index + 1] > 0) {
            triplet_list.emplace_back(i, pos_to_index[index + 1], -1.0);
        }
        if (y > 0 && mask_data[index - width] > 0) {
            triplet_list.emplace_back(i, pos_to_index[index - width], -1.0);
        }
        if (y + 1 < height && mask_data[index + width] > 0) {
            triplet_list.emplace_back(i, pos_to_index[index + width], -1.0);
        }
    }

    A.setFromTriplets(triplet_list.begin(), triplet_list.end());
    A.makeCompressed();
}

void MixingGradients::set_B()
{
    std::vector<bool> is_inside(width * height);
    for (int i = 0; i < mask_size; i++)
    {
        int x = interior_points[i].x;
        int y = interior_points[i].y;
        is_inside[y * width + x] = true;
    }

    for (int type = 0; type < 3; type++)
    {
        B[type].resize(mask_size);
        
        #pragma omp parallel for
        for (int i = 0; i < mask_size; i++)
        {
            int x = interior_points[i].x;
            int y = interior_points[i].y;

            double value = mixinggradient(x, y, -1, 0, type) + mixinggradient(x, y, 1, 0, type) 
                        + mixinggradient(x, y, 0, -1, type) + mixinggradient(x, y, 0, 1, type);

            if (x - 1 >= 0 && !is_inside[y * width + (x - 1)])
                value += f(x - 1, y, type);
            if (x + 1 < width && !is_inside[y * width + (x + 1)])
                value += f(x + 1, y, type);
            if (y - 1 >= 0 && !is_inside[(y - 1) * width + x])
                value += f(x, y - 1, type);
            if (y + 1 < height && !is_inside[(y + 1) * width + x])
                value += f(x, y + 1, type);

            B[type](i) = value;
        }
    }
}

double MixingGradients::mixinggradient(int x, int y, int dx, int dy, int type)
{
    int neighbor_x = x + dx;
    int neighbor_y = y + dy;

    double src_grad = g(x, y, type) - g(neighbor_x, neighbor_y, type);
    double tar_grad = f(x, y, type) - f(neighbor_x, neighbor_y, type);

    if (std::abs(src_grad) > std::abs(tar_grad))
        return src_grad;
    else
        return tar_grad;
}
}  // namespace USTC_CG
