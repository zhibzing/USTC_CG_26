#include "clone.h"
#include "common/image_widget.h"
#include "../source_image_widget.h"
#include "../target_image_widget.h"
#include "Eigen/Sparse"

#include <vector>

namespace USTC_CG
{
void Clone::update(std::shared_ptr<SourceImageWidget> src, std::shared_ptr<Image> tar, 
                std::shared_ptr<Image> mask, ImVec2 tar_position)
{
    src_img_ = src->get_data();
    tar_img_ = std::make_shared<Image>(*tar);
    src_selected_mask_ = mask;

    offset_x_ = tar_position.x - src->get_position().x;
    offset_y_ = tar_position.y - src->get_position().y;

    width = mask->width();
    height = mask->height();
    tar_width = tar_img_->width();
    tar_height = tar_img_->height();

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            int tar_x = x + offset_x_;
            int tar_y = y + offset_y_;
            if (mask->get_pixel(x, y)[0] > 0 && 
                tar_x >= 0 && tar_x < tar_width && tar_y >= 0 && tar_y < tar_height)
            {
                int index = y * width + x;
                interior_points[mask_size] = ImVec2(x, y);
                pos_to_index[index] = mask_size;
                mask_size++;
            }
        }
    }

    cachesource();
    cachetarget();
}

void Clone::cachesource()
{
    if (is_src_cached) 
        return;
        
    src_cache_.resize(width * height * 3);
        
    #pragma omp parallel for
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            auto pixel = src_img_->get_pixel(x, y);
            int index = (y * width + x) * 3;
            src_cache_[index] = pixel[0];
            src_cache_[index + 1] = pixel[1];
            src_cache_[index + 2] = pixel[2];
        }
    }
    is_src_cached = true;
}

void Clone::cachetarget()
{
    if (is_tar_cached) 
        return;
    
    tar_cache_.resize(tar_width * tar_height * 3);
        
    #pragma omp parallel for
    for  (int y = 0; y < tar_height; y++){
        for  (int x = 0; x < tar_width; x++){
            auto pixel = tar_img_->get_pixel(x, y);
            int index = (y * tar_width + x) * 3;
            tar_cache_[index] = pixel[0];
            tar_cache_[index + 1] = pixel[1];
            tar_cache_[index + 2] = pixel[2];
        }
    }
    is_tar_cached = true;
}

unsigned char Clone::f(int x, int y, int type)
{
    int target_x = x + offset_x_;
    int target_y = y + offset_y_;
    if (target_x < 0)
        target_x = 0;
    if (target_x >= tar_width)
        target_x = tar_width - 1;
    if (target_y < 0)
        target_y = 0;
    if (target_y >= tar_height)
        target_y = tar_height - 1;

    return tar_cache_[3 * (target_y * tar_width + target_x) + type];
}

unsigned char Clone::g(int x, int y, int type)
{   
    int source_x = x;
    int source_y = y;
    if (source_x < 0)
        source_x = 0;
    if (source_x >= width)
        source_x = width - 1;
    if (source_y < 0)
        source_y = 0;
    if (source_y >= height)
        source_y = height - 1;
    
    return src_cache_[3 * (source_y * width + source_x) + type];
}
}  // namespace USTC_CG
