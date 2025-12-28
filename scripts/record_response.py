import socket
import pandas as pd
import time

def record_step_response(filename, duration=10):
    host, port = '127.0.0.1', 8080
    results = []
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    
    print(f"Merekam respons selama {duration} detik...")
    start_time = time.time()
    
    while (time.time() - start_time) < duration:
        # Kirim perintah konstan (Step Input)
        sock.sendall("control 1.0 0.0 0.0".encode())
        
        # Terima posisi saat ini
        data = sock.recv(1024).decode().split(',')
        curr_time = time.time() - start_time
        
        # Simpan waktu, target_x (1.0), dan posisi_x aktual
        results.append([curr_time, 1.0, float(data[0])])
        time.sleep(0.05)
        
    df = pd.DataFrame(results, columns=['Time', 'Target', 'Actual'])
    df.to_csv(f"../data/{filename}", index=False)
    print(f"Data tersimpan di data/{filename}")
    sock.close()

if __name__ == "__main__":
    record_step_response("mecanum_rl_response.csv")