import gymnasium as gym
import numpy as np
import socket
import time
from gymnasium import spaces

class MecanumEnv(gym.Env):
    def __init__(self):
        super(MecanumEnv, self).__init__()
        
        # Action Space: [vx, vy, omega] 
        # Kecepatan linear dan angular antara -1.0 sampai 1.0
        self.action_space = spaces.Box(low=-1.0, high=1.0, shape=(3,), dtype=np.float32)
        
        # Observation Space: [x, y, theta, target_x, target_y]
        self.observation_space = spaces.Box(low=-10.0, high=10.0, shape=(5,), dtype=np.float32)
        
        # Koneksi Socket
        self.host = '127.0.0.1'
        self.port = 8080
        self.sock = None

    def _get_connection(self):
        if self.sock is None:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(5.0)
            try:
                self.sock.connect((self.host, self.port))
            except ConnectionRefusedError:
                print("Error: Pastikan simulasi C (mecanum_sim.exe) sudah berjalan!")
                raise
        return self.sock

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        sock = self._get_connection()
        sock.sendall("reset".encode())
        
        # Terima state awal dari C
        data = sock.recv(1024).decode().split(',')
        obs = np.array(data, dtype=np.float32)
        return obs, {}

    def step(self, action):
        sock = self._get_connection()
        
        # Kirim aksi ke C: "control vx vy omega"
        msg = f"control {action[0]} {action[1]} {action[2]}"
        sock.sendall(msg.encode())
        
        # Terima state baru setelah aksi dilakukan
        try:
            data = sock.recv(1024).decode().split(',')
            obs = np.array(data, dtype=np.float32)
        except:
            obs = np.zeros(5, dtype=np.float32)

        # --- LOGIKA REWARD (Reward Function) ---
        # 1. Hitung jarak Euclidean ke target
        current_pos = obs[0:2]
        target_pos = obs[3:5]
        dist = np.linalg.norm(current_pos - target_pos)
        
        # 2. Reward negatif berdasarkan jarak (semakin dekat, penalti semakin kecil)
        reward = -dist * 0.1
        
        # 3. Bonus jika mencapai target
        terminated = False
        if dist < 0.2:
            reward += 100.0
            terminated = True
            
        # 4. Penalti jika keluar batas (misal area simulasi -5 sampai 5)
        if np.any(np.abs(current_pos) > 4.8):
            reward -= 50.0
            terminated = True

        truncated = False
        return obs, reward, terminated, truncated, {}

    def close(self):
        if self.sock:
            self.sock.close()
            self.sock = None