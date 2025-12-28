from stable_baselines3 import PPO
from mecanum_env import MecanumEnv
import time

def test():
    env = MecanumEnv()
    
    # Load model yang sudah dilatih
    model = PPO.load("../models/mecanum_ppo_final")
    
    obs, _ = env.reset()
    print("Menjalankan testing model...")
    
    for _ in range(1000):
        # Model memprediksi aksi terbaik berdasarkan observasi saat ini
        action, _states = model.predict(obs, deterministic=True)
        obs, reward, terminated, truncated, info = env.step(action)
        
        if terminated or truncated:
            print("Target tercapai atau keluar batas! Resetting...")
            obs, _ = env.reset()
            time.sleep(1)
            
    env.close()

if __name__ == "__main__":
    test()