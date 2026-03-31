import torch
from forward_noising import (
    get_index_from_list,
    sqrt_alphas_cumprod,
    sqrt_one_minus_alphas_cumprod,
    betas,
    posterior_variance,
    sqrt_recip_alphas,
    forward_diffusion_sample,
)
import matplotlib.pyplot as plt
from dataloader import show_tensor_image
from unet import SimpleUnet
import numpy as np
import cv2 as cv


# TODO: 你需要在这个函数中实现单步去噪过程
@torch.no_grad()
def sample_timestep(model, x, t):
    beta_t = get_index_from_list(betas, t, x.shape)
    sqrt_recip_alpha_t = get_index_from_list(sqrt_recip_alphas, t, x.shape)
    sqrt_one_minus_alphas_cumprod_t = get_index_from_list(
        sqrt_one_minus_alphas_cumprod, t, x.shape
    )
    posterior_variance_t = get_index_from_list(posterior_variance, t, x.shape)

    eps_pred = model(x, t)
    
    model_mean = sqrt_recip_alpha_t * (
        x - (beta_t * eps_pred) / sqrt_one_minus_alphas_cumprod_t
    )

    if t.item() == 0:
        res = model_mean
    else:
        noise = torch.randn_like(x)
        res = model_mean + 0.1 * torch.sqrt(posterior_variance_t) * noise

    return torch.clamp(res, -1.0, 1.0)

# TODO: 你需要在这个函数中完成对纯高斯噪声的去噪，并输出对应的去噪图片
# 你需要调用上面的sample_timestep函数，以实现单步去噪
@torch.no_grad()
def sample_plot_image(model, device, img_size, T):
    model.eval()
    x = torch.randn((1, 3, img_size, img_size), device=device)
    
    print(f"Initial noise - min: {x.min():.2f}, max: {x.max():.2f}, mean: {x.mean():.2f}")

    for t in reversed(range(T)):
        t_tensor = torch.full((1,), t, device=device, dtype=torch.long)
        x = sample_timestep(model, x, t_tensor)
        x = torch.clamp(x, -1.0, 1.0)
        
        if t % 50 == 0:
            print(f"Step {t} - min: {x.min():.2f}, max: {x.max():.2f}, mean: {x.mean():.2f}")

    print(f"Final image - min: {x.min():.2f}, max: {x.max():.2f}, mean: {x.mean():.2f}")
    return x


# TODO: 你需要在这个函数中完成模型以及其他相关资源的加载，并调用sample_plot_image进行去噪，以生成图片
def test_image_generation():
    T = 300
    img_size = 256
    t_noise = 100
    device = "cuda" if torch.cuda.is_available() else "cpu"
    
    model = SimpleUnet().to(device)
    checkpoint = torch.load("./ddpm_mse_epochs_100.pth", map_location=device)
    model.load_state_dict(checkpoint)
    model.eval()

    import os
    dataset_path = "./datasets-1/train"
    img_path = None
    for root, dirs, files in os.walk(dataset_path):
        for file in files:
            if file.lower().endswith(('.jpg', '.png', '.jpeg')):
                img_path = os.path.join(root, file)
                break
        if img_path:
            break
    if not img_path:
        print("未找到图像！")
        return

    img = cv.imread(img_path)
    img = cv.cvtColor(img, cv.COLOR_BGR2RGB)
    img = cv.resize(img, (img_size, img_size))
    img_tensor = torch.from_numpy(img).float().permute(2, 0, 1) / 255.0
    img_tensor = img_tensor * 2 - 1
    img_tensor = img_tensor.unsqueeze(0).to(device)

    t_tensor = torch.tensor([t_noise], device=device)
    noisy_img, _ = forward_diffusion_sample(img_tensor, t_tensor, device)

    x = noisy_img
    for t in reversed(range(0, t_noise + 1)):
        t_step = torch.full((1,), t, device=device, dtype=torch.long)
        x = sample_timestep(model, x, t_step)
        x = torch.clamp(x, -1.0, 1.0)

    plt.figure(figsize=(15,5))
    plt.subplot(131), plt.title("Original"), show_tensor_image(img_tensor.cpu())
    plt.subplot(132), plt.title(f"Noisy t={t_noise}"), show_tensor_image(noisy_img.cpu())
    plt.subplot(133), plt.title("Restored"), show_tensor_image(x.cpu())
    plt.tight_layout()
    plt.show()

    return x


# TODO：你需要在这个函数中实现图像的补充
# Follows: RePaint: Inpainting using Denoising Diffusion Probabilistic Models
@torch.no_grad()
def inpaint(model, device, img, mask):
    pass

def test_image_inpainting():
    pass

if __name__ == "__main__":
    test_image_generation()
    test_image_inpainting()
