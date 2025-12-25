import time
import serial
import pandas as pd
from sklearn.linear_model import LinearRegression

# ==== SESUAIKAN ====
PORT = "COM3"
BAUD = 115200

# ==== DATA LATIH SEDERHANA (boleh kamu ganti pakai data eksperimen sendiri) ====
data = {
    "PWM": [50, 80, 120, 160, 200, 230],
    "RPM": [260, 410, 610, 780, 950, 1020]
}
df = pd.DataFrame(data)

# ==== TRAIN MODEL ====
model = LinearRegression()
model.fit(df[["PWM"]], df["RPM"])
print("Model AI trained (Linear Regression)")
print(f"Model: RPM ≈ {model.coef_[0]:.3f}*PWM + {model.intercept_:.3f}\n")

# ==== CONNECT SERIAL ====
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)
print(f"Connected to {ser.name}")
print("Membaca data dari ESP32 format: PWM,RPM")
print("Tekan Ctrl+C untuk berhenti.\n")

try:
    while True:
        line = ser.readline().decode(errors="ignore").strip()
        if not line:
            continue

        # skip header
        if line.lower().startswith("pwm"):
            continue

        if "," in line:
            parts = line.split(",")
            if len(parts) >= 2:
                try:
                    pwm = int(float(parts[0].strip()))
                    actual_rpm = float(parts[1].strip())

                    predicted_rpm = float(model.predict([[pwm]])[0])

                    # Deteksi abnormal sederhana
                    status = "NORMAL"
                    if actual_rpm < 0.7 * predicted_rpm:
                        status = "ABNORMAL / OVERLOAD"

                    print(f"PWM={pwm:3d} | RPM_actual={actual_rpm:8.2f} | RPM_pred={predicted_rpm:8.2f} | {status}")

                except:
                    pass

        time.sleep(0.05)

except KeyboardInterrupt:
    print("\nStop.")

finally:
    try:
        ser.close()
    except:
        pass
