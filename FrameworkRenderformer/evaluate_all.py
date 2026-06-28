import torch
import torch.nn.functional as F
import math
import numpy as np
from pathlib import Path
from PIL import Image
from torch.utils.data import DataLoader
from baseline_model import CourseRenderFormerWrapper, build_baseline_config
from baseline_data import PtSceneDataset, renderformer_baseline_collate
from train_course_baseline import move_to_device
from local_renderformer.models.config import RenderFormerConfig

# ===== 测试数据 =====
test_data_path = "data/test_datalow"
out_dir = Path("runs/evaluation_all")
out_dir.mkdir(parents=True, exist_ok=True)
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# ===== 三个模型的配置与 checkpoint =====
models_to_eval = [
    {
        "name": "RoPE_DPT_20000",
        "checkpoint": "runs/experiment/checkpoints/step_20000.pt",
        "use_dpt": True,
        "pe_type": "rope",
    },
    {
        "name": "RoPE_NoDPT_10000",
        "checkpoint": "runs/ablation_no_dpt/checkpoints/step_10000.pt",
        "use_dpt": False,
        "pe_type": "rope",
    },
    {
        "name": "NeRF_DPT_10000",
        "checkpoint": "runs/ablation_nerf_pe/checkpoints/step_10000.pt",
        "use_dpt": True,
        "pe_type": "nerf",
    },
]

# 保存所有结果的字典
all_results = {}

def save_comparison(pred, gt, path):
    """保存 GT、预测、误差图（5倍放大）的拼接图"""
    # GT 对数映射
    gt_img = gt[0].cpu()
    gt_img = torch.log1p(gt_img) / math.log1p(50)
    gt_img = torch.clamp(gt_img, 0, 1)
    gt_img = torch.pow(gt_img, 1 / 2.2)
    gt_img = gt_img.permute(1, 2, 0).numpy()
    gt_img = np.clip(gt_img * 255, 0, 255).astype(np.uint8)

    # 预测 对数映射
    pred_img = pred[0].cpu()
    pred_img = torch.log1p(pred_img) / math.log1p(50)
    pred_img = torch.clamp(pred_img, 0, 1)
    pred_img = torch.pow(pred_img, 1 / 2.2)
    pred_img = pred_img.permute(1, 2, 0).numpy()
    pred_img = np.clip(pred_img * 255, 0, 255).astype(np.uint8)

    # 误差图 (放大5倍)
    diff = torch.abs(pred[0] - gt[0]).cpu()
    diff = torch.clamp(diff * 5, 0, 1)
    diff_img = diff.permute(1, 2, 0).numpy()
    diff_img = np.clip(diff_img * 255, 0, 255).astype(np.uint8)

    # 拼接：GT | 预测 | 误差图
    canvas = np.concatenate([gt_img, pred_img, diff_img], axis=1)
    Image.fromarray(canvas).save(path)

for model_info in models_to_eval:
    print(f"\n{'='*60}")
    print(f"Evaluating: {model_info['name']}")
    print(f"{'='*60}")

    # 构建配置对象，直接传入 pe_type
    config = RenderFormerConfig(
        latent_dim=256,
        num_layers=4,
        num_heads=4,
        dim_feedforward=1024,
        num_register_tokens=4,
        pe_type=model_info["pe_type"],
        vertex_pe_num_freqs=6,
        use_vn_encoder=True,
        vn_pe_num_freqs=6,
        texture_encode_patch_size=1,
        texture_channels=10,
        view_transformer_latent_dim=256,
        view_transformer_ffn_hidden_dim=1024,
        view_transformer_n_heads=4,
        view_transformer_n_layers=6,
        patch_size=8,
        use_dpt_decoder=model_info["use_dpt"],
        turn_to_cam_coord=True,
        use_ldr=False,
        ffn_opt="checkpoint",
    )

    model = CourseRenderFormerWrapper(config).to(device)
    ckpt = torch.load(model_info["checkpoint"], map_location=device, weights_only=False)
    model.load_state_dict(ckpt["model_state_dict"])
    model.eval()

    dataset = PtSceneDataset(test_data_path)
    loader = DataLoader(dataset, batch_size=1, shuffle=False,
                        collate_fn=renderformer_baseline_collate)

    psnr_list = []
    for idx, batch in enumerate(loader):
        batch = move_to_device(batch, device)
        gt = batch["gt_image"].float()

        with torch.no_grad():
            pred = model(batch)

        # 直接使用线性 HDR，负值截断到 0
        pred_linear = torch.clamp(pred, min=0.0)
        gt_linear = gt

        mse = F.mse_loss(pred_linear, gt_linear)
        psnr = 10 * torch.log10(1.0 / (mse + 1e-8)).item()
        psnr_list.append(psnr)

        # 为每个模型的每个样本保存对比图
        model_out_dir = out_dir / model_info['name']
        model_out_dir.mkdir(parents=True, exist_ok=True)
        save_path = model_out_dir / f"sample_{idx:03d}.png"
        save_comparison(pred_linear, gt_linear, save_path)

        print(f"  Sample {idx}: PSNR = {psnr:.2f} dB")

    avg_psnr = np.mean(psnr_list)
    std_psnr = np.std(psnr_list)
    print(f"  Average PSNR = {avg_psnr:.2f} dB ± {std_psnr:.2f}")
    all_results[model_info['name']] = {
        "avg_psnr": avg_psnr,
        "std_psnr": std_psnr,
        "per_sample": psnr_list
    }

# ===== 输出对比表格 =====
print(f"\n{'='*60}")
print("Summary Comparison")
print(f"{'='*60}")
print(f"{'Model':<25} {'Avg PSNR':>10} {'Std':>8}")
print("-" * 45)
for name, result in all_results.items():
    print(f"{name:<25} {result['avg_psnr']:>8.2f} dB {result['std_psnr']:>7.2f}")