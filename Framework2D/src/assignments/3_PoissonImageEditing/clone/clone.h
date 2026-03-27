#pragma once

#include "../source_image_widget.h"
#include "Eigen/Dense"

#include <vector>
#include <map>

namespace USTC_CG
{
class Clone
{
   public:
    virtual ~Clone() = default;

    // solve Poisson equation,return a result image
    // (same size as target image which replaced the selected region)
    virtual std::shared_ptr<Image> solve() = 0;
    void update(std::shared_ptr<SourceImageWidget>, std::shared_ptr<Image> tar, 
                std::shared_ptr<Image> mask, ImVec2 tar_position);

   protected:
    std::shared_ptr<Image> src_img_; // source image
    std::shared_ptr<Image> tar_img_; // target image
    std::shared_ptr<Image> src_selected_mask_; // select region mask

    int offset_x_, offset_y_; // position of select region mask in target image
    int width, height; // width and height of mask/source image
    int tar_width, tar_height; // width and height of target image

    int mask_size = 0; // num of points in select region
    std::unordered_map<int, ImVec2> interior_points; // set index for interior_points
    std::unordered_map<int, int> pos_to_index; // get index of pixel

    std::vector<unsigned char> src_cache_, tar_cache_; // cache source image and target image
    bool is_src_cached = false, is_tar_cached = false;

   protected:
    unsigned char f(int x, int y, int type); // get pixel value in target image
    unsigned char g(int x, int y, int type); // get pixel value in source image

   private:
    void cachesource();
    void cachetarget();
};
}