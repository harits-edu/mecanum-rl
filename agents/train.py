from stable_baselines3 import PPO
from mecanum_env import MecanumEnv
import os

def main():
    # Buat folder model jika belum ada
    model_dir = "../models"
    log_dir = "./mecanum_tensorboard"
    os.makedirs(model_dir, exist_ok=True)

    # Inisialisasi Environment
    env = MecanumEnv()

    # Inisialisasi Model PPO
    # MlpPolicy: Policy jaringan saraf standar (Multi-layer Perceptron)
    model = PPO("MlpPolicy", env, verbose=1, tensorboard_log=log_dir)

    print("--- Memulai Training Mecanum RL ---")
    # Training sebanyak 200.000 langkah
    model.learn(total_timesteps=200000, tb_log_name="PPO_Mecanum")

    # Simpan hasil training
    model.save(f"{model_dir}/mecanum_ppo_final")
    print(f"Training selesai. Model disimpan di {model_dir}")

    env.close()

if __name__ == "__main__":
    main()