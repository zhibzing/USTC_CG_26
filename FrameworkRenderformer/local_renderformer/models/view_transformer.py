import torch
import torch.nn as nn
import torch.nn.functional as F

from .config import RenderFormerConfig
from ..encodings.nerf_encoding import NeRFEncoding
from ..layers.attention import TransformerDecoder
from ..layers.dpt import DPTHead
from ..compat_einops import rearrange


class ViewTransformer(nn.Module):
    def __init__(self, config: RenderFormerConfig):
        super().__init__()
        self.config = config
        if config.pe_type == 'nerf':
            self.pos_pe = NeRFEncoding(
                in_dim=9,
                num_frequencies=config.vertex_pe_num_freqs,
                include_input=True
            )
            self.pe_token_proj = nn.Linear(
                self.pos_pe.get_out_dim(),
                config.view_transformer_latent_dim
            )
            if config.norm_type == 'layer_norm':
                self.token_pos_pe_norm = nn.LayerNorm(config.view_transformer_latent_dim)
            elif config.norm_type == 'rms_norm':
                self.token_pos_pe_norm = nn.RMSNorm(config.view_transformer_latent_dim)
            else:
                raise ValueError(f"Unsupported normalization type: {config.norm_type}")
            self.rope_dim = None

            # 2D position encoding (unused for RoPE)
            self.ray_patch_pos_pe = NeRFEncoding(
                in_dim=2,
                num_frequencies=config.vn_pe_num_freqs,
                include_input=True
            )
            self.ray_patch_pos_proj = nn.Linear(
                self.ray_patch_pos_pe.get_out_dim(),
                config.view_transformer_latent_dim
            )
            if config.norm_type == 'layer_norm':
                self.ray_patch_pos_norm = nn.LayerNorm(config.view_transformer_latent_dim)
            elif config.norm_type == 'rms_norm':
                self.ray_patch_pos_norm = nn.RMSNorm(config.view_transformer_latent_dim)
            else:
                raise ValueError(f"Unsupported normalization type: {config.norm_type}")

        elif config.pe_type == 'rope':
            self.rope_dim = min(
                config.vertex_pe_num_freqs,
                config.view_transformer_latent_dim // config.view_transformer_n_heads // 18 * 2
            )
            # No absolute PE modules needed
            self.pos_pe = None
            self.pe_token_proj = None
            self.token_pos_pe_norm = None
            self.ray_patch_pos_pe = None
            self.ray_patch_pos_proj = None
            self.ray_patch_pos_norm = None
        else:
            raise ValueError(f"Unsupported positional encoding type: {config.pe_type}")

        self.ray_map_patch_token = nn.Parameter(torch.zeros(1, 1, config.view_transformer_latent_dim))

        if config.vdir_pe_type == 'nerf':
            self.vdir_pe = NeRFEncoding(
                in_dim=3,
                num_frequencies=config.vdir_num_freqs,
                include_input=True
            )
            self.ray_map_encoder = nn.Linear(
                self.vdir_pe.get_out_dim() * config.patch_size * config.patch_size,
                config.view_transformer_latent_dim
            )
            if config.norm_type == 'layer_norm':
                self.ray_map_encoder_norm = nn.LayerNorm(config.view_transformer_latent_dim)
            elif config.norm_type == 'rms_norm':
                self.ray_map_encoder_norm = nn.RMSNorm(config.view_transformer_latent_dim)
            else:
                raise ValueError(f"Unsupported normalization type: {config.norm_type}")
        else:
            raise ValueError(f"Unsupported view direction positional encoding type: {config.vdir_pe_type}")

        self.transformer = TransformerDecoder(
            num_layers=self.config.view_transformer_n_layers,
            num_heads=self.config.view_transformer_n_heads,
            hidden_dim=self.config.view_transformer_latent_dim,
            ctx_dim=self.config.latent_dim,
            ffn_hidden_dim=self.config.view_transformer_ffn_hidden_dim,
            dropout=self.config.dropout,
            activation=self.config.activation,
            norm_type=self.config.norm_type,
            norm_first=self.config.norm_first,
            rope_dim=self.rope_dim,
            rope_type=self.config.rope_type,
            rope_double_max_freq=self.config.rope_double_max_freq,
            qk_norm=self.config.qk_norm,
            bias=self.config.bias,
            include_self_attn=self.config.view_transformer_include_self_attn,
            use_swin_attn=self.config.view_transformer_use_swin_attn,
            ffn_opt=self.config.ffn_opt
        )
        if not config.use_dpt_decoder:
            self.out_proj = nn.Linear(
                self.config.view_transformer_latent_dim,
                self.config.patch_size * self.config.patch_size * (4 if config.include_alpha else 3)
            )
        else:
            self.out_dpt = DPTHead(
                in_channels=self.config.view_transformer_latent_dim,
                features=self.config.dpt_features,
                out_channels=self.config.dpt_out_channels,
                out_dim=4 if config.include_alpha else 3
            )
            self.out_layers = list(
                range(self.config.view_transformer_n_layers - 4, self.config.view_transformer_n_layers)
            ) if self.config.dpt_out_layers is None else self.config.dpt_out_layers

        self.out_proj_act = nn.Identity()

    def forward(self, camera_o, ray_map, tri_tokens, tri_pos, valid_mask, tf32_mode=False):
        B, H, W, _ = ray_map.shape
        P = self.config.patch_size
        patch_h, patch_w = H // P, W // P

        # 1. NeRF PE on ray directions
        vdir_encoded = self.vdir_pe(ray_map)  # [B, H, W, C]

        # 2. Unfold into patches
        B, H_img, W_img, C = vdir_encoded.shape
        vdir_encoded = vdir_encoded.permute(0, 3, 1, 2)          # [B, C, H, W]
        patches = F.unfold(vdir_encoded, kernel_size=P, stride=P) # [B, C*P*P, N]
        flattened = patches.permute(0, 2, 1).contiguous()         # [B, N, C*P*P]

        # 3. Linear projection + normalization + learnable patch token
        ray_tokens = self.ray_map_encoder_norm(self.ray_map_encoder(flattened))
        ray_tokens = ray_tokens + self.ray_map_patch_token

        # 4. Prepare per-patch position for RoPE (or absolute PE)
        if self.config.pe_type == 'rope':
            # Generate 2D normalized grid coordinates for each patch ([-1, 1] range)
            ys = torch.linspace(-1, 1, patch_h, device=ray_tokens.device)
            xs = torch.linspace(-1, 1, patch_w, device=ray_tokens.device)
            grid_y, grid_x = torch.meshgrid(ys, xs, indexing='ij')  # (patch_h, patch_w)
            grid = torch.stack([grid_x, grid_y], dim=-1)            # (patch_h, patch_w, 2)
            grid = grid.reshape(1, patch_h * patch_w, 2).expand(B, -1, -1)  # (B, N, 2)

            # Extend to 9D to match triangle position format: (x, y, 0) repeated 3 times
            z = torch.zeros_like(grid[..., :1])  # (B, N, 1)
            pos_3d = torch.cat([grid, z], dim=-1)  # (B, N, 3)
            ray_token_pos = pos_3d  # (B, N, 3)

        elif self.config.pe_type == 'nerf':
            # All patches share camera origin, expand to 9D for absolute PE
            ray_token_pos = camera_o.unsqueeze(1).expand(-1, ray_tokens.size(1), -1)  # [B, N, 3]
            ray_pos_enc_input = ray_token_pos.repeat(1, 1, 3)  # [B, N, 9]
            ray_pos_enc = self.token_pos_pe_norm(self.pe_token_proj(self.pos_pe(ray_pos_enc_input)))
            ray_tokens = ray_tokens + ray_pos_enc

            # Add absolute PE to triangle tokens in camera space
            tri_pos_enc = self.token_pos_pe_norm(self.pe_token_proj(self.pos_pe(tri_pos)))
            tri_tokens = tri_tokens + tri_pos_enc  # new tensor

            # Pass camera origin for RoPE (won't be used because rope_dim=None)
            ray_token_pos = camera_o.unsqueeze(1).expand(-1, ray_tokens.size(1), -1)
        else:
            raise ValueError("Unknown pe_type")

        # Cross-attention decoder
        if self.config.use_dpt_decoder:
            with torch.autocast(device_type="cuda", dtype=torch.float32 if tf32_mode else torch.bfloat16):
                out_features = self.transformer(
                    ray_tokens, tri_tokens,
                    src_key_padding_mask=valid_mask,
                    triangle_pos=tri_pos,
                    ray_pos=ray_token_pos,
                    out_layers=self.out_layers,
                    tf32_mode=tf32_mode,
                    patch_h=patch_h, patch_w=patch_w
                )
            decoded_img = self.out_dpt(out_features, patch_h, patch_w, patch_size=self.config.patch_size)
            return self.out_proj_act(decoded_img)
        else:
            seq = self.transformer(
                ray_tokens, tri_tokens,
                src_key_padding_mask=valid_mask,
                triangle_pos=tri_pos,
                ray_pos=ray_token_pos,
                tf32_mode=tf32_mode,
                patch_h=patch_h, patch_w=patch_w
            )  # [B, N, D]
            decoded_patches = self.out_proj_act(self.out_proj(seq))  # [B, N, C_out*P*P]

            # Rebuild image using fold
            B_batch, N_patches, _ = decoded_patches.shape
            C_out = decoded_patches.shape[-1] // (P * P)
            decoded_patches = decoded_patches.permute(0, 2, 1)       # [B, C_out*P*P, N]
            decoded_img = F.fold(
                decoded_patches,
                output_size=(H, W),
                kernel_size=P,
                stride=P,
            )  # [B, C_out, H, W]
            return decoded_img
