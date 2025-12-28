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
        # Kita tetap menggunakan 5 observasi agar agen fokus pada koordinat
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
        
        # Mengirim perintah reset ke simulasi C
        sock.sendall("reset".encode())
        
        # Terima state awal (C mengirim 6 data, kita ambil 5 untuk observasi)
        try:
            raw_data = sock.recv(1024).decode().split(',')
            # Ambil x, y, theta, target_x, target_y
            obs = np.array(raw_data[:5], dtype=np.float32)
        except Exception as e:
            print(f"Reset Error: {e}")
            obs = np.zeros(5, dtype=np.float32)
            
        return obs, {}

    def step(self, action):
        sock = self._get_connection()
        
        # 1. Kirim aksi ke C: "control vx vy omega"
        msg = f"control {action[0]} {action[1]} {action[2]}"
        sock.sendall(msg.encode())
        
        # 2. Terima state baru (Data: x, y, theta, tx, ty, collision)
        try:
            raw_data = sock.recv(1024).decode().split(',')
            
            # Parsing 5 data pertama untuk observasi agen
            obs = np.array(raw_data[:5], dtype=np.float32)
            
            # Parsing data ke-6 untuk status tabrakan (0 atau 1)
            collision = int(raw_data[5])
        except Exception as e:
            # Fallback jika terjadi error pembacaan socket
            obs = np.zeros(5, dtype=np.float32)
            collision = 0

        # --- LOGIKA REWARD (Reward Function) ---
        current_pos = obs[0:2]
        target_pos = obs[3:5]
        dist = np.linalg.norm(current_pos - target_pos)
        
        # A. Reward dasar: Penalti jarak (semakin dekat, penalti semakin kecil)
        reward = -dist * 0.1
        
        terminated = False
        truncated = False

        # B. Bonus jika mencapai target
        if dist < 0.25:
            reward += 150.0 # Bonus sukses
            terminated = True
            print("--- Target Tercapai! ---")

        # C. PENALTI TABRAKAN (Fitur Baru)
        if collision == 1:
            reward -= 200.0   # Hukuman berat agar agen menghindari rintangan
            terminated = True # Episode berakhir karena robot "rusak"
            print("--- TABRAKAN! Robot Menabrak Rintangan ---")
            
        # D. Penalti jika keluar batas simulasi
        if np.any(np.abs(current_pos) > 4.8):
            reward -= 100.0
            terminated = True
            print("--- Keluar Batas! ---")

        return obs, reward, terminated, truncated, {}

    def close(self):
        if self.sock:
            self.sock.close()
            self.sock = None