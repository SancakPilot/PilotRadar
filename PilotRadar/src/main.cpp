#include <M5Unified.h>
#include <math.h>
#include <WiFi.h>

bool lastMovementState = false;

int menuScroll = 0;

String showSSID = "";
int showRSSI = 0;
float deviceAngle = 0;
String targetSSID = "";
int bestRSSI = -100;
unsigned long lastWifiScan = 0;
int lastRSSI = -100;
bool wifiMovement = false;
int movementPower = 0;
unsigned long lastGyroTime = 0;

float imuAngle = 0;

float gx, gy, gz;

M5Canvas canvas(&M5.Display);

constexpr int WIDTH  = 240;
constexpr int HEIGHT = 135;

constexpr int CX = WIDTH / 2;
constexpr int CY = HEIGHT / 2 + 5;

constexpr int RADAR_RADIUS = 58;

float sweepAngle = 0.0f;
float imuYaw = 0;
unsigned long lastImuTime = 0;


// ===== MENU =====
bool menuOpen = false;
int menuIndex = 0;

const int MAX_WIFI = 20;

String wifiList[MAX_WIFI];
int wifiRSSI[MAX_WIFI];

bool autoTarget = true;
String lockedSSID = "";

unsigned long btnBTime = 0;
bool waitingBtnB = false;

void drawRadarBackground()
{
 
    canvas.fillScreen(BLACK);
    canvas.setCursor(5,95);
canvas.print("WIFI:");
canvas.print(targetSSID);

canvas.setCursor(175,45);
canvas.setTextColor(GREEN);
canvas.setTextSize(1);

canvas.print("RSSI:");
canvas.print(bestRSSI);


canvas.setCursor(175,60);
canvas.print("PWR:");
canvas.print(movementPower);


canvas.setCursor(175,75);

if(wifiMovement)
{
    canvas.setTextColor(RED);
    canvas.print("MOVE");
}
else
{
    canvas.setTextColor(GREEN);
    canvas.print("CLEAR");
}

    canvas.setTextColor(CYAN);
    canvas.setTextSize(2);
    canvas.setCursor(10,5);
    canvas.println("PILOT RADAR");

    canvas.drawCircle(CX,CY,20,GREEN);
    canvas.drawCircle(CX,CY,40,GREEN);
    canvas.drawCircle(CX,CY,58,GREEN);

    canvas.drawLine(CX-58,CY,CX+58,CY,GREEN);
    canvas.drawLine(CX,CY-58,CX,CY+58,GREEN);

    canvas.fillCircle(CX,CY,3,GREEN);

    canvas.setTextSize(1);
    canvas.setCursor(8,120);
    canvas.setTextColor(GREEN);
    canvas.print("STATUS : SCANNING...");
}
void drawSweep(float angle)
{
    float r = angle * DEG_TO_RAD;

    int x = CX + cos(r) * RADAR_RADIUS;
    int y = CY + sin(r) * RADAR_RADIUS;

    canvas.drawLine(CX, CY, x, y, GREEN);
}

void drawProximity()
{
    if(bestRSSI > -100)
    {
        // RSSI -> yarıçap
        int radius = map(bestRSSI, -90, -30, 55, 8);

        radius = constrain(radius, 8, 55);

        if(wifiMovement)
        {
            canvas.drawCircle(CX, CY, radius, RED);
            canvas.fillCircle(CX, CY, 3, RED);
        }
        else
        {
            canvas.drawCircle(CX, CY, radius, GREEN);
        }
    }
}

void drawMenu()
{
    canvas.fillScreen(BLACK);

    canvas.setTextSize(2);
    canvas.setTextColor(GREEN);
    canvas.setCursor(10,5);
    canvas.println("SELECT TARGET");

    canvas.setTextSize(1);

    int y = 35 - menuScroll;

    // AUTO
    if(menuIndex == 0)
        canvas.setTextColor(YELLOW);
    else
        canvas.setTextColor(GREEN);

    canvas.setCursor(10,y);
    canvas.print("> AUTO");

    y += 15;

    for(int i = 0; i < MAX_WIFI; i++)
    {
        if(wifiList[i] == "")
            break;

        if(menuIndex == i + 1)
            canvas.setTextColor(YELLOW);
        else
            canvas.setTextColor(GREEN);

        canvas.setCursor(10,y);
        canvas.print(i + 1);
        canvas.print(". ");
        canvas.print(wifiList[i]);

        canvas.setCursor(170,y);
        canvas.print(wifiRSSI[i]);

        y += 15;
    }

    canvas.setTextColor(GREEN);
    canvas.setCursor(10,120);
    canvas.print("A:SELECT  B:NEXT");
}

void setup()
{
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Speaker.setVolume(255);
    WiFi.mode(WIFI_STA);
WiFi.disconnect();
WiFi.scanNetworks(true);

Serial.println("WiFi Radar Basladi");
    
    M5.Imu.init();

    M5.Display.setRotation(3);

    canvas.createSprite(WIDTH,HEIGHT);
    M5.Imu.begin();

    drawRadarBackground();

    canvas.pushSprite(0,0);
}
void loop()
{
  
    M5.update();

   if (M5.BtnA.wasPressed())
{
    if(menuOpen)
    {
        // SEÇİM YAP
        if(menuIndex == 0)
        {
            autoTarget = true;
            lockedSSID = "";
        }
        else
        {
            autoTarget = false;
            lockedSSID = wifiList[menuIndex - 1];
            targetSSID = lockedSSID;
        }

        menuOpen = false;
    }
    else
    {
        // Menü aç
        menuOpen = true;
    }
}

if(M5.BtnB.wasPressed())
{
    unsigned long now = millis();

    if(waitingBtnB && (now - btnBTime < 800))
    {
        // ÇİFT BASIŞ = GERİ
        menuIndex--;

        if(menuIndex < 0)
            menuIndex = 0;

        waitingBtnB = false;
    }
    else
    {
        // İlk basış
        btnBTime = now;
        waitingBtnB = true;
    }
}

if(waitingBtnB && (millis() - btnBTime >= 800))
{
    // TEK BASIŞ = İLERİ

    int maxItems = 1;

    for(int i = 0; i < MAX_WIFI; i++)
    {
        if(wifiList[i] != "")
            maxItems++;
    }

    menuIndex++;

    if(menuIndex >= maxItems)
        menuIndex = 0;

        if(menuIndex > 4)
{
    menuScroll = (menuIndex - 4) * 15;
}
else
{
    menuScroll = 0;
}

    waitingBtnB = false;
}
    int networks = WiFi.scanComplete();

if(networks >= 0)
{
    bestRSSI = -100;
    targetSSID = "";

   // Listeyi temizle
for(int i = 0; i < MAX_WIFI; i++)
{
    wifiList[i] = "";
    wifiRSSI[i] = 0;
}

int count = min(networks, MAX_WIFI);

// Listeyi doldur
for(int i = 0; i < count; i++)
{
    wifiList[i] = WiFi.SSID(i);
    wifiRSSI[i] = WiFi.RSSI(i);
}
// RSSI'ye göre büyükten küçüğe sırala
for(int i = 0; i < count - 1; i++)
{
    for(int j = i + 1; j < count; j++)
    {
        if(wifiRSSI[j] > wifiRSSI[i])
        {
            // RSSI değiştir
            int tempRSSI = wifiRSSI[i];
            wifiRSSI[i] = wifiRSSI[j];
            wifiRSSI[j] = tempRSSI;

            // SSID değiştir
            String tempSSID = wifiList[i];
            wifiList[i] = wifiList[j];
            wifiList[j] = tempSSID;
        }
    }
}

// AUTO modunda en güçlü ağı seç
if(autoTarget)
{
    for(int i = 0; i < count; i++)
    {
        if(wifiRSSI[i] > bestRSSI)
        {
            bestRSSI = wifiRSSI[i];
            targetSSID = wifiList[i];
        }
    }
}
else
{
    for(int i = 0; i < count; i++)
    {
        if(wifiList[i] == lockedSSID)
        {
            bestRSSI = wifiRSSI[i];
            targetSSID = lockedSSID;
            break;
        }
    }
}

    int change = abs(bestRSSI - lastRSSI);

    movementPower = constrain(change * 15, 0, 100);

    wifiMovement = (change > 3);

    if(wifiMovement && !lastMovementState)
{
M5.Speaker.tone(1500, 300); // 1000Hz, 1 saniye bip
}

lastMovementState = wifiMovement;

    lastRSSI = bestRSSI;

 WiFi.scanNetworks(true);    // Yeni taramayı başlat
}
    if(M5.Imu.update())
{
    auto imu = M5.Imu.getImuData();

    unsigned long now = millis();

    if(lastGyroTime != 0)
    {
        float dt = (now - lastGyroTime) / 1000.0f;

        deviceAngle += imu.gyro.z * dt;
    }

    lastGyroTime = now;
} 
{
    auto imuData = M5.Imu.getImuData();

    unsigned long now = millis();

    if(lastImuTime != 0)
    {
        float dt = (now - lastImuTime) / 1000.0f;

        imuYaw += imuData.gyro.z * dt;
    }

    lastImuTime = now;
}
    M5.Imu.getGyro(&gx, &gy, &gz);

    float gx, gy, gz;

M5.Imu.getGyro(&gx, &gy, &gz);

imuAngle += gz * 0.02;

if(menuOpen)
{
    drawMenu();
}
else
{
drawRadarBackground();

drawSweep(sweepAngle);

drawProximity();
}

canvas.pushSprite(0,0);

    sweepAngle += 2.0;

    if(sweepAngle >= 360)
        sweepAngle = 0;

    delay(16);
}