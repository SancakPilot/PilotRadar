# 🛰️ PILOT RADAR

[English](#english) | [Türkçe](#türkçe)

---

# English

## 🚀 Experience the Ultimate Tactical Wi-Fi & Compass Radar!
**PilotRadar** is a cutting-edge, real-time tactical Wi-Fi scanner and motion-sensing compass radar developed exclusively for the **M5StickS3 (ESP32-S3)**. By blending high-speed asynchronous Wi-Fi scanning with advanced Inertial Measurement Unit (IMU) sensor fusion, PilotRadar transforms your pocket-sized M5StickS3 into an interactive reconnaissance dashboard!

Track signal sources, monitor device stability, detect human or object movement via Wi-Fi signal disruptions (Wi-Fi Sensing), and watch the radar dial align dynamically with the physical world!

---

## ✨ Outstanding Features
* **🛰️ Left-Side Tactical Radar:** A beautifully animated circular radar sweep that dynamically adjusts its sweeps based on physical orientation.
* **🧭 Dynamic Compass Integration:** A bright orange **"N" (North)** marker rotates fluidly around the radar grid according to physical orientation.
* **📡 Wi-Fi Sensing & Motion Alerts:** Monitors target Wi-Fi RSSI fluctuations. If a sudden signal change is detected (due to physical objects or human movement blocking the path), a sound alert is triggered and the target displays in vibrant red.
* **🔴🔵 Device Stability Guardian:** Detects whether you are holding the device steady or rotating it, displaying real-time state alerts (`DEV: STEADY` / `DEV: UNSTABLE`).
* **📟 Terminal Log Panel:** A dedicated terminal box outlined with a thin orange frame where system events and target alerts flow in bright cyan.
* **⚙️ Manual Gyro Calibration:** Calibration UI upon startup to align your compass, with the ability to recalibrate at any time using a long press.

---

## 🧭 How It Works & Control Guide

### 1. Startup Calibration
When powered on, PilotRadar enters **Calibration Mode** to establish its orientation:
1. Align the device's physical front towards **North** (using your phone or a real compass).
2. The red **"N"** on the screen rotates with the gyroscope. Align it with the red fixed pointer at the bottom.
3. Click **Button B** once. The device beeps to confirm, zeroes the gyroscope, and switches to the Radar interface.
4. *To recalibrate at any time:* **Long-press Button B for 1 second** on the main radar screen.

### 2. General Controls
* **Button A (Short Click):** Opens/closes the target selection menu. Use this to select a specific Wi-Fi SSID to track or set it to `AUTO` (automatically tracks the strongest nearby network).
* **Button B (Short Click):** While in the menu, cycles forward through detected networks.
* **Button B (Double Click within 800ms):** While in the menu, cycles backward (upwards) through detected networks.

---

## 🧠 Technical Architecture & Math

### 🛰️ Compass Navigation Mathematics
Because the M5StickS3 IMU lacks a physical magnetometer, heading is calculated by integrating the gyroscope's Z-axis angular velocity over time:
$$\theta_{\text{North}} = -90^\circ - \text{imuYaw}$$
$$x = CX + \cos(\theta_{\text{North}} \times \frac{\pi}{180}) \times (R - 7)$$
$$y = CY + \sin(\theta_{\text{North}} \times \frac{\pi}{180}) \times (R - 7)$$

### 📡 Wi-Fi Signal Smoothing (EMA Filter)
To eliminate ambient noise from Wi-Fi signals, we implement an **Exponential Moving Average (EMA)** filter to calculate signal fluctuations:
$$\text{Change}_{\text{smoothed}} = (\text{Change}_{\text{smoothed}} \times 0.6) + (\text{Change}_{\text{instant}} \times 0.4)$$
If $\text{Change}_{\text{smoothed}} > 2.5\text{ dBm}$, a motion alert is registered and logged on the terminal with its heading and proximity:
`M: 180d | Close` (Movement detected at 180° heading, close distance).

---

# Türkçe

## 🚀 Taktiksel Wi-Fi ve Pusula Radarını Deneyimleyin!
**PilotRadar**, **M5StickS3 (ESP32-S3)** için özel olarak geliştirilmiş gerçek zamanlı, taktiksel bir Wi-Fi tarayıcı ve hareket duyarlı pusula radarıdır! Yüksek hızlı asenkron Wi-Fi taramasını gelişmiş Atalet Ölçüm Birimi (IMU) sensör füzyonuyla harmanlayan PilotRadar, avuç içi boyutundaki M5StickS3 cihazınızı etkileşimli bir keşif paneline dönüştürür!

Sinyal kaynaklarını izleyin, cihazınızın kararlılık durumunu takip edin, Wi-Fi sinyal bozulmalarından (Wi-Fi Sensing) yararlanarak insan veya nesne hareketlerini algılayın ve radar ekranının gerçek dünyayla dinamik olarak nasıl hizalandığını izleyin!

---

## ✨ Öne Çıkan Özellikler
* **🛰️ Sol Taraf Taktiksel Radar:** Cihazın fiziksel yönüne göre dönen, mükemmel şekilde animasyonlu yeşil dairesel radar ekranı.
* **🧭 Dinamik Pusula Entegrasyonu:** Parlak turuncu renkli **"N" (Kuzey)** göstergesi, cihaz döndükçe radar çemberinin etrafında akıcı bir şekilde konum değiştirir.
* **📡 Wi-Fi Sensing & Hareket Alarmları:** Hedef Wi-Fi ağının RSSI dalgalanmalarını izler. Sinyalde ani bir değişim olduğunda (örneğin aradan bir insan geçip sinyali kestiğinde) sesli alarm verir ve hedefi kırmızı renge boyar.
* **🔴🔵 Cihaz Kararlılık Koruması:** Cihazı sabit tutup tutmadığınızı algılayarak ekranda anlık durum bildirimi sunar (`DEV: STEADY` / `DEV: UNSTABLE`).
* **📟 Terminal Log Paneli:** Sağ tarafta konumlanmış, turuncu çerçeveli ve turkuaz (cyan) renkli akan taktiksel log ekranı.
* **⚙️ Manuel Jiroskop Kalibrasyonu:** Açılışta kuzey yönünü hassas olarak ayarlayabileceğiniz kalibrasyon arayüzü ve radar ekranındayken tek tuşla yeniden kalibre edebilme imkanı.

---

## 🧭 Çalışma Mantığı ve Kullanım Kılavuzu

### 1. Başlangıç Kalibrasyonu
Cihaz açıldığında yönünü sıfırlamak için **Kalibrasyon Modunda** başlar:
1. Cihazın fiziksel ön tarafını **Kuzey** yönüne çevirin (telefonunuzdaki pusuladan veya gerçek bir pusuladan yararlanabilirsiniz).
2. Ekranda dönen kırmızı **"N"** harfini, alt kısımdaki sabit duran kırmızı ok ucu ile hizalayın.
3. **BtnB** butonuna bir kez basın. Onay sesi çalınacak, jiroskop sıfırlanacak ve Radar arayüzü başlayacaktır.
4. *İstediğiniz zaman yeniden kalibre etmek için:* Radar ekranındayken **BtnB butonuna 1 saniye boyunca uzun basın**.

### 2. Genel Kontroller
* **Button A (Kısa Basış):** Hedef seçme menüsünü açar/kapatır. Menüden belirli bir Wi-Fi ağına kilitleyebilir (`Lock`) veya otomatik modda bırakabilirsiniz (`AUTO` - en güçlü ağı takip eder).
* **Button B (Kısa Basış):** Menüde gezinirken listedeki bir sonraki ağa geçer (aşağı kaydırır).
* **Button B (800ms içinde çift tıklama):** Menüde gezinirken bir önceki ağa geçer (yukarı kaydırır).

---

## 🧠 Teknik Mimari ve Matematiksel Altyapı

### 🛰️ Pusula Navigasyon Matematiği
M5StickS3 üzerinde fiziksel bir manyetometre bulunmadığından, yön hesabı jiroskop Z ekseninin zamana göre entegre edilmesiyle bulunur:
$$\theta_{\text{Kuzey}} = -90^\circ - \text{imuYaw}$$
$$x = CX + \cos(\theta_{\text{Kuzey}} \times \frac{\pi}{180}) \times (R - 7)$$
$$y = CY + \sin(\theta_{\text{Kuzey}} \times \frac{\pi}{180}) \times (R - 7)$$

### 📡 Sinyal Yumuşatma (EMA Filtresi)
Wi-Fi ortam gürültüsünü filtrelemek ve kararlı bir hareket algılama sunmak amacıyla **EMA (Üstel Hareketli Ortalama)** filtresi kullanılmıştır:
$$\text{Değişim}_{\text{yumuşatılmış}} = (\text{Değişim}_{\text{yumuşatılmış}} \times 0.6) + (\text{Değişim}_{\text{anlık}} \times 0.4)$$
Eğer $\text{Değişim}_{\text{yumuşatılmış}} > 2.5\text{ dBm}$ değeri aşılırsa hareket algılanır ve log paneline yön bilgisiyle birlikte yazdırılır:
`M: 180d | Close` (180° yönünde, yakın mesafede hareket tespit edildi).
