import pandas as pd
import matplotlib.pyplot as plt
import os

def plot_results(log_dir):
    # Catatan: Script ini mengasumsikan Anda mengekspor data TensorBoard ke CSV 
    # atau menggunakan Monitor wrapper dari Stable Baselines3
    try:
        # Mencari file log di folder tensorboard
        # Untuk implementasi simpel, kita asumsikan ada file 'progress.csv'
        data = pd.read_csv(f"{log_dir}/progress.csv")
        
        plt.figure(figsize=(10, 5))
        plt.plot(data['iterations'], data['ep_rew_mean'], label='Average Reward')
        plt.title('Mecanum RL Learning Curve', fontsize=14)
        plt.xlabel('Iterations', fontsize=12)
        plt.ylabel('Mean Reward', fontsize=12)
        plt.grid(True, linestyle='--', alpha=0.6)
        plt.legend()
        
        plt.savefig('training_curve.png')
        plt.show()
        print("Grafik training berhasil disimpan sebagai 'training_curve.png'")
    except FileNotFoundError:
        print("Error: File log tidak ditemukan. Pastikan training sudah berjalan.")

if __name__ == "__main__":
    plot_results("../agents/mecanum_tensorboard")