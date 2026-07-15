#include <M5Unified.h>
#include <math.h>
#include <WiFi.h>

#define CL_BLACK   0x0000
#define CL_GREEN   0x07E0
#define CL_RED     0xF800
#define CL_CYAN    0x07FF
#define CL_YELLOW  0xFFE0
#define CL_ORANGE  0xFDA0
#define CL_WHITE   0xFFFF

enum RadarState {
    STATE_CALIBRATION,
    STATE_RADAR
};

RadarState currentState = STATE_CALIBRATION;
bool lastMovementState = false;
int menuScroll = 0;

String targetSSID = "";
int bestRSSI = -100;
int lastRSSI = -100;
bool wifiMovement = false;
int movementPower = 0;
float smoothedChange = 0.0f;

String logs[4] = {"", "", "", ""};
void addLog(String msg) {
    for (int i = 0; i < 3; i++) {
        logs[i] = logs[i+1];
    }
    logs[3] = msg;
}

M5Canvas canvas(&M5.Display);

constexpr int WIDTH  = 240;
constexpr int HEIGHT = 135;

constexpr int CX = 68;
constexpr int CY = 68;
constexpr int RADAR_RADIUS = 55;

float sweepAngle = 0.0f;
float imuYaw = 0;
unsigned long lastImuTime = 0;
bool deviceMoving = false;

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
    canvas.fillScreen(CL_BLACK);
    
    canvas.drawCircle(CX, CY, 18, CL_GREEN);
    canvas.drawCircle(CX, CY, 36, CL_GREEN);
    canvas.drawCircle(CX, CY, RADAR_RADIUS, CL_GREEN);
    canvas.drawLine(CX - RADAR_RADIUS, CY, CX + RADAR_RADIUS, CY, CL_GREEN);
    canvas.drawLine(CX, CY - RADAR_RADIUS, CX, CY + RADAR_RADIUS, CL_GREEN);
    canvas.fillCircle(CX, CY, 3, CL_GREEN);

    float r = (-90.0f - imuYaw) * DEG_TO_RAD;
    int nx = CX + cos(r) * (RADAR_RADIUS - 7);
    int ny = CY + sin(r) * (RADAR_RADIUS - 7);
    canvas.setTextColor(CL_ORANGE);
    canvas.setTextSize(1);
    canvas.setCursor(nx - 3, ny - 4);
    canvas.print("N");

    canvas.setTextColor(CL_WHITE);
    canvas.setTextSize(1);
    canvas.setCursor(132, 5);
    canvas.print("WIFI:");
    canvas.print(targetSSID.substring(0, 10));

    canvas.setCursor(132, 17);
    canvas.print("RSSI:");
    canvas.print(bestRSSI);

    canvas.setCursor(132, 29);
    canvas.print("PWR :");
    canvas.print(movementPower);

    canvas.setCursor(132, 41);
    canvas.print("W-MV:");
    if (wifiMovement) {
        canvas.setTextColor(CL_RED);
        canvas.print("MOVE");
    } else {
        canvas.setTextColor(CL_GREEN);
        canvas.print("CLEAR");
    }

    canvas.drawRect(130, 53, 106, 78, CL_ORANGE);
    
    canvas.setTextColor(CL_CYAN);
    for (int i = 0; i < 4; i++) {
        if (logs[i] != "") {
            canvas.setCursor(134, 57 + (i * 18));
            canvas.print(logs[i]);
        }
    }

    canvas.setCursor(10, 125);
    if (deviceMoving) {
        canvas.setTextColor(CL_RED);
        canvas.print("DEV: UNSTABLE");
    } else {
        canvas.setTextColor(CL_CYAN);
        canvas.print("DEV: STEADY");
    }
}

void drawSweep(float angle)
{
    float r = angle * DEG_TO_RAD;
    int x = CX + cos(r) * RADAR_RADIUS;
    int y = CY + sin(r) * RADAR_RADIUS;
    canvas.drawLine(CX, CY, x, y, CL_GREEN);
}

void drawProximity()
{
    if (bestRSSI > -100)
    {
        int radius = map(bestRSSI, -90, -30, RADAR_RADIUS - 3, 6);
        radius = constrain(radius, 6, RADAR_RADIUS - 3);

        if (wifiMovement)
        {
            canvas.drawCircle(CX, CY, radius, CL_RED);
            canvas.fillCircle(CX, CY, 3, CL_RED);
        }
        else
        {
            canvas.drawCircle(CX, CY, radius, CL_GREEN);
        }
    }
}

void drawMenu()
{
    canvas.fillScreen(CL_BLACK);
    canvas.setTextSize(2);
    canvas.setTextColor(CL_GREEN);
    canvas.setCursor(10, 5);
    canvas.println("SELECT TARGET");

    canvas.setTextSize(1);
    int y = 35 - menuScroll;

    if (menuIndex == 0)
        canvas.setTextColor(CL_YELLOW);
    else
        canvas.setTextColor(CL_GREEN);

    canvas.setCursor(10, y);
    canvas.print("> AUTO");
    y += 15;

    for (int i = 0; i < MAX_WIFI; i++)
    {
        if (wifiList[i] == "")
            break;

        if (menuIndex == i + 1)
            canvas.setTextColor(CL_YELLOW);
        else
            canvas.setTextColor(CL_GREEN);

        canvas.setCursor(10, y);
        canvas.print(i + 1);
        canvas.print(". ");
        canvas.print(wifiList[i].substring(0, 18));

        canvas.setCursor(180, y);
        canvas.print(wifiRSSI[i]);
        y += 15;
    }

    canvas.setTextColor(CL_GREEN);
    canvas.setCursor(10, 120);
    canvas.print("A:SELECT  B:NEXT");
}

void drawCalibrationScreen() {
    canvas.fillScreen(CL_BLACK);
    
    canvas.drawCircle(WIDTH / 2, 50, 36, CL_GREEN);
    canvas.drawCircle(WIDTH / 2, 50, 2, CL_GREEN);

    float r = (-90.0f - imuYaw) * DEG_TO_RAD;
    int nx = (WIDTH / 2) + cos(r) * 26;
    int ny = 50 + sin(r) * 26;
    canvas.setTextColor(CL_RED);
    canvas.setTextSize(2);
    canvas.setCursor(nx - 5, ny - 7);
    canvas.print("N");

    canvas.fillTriangle(WIDTH / 2, 80, (WIDTH / 2) - 6, 88, (WIDTH / 2) + 6, 88, CL_RED);

    canvas.setTextSize(1);
    canvas.setTextColor(CL_YELLOW);
    if ((millis() / 400) % 2 == 0) {
        canvas.setCursor(12, 100);
        canvas.print("TURN DEV TO NORTH & PRESS B");
    }
    
    canvas.setTextColor(CL_CYAN);
    canvas.setCursor(12, 118);
    int currentHeading = ((int)(-imuYaw) % 360 + 360) % 360;
    canvas.print("CURRENT HEADING: ");
    canvas.print(currentHeading);
    canvas.print(" deg");
}

void setup()
{
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Speaker.setVolume(255);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    M5.Imu.init();
    M5.Display.setRotation(3);
    canvas.createSprite(WIDTH, HEIGHT);
    
    addLog("Radar Init OK");
}

void loop()
{
    M5.update();

    if (currentState == STATE_RADAR && M5.BtnB.pressedFor(1000)) {
        currentState = STATE_CALIBRATION;
        M5.Speaker.tone(1200, 100);
        delay(300);
    }

    if (M5.Imu.update())
    {
        auto imu = M5.Imu.getImuData();
        unsigned long now = millis();

        if (lastImuTime != 0)
        {
            float dt = (now - lastImuTime) / 1000.0f;
            imuYaw += imu.gyro.z * dt;
        }
        lastImuTime = now;

        float gyroMag = sqrt(imu.gyro.x*imu.gyro.x + imu.gyro.y*imu.gyro.y + imu.gyro.z*imu.gyro.z);
        deviceMoving = (gyroMag > 10.0f);
    }

    if (currentState == STATE_CALIBRATION) {
        drawCalibrationScreen();
        canvas.pushSprite(0, 0);

        if (M5.BtnB.wasPressed()) {
            imuYaw = 0.0f;
            lastImuTime = millis();
            currentState = STATE_RADAR;
            
            M5.Speaker.tone(2000, 200);
            WiFi.scanNetworks(true);
            addLog("Calibrated OK");
        }
        delay(16);
        return;
    }

    if (M5.BtnA.wasPressed())
    {
        if (menuOpen)
        {
            if (menuIndex == 0)
            {
                autoTarget = true;
                lockedSSID = "";
                addLog("Mode: Auto");
            }
            else
            {
                autoTarget = false;
                lockedSSID = wifiList[menuIndex - 1];
                targetSSID = lockedSSID;
                addLog("Lock: " + targetSSID.substring(0, 8));
            }
            menuOpen = false;
        }
        else
        {
            menuOpen = true;
        }
    }

    if (M5.BtnB.wasPressed())
    {
        unsigned long now = millis();
        if (waitingBtnB && (now - btnBTime < 800))
        {
            menuIndex--;
            if (menuIndex < 0)
                menuIndex = 0;
            waitingBtnB = false;
        }
        else
        {
            btnBTime = now;
            waitingBtnB = true;
        }
    }

    if (waitingBtnB && (millis() - btnBTime >= 800))
    {
        int maxItems = 1;
        for (int i = 0; i < MAX_WIFI; i++)
        {
            if (wifiList[i] != "")
                maxItems++;
        }

        menuIndex++;
        if (menuIndex >= maxItems)
            menuIndex = 0;

        if (menuIndex > 4)
            menuScroll = (menuIndex - 4) * 15;
        else
            menuScroll = 0;

        waitingBtnB = false;
    }

    int networks = WiFi.scanComplete();
    if (networks >= 0)
    {
        bestRSSI = -100;
        targetSSID = "";

        for (int i = 0; i < MAX_WIFI; i++)
        {
            wifiList[i] = "";
            wifiRSSI[i] = 0;
        }

        int count = min(networks, MAX_WIFI);
        for (int i = 0; i < count; i++)
        {
            wifiList[i] = WiFi.SSID(i);
            wifiRSSI[i] = WiFi.RSSI(i);
        }

        for (int i = 0; i < count - 1; i++)
        {
            for (int j = i + 1; j < count; j++)
            {
                if (wifiRSSI[j] > wifiRSSI[i])
                {
                    int tempRSSI = wifiRSSI[i];
                    wifiRSSI[i] = wifiRSSI[j];
                    wifiRSSI[j] = tempRSSI;

                    String tempSSID = wifiList[i];
                    wifiList[i] = wifiList[j];
                    wifiList[j] = tempSSID;
                }
            }
        }

        if (autoTarget)
        {
            for (int i = 0; i < count; i++)
            {
                if (wifiRSSI[i] > bestRSSI)
                {
                    bestRSSI = wifiRSSI[i];
                    targetSSID = wifiList[i];
                }
            }
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                if (wifiList[i] == lockedSSID)
                {
                    bestRSSI = wifiRSSI[i];
                    targetSSID = lockedSSID;
                    break;
                }
            }
        }

        float instantChange = abs(bestRSSI - lastRSSI);
        smoothedChange = (smoothedChange * 0.6f) + (instantChange * 0.4f);
        movementPower = constrain((int)(smoothedChange * 15), 0, 100);
        wifiMovement = (smoothedChange > 2.5f);

        if (wifiMovement)
        {
            if (!lastMovementState) {
                M5.Speaker.tone(1500, 300);
            }
            
            String distStr = "Far";
            if (bestRSSI >= -40) distStr = "V.Close";
            else if (bestRSSI >= -55) distStr = "Close";
            else if (bestRSSI >= -70) distStr = "Medium";
            
            int heading = ((int)(-imuYaw) % 360 + 360) % 360;
            
            addLog("M:" + String(heading) + "d | " + distStr);
        }

        lastMovementState = wifiMovement;
        lastRSSI = bestRSSI;

        WiFi.scanNetworks(true);
    }

    if (menuOpen)
    {
        drawMenu();
    }
    else
    {
        drawRadarBackground();
        drawSweep(sweepAngle - imuYaw);
        drawProximity();
    }

    canvas.pushSprite(0, 0);

    sweepAngle += 2.0f;
    if (sweepAngle >= 360.0f)
        sweepAngle = 0.0f;

    delay(16);
}