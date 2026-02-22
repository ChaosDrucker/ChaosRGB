#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <time.h>

/* ===================== WIFI CREDENTIALS ===================== */
const char* ssid = "Dein_WLAN";
const char* password = "DEIN_PW";

/* ===================== NTP TIME CONFIG ===================== */
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;      // GMT+1 (Deutschland)
const int daylightOffset_sec = 3600;  // Sommerzeit +1 Stunde

/* ===================== RACE FORWARD DECLARATIONS ===================== */
void raceGameLoop();
void startRaceGame();
void raceMove(uint8_t p);
void checkRaceCollision(uint8_t attacker);
void eliminateRacePlayer(uint8_t p);
void updateRaceRing();
void displayRaceElimination(uint8_t p);

/* ===================== RACE STATE ===================== */
enum RaceState {
  RACE_IDLE,
  RACE_GAME,
  RACE_GAMEOVER
};
RaceState raceState = RACE_IDLE;

/* ===================== MINI LINE FORWARD DECLARATIONS ===================== */
void miniLineLoop();
void miniLineIdle();
void miniLineStartGame();
void miniLineRunGame();
void miniLineGameOver();

/* ===================== MINI LINE STATE ===================== */
enum MiniLineState {
  MINILINE_INTRO,
  MINILINE_IDLE,
  MINILINE_COUNTDOWN,
  MINILINE_GAME,
  MINILINE_GAMEOVER
};
MiniLineState miniLineState = MINILINE_INTRO;

// Mini LINE rotation - Start position at LED 23 instead of LED 0
#define MINILINE_START_OFFSET 23

/* ===================== CHAOS RUN FORWARD DECLARATIONS ===================== */
void chaosRunLoop();
void chaosRunIdle();
void chaosRunStartGame();
void chaosRunGame();
void chaosRunGameOver();

/* ===================== CHAOS RUN STATE ===================== */
enum ChaosRunState {
  CHAOSRUN_INTRO,
  CHAOSRUN_IDLE,
  CHAOSRUN_COUNTDOWN,
  CHAOSRUN_GAME,
  CHAOSRUN_GAMEOVER
};
ChaosRunState chaosRunState = CHAOSRUN_INTRO;

/* ===================== BUILDER FORWARD DECLARATIONS ===================== */
void builderLoop();
void builderIdle();
void builderStartGame();
void builderGame();
void builderGameOver();

/* ===================== BUILDER STATE ===================== */
enum BuilderState {
  BUILDER_INTRO,
  BUILDER_IDLE,
  BUILDER_COUNTDOWN,
  BUILDER_GAME,
  BUILDER_GAMEOVER
};
BuilderState builderState = BUILDER_INTRO;

/* ===================== BUILD ALONE FORWARD DECLARATIONS ===================== */
void buildAloneLoop();
void buildAloneIdle();
void buildAloneStartGame();
void buildAloneGame();
void buildAloneGameOver();

/* ===================== BUILD ALONE STATE ===================== */
enum BuildAloneState {
  BUILDALONE_INTRO,
  BUILDALONE_IDLE,
  BUILDALONE_COUNTDOWN,
  BUILDALONE_GAME,
  BUILDALONE_GAMEOVER
};
BuildAloneState buildAloneState = BUILDALONE_INTRO;

/* ===================== COUNTDOWN GAME FORWARD DECLARATIONS ===================== */
void countdownGameLoop();
void countdownGameIdle();
void countdownGameStart();
void countdownGameRunning();
void countdownGameOver();

/* ===================== COUNTDOWN GAME STATE ===================== */
enum CountdownGameState {
  COUNTDOWNGAME_INTRO,
  COUNTDOWNGAME_IDLE,
  COUNTDOWNGAME_RUNNING,
  COUNTDOWNGAME_GAMEOVER
};
CountdownGameState countdownGameState = COUNTDOWNGAME_INTRO;

/* ===================== REAKTION FORWARD DECLARATIONS ===================== */
void reaktionLoop();
void reaktionIdle();
void reaktionStartGame();
void reaktionGame();
void reaktionGameOver();

/* ===================== REAKTION STATE ===================== */
enum ReaktionState {
  REAKTION_INTRO,
  REAKTION_IDLE,
  REAKTION_COUNTDOWN,
  REAKTION_GAME,
  REAKTION_GAMEOVER
};
ReaktionState reaktionState = REAKTION_INTRO;

/* ===================== SIMON MULTI FORWARD DECLARATIONS ===================== */
void simonMultiLoop();
void colorRunLoop();
void simonMultiIdle();
void simonMultiGame();
void simonMultiGameOver();

/* ===================== SIMON MULTI STATE ===================== */
enum SimonMultiState {
  SIMONMULTI_INTRO,
  SIMONMULTI_IDLE,
  SIMONMULTI_COUNTDOWN,
  SIMONMULTI_GAME,
  SIMONMULTI_GAMEOVER
};
SimonMultiState simonMultiState = SIMONMULTI_INTRO;

/* ===================== COLOR RUN STATE ===================== */
enum ColorRunState {
  COLORRUN_IDLE,
  COLORRUN_COUNTDOWN,
  COLORRUN_GAME,
  COLORRUN_GAMEOVER
};
ColorRunState colorRunState = COLORRUN_IDLE;

/* ===================== SPEED FORWARD DECLARATIONS ===================== */
void speedLoop();
void speedIdle();
void speedStartGame();
void speedGame();
void speedGameOver();

/* ===================== SPEED STATE ===================== */
enum SpeedState {
  SPEED_INTRO,
  SPEED_IDLE,
  SPEED_COUNTDOWN,
  SPEED_GAME,
  SPEED_GAMEOVER
};
SpeedState speedState = SPEED_INTRO;

/* ===================== ENEMYS FORWARD DECLARATIONS ===================== */
void enemysLoop();
void enemysIdle();
void enemysStartGame();
void enemysGame();
void enemysGameOver();

/* ===================== ENEMYS STATE ===================== */
enum EnemysState {
  ENEMYS_INTRO,
  ENEMYS_IDLE,
  ENEMYS_COUNTDOWN,
  ENEMYS_GAME,
  ENEMYS_GAMEOVER
};
EnemysState enemysState = ENEMYS_INTRO;

/* ===================== CLOCK FORWARD DECLARATIONS ===================== */
void clockLoop();
void clockDisplay();
void clockDrawRing(int hours, int minutes, int seconds, int milliseconds);

/* ===================== CLOCK STATE ===================== */
enum ClockState {
  CLOCK_INTRO,
  CLOCK_DISPLAY
};
ClockState clockState = CLOCK_INTRO;

// Clock variables
bool wifiConnected = false;
unsigned long lastIdleActivity = 0;
#define IDLE_TIMEOUT_MS 120000  // 2 Minuten bis zur Uhr-Anzeige

/* ===================== HARDWARE ===================== */
#define LED_PIN 32
#define LED_COUNT 150

#define BTN_RED     18
#define BTN_GREEN   17
#define BTN_BLUE    25
#define BTN_YELLOW  27

#define LED_BTN_RED     19
#define LED_BTN_GREEN   16
#define LED_BTN_BLUE    33
#define LED_BTN_YELLOW  26

#define BUZZER_PIN 23

/* ================= OLED ================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ================= EEPROM ================= */
#define EEPROM_HIGHSCORE_ADDR   0
#define EEPROM_BRIGHTNESS_ADDR  4
#define EEPROM_SIMON_HIGHSCORE_ADDR 8
#define EEPROM_MINILINE_HIGHSCORE_ADDR 12
#define EEPROM_BUILDALONE_HIGHSCORE_ADDR 16
#define EEPROM_COUNTDOWNGAME_HIGHSCORE_ADDR 20
#define EEPROM_REAKTION_HIGHSCORE_ADDR 24
#define EEPROM_REAKTION_HIGHHITS_ADDR 28
#define EEPROM_SIMONMULTI_HIGHSCORE_ADDR 32
#define EEPROM_COLORRUN_HIGHSCORE_ADDR 36
#define EEPROM_SPEED_HIGHSCORE_ADDR 40
#define EEPROM_ENEMYS_HIGHSCORE_ADDR 44
#define EEPROM_SOUND_MUTED_ADDR 48
#define EEPROM_REFLEXRACE_HIGHSCORE_ADDR 52

/* ================= GAME ================= */
#define MAX_ENEMIES 200
#define MAX_SHOTS   50

#define BASE_ENEMY_SPEED 360
#define SPEED_STEP 10
#define SPEED_MIN 80

#define SHOT_SPEED 5

/* ================= COLORS ================= */
// Better yellow - less green tint, more orange/amber
#define COLOR_YELLOW_R 255
#define COLOR_YELLOW_G 180
#define COLOR_YELLOW_B 0
#define YELLOW_RESET_TIME 5000UL

#define BRIGHTNESS_MIN 5
#define BRIGHTNESS_MAX 100
#define BRIGHTNESS_STEP 5

#define GREEN_RESET_TIME 5000UL   // Highscore Reset

/* ================= OBJECTS ================= */
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

/* Race LED Ring */
#define RACE_LED_PIN   14
#define RACE_LED_COUNT 36
Adafruit_NeoPixel raceRing(RACE_LED_COUNT, RACE_LED_PIN, NEO_GRB + NEO_KHZ800);

/* ================= STATE ================= */
enum GameState { INTRO, IDLE, COUNTDOWN, GAME, GAMEOVER };
GameState gameState = INTRO;

/* ================= SYSTEM GAME SELECTION ================= */
enum SystemGame {
  SYS_GAME_SIMON,      // Internal - LED Ring
  SYS_GAME_RACE,       // Internal - LED Ring
  SYS_GAME_MINILINE,   // Internal - LED Ring (mini version of THE LINE)
  SYS_GAME_CHAOSRUN,   // Internal - LED Ring
  SYS_GAME_BUILDER,    // Internal - LED Ring
  SYS_GAME_BUILDALONE, // Internal - LED Ring
  SYS_GAME_COUNTDOWNGAME, // Internal - LED Ring
  SYS_GAME_REAKTION,   // Internal - LED Ring
  SYS_GAME_SIMONMULTI, // Internal - LED Ring
  SYS_GAME_COLORRUN,   // Internal - LED Ring
  SYS_GAME_SPEED,      // Internal - LED Ring (button mashing speed game)
  SYS_GAME_REFLEXRACE, // EXTERNAL - Main LED strip - Reflex reaction game
  SYS_GAME_ENEMYS,     // EXTERNAL - Main LED strip - Color matching shooter
  SYS_GAME_CHAOS,      // EXTERNAL - THE LINE on main LED strip (GPIO32)
  SYS_GAME_CLOCK       // Clock always last
};
#define NUM_GAMES 15
SystemGame currentSystemGame = SYS_GAME_SIMON;  // Boot with SIMON

/* ================= STRUCTS ================= */
struct Enemy { uint32_t color; };
struct Shot  { int pos; uint32_t color; bool active; };

/* ================= VARIABLES ================= */
Enemy enemies[MAX_ENEMIES];
Shot shots[MAX_SHOTS];

int enemyCount = 0;
int roundNr = 1;
int lastFinishedRound = 0;
int highScore = 0;
int simonHighScore = 0;
int miniLineHighScore = 0;
uint32_t buildAloneHighScore = 999999;  // Best time in milliseconds (lower is better)
uint32_t countdownBestDiff = 999999;    // Best accuracy (smallest difference in ms)
int32_t reaktionHighScore = 0;
int32_t reaktionHighHits = 0;           // Best hit count
uint32_t simonMultiHighScore = 0;
uint32_t colorRunHighScore = 0;
uint32_t speedHighScore = 999999;       // Best time in milliseconds (lower is better)
uint32_t enemysHighScore = 0;           // Highest level reached

int enemySpeed;
int waveOffset;

uint8_t brightness = 40;
bool soundMuted = false;  // Global sound mute toggle
unsigned long greenHoldStart = 0;  // For 2-second hold detection
bool greenHeldForMute = false;

// Green button long press for game switching
unsigned long greenLongPressStart = 0;
bool greenLongPressActive = false;
bool greenLongPressDone = false;

unsigned long lastEnemyMove = 0;
unsigned long lastShotMove  = 0;
unsigned long rainbowTimer  = 0;

/* Yellow reset */
unsigned long yellowPressStart = 0;
bool yellowHeld = false;
bool ignoreYellowUntilRelease = false;

/* Blink yellow */
unsigned long yellowBlinkTimer = 0;
bool yellowBlinkState = false;

/* Button edges */
bool lastRed = HIGH;
bool lastGreen = HIGH;
bool lastBlue = HIGH;

/* ================= HELPERS ================= */
int centerX(String t, int s) {
  return (SCREEN_WIDTH - t.length() * 6 * s) / 2;
}

uint32_t randomColor() {
  switch (random(3)) {
    case 0: return strip.Color(255,0,0);
    case 1: return strip.Color(0,255,0);
    default:return strip.Color(0,0,255);
  }
}

/* ================= SOUND CONTROL ================= */
void playTone(uint8_t pin, unsigned int frequency, unsigned long duration) {
  if (!soundMuted) {
    tone(pin, frequency, duration);
  }
}

void drawSpeakerIcon() {
  // Draw speaker icon in top-right corner
  if (soundMuted) {
    // Speaker with X (muted)
    display.fillRect(110, 2, 3, 8, WHITE);  // Speaker body
    display.fillTriangle(113, 2, 113, 9, 118, 5, WHITE);  // Speaker cone
    display.drawLine(120, 2, 125, 7, WHITE);  // X
    display.drawLine(120, 7, 125, 2, WHITE);  // X
  } else {
    // Speaker with sound waves
    display.fillRect(110, 2, 3, 8, WHITE);  // Speaker body
    display.fillTriangle(113, 2, 113, 9, 118, 5, WHITE);  // Speaker cone
    display.drawPixel(120, 3, WHITE);  // Sound wave
    display.drawPixel(121, 4, WHITE);
    display.drawPixel(120, 6, WHITE);
    display.drawPixel(122, 5, WHITE);
    display.drawPixel(124, 5, WHITE);
  }
}

void checkMuteToggle() {
  // Check for 2-second green button hold to toggle mute
  bool greenPressed = (digitalRead(BTN_GREEN) == LOW);

  if (greenPressed) {
    // Nutze die gleiche Zeit-Tracking wie handleSystemGreenLongPress
    // greenLongPressStart wird von handleSystemGreenLongPress gesetzt

    // Nur triggern wenn:
    // 1. Taste länger als 2 Sekunden gedrückt
    // 2. Noch nicht getriggert (greenHeldForMute)
    // 3. Der Spielwechsel bei 200ms ist schon vorbei (greenLongPressDone)
    if (greenLongPressActive && greenLongPressDone && !greenHeldForMute &&
        (millis() - greenLongPressStart >= 2000)) {
      // Held for 2 seconds - toggle mute
      soundMuted = !soundMuted;
      greenHeldForMute = true;  // Prevent repeated toggles

      // Save to EEPROM
      uint8_t mutedByte = soundMuted ? 1 : 0;
      EEPROM.put(EEPROM_SOUND_MUTED_ADDR, mutedByte);
      EEPROM.commit();

      // Give feedback
      if (!soundMuted) {
        tone(BUZZER_PIN, 1000, 100);  // Use regular tone (we just unmuted, so play feedback sound)
      }
    }
  } else {
    // Button released - reset tracking
    greenHeldForMute = false;
  }
}

/* ================= RESET ================= */
void resetToIdle() {
  // Reset CHAOS game variables
  enemyCount = 0;
  roundNr = 1;
  waveOffset = LED_COUNT - 1;
  for (auto &s : shots) s.active = false;

  // Reset Race game
  raceState = RACE_IDLE;

  // Reset MiniLine game
  miniLineState = MINILINE_IDLE;

  // Reset ChaosRun game
  chaosRunState = CHAOSRUN_IDLE;

  // Reset Builder game
  builderState = BUILDER_IDLE;

  // Reset BuildAlone game
  buildAloneState = BUILDALONE_IDLE;

  // Reset Countdown game
  countdownGameState = COUNTDOWNGAME_IDLE;

  // Reset Reaktion game
  reaktionState = REAKTION_IDLE;

  // Reset SimonMulti game
  simonMultiState = SIMONMULTI_IDLE;

  // Reset ColorRun game
  colorRunState = COLORRUN_IDLE;

  // Reset Speed game
  speedState = SPEED_IDLE;

  // Reset Enemys game
  enemysState = ENEMYS_IDLE;

  // Reset Clock
  clockState = CLOCK_INTRO;

  // Clear all LED strips
  strip.clear();
  strip.show();
  raceRing.clear();
  raceRing.show();

  // Reset to CHAOS game and IDLE state
  currentSystemGame = SYS_GAME_CHAOS;
  gameState = IDLE;

  // Reset idle activity timer
  lastIdleActivity = millis();

  ignoreYellowUntilRelease = true;
}

/* ================= SETUP ================= */
void setup() {
  // Initialize pins first for EEPROM reset check
  pinMode(BTN_RED, INPUT_PULLUP);
  pinMode(BTN_GREEN, INPUT_PULLUP);
  pinMode(BTN_BLUE, INPUT_PULLUP);
  pinMode(BTN_YELLOW, INPUT_PULLUP);

  EEPROM.begin(64);  // Increased to 64 bytes for all games

  // Check for EEPROM reset: All 4 buttons held for 10 seconds at boot
  bool allButtonsPressed = (digitalRead(BTN_RED) == LOW &&
                            digitalRead(BTN_GREEN) == LOW &&
                            digitalRead(BTN_BLUE) == LOW &&
                            digitalRead(BTN_YELLOW) == LOW);

  if (allButtonsPressed) {
    // Initialize display and outputs for visual feedback during hold
    pinMode(LED_BTN_RED, OUTPUT);
    pinMode(LED_BTN_GREEN, OUTPUT);
    pinMode(LED_BTN_BLUE, OUTPUT);
    pinMode(LED_BTN_YELLOW, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextColor(SSD1306_WHITE);

    // Show countdown message
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(centerX("HOLD", 2), 10);
    display.print("HOLD");
    display.setCursor(centerX("10 SEC", 2), 30);
    display.print("10 SEC");
    display.display();

    // Blink LEDs and wait for 10 seconds
    unsigned long startTime = millis();
    bool buttonsStillHeld = true;

    while (millis() - startTime < 10000 && buttonsStillHeld) {
      // Check if all buttons still held
      buttonsStillHeld = (digitalRead(BTN_RED) == LOW &&
                          digitalRead(BTN_GREEN) == LOW &&
                          digitalRead(BTN_BLUE) == LOW &&
                          digitalRead(BTN_YELLOW) == LOW);

      if (!buttonsStillHeld) break;

      // Blink all button LEDs
      bool ledState = ((millis() - startTime) % 500) < 250;
      digitalWrite(LED_BTN_RED, ledState ? HIGH : LOW);
      digitalWrite(LED_BTN_GREEN, ledState ? HIGH : LOW);
      digitalWrite(LED_BTN_BLUE, ledState ? HIGH : LOW);
      digitalWrite(LED_BTN_YELLOW, ledState ? HIGH : LOW);

      // Show countdown on display
      int secondsLeft = 10 - ((millis() - startTime) / 1000);
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(centerX("HOLD", 2), 10);
      display.print("HOLD");
      display.setTextSize(4);
      display.setCursor(centerX(String(secondsLeft), 4), 32);
      display.print(secondsLeft);
      display.display();

      delay(100);
    }

    // Turn off LEDs
    digitalWrite(LED_BTN_RED, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    digitalWrite(LED_BTN_BLUE, LOW);
    digitalWrite(LED_BTN_YELLOW, LOW);

    if (buttonsStillHeld) {
      // 10 seconds completed - Reset ALL EEPROM values
    highScore = 0;
    brightness = 40;
    simonHighScore = 0;
    miniLineHighScore = 0;
    buildAloneHighScore = 0;
    countdownBestDiff = 999999;
    reaktionHighScore = 0;
    reaktionHighHits = 0;
    simonMultiHighScore = 0;
    colorRunHighScore = 0;
    speedHighScore = 999999;
    enemysHighScore = 0;
    soundMuted = false;  // Reset sound to unmuted

    EEPROM.put(EEPROM_HIGHSCORE_ADDR, highScore);
    EEPROM.put(EEPROM_BRIGHTNESS_ADDR, brightness);
    EEPROM.put(EEPROM_SIMON_HIGHSCORE_ADDR, simonHighScore);
    EEPROM.put(EEPROM_MINILINE_HIGHSCORE_ADDR, miniLineHighScore);
    EEPROM.put(EEPROM_BUILDALONE_HIGHSCORE_ADDR, buildAloneHighScore);
    EEPROM.put(EEPROM_COUNTDOWNGAME_HIGHSCORE_ADDR, countdownBestDiff);
    EEPROM.put(EEPROM_REAKTION_HIGHSCORE_ADDR, reaktionHighScore);
    EEPROM.put(EEPROM_REAKTION_HIGHHITS_ADDR, reaktionHighHits);
    EEPROM.put(EEPROM_SIMONMULTI_HIGHSCORE_ADDR, simonMultiHighScore);
    EEPROM.put(EEPROM_COLORRUN_HIGHSCORE_ADDR, colorRunHighScore);
    EEPROM.put(EEPROM_SPEED_HIGHSCORE_ADDR, speedHighScore);
    EEPROM.put(EEPROM_ENEMYS_HIGHSCORE_ADDR, enemysHighScore);
    uint8_t mutedByte = 0;  // Unmuted
    EEPROM.put(EEPROM_SOUND_MUTED_ADDR, mutedByte);
    EEPROM.commit();

      // Show success message
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(centerX("EEPROM", 2), 10);
      display.print("EEPROM");
      display.setCursor(centerX("RESET!", 2), 30);
      display.print("RESET!");
      display.display();

      playTone(BUZZER_PIN, 2000, 200);
      delay(300);
      playTone(BUZZER_PIN, 1500, 200);
      delay(300);
      playTone(BUZZER_PIN, 1000, 400);
      delay(2000);

      // Wait for buttons to be released
      while (digitalRead(BTN_RED) == LOW || digitalRead(BTN_GREEN) == LOW ||
             digitalRead(BTN_BLUE) == LOW || digitalRead(BTN_YELLOW) == LOW) {
        delay(10);
      }
    } else {
      // Buttons released before 10 seconds - cancel reset
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(centerX("RESET", 2), 10);
      display.print("RESET");
      display.setCursor(centerX("CANCELLED", 2), 30);
      display.print("CANCELLED");
      display.display();

      playTone(BUZZER_PIN, 400, 300);
      delay(1500);
    }
  } else {
    // Normal boot - load from EEPROM
    EEPROM.get(EEPROM_HIGHSCORE_ADDR, highScore);
    EEPROM.get(EEPROM_BRIGHTNESS_ADDR, brightness);
    EEPROM.get(EEPROM_SIMON_HIGHSCORE_ADDR, simonHighScore);
    EEPROM.get(EEPROM_MINILINE_HIGHSCORE_ADDR, miniLineHighScore);

    // Load sound mute status
    uint8_t mutedByte = 0;
    EEPROM.get(EEPROM_SOUND_MUTED_ADDR, mutedByte);
    soundMuted = (mutedByte == 1);

    if (brightness < BRIGHTNESS_MIN || brightness > BRIGHTNESS_MAX)
      brightness = 40;
  }

  // Initialize outputs (only if not already done in EEPROM reset)
  if (!allButtonsPressed) {
    pinMode(LED_BTN_RED, OUTPUT);
    pinMode(LED_BTN_GREEN, OUTPUT);
    pinMode(LED_BTN_BLUE, OUTPUT);
    pinMode(LED_BTN_YELLOW, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
  }

  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

  raceRing.begin();
  raceRing.setBrightness(brightness);
  raceRing.clear();
  raceRing.show();

  // Initialize display (only if not already done in EEPROM reset)
  if (!allButtonsPressed) {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextColor(SSD1306_WHITE);
  }

  randomSeed(esp_random());

  // Initialize WiFi for clock
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("WiFi verbinden...");
  display.display();

  // Set WiFi mode to station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Start WiFi connection
  WiFi.begin(ssid, password);

  // Wait up to 10 seconds for WiFi with visual feedback
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 100) {
    delay(100);
    wifiAttempts++;

    // Update display every second
    if (wifiAttempts % 10 == 0) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("WiFi verbinden...");
      display.setCursor(0, 10);
      display.print("Versuch: ");
      display.print(wifiAttempts / 10);
      display.print("/10");
      display.display();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;

    // Configure NTP time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("WiFi OK!");
    display.setCursor(0, 10);
    display.print("SSID: ");
    display.print(ssid);
    display.setCursor(0, 20);
    display.print("IP: ");
    display.print(WiFi.localIP());
    display.display();
    delay(2000);
  } else {
    wifiConnected = false;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("WiFi FEHLER!");
    display.setCursor(0, 10);
    display.print("SSID: ");
    display.print(ssid);
    display.setCursor(0, 20);
    display.print("Uhr nicht");
    display.setCursor(0, 30);
    display.print("verfuegbar");
    display.display();
    delay(2000);
  }

  // BOOT ANIMATION - Show CHAOS RGB startup
  bootAnimation();

  // Initialize idle timer
  lastIdleActivity = millis();
}

/* ================= BOOT ANIMATION ================= */
void bootAnimation() {
  // Clear everything first
  strip.clear();
  strip.show();
  raceRing.clear();
  raceRing.show();

  // Display "CHAOS RGB" - Hardware name
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(centerX("CHAOS", 3), 10);
  display.print("CHAOS");
  display.setTextSize(2);
  display.setCursor(centerX("RGB", 2), 45);
  display.print("RGB");
  display.display();

  // Boot sound - rising arpeggio
  int bootNotes[] = {262, 330, 392, 523, 659, 784};  // C4, E4, G4, C5, E5, G5
  int bootDurations[] = {100, 100, 100, 100, 100, 200};

  // LED Animation - Fill both strips with rainbow
  for (int i = 0; i < 6; i++) {
    // Play note
    playTone(BUZZER_PIN, bootNotes[i], bootDurations[i]);

    // Light up segments
    int segmentSize = LED_COUNT / 6;
    int ringSegmentSize = RACE_LED_COUNT / 6;

    // Main strip - fill with rainbow colors
    for (int j = 0; j <= i * segmentSize && j < LED_COUNT; j++) {
      uint16_t hue = (j * 65536L / LED_COUNT);
      strip.setPixelColor(j, strip.gamma32(strip.ColorHSV(hue)));
    }
    strip.show();

    // Ring - fill with rainbow colors
    for (int j = 0; j <= i * ringSegmentSize && j < RACE_LED_COUNT; j++) {
      uint16_t hue = (j * 65536L / RACE_LED_COUNT);
      raceRing.setPixelColor(j, raceRing.gamma32(raceRing.ColorHSV(hue)));
    }
    raceRing.show();

    // Flash all button LEDs in sequence
    digitalWrite(LED_BTN_RED, (i >= 0) ? HIGH : LOW);
    digitalWrite(LED_BTN_GREEN, (i >= 1) ? HIGH : LOW);
    digitalWrite(LED_BTN_BLUE, (i >= 2) ? HIGH : LOW);
    digitalWrite(LED_BTN_YELLOW, (i >= 3) ? HIGH : LOW);

    delay(bootDurations[i] + 50);
  }

  // Final flash - all LEDs white
  strip.fill(strip.Color(255, 255, 255));
  strip.show();
  raceRing.fill(raceRing.Color(255, 255, 255));
  raceRing.show();

  digitalWrite(LED_BTN_RED, HIGH);
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_BLUE, HIGH);
  digitalWrite(LED_BTN_YELLOW, HIGH);

  playTone(BUZZER_PIN, 1047, 300);  // High C
  delay(300);

  // Fade out
  for (int brightness = 255; brightness >= 0; brightness -= 15) {
    strip.fill(strip.Color(brightness, brightness, brightness));
    strip.show();
    raceRing.fill(raceRing.Color(brightness, brightness, brightness));
    raceRing.show();
    delay(20);
  }

  // Turn off everything
  strip.clear();
  strip.show();
  raceRing.clear();
  raceRing.show();
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);

  delay(300);
}

/* ================= LOOP ================= */
void chaosLoop() {

  handleYellowReset();
  updateButtonLEDs();

  switch (gameState) {
    case INTRO:     showIntro(); gameState = IDLE; break;
    case IDLE:      idle(); break;
    case COUNTDOWN: runCountdown(); startGame(); gameState = GAME; break;
    case GAME:      runGame(); break;
    case GAMEOVER:  runGameOver(); gameState = IDLE; break;
  }
}

/* ================= YELLOW RESET (DISABLED - only manages ignore flag) ================= */
void handleYellowReset() {
  bool yellowPressed = digitalRead(BTN_YELLOW) == LOW;

  // Only manage the ignore flag, no reset functionality
  if (ignoreYellowUntilRelease && !yellowPressed)
    ignoreYellowUntilRelease = false;
}

/* ================= GLOBAL 4-BUTTON RESET ================= */
unsigned long fourButtonPressStart = 0;
bool fourButtonsHeld = false;
bool threeSecondResetDone = false;
bool countdownStarted = false;
bool eepromCountdownActive = false;  // Prevents game loops from updating display during countdown

void handleFourButtonReset() {
  bool allPressed = (digitalRead(BTN_RED) == LOW &&
                     digitalRead(BTN_GREEN) == LOW &&
                     digitalRead(BTN_BLUE) == LOW &&
                     digitalRead(BTN_YELLOW) == LOW);

  if (allPressed && !fourButtonsHeld) {
    fourButtonsHeld = true;
    fourButtonPressStart = millis();
    threeSecondResetDone = false;
    countdownStarted = false;
  }

  if (!allPressed) {
    fourButtonsHeld = false;
    threeSecondResetDone = false;
    countdownStarted = false;
    eepromCountdownActive = false;
  }

  unsigned long holdDuration = millis() - fourButtonPressStart;

  // 3 seconds - reset to IDLE/menu
  if (fourButtonsHeld && !threeSecondResetDone && holdDuration >= 3000 && holdDuration < 5000) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(centerX("RESET", 2), 20);
    display.print("RESET");
    display.setCursor(centerX("TO MENU", 2), 40);
    display.print("TO MENU");
    display.display();

    playTone(BUZZER_PIN, 1000, 200);
    delay(300);
    playTone(BUZZER_PIN, 800, 200);
    delay(300);
    playTone(BUZZER_PIN, 600, 400);
    delay(1000);

    // Reset to idle for current game
    resetToIdle();
    threeSecondResetDone = true;
    ignoreYellowUntilRelease = true;
  }

  // 5-10 seconds - Show countdown warning for EEPROM reset
  if (fourButtonsHeld && threeSecondResetDone && holdDuration >= 5000 && holdDuration < 10000) {
    eepromCountdownActive = true;  // Block game loops from updating display

    // Show countdown 5, 4, 3, 2, 1
    int secondsLeft = 10 - (holdDuration / 1000);

    static int lastSecondShown = -1;
    if (secondsLeft != lastSecondShown) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(centerX("EEPROM", 2), 5);
      display.print("EEPROM");
      display.setCursor(centerX("RESET IN", 2), 25);
      display.print("RESET IN");

      display.setTextSize(4);
      display.setCursor(centerX(String(secondsLeft), 4), 45);
      display.print(secondsLeft);
      display.display();

      playTone(BUZZER_PIN, 800 + secondsLeft * 100, 100);
      lastSecondShown = secondsLeft;
    }
    return;  // Don't process anything else during countdown
  } else {
    eepromCountdownActive = false;  // Re-enable game loops
  }

  // 10 seconds - EEPROM reset
  if (fourButtonsHeld && holdDuration >= 10000) {
    // Reset ALL EEPROM values
    highScore = 0;
    brightness = 40;
    simonHighScore = 0;
    miniLineHighScore = 0;
    buildAloneHighScore = 999999;
    countdownBestDiff = 999999;
    reaktionHighScore = 0;
    reaktionHighHits = 0;
    simonMultiHighScore = 0;
    colorRunHighScore = 0;
    speedHighScore = 999999;
    enemysHighScore = 0;
    soundMuted = false;  // Reset sound to unmuted

    EEPROM.put(EEPROM_HIGHSCORE_ADDR, highScore);
    EEPROM.put(EEPROM_BRIGHTNESS_ADDR, brightness);
    EEPROM.put(EEPROM_SIMON_HIGHSCORE_ADDR, simonHighScore);
    EEPROM.put(EEPROM_MINILINE_HIGHSCORE_ADDR, miniLineHighScore);
    EEPROM.put(EEPROM_BUILDALONE_HIGHSCORE_ADDR, buildAloneHighScore);
    EEPROM.put(EEPROM_COUNTDOWNGAME_HIGHSCORE_ADDR, countdownBestDiff);
    EEPROM.put(EEPROM_REAKTION_HIGHSCORE_ADDR, reaktionHighScore);
    EEPROM.put(EEPROM_REAKTION_HIGHHITS_ADDR, reaktionHighHits);
    EEPROM.put(EEPROM_SIMONMULTI_HIGHSCORE_ADDR, simonMultiHighScore);
    EEPROM.put(EEPROM_COLORRUN_HIGHSCORE_ADDR, colorRunHighScore);
    EEPROM.put(EEPROM_SPEED_HIGHSCORE_ADDR, speedHighScore);
    EEPROM.put(EEPROM_ENEMYS_HIGHSCORE_ADDR, enemysHighScore);
    uint8_t mutedByte = 0;  // Unmuted
    EEPROM.put(EEPROM_SOUND_MUTED_ADDR, mutedByte);
    EEPROM.commit();

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(centerX("EEPROM", 2), 10);
    display.print("EEPROM");
    display.setCursor(centerX("RESET!", 2), 30);
    display.print("RESET!");
    display.display();

    playTone(BUZZER_PIN, 2000, 200);
    delay(300);
    playTone(BUZZER_PIN, 1500, 200);
    delay(300);
    playTone(BUZZER_PIN, 1000, 400);
    delay(2000);

    fourButtonsHeld = false;
    threeSecondResetDone = false;
    eepromCountdownActive = false;
    ignoreYellowUntilRelease = true;

    // Reset to CHAOS idle after EEPROM reset
    resetToIdle();
  }
}


/* ================= BUTTON LEDS ================= */
void updateButtonLEDs() {
  if (gameState == GAME) {
    digitalWrite(LED_BTN_RED, HIGH);
    digitalWrite(LED_BTN_GREEN, HIGH);
    digitalWrite(LED_BTN_BLUE, HIGH);
    digitalWrite(LED_BTN_YELLOW, LOW);
  } else {
    digitalWrite(LED_BTN_RED, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    digitalWrite(LED_BTN_BLUE, LOW);
  }
}

/* ================= INTRO ================= */
void showIntro() {
  int notes[] = {392,523,587,659,784,659,587,784};
  int dur[]   = {150,150,150,250,350,150,150,500};

  for (int i = 0; i < 8; i++) {
    playTone(BUZZER_PIN, notes[i], dur[i]);
    delay(dur[i] + 30);
  }

  display.clearDisplay();
  display.setTextSize(2);

  // Show different text based on current game
  switch (currentSystemGame) {
    case SYS_GAME_SIMON:
      display.setCursor(centerX("SIMON", 2), 24);
      display.print("SIMON");
      break;
    case SYS_GAME_RACE:
      display.setCursor(centerX("RACE", 2), 24);
      display.print("RACE");
      break;
    case SYS_GAME_MINILINE:
      display.setCursor(centerX("MINI", 2), 16);
      display.print("MINI");
      display.setCursor(centerX("LINE", 2), 36);
      display.print("LINE");
      break;
    case SYS_GAME_CHAOSRUN:
      display.setCursor(centerX("CHAOS", 2), 16);
      display.print("CHAOS");
      display.setCursor(centerX("RUN", 2), 36);
      display.print("RUN");
      break;
    case SYS_GAME_BUILDER:
      display.setCursor(centerX("BUILDER", 2), 24);
      display.print("BUILDER");
      break;
    case SYS_GAME_BUILDALONE:
      display.setCursor(centerX("BUILD", 2), 16);
      display.print("BUILD");
      display.setCursor(centerX("ALONE", 2), 36);
      display.print("ALONE");
      break;
    case SYS_GAME_COUNTDOWNGAME:
      display.setCursor(centerX("COUNT", 2), 16);
      display.print("COUNT");
      display.setCursor(centerX("DOWN", 2), 36);
      display.print("DOWN");
      break;
    case SYS_GAME_REAKTION:
      display.setCursor(centerX("REAK", 2), 16);
      display.print("REAK");
      display.setCursor(centerX("TION", 2), 36);
      display.print("TION");
      break;
    case SYS_GAME_SIMONMULTI:
      display.setCursor(centerX("SIMON", 2), 16);
      display.print("SIMON");
      display.setCursor(centerX("MULTI", 2), 36);
      display.print("MULTI");
      break;
    case SYS_GAME_COLORRUN:
      display.setCursor(centerX("COLOR", 2), 16);
      display.print("COLOR");
      display.setCursor(centerX("RUN", 2), 36);
      display.print("RUN");
      break;
    case SYS_GAME_SPEED:
      display.setCursor(centerX("SPEED", 2), 24);
      display.print("SPEED");
      break;
    case SYS_GAME_REFLEXRACE:
      display.setCursor(centerX("REFLEX", 2), 8);
      display.print("REFLEX");
      display.setCursor(centerX("RACE", 2), 28);
      display.print("RACE");
      break;
    case SYS_GAME_ENEMYS:
      display.setCursor(centerX("ENEMYS", 2), 24);
      display.print("ENEMYS");
      break;
    case SYS_GAME_CHAOS:
      display.setCursor(centerX("THE", 2), 16);
      display.print("THE");
      display.setCursor(centerX("LINE", 2), 36);
      display.print("LINE");
      break;
    case SYS_GAME_CLOCK:
      display.setCursor(centerX("UHR", 2), 24);
      display.print("UHR");
      break;
  }

  display.display();
  delay(2000);
}

/* ================= IDLE ================= */
void idle() {
  // LED Strip (GPIO32) with Rainbow
  idleRainbow();
  
  // LED Ring (GPIO14) OFF
  raceRing.clear();
  raceRing.show();
  
  softBlinkYellow();
  handleBrightness();
  drawIdleDisplay();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    gameState = COUNTDOWN;
}

/* ---------- IDLE HELPERS ---------- */
void idleRainbow() {
  if (millis() - rainbowTimer < 30) return;
  rainbowTimer = millis();
  static uint16_t h = 0;
  for (int i = 0; i < LED_COUNT; i++)
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(h + i * 65535L / LED_COUNT)));
  strip.show();
  h += 40;
}

void softBlinkYellow() {
  if (millis() - yellowBlinkTimer > 700) {
    yellowBlinkTimer = millis();
    yellowBlinkState = !yellowBlinkState;
    digitalWrite(LED_BTN_YELLOW, yellowBlinkState);
  }
}

void handleBrightness() {
  bool r = digitalRead(BTN_RED);
  bool b = digitalRead(BTN_BLUE);

  if (lastRed == HIGH && r == LOW && brightness > BRIGHTNESS_MIN) {
    brightness -= BRIGHTNESS_STEP;
    strip.setBrightness(brightness);
    EEPROM.put(EEPROM_BRIGHTNESS_ADDR, brightness);
    EEPROM.commit();
  }

  if (lastBlue == HIGH && b == LOW && brightness < BRIGHTNESS_MAX) {
    brightness += BRIGHTNESS_STEP;
    strip.setBrightness(brightness);
    EEPROM.put(EEPROM_BRIGHTNESS_ADDR, brightness);
    EEPROM.commit();
  }

  lastRed = r;
  lastBlue = b;
}

void handleMiniLineBrightness() {
  bool r = digitalRead(BTN_RED);
  bool b = digitalRead(BTN_BLUE);

  if (lastRed == HIGH && r == LOW && brightness > BRIGHTNESS_MIN) {
    brightness -= BRIGHTNESS_STEP;
    raceRing.setBrightness(brightness);
    EEPROM.put(EEPROM_BRIGHTNESS_ADDR, brightness);
    EEPROM.commit();
  }

  if (lastBlue == HIGH && b == LOW && brightness < 200) {  // Max 200 for MINI LINE
    brightness += BRIGHTNESS_STEP;
    raceRing.setBrightness(brightness);
    EEPROM.put(EEPROM_BRIGHTNESS_ADDR, brightness);
    EEPROM.commit();
  }

  lastRed = r;
  lastBlue = b;
}

void drawIdleDisplay() {
  display.clearDisplay();

  // Brightness top left
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Title (smaller, calmer)
  display.setTextSize(2);
  display.setCursor(centerX("THE LINE",2), 12);
  display.print("THE LINE");

  // Highscore
  display.setTextSize(1);
  display.setCursor(centerX("HI-SCORE:",1), 30);
  display.print("HI-SCORE:");

  display.setTextSize(2);
  display.setCursor(centerX(String(highScore),2), 40);
  display.print(highScore);

  display.display();
}

/* ================= COUNTDOWN ================= */

// =====================
// RACE COUNTDOWN (based on SIMON countdown)
// =====================
void runRaceCountdown() {
  for (int i = 3; i >= 1; i--) {
    digitalWrite(LED_BTN_RED, HIGH);
    digitalWrite(LED_BTN_GREEN, HIGH);
    digitalWrite(LED_BTN_BLUE, HIGH);
    digitalWrite(LED_BTN_YELLOW, HIGH);
    playTone(BUZZER_PIN, 1200, 150);

    display.clearDisplay();
    display.setTextSize(4);
    display.setCursor(centerX(String(i),4),16);
    display.print(i);
    display.display();

    delay(300);
    digitalWrite(LED_BTN_RED, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    digitalWrite(LED_BTN_BLUE, LOW);
    digitalWrite(LED_BTN_YELLOW, LOW);
    delay(700);
  }
  playTone(BUZZER_PIN, 2200, 400);
  delay(400);
}

void runCountdown() {
  for (int i = 3; i >= 1; i--) {
    digitalWrite(LED_BTN_RED, HIGH);
    digitalWrite(LED_BTN_GREEN, HIGH);
    digitalWrite(LED_BTN_BLUE, HIGH);

    playTone(BUZZER_PIN, 1200, 150);

    display.clearDisplay();
    display.setTextSize(4);
    display.setCursor(centerX(String(i),4),16);
    display.print(i);
    display.display();

    delay(300);
    digitalWrite(LED_BTN_RED, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    digitalWrite(LED_BTN_BLUE, LOW);
    delay(700);
  }
  playTone(BUZZER_PIN, 2200, 400);
  delay(400);
}

/* ================= BUILDER COUNTDOWN (only BLUE + GREEN) ================= */
void runBuilderCountdown() {
  for (int i = 3; i >= 1; i--) {
    digitalWrite(LED_BTN_BLUE, HIGH);
    digitalWrite(LED_BTN_GREEN, HIGH);
    // RED and YELLOW stay OFF

    playTone(BUZZER_PIN, 1200, 150);

    display.clearDisplay();
    display.setTextSize(4);
    display.setCursor(centerX(String(i),4),16);
    display.print(i);
    display.display();

    delay(300);
    digitalWrite(LED_BTN_BLUE, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    delay(700);
  }
  playTone(BUZZER_PIN, 2200, 400);
  delay(400);
}

/* ================= BUILD ALONE COUNTDOWN (only RED + GREEN) ================= */
void runBuildAloneCountdown() {
  for (int i = 3; i >= 1; i--) {
    digitalWrite(LED_BTN_RED, HIGH);
    digitalWrite(LED_BTN_GREEN, HIGH);
    // BLUE and YELLOW stay OFF

    playTone(BUZZER_PIN, 1200, 150);

    display.clearDisplay();
    display.setTextSize(4);
    display.setCursor(centerX(String(i),4),16);
    display.print(i);
    display.display();

    delay(300);
    digitalWrite(LED_BTN_RED, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    delay(700);
  }
  playTone(BUZZER_PIN, 2200, 400);
  delay(400);
}

/* ================= SIMON MULTI COUNTDOWN (all 4 buttons) ================= */
void runSimonMultiCountdown() {
  for (int i = 3; i >= 1; i--) {
    digitalWrite(LED_BTN_RED, HIGH);
    digitalWrite(LED_BTN_GREEN, HIGH);
    digitalWrite(LED_BTN_BLUE, HIGH);
    digitalWrite(LED_BTN_YELLOW, HIGH);

    playTone(BUZZER_PIN, 1200, 150);

    display.clearDisplay();
    display.setTextSize(4);
    display.setCursor(centerX(String(i),4),16);
    display.print(i);
    display.display();

    delay(300);
    digitalWrite(LED_BTN_RED, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    digitalWrite(LED_BTN_BLUE, LOW);
    digitalWrite(LED_BTN_YELLOW, LOW);
    delay(700);
  }
  playTone(BUZZER_PIN, 2200, 400);
  delay(400);
}

/* ================= SPEED COUNTDOWN (only green button) ================= */
void runSpeedCountdown() {
  for (int i = 3; i >= 1; i--) {
    digitalWrite(LED_BTN_GREEN, HIGH);
    // RED, BLUE and YELLOW stay OFF

    playTone(BUZZER_PIN, 1200, 150);

    display.clearDisplay();
    display.setTextSize(4);
    display.setCursor(centerX(String(i),4),16);
    display.print(i);
    display.display();

    delay(300);
    digitalWrite(LED_BTN_GREEN, LOW);
    delay(700);
  }
  playTone(BUZZER_PIN, 2200, 400);
  delay(400);
}

/* ================= START GAME ================= */
void startGame() {
  enemyCount = roundNr;
  for (int i = 0; i < enemyCount; i++)
    enemies[i].color = randomColor();

  enemySpeed = max(BASE_ENEMY_SPEED - roundNr * 10, SPEED_MIN);
  waveOffset = LED_COUNT - 1;

  // DON'T clear shots - they should continue between rounds!
  // Only clear strip visuals
  strip.clear();
  strip.show();
}

/* ================= RUN GAME ================= */
void runGame() {
  unsigned long now = millis();

  // Move enemies forward
  if (now - lastEnemyMove > enemySpeed) {
    lastEnemyMove = now;
    waveOffset--;
    if (waveOffset <= 0) {
      lastFinishedRound = roundNr;
      gameState = GAMEOVER;
    }
  }

  // Move shots forward
  if (now - lastShotMove > SHOT_SPEED) {
    lastShotMove = now;
    for (auto &s : shots)
      if (s.active && ++s.pos >= LED_COUNT)
        s.active = false;
  }

  handleShots();
  checkCollision();
  drawStrip();
  drawGameDisplay();
}

void handleShots() {
  bool r = digitalRead(BTN_RED);
  bool g = digitalRead(BTN_GREEN);
  bool b = digitalRead(BTN_BLUE);

  if (lastRed == HIGH && r == LOW) spawnShot(strip.Color(255,0,0));
  if (lastGreen == HIGH && g == LOW) spawnShot(strip.Color(0,255,0));
  if (lastBlue == HIGH && b == LOW) spawnShot(strip.Color(0,0,255));

  lastRed = r;
  lastGreen = g;
  lastBlue = b;
}

void spawnShot(uint32_t c) {
  for (auto &s : shots)
    if (!s.active) {
      s = {0, c, true};
      playTone(BUZZER_PIN, 800, 30);
      return;
    }
}

/* ---------- COLLISION (mit Offset-Fix) ---------- */
void checkCollision() {

  if (enemyCount == 0) {
    roundNr++;
    startGame();
    return;
  }

  for (auto &s : shots) {
    if (!s.active) continue;

    if (s.pos >= waveOffset) {
      s.active = false;

      if (s.color == enemies[0].color) {
        // Correct color - remove enemy
        for (int i = 0; i < enemyCount - 1; i++)
          enemies[i] = enemies[i + 1];
        enemyCount--;
        waveOffset++;   // Tempo compensation
        playTone(BUZZER_PIN, 1500, 50);
      } else {
        // Wrong color - shot becomes enemy at the back
        enemies[enemyCount++].color = s.color;
        enemySpeed = max(enemySpeed - SPEED_STEP, SPEED_MIN);
        playTone(BUZZER_PIN, 300, 100);
      }
      return;
    }
  }
}

/* ================= DRAW ================= */
void drawStrip() {
  strip.clear();

  for (int i = 0; i < enemyCount; i++) {
    int p = waveOffset + i;
    if (p >= 0 && p < LED_COUNT)
      strip.setPixelColor(p, enemies[i].color);
  }

  for (auto &s : shots)
    if (s.active && s.pos < LED_COUNT)
      strip.setPixelColor(s.pos, s.color);

  strip.show();
}

void drawGameDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(centerX("RUNDE",1),10);
  display.print("RUNDE");

  display.setTextSize(3);
  display.setCursor(centerX(String(roundNr),3),30);
  display.print(roundNr);

  display.display();
}

/* ================= GAME OVER ================= */
void runGameOver() {
  int notes[] = {392,330,262,196};
  for (int i = 0; i < 4; i++) {
    playTone(BUZZER_PIN, notes[i], 300);
    delay(350);
  }

  unsigned long start = millis();
  bool on = false;
  while (millis() - start < 8000) {
    on = !on;
    strip.clear();
    if (on)
      for (int i = 0; i < LED_COUNT; i++)
        strip.setPixelColor(i, strip.Color(255,0,0));
    strip.show();

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(centerX("RUNDE ERREICHT",1),10);
    display.print("RUNDE ERREICHT");

    display.setTextSize(3);
    display.setCursor(centerX(String(lastFinishedRound),3),30);
    display.print(lastFinishedRound);
    display.display();

    delay(400);
  }

  if (lastFinishedRound > highScore) {
    highScore = lastFinishedRound;
    EEPROM.put(EEPROM_HIGHSCORE_ADDR, highScore);
    EEPROM.commit();
  }

  roundNr = 1;
}

/* ===================== SIMON SAYS GAME ===================== */

#define SIMON_MAX_LEVEL 100
#define SIMON_LED_ON_TIME 400
#define SIMON_LED_OFF_TIME 200

enum SimonState {
  SIMON_IDLE,
  SIMON_SHOW_SEQUENCE,
  SIMON_PLAYER_INPUT,
  SIMON_GAME_OVER
};

SimonState simonState = SIMON_IDLE;

uint8_t simonSequence[SIMON_MAX_LEVEL];
uint8_t simonLevel = 1;
uint8_t simonInputIndex = 0;
uint8_t simonShowIndex = 0;
unsigned long simonTimer = 0;

void simonAllLedsOff() {
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);
}

void simonAllLedsOn() {
  digitalWrite(LED_BTN_RED, HIGH);
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_BLUE, HIGH);
  digitalWrite(LED_BTN_YELLOW, HIGH);
}

/* ---------- SIMON COUNTDOWN ---------- */
void runSimonCountdown() {
  simonAllLedsOff();

  for (int i = 3; i > 0; i--) {
    // Turn ON all 4 buttons (including yellow)
    simonAllLedsOn();
    playTone(BUZZER_PIN, 1200, 150);

    display.clearDisplay();
    display.setTextSize(4);
    display.setCursor(centerX(String(i),4),16);
    display.print(i);
    display.display();

    delay(300);
    simonAllLedsOff();
    delay(700);
  }
  playTone(BUZZER_PIN, 2200, 400);
  delay(400);
}

/* ---------- CHAOS GAME OVER ---------- */

void runSimonGameOver(int finalScore, bool isNewHighscore) {
  // Chaos-style game over sound + blink simultaneously
  for (int i = 0; i < 4; i++) {
    simonAllLedsOn();
    raceRing.fill(raceRing.Color(255, 0, 0));  // Red flash on ring
    raceRing.show();
    playTone(BUZZER_PIN, (i < 2) ? 400 : 200, (i < 2) ? 200 : 400);
    delay(200);
    simonAllLedsOff();
    simonClearRing();
    delay(150);
  }

  // Faster replay of last sequence
  simonShowIndex = 0;
  simonTimer = millis();

  while (simonShowIndex < simonLevel * 2) {
    unsigned long interval =
      (simonShowIndex % 2 == 0)
      ? (SIMON_LED_ON_TIME / 2)
      : (SIMON_LED_OFF_TIME / 2);

    if (millis() - simonTimer >= interval) {
      simonTimer = millis();

      if (simonShowIndex % 2 == 0) {
        uint8_t c = simonSequence[simonShowIndex / 2];
        simonAllLedsOff();
        if (c == 0) digitalWrite(LED_BTN_YELLOW, HIGH);
        if (c == 1) digitalWrite(LED_BTN_BLUE, HIGH);
        if (c == 2) digitalWrite(LED_BTN_GREEN, HIGH);
        if (c == 3) digitalWrite(LED_BTN_RED, HIGH);
        simonShowRingColor(c);  // Show on ring
      } else {
        simonAllLedsOff();
        simonClearRing();
      }

      simonShowIndex++;
    }
  }

  simonAllLedsOff();
  simonClearRing();
  delay(400);

  // Display final score
  display.clearDisplay();
  display.setTextSize(2);

  if (isNewHighscore) {
    display.setCursor(centerX("NEW", 2), 0);
    display.print("NEW");
    display.setCursor(centerX("RECORD!", 2), 18);
    display.print("RECORD!");
  } else {
    display.setCursor(centerX("GAME", 2), 0);
    display.print("GAME");
    display.setCursor(centerX("OVER", 2), 18);
    display.print("OVER");
  }

  display.setTextSize(1);
  display.setCursor(centerX("LEVEL:", 1), 40);
  display.print("LEVEL:");

  display.setTextSize(2);
  display.setCursor(centerX(String(finalScore), 2), 50);
  display.print(finalScore);

  display.display();

  playTone(BUZZER_PIN, 1500, 500);
  delay(5000);
}


/* ---------- SIMON START ---------- */
void simonStartGame() {
  randomSeed(millis());
  for (int i = 0; i < SIMON_MAX_LEVEL; i++) {
    simonSequence[i] = random(0, 4);
  }

  simonLevel = 1;
  simonInputIndex = 0;
  simonShowIndex = 0;

  // Clear ring for SIMON (already initialized in setup)
  raceRing.clear();
  raceRing.show();

  runSimonCountdown();
  simonTimer = millis();
  simonState = SIMON_SHOW_SEQUENCE;
}

/* ---------- SIMON LOOP ---------- */
void simonGameLoop() {

  switch (simonState) {

    case SIMON_IDLE: {
      static unsigned long blinkTimer = 0;
      static bool ledOn = false;

      if (millis() - blinkTimer > 700) {
        blinkTimer = millis();
        ledOn = !ledOn;
        digitalWrite(LED_BTN_YELLOW, ledOn ? HIGH : LOW);
      }

      // Turn off LED strip (only ring for SIMON)
      strip.clear();
      strip.show();
      
      // Brightness control with RED/BLUE buttons
      handleMiniLineBrightness();

      // Show rotating quarters on ring (4 colors)
      if (currentSystemGame == SYS_GAME_SIMON) {
        static unsigned long rotateTimer = 0;
        static float rotationOffset = 0;
        
        if (millis() - rotateTimer >= 30) {
          rotateTimer = millis();
          rotationOffset += 0.5;  // Slow rotation
          if (rotationOffset >= RACE_LED_COUNT) rotationOffset = 0;
          
          raceRing.clear();
          for (int i = 0; i < RACE_LED_COUNT; i++) {
            int pos = (i + (int)rotationOffset) % RACE_LED_COUNT;
            uint32_t color;
            
            if (pos < RACE_LED_COUNT / 4) {
              color = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B);  // YELLOW
            } else if (pos < RACE_LED_COUNT / 2) {
              color = raceRing.Color(0, 0, 255);    // BLUE
            } else if (pos < 3 * RACE_LED_COUNT / 4) {
              color = raceRing.Color(0, 255, 0);    // GREEN
            } else {
              color = raceRing.Color(255, 0, 0);    // RED
            }
            
            raceRing.setPixelColor(i, color);
          }
          raceRing.show();
        }
      }

      display.clearDisplay();

      // Brightness top left
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("BR:");
      display.print(brightness);

      // SIMON title - smaller (textsize 2 instead of 3)
      display.setTextSize(2);
      display.setCursor(centerX("SIMON",2), 12);
      display.print("SIMON");

      display.setTextSize(1);
      display.setCursor(centerX("HI-SCORE:",1), 34);
      display.print("HI-SCORE:");

      display.setTextSize(2);
      display.setCursor(centerX(String(simonHighScore),2), 44);
      display.print(simonHighScore);

      // Check for mute toggle (2-second green button hold)
      checkMuteToggle();

      // Draw speaker icon
      drawSpeakerIcon();

      display.display();

      if (digitalRead(BTN_YELLOW) == LOW) {
        simonAllLedsOff();
        delay(200);
        simonStartGame();
      }
      break;
    }

    case SIMON_SHOW_SEQUENCE: {
      display.clearDisplay();

      display.setTextSize(1);
      display.setCursor(centerX("RUNDE",1), 10);
      display.print("RUNDE");

      display.setTextSize(3);
      display.setCursor(centerX(String(simonLevel),3), 30);
      display.print(simonLevel);

      display.display();

      unsigned long interval =
        (simonShowIndex % 2 == 0)
        ? SIMON_LED_ON_TIME
        : SIMON_LED_OFF_TIME;

      if (millis() - simonTimer >= interval) {
        simonTimer = millis();

        if (simonShowIndex % 2 == 0) {
          uint8_t c = simonSequence[simonShowIndex / 2];
          simonAllLedsOff();
          if (c == 0) digitalWrite(LED_BTN_YELLOW, HIGH);
          if (c == 1) digitalWrite(LED_BTN_BLUE, HIGH);
          if (c == 2) digitalWrite(LED_BTN_GREEN, HIGH);
          if (c == 3) digitalWrite(LED_BTN_RED, HIGH);
          // Show color on LED ring
          simonShowRingColor(c);
        } else {
          simonAllLedsOff();
          simonClearRing();
        }

        simonShowIndex++;

        if (simonShowIndex >= simonLevel * 2) {
          simonShowIndex = 0;
          simonInputIndex = 0;
          simonClearRing();  // Clear ring when sequence done
          simonState = SIMON_PLAYER_INPUT;
        }
      }
      break;
    }

    case SIMON_PLAYER_INPUT: {
      display.clearDisplay();

      display.setTextSize(1);
      display.setCursor(centerX("RUNDE",1), 10);
      display.print("RUNDE");

      display.setTextSize(3);
      display.setCursor(centerX(String(simonLevel),3), 30);
      display.print(simonLevel);

      display.display();

      int pressed = -1;

      if (digitalRead(BTN_YELLOW) == LOW) pressed = 0;
      if (digitalRead(BTN_BLUE)   == LOW) pressed = 1;
      if (digitalRead(BTN_GREEN)  == LOW) pressed = 2;
      if (digitalRead(BTN_RED)    == LOW) pressed = 3;

      if (pressed != -1) {
        // visual feedback
        if (pressed == 0) digitalWrite(LED_BTN_YELLOW, HIGH);
        if (pressed == 1) digitalWrite(LED_BTN_BLUE, HIGH);
        if (pressed == 2) digitalWrite(LED_BTN_GREEN, HIGH);
        if (pressed == 3) digitalWrite(LED_BTN_RED, HIGH);
        simonShowRingColor(pressed);  // Show on LED ring
        delay(150);
        simonAllLedsOff();
        simonClearRing();
        delay(150);

        if (pressed != simonSequence[simonInputIndex]) {
          simonState = SIMON_GAME_OVER;
        } else {
          simonInputIndex++;
          if (simonInputIndex >= simonLevel) {
            simonLevel++;
            simonShowIndex = 0;
            simonTimer = millis();
            simonState = SIMON_SHOW_SEQUENCE;
          }
        }
      }
      break;
    }

    case SIMON_GAME_OVER: {
      int finalScore = simonLevel - 1;
      bool isNewHighscore = (finalScore > simonHighScore);

      if (isNewHighscore) {
        simonHighScore = finalScore;
        EEPROM.put(EEPROM_SIMON_HIGHSCORE_ADDR, simonHighScore);
        EEPROM.commit();
      }

      runSimonGameOver(finalScore, isNewHighscore);
      simonState = SIMON_IDLE;
      break;
    }
  }
}

/* ===================== SYSTEM GAME SWITCHING ===================== */

void handleSystemGreenLongPress() {
  bool greenPressed = digitalRead(BTN_GREEN) == LOW;

  if (greenPressed && !greenLongPressActive) {
    greenLongPressActive = true;
    greenLongPressStart = millis();
    greenLongPressDone = false;
  }

  if (!greenPressed) {
    greenLongPressActive = false;
    greenLongPressDone = false;
  }

  if (greenLongPressActive && !greenLongPressDone &&
      millis() - greenLongPressStart >= 200) {  // 1 second hold

    // Only allow game switching when in IDLE state
    bool inIdle = false;
    switch (currentSystemGame) {
      case SYS_GAME_SIMON:
        inIdle = (simonState == SIMON_IDLE);
        break;
      case SYS_GAME_RACE:
        inIdle = (raceState == RACE_IDLE);
        break;
      case SYS_GAME_MINILINE:
        inIdle = (miniLineState == MINILINE_IDLE);
        break;
      case SYS_GAME_CHAOSRUN:
        inIdle = (chaosRunState == CHAOSRUN_IDLE);
        break;
      case SYS_GAME_BUILDER:
        inIdle = (builderState == BUILDER_IDLE);
        break;
      case SYS_GAME_BUILDALONE:
        inIdle = (buildAloneState == BUILDALONE_IDLE);
        break;
      case SYS_GAME_COUNTDOWNGAME:
        inIdle = (countdownGameState == COUNTDOWNGAME_IDLE);
        break;
      case SYS_GAME_REAKTION:
        inIdle = (reaktionState == REAKTION_IDLE);
        break;
      case SYS_GAME_SIMONMULTI:
        inIdle = (simonMultiState == SIMONMULTI_IDLE);
        break;
      case SYS_GAME_COLORRUN:
        inIdle = (colorRunState == COLORRUN_IDLE);
        break;
      case SYS_GAME_SPEED:
        inIdle = (speedState == SPEED_IDLE);
        break;
      case SYS_GAME_REFLEXRACE:
        inIdle = (reflexRaceState == REFLEXRACE_IDLE);
        break;
      case SYS_GAME_ENEMYS:
        inIdle = (enemysState == ENEMYS_IDLE);
        break;
      case SYS_GAME_CHAOS:
        inIdle = (gameState == IDLE);
        break;
      case SYS_GAME_CLOCK:
        inIdle = (clockState == CLOCK_DISPLAY);
        break;
    }

    if (!inIdle) {
      // Not in IDLE - ignore green button press
      greenLongPressDone = true;
      return;
    }

    // Switch game (12 games total, including CLOCK)
    currentSystemGame = (SystemGame)((currentSystemGame + 1) % NUM_GAMES);

    // Reset ALL game states
    simonState = SIMON_IDLE;
    raceState = RACE_IDLE;
    miniLineState = MINILINE_IDLE;
    chaosRunState = CHAOSRUN_IDLE;
    builderState = BUILDER_IDLE;
    buildAloneState = BUILDALONE_IDLE;
    countdownGameState = COUNTDOWNGAME_IDLE;
    reaktionState = REAKTION_IDLE;
    simonMultiState = SIMONMULTI_IDLE;
    colorRunState = COLORRUN_IDLE;
    speedState = SPEED_IDLE;
    reflexRaceState = REFLEXRACE_INTRO;
    clockState = CLOCK_INTRO;  // Always reset clock to INTRO

    // Set main game state - but skip INTRO when going to CHAOS
    if (currentSystemGame == SYS_GAME_CHAOS) {
      gameState = IDLE;  // Skip INTRO, go directly to IDLE
    } else {
      gameState = INTRO;  // All other games (including CLOCK) show INTRO
    }

    // Single beep feedback
    digitalWrite(LED_BTN_GREEN, HIGH);
    playTone(BUZZER_PIN, 1200, 150);
    delay(200);
    digitalWrite(LED_BTN_GREEN, LOW);

    greenLongPressDone = true;
  }
}

/* =====================================================
   ====================== LOOP =========================
   ===================================================== */

// Check for any button activity to reset idle timer
void checkIdleActivity() {
  static bool lastAnyButton = false;
  bool anyButton = (digitalRead(BTN_RED) == LOW || digitalRead(BTN_GREEN) == LOW ||
                    digitalRead(BTN_BLUE) == LOW || digitalRead(BTN_YELLOW) == LOW);

  if (anyButton && !lastAnyButton) {
    // Button was just pressed - reset idle timer
    lastIdleActivity = millis();
  }
  lastAnyButton = anyButton;
}

// Check if idle timeout has passed and switch to clock
void checkIdleTimeout() {
  // Only check if WiFi is connected and not already in clock mode
  if (!wifiConnected) return;
  if (currentSystemGame == SYS_GAME_CLOCK) return;

  // Check if we're in an IDLE state (not in active game)
  bool inIdleState = false;
  switch (currentSystemGame) {
    case SYS_GAME_SIMON:
      inIdleState = (simonState == SIMON_IDLE);
      break;
    case SYS_GAME_RACE:
      inIdleState = (raceState == RACE_IDLE);
      break;
    case SYS_GAME_MINILINE:
      inIdleState = (miniLineState == MINILINE_IDLE);
      break;
    case SYS_GAME_CHAOSRUN:
      inIdleState = (chaosRunState == CHAOSRUN_IDLE);
      break;
    case SYS_GAME_BUILDER:
      inIdleState = (builderState == BUILDER_IDLE);
      break;
    case SYS_GAME_BUILDALONE:
      inIdleState = (buildAloneState == BUILDALONE_IDLE);
      break;
    case SYS_GAME_COUNTDOWNGAME:
      inIdleState = (countdownGameState == COUNTDOWNGAME_IDLE);
      break;
    case SYS_GAME_REAKTION:
      inIdleState = (reaktionState == REAKTION_IDLE);
      break;
    case SYS_GAME_SIMONMULTI:
      inIdleState = (simonMultiState == SIMONMULTI_IDLE);
      break;
    case SYS_GAME_COLORRUN:
      inIdleState = (colorRunState == COLORRUN_IDLE);
      break;
    case SYS_GAME_SPEED:
      inIdleState = (speedState == SPEED_IDLE);
      break;
    case SYS_GAME_ENEMYS:
      inIdleState = (enemysState == ENEMYS_IDLE);
      break;
    case SYS_GAME_CHAOS:
      inIdleState = (gameState == IDLE);
      break;
    default:
      break;
  }

  // If in idle state and timeout exceeded, switch to clock
  if (inIdleState && (millis() - lastIdleActivity >= IDLE_TIMEOUT_MS)) {
    currentSystemGame = SYS_GAME_CLOCK;
    clockState = CLOCK_INTRO;  // Show intro first
  }
}

void loop() {
  handleFourButtonReset();
  handleSystemGreenLongPress();
  checkIdleActivity();
  checkIdleTimeout();

  switch (currentSystemGame) {
    case SYS_GAME_SIMON: simonGameLoop(); break;
    case SYS_GAME_RACE:  raceGameLoop(); break;
    case SYS_GAME_MINILINE: miniLineLoop(); break;
    case SYS_GAME_CHAOSRUN: chaosRunLoop(); break;
    case SYS_GAME_BUILDER: builderLoop(); break;
    case SYS_GAME_BUILDALONE: buildAloneLoop(); break;
    case SYS_GAME_COUNTDOWNGAME: countdownGameLoop(); break;
    case SYS_GAME_REAKTION: reaktionLoop(); break;
    case SYS_GAME_SIMONMULTI: simonMultiLoop(); break;
    case SYS_GAME_COLORRUN: colorRunLoop(); break;
    case SYS_GAME_SPEED: speedLoop(); break;
    case SYS_GAME_REFLEXRACE: reflexRaceLoop(); break;
    case SYS_GAME_ENEMYS: enemysLoop(); break;
    case SYS_GAME_CHAOS: chaosLoop(); break;
    case SYS_GAME_CLOCK: clockLoop(); break;
  }
}

/* ===================== MINI LINE IMPLEMENTATION ===================== */

// MINI LINE uses same structs as THE LINE
Enemy miniEnemies[MAX_ENEMIES];
Shot miniShots[MAX_SHOTS];

int miniEnemyCount = 0;
int miniRoundNr = 1;
int miniLastFinishedRound = 0;
int miniEnemySpeed;
int miniWaveOffset;

unsigned long miniLastEnemyMove = 0;
unsigned long miniLastShotMove = 0;

bool miniLastRed = HIGH;
bool miniLastGreen = HIGH;
bool miniLastBlue = HIGH;

void miniLineLoop() {
  handleYellowReset();
  updateButtonLEDs();

  switch (miniLineState) {
    case MINILINE_INTRO:     showIntro(); miniLineState = MINILINE_IDLE; break;
    case MINILINE_IDLE:      miniLineIdle(); break;
    case MINILINE_COUNTDOWN: runCountdown(); miniLineStartGame(); miniLineState = MINILINE_GAME; break;
    case MINILINE_GAME:      miniLineRunGame(); break;
    case MINILINE_GAMEOVER:  miniLineGameOver(); miniLineState = MINILINE_IDLE; break;
  }
}

void miniLineIdle() {
  // Rainbow on ring
  static unsigned long rainbowTimer = 0;
  if (millis() - rainbowTimer >= 30) {
    rainbowTimer = millis();
    static uint16_t h = 0;
    for (int i = 0; i < RACE_LED_COUNT; i++)
      raceRing.setPixelColor(i, raceRing.gamma32(raceRing.ColorHSV(h + i * 65535L / RACE_LED_COUNT)));
    raceRing.show();
    h += 40;
  }

  softBlinkYellow();
  handleMiniLineBrightness();  // Use MINI LINE brightness (max 200)
  
  // Turn off LED strip (only ring for MINI LINE)
  strip.clear();
  strip.show();
  
  // Display
  display.clearDisplay();
  
  // Title - MINI smaller, LINE bigger
  display.setTextSize(1);
  display.setCursor(centerX("MINI",1), 8);
  display.print("MINI");
  
  display.setTextSize(2);
  display.setCursor(centerX("LINE",2), 18);
  display.print("LINE");
  
  // Highscore
  display.setTextSize(1);
  display.setCursor(centerX("HI-SCORE:",1), 40);
  display.print("HI-SCORE:");
  
  display.setTextSize(2);
  display.setCursor(centerX(String(miniLineHighScore),2), 48);
  display.print(miniLineHighScore);
  
  // Brightness display (same as THE LINE)
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    miniLineState = MINILINE_COUNTDOWN;
}

void miniLineStartGame() {
  miniEnemyCount = miniRoundNr;
  for (int i = 0; i < miniEnemyCount; i++)
    miniEnemies[i].color = randomColor();

  miniEnemySpeed = max(BASE_ENEMY_SPEED - miniRoundNr * 10, SPEED_MIN);
  miniWaveOffset = RACE_LED_COUNT - 1;

  // DON'T clear shots - they should continue between rounds!
  raceRing.clear();
  raceRing.show();
}

void miniLineRunGame() {
  unsigned long now = millis();

  // Turn on button LEDs during game (YELLOW stays OFF)
  digitalWrite(LED_BTN_RED, HIGH);
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_BLUE, HIGH);
  digitalWrite(LED_BTN_YELLOW, LOW);

  // Move enemies forward
  if (now - miniLastEnemyMove > miniEnemySpeed) {
    miniLastEnemyMove = now;
    miniWaveOffset--;
    if (miniWaveOffset <= 0) {
      miniLastFinishedRound = miniRoundNr;
      miniLineState = MINILINE_GAMEOVER;
    }
  }

  // Move shots forward
  if (now - miniLastShotMove > SHOT_SPEED) {
    miniLastShotMove = now;
    for (auto &s : miniShots)
      if (s.active && ++s.pos >= RACE_LED_COUNT)
        s.active = false;
  }

  miniLineHandleShots();
  miniLineCheckCollision();
  miniLineDrawRing();
  miniLineDrawDisplay();
}

void miniLineHandleShots() {
  bool r = digitalRead(BTN_RED);
  bool g = digitalRead(BTN_GREEN);
  bool b = digitalRead(BTN_BLUE);

  if (miniLastRed == HIGH && r == LOW) miniLineSpawnShot(raceRing.Color(255,0,0));
  if (miniLastGreen == HIGH && g == LOW) miniLineSpawnShot(raceRing.Color(0,255,0));
  if (miniLastBlue == HIGH && b == LOW) miniLineSpawnShot(raceRing.Color(0,0,255));

  miniLastRed = r;
  miniLastGreen = g;
  miniLastBlue = b;
}

void miniLineSpawnShot(uint32_t c) {
  for (auto &s : miniShots)
    if (!s.active) {
      s = {0, c, true};
      playTone(BUZZER_PIN, 800, 30);
      return;
    }
}

void miniLineCheckCollision() {
  if (miniEnemyCount == 0) {
    miniRoundNr++;
    miniLineStartGame();
    return;
  }

  for (auto &s : miniShots) {
    if (!s.active) continue;

    if (s.pos >= miniWaveOffset) {
      s.active = false;

      if (s.color == miniEnemies[0].color) {
        // Correct color - remove enemy
        for (int i = 0; i < miniEnemyCount - 1; i++)
          miniEnemies[i] = miniEnemies[i + 1];
        miniEnemyCount--;
        miniWaveOffset++;   // Tempo compensation
        playTone(BUZZER_PIN, 1500, 50);
      } else {
        // Wrong color - shot becomes enemy at the back
        miniEnemies[miniEnemyCount++].color = s.color;
        miniEnemySpeed = max(miniEnemySpeed - SPEED_STEP, SPEED_MIN);
        playTone(BUZZER_PIN, 300, 100);
      }
      return;
    }
  }
}

void miniLineDrawRing() {
  raceRing.clear();

  for (int i = 0; i < miniEnemyCount; i++) {
    int p = miniWaveOffset + i;
    if (p >= 0 && p < RACE_LED_COUNT) {
      int displayPos = (p + MINILINE_START_OFFSET) % RACE_LED_COUNT;
      raceRing.setPixelColor(displayPos, miniEnemies[i].color);
    }
  }

  for (auto &s : miniShots)
    if (s.active && s.pos < RACE_LED_COUNT) {
      int displayPos = (s.pos + MINILINE_START_OFFSET) % RACE_LED_COUNT;
      raceRing.setPixelColor(displayPos, s.color);
    }

  // White marker at LED 23 (spawn position)
  raceRing.setPixelColor(MINILINE_START_OFFSET, raceRing.Color(255, 255, 255));

  raceRing.show();
}

void miniLineDrawDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(centerX("RUNDE",1),10);
  display.print("RUNDE");

  display.setTextSize(3);
  display.setCursor(centerX(String(miniRoundNr),3),30);
  display.print(miniRoundNr);

  display.display();
}

void miniLineGameOver() {
  int notes[] = {392,330,262,196};
  for (int i = 0; i < 4; i++) {
    playTone(BUZZER_PIN, notes[i], 300);
    delay(350);
  }

  unsigned long start = millis();
  bool on = false;
  while (millis() - start < 8000) {
    on = !on;
    raceRing.clear();
    if (on) {
      for (int i = 0; i < RACE_LED_COUNT; i++)
        raceRing.setPixelColor(i, raceRing.Color(255,0,0));
    }
    raceRing.show();

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(centerX("RUNDE ERREICHT",1),10);
    display.print("RUNDE ERREICHT");

    display.setTextSize(3);
    display.setCursor(centerX(String(miniLastFinishedRound),3),30);
    display.print(miniLastFinishedRound);
    display.display();

    delay(400);
  }

  if (miniLastFinishedRound > miniLineHighScore) {
    miniLineHighScore = miniLastFinishedRound;
    EEPROM.put(EEPROM_MINILINE_HIGHSCORE_ADDR, miniLineHighScore);
    EEPROM.commit();
  }

  miniRoundNr = 1;
}

/* ===================== RACE IMPLEMENTATION ===================== */

const uint8_t raceStartPos[4] = {23, 14, 5, 32};
bool raceActive[4];
uint8_t racePos[4];
uint8_t racePlayersLeft = 0;

// Button edge detection for Race
bool raceLastBtnRed = HIGH;
bool raceLastBtnGreen = HIGH;
bool raceLastBtnYellow = HIGH;
bool raceLastBtnBlue = HIGH;

// Elimination display timer
unsigned long raceEliminationTimer = 0;
bool raceShowingElimination = false;

/* ================= SIMON LED RING HELPER ================= */
void simonShowRingColor(uint8_t colorIndex) {
  uint32_t color;
  if (colorIndex == 0) color = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B);  // Yellow
  else if (colorIndex == 1) color = raceRing.Color(0, 0, 255);  // Blue
  else if (colorIndex == 2) color = raceRing.Color(0, 255, 0);  // Green
  else color = raceRing.Color(255, 0, 0);  // Red
  
  raceRing.fill(color);
  raceRing.show();
}

void simonClearRing() {
  raceRing.clear();
  raceRing.show();
}

void startRaceGame() {
  // Enable all player button LEDs
  digitalWrite(LED_BTN_RED, HIGH);
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_YELLOW, HIGH);
  digitalWrite(LED_BTN_BLUE, HIGH);

  // Clear ring for RACE (already initialized in setup)
  raceRing.clear();
  raceRing.show();
  
  racePlayersLeft = 4;
  for (uint8_t i = 0; i < 4; i++) {
    raceActive[i] = true;
    racePos[i] = raceStartPos[i];
  }
  
  // Reset button states
  raceLastBtnRed = HIGH;
  raceLastBtnGreen = HIGH;
  raceLastBtnYellow = HIGH;
  raceLastBtnBlue = HIGH;
  
  raceShowingElimination = false;
  
  updateRaceRing();
  
  // Update display
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("RACE!", 2), 10);
  display.print("RACE!");
  display.setTextSize(1);
  display.setCursor(centerX("4 Players", 1), 40);
  display.print("4 Players");
  display.display();
}

void raceGameLoop() {
  if (raceState == RACE_IDLE) {
    // Ensure continuous blinking in idle
    softBlinkYellow();
    
    // Turn off LED strip (only ring for RACE)
    strip.clear();
    strip.show();
    
    // Brightness control with RED/BLUE buttons
    handleMiniLineBrightness();
    
    // Show 4 start positions on ring
    raceRing.clear();
    raceRing.setPixelColor(raceStartPos[0], raceRing.Color(255, 0, 0));     // RED at 23
    raceRing.setPixelColor(raceStartPos[1], raceRing.Color(0, 255, 0));     // GREEN at 14
    raceRing.setPixelColor(raceStartPos[2], raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B));   // YELLOW at 5
    raceRing.setPixelColor(raceStartPos[3], raceRing.Color(0, 0, 255));     // BLUE at 32
    raceRing.show();
    
    display.clearDisplay();
    
    // Brightness top left
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("BR:");
    display.print(brightness);
    
    // "THE" small above "RACE"
    display.setTextSize(1);
    display.setCursor(centerX("THE", 1), 8);
    display.print("THE");
    
    display.setTextSize(3);
    display.setCursor(centerX("RACE", 3), 20);
    display.print("RACE");
    
    display.setTextSize(1);
    display.setCursor(centerX("4 Players", 1), 50);
    display.print("4 Players");

    // Check for mute toggle (2-second green button hold)
    checkMuteToggle();

    // Draw speaker icon
    drawSpeakerIcon();

    display.display();

    if (digitalRead(BTN_YELLOW) == LOW) {
      runRaceCountdown();  // Use RACE countdown with all 4 buttons!
      startRaceGame();
      raceState = RACE_GAME;
    }
    return;
  }

  if (raceState == RACE_GAME) {
    // Handle elimination display timeout
    if (raceShowingElimination && millis() - raceEliminationTimer > 1500) {
      raceShowingElimination = false;
      // Update normal game display
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(centerX("RACE!", 2), 10);
      display.print("RACE!");
      display.setTextSize(1);
      String playersText = String(racePlayersLeft) + " Player";
      if (racePlayersLeft != 1) playersText += "s";
      display.setCursor(centerX(playersText, 1), 40);
      display.print(playersText);
      display.display();
    }
    
    // Edge detection for button presses
    bool currRed = digitalRead(BTN_RED);
    bool currGreen = digitalRead(BTN_GREEN);
    bool currYellow = digitalRead(BTN_YELLOW);
    bool currBlue = digitalRead(BTN_BLUE);

    bool moved = false;  // Track if any move happened this loop

    // Red button - Player 0
    if (raceLastBtnRed == HIGH && currRed == LOW) {
      raceMove(0);
      moved = true;
      raceLastBtnRed = LOW;
      delay(30);  // Immediate debounce
      // Update state after debounce to catch any releases
      raceLastBtnRed = digitalRead(BTN_RED);
    } else {
      raceLastBtnRed = currRed;
    }

    // Green button - Player 1
    if (raceLastBtnGreen == HIGH && currGreen == LOW) {
      raceMove(1);
      moved = true;
      raceLastBtnGreen = LOW;
      delay(30);  // Immediate debounce
      raceLastBtnGreen = digitalRead(BTN_GREEN);
    } else {
      raceLastBtnGreen = currGreen;
    }

    // Yellow button - Player 2
    if (raceLastBtnYellow == HIGH && currYellow == LOW) {
      raceMove(2);
      moved = true;
      raceLastBtnYellow = LOW;
      delay(30);  // Immediate debounce
      raceLastBtnYellow = digitalRead(BTN_YELLOW);
    } else {
      raceLastBtnYellow = currYellow;
    }

    // Blue button - Player 3
    if (raceLastBtnBlue == HIGH && currBlue == LOW) {
      raceMove(3);
      moved = true;
      raceLastBtnBlue = LOW;
      delay(30);  // Immediate debounce
      raceLastBtnBlue = digitalRead(BTN_BLUE);
    } else {
      raceLastBtnBlue = currBlue;
    }

    // Additional small delay if any button was pressed
    if (moved) {
      delay(10);  // Small additional delay
    }
    
    // Check for game over
    if (racePlayersLeft <= 1) {
      raceState = RACE_GAMEOVER;
    }
    return;
  }

  if (raceState == RACE_GAMEOVER) {
    // Find the winner (the only active player left)
    uint8_t winner = 0;
    for (uint8_t i = 0; i < 4; i++) {
      if (raceActive[i]) {
        winner = i;
        break;
      }
    }
    
    // Determine winner color name and LED color
    String winnerName;
    uint32_t winnerColor;
    
    if (winner == 0) {
      winnerName = "RED";
      winnerColor = raceRing.Color(255, 0, 0);
    } else if (winner == 1) {
      winnerName = "GREEN";
      winnerColor = raceRing.Color(0, 255, 0);
    } else if (winner == 2) {
      winnerName = "YELLOW";
      winnerColor = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B);
    } else {
      winnerName = "BLUE";
      winnerColor = raceRing.Color(0, 0, 255);
    }
    
    // Victory animation - fade entire ring in winner color
    for (int i = 0; i < 8; i++) {
      raceRing.fill(winnerColor);
      raceRing.show();
      playTone(BUZZER_PIN, 800 + i * 100, 150);
      delay(200);
      raceRing.clear();
      raceRing.show();
      delay(200);
    }
    
    // Show final winner color
    raceRing.fill(winnerColor);
    raceRing.show();
    
    // Display winner
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(centerX(winnerName, 2), 10);
    display.print(winnerName);
    display.setCursor(centerX("WIN!", 2), 35);
    display.print("WIN!");
    display.display();
    
    playTone(BUZZER_PIN, 1200, 500);
    delay(3000);
    
    // Turn off all button LEDs
    digitalWrite(LED_BTN_RED, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    digitalWrite(LED_BTN_YELLOW, LOW);
    digitalWrite(LED_BTN_BLUE, LOW);
    
    raceState = RACE_IDLE;
  }
}

void raceMove(uint8_t p) {
  if (!raceActive[p]) return;
  
  racePos[p] = (racePos[p] + 1) % RACE_LED_COUNT;
  checkRaceCollision(p);
  updateRaceRing();
}

void checkRaceCollision(uint8_t attacker) {
  for (uint8_t i = 0; i < 4; i++) {
    if (i != attacker && raceActive[i] && racePos[i] == racePos[attacker]) {
      eliminateRacePlayer(i);
    }
  }
}

void eliminateRacePlayer(uint8_t p) {
  // Dissolve effect with blinking
  for (uint8_t blink = 0; blink < 5; blink++) {
    raceActive[p] = false;
    updateRaceRing();
    playTone(BUZZER_PIN, 200 - (blink * 30), 80);
    delay(80);
    
    raceActive[p] = true;
    updateRaceRing();
    delay(80);
  }
  
  // Final elimination
  raceActive[p] = false;
  racePlayersLeft--;
  updateRaceRing();
  
  // Turn off player's button LED
  if (p == 0) digitalWrite(LED_BTN_RED, LOW);
  if (p == 1) digitalWrite(LED_BTN_GREEN, LOW);
  if (p == 2) digitalWrite(LED_BTN_YELLOW, LOW);
  if (p == 3) digitalWrite(LED_BTN_BLUE, LOW);
  
  // Show elimination on display
  displayRaceElimination(p);
  
  // Final elimination sound
  playTone(BUZZER_PIN, 150, 300);
}

void displayRaceElimination(uint8_t p) {
  String colorName;
  if (p == 0) colorName = "RED";
  else if (p == 1) colorName = "GREEN";
  else if (p == 2) colorName = "YELLOW";
  else colorName = "BLUE";
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX(colorName, 2), 10);
  display.print(colorName);
  display.setCursor(centerX("OUT!", 2), 35);
  display.print("OUT!");
  display.display();
  
  raceShowingElimination = true;
  raceEliminationTimer = millis();
}

void updateRaceRing() {
  raceRing.clear();
  for (uint8_t i = 0; i < 4; i++) {
    if (!raceActive[i]) continue;
    
    uint32_t c = (i == 0) ? raceRing.Color(255, 0, 0) :
                 (i == 1) ? raceRing.Color(0, 255, 0) :
                 (i == 2) ? raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B) :
                            raceRing.Color(0, 0, 255);
    raceRing.setPixelColor(racePos[i], c);
  }
  raceRing.show();
}

/* ===================== CHAOS RUN IMPLEMENTATION ===================== */

// CHAOS RUN game variables
uint8_t chaosRunTargetPos = 0;
uint8_t chaosRunRunnerPos = 0;
uint8_t chaosRunRunnerStartPos = 0;  // Track where runner started
int8_t chaosRunDirection = 1;  // Runner direction: 1 = clockwise, -1 = counter-clockwise
uint8_t chaosRunRound = 0;
uint8_t chaosRunScore[4] = {0, 0, 0, 0};  // RED, GREEN, YELLOW, BLUE
bool chaosRunPlayerLocked[4] = {false, false, false, false};
unsigned long chaosRunLockTime[4] = {0, 0, 0, 0};
unsigned long chaosRunLastMove = 0;
uint16_t chaosRunSpeed = 150;  // milliseconds per step

// Button edge detection for CHAOS RUN
bool chaosRunLastBtnRed = HIGH;
bool chaosRunLastBtnGreen = HIGH;
bool chaosRunLastBtnYellow = HIGH;
bool chaosRunLastBtnBlue = HIGH;

#define CHAOSRUN_MAX_ROUNDS 50
#define CHAOSRUN_LOCK_TIME 5000  // 5 seconds lock
#define CHAOSRUN_INITIAL_SPEED 150
#define CHAOSRUN_SPEED_DECREASE 3  // Speed up by 3ms each round

void chaosRunLoop() {
  handleYellowReset();

  switch (chaosRunState) {
    case CHAOSRUN_INTRO:
      showIntro();
      chaosRunState = CHAOSRUN_IDLE;
      break;
    case CHAOSRUN_IDLE:
      chaosRunIdle();
      break;
    case CHAOSRUN_COUNTDOWN:
      runRaceCountdown();  // Use RACE countdown with all 4 buttons!
      chaosRunStartGame();
      chaosRunState = CHAOSRUN_GAME;
      break;
    case CHAOSRUN_GAME:
      chaosRunGame();
      break;
    case CHAOSRUN_GAMEOVER:
      chaosRunGameOver();
      chaosRunState = CHAOSRUN_IDLE;
      break;
  }
}

void chaosRunIdle() {
  // Rainbow on ring
  static unsigned long rainbowTimer = 0;
  if (millis() - rainbowTimer >= 30) {
    rainbowTimer = millis();
    static uint16_t h = 0;
    for (int i = 0; i < RACE_LED_COUNT; i++)
      raceRing.setPixelColor(i, raceRing.gamma32(raceRing.ColorHSV(h + i * 65535L / RACE_LED_COUNT)));
    raceRing.show();
    h += 40;
  }

  softBlinkYellow();
  handleMiniLineBrightness();  // Brightness control (5-200)

  // Turn off LED strip (only ring for CHAOS RUN)
  strip.clear();
  strip.show();

  // Display
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("CHAOS", 2), 10);
  display.print("CHAOS");
  display.setCursor(centerX("RUN", 2), 30);
  display.print("RUN");
  display.setTextSize(1);
  display.setCursor(centerX("4 Players", 1), 50);
  display.print("4 Players");
  
  // Brightness display
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    chaosRunState = CHAOSRUN_COUNTDOWN;
}

void chaosRunStartGame() {
  // Reset game state
  chaosRunRound = 0;
  chaosRunSpeed = CHAOSRUN_INITIAL_SPEED;
  for (uint8_t i = 0; i < 4; i++) {
    chaosRunScore[i] = 0;
    chaosRunPlayerLocked[i] = false;
    chaosRunLockTime[i] = 0;
  }

  // All button LEDs on
  digitalWrite(LED_BTN_RED, HIGH);
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_YELLOW, HIGH);
  digitalWrite(LED_BTN_BLUE, HIGH);

  // Reset button states
  chaosRunLastBtnRed = HIGH;
  chaosRunLastBtnGreen = HIGH;
  chaosRunLastBtnYellow = HIGH;
  chaosRunLastBtnBlue = HIGH;

  // Start first round
  chaosRunNewRound();
}

void chaosRunNewRound() {
  chaosRunRound++;
  
  // Speed up
  if (chaosRunSpeed > 20) {
    chaosRunSpeed -= CHAOSRUN_SPEED_DECREASE;
  }

  // Random target position
  chaosRunTargetPos = random(2, RACE_LED_COUNT - 2);  // Leave space for buffer zone
  
  // Runner starts at opposite side
  chaosRunRunnerPos = (chaosRunTargetPos + RACE_LED_COUNT / 2) % RACE_LED_COUNT;
  chaosRunRunnerStartPos = chaosRunRunnerPos;  // Remember start position
  
  // Random direction (50% clockwise, 50% counter-clockwise)
  chaosRunDirection = (random(0, 2) == 0) ? 1 : -1;
  
  chaosRunLastMove = millis();

  // Clear ring and show initial state
  chaosRunDrawRing();
}

void chaosRunGame() {
  unsigned long now = millis();

  // Handle player locks (5 second timeout)
  for (uint8_t i = 0; i < 4; i++) {
    if (chaosRunPlayerLocked[i]) {
      unsigned long lockElapsed = now - chaosRunLockTime[i];
      
      // Blink warning 500ms before unlock
      if (lockElapsed >= CHAOSRUN_LOCK_TIME - 500 && lockElapsed < CHAOSRUN_LOCK_TIME) {
        static unsigned long blinkTimer = 0;
        if (now - blinkTimer > 100) {
          blinkTimer = now;
          // Toggle LED
          bool ledState = digitalRead(i == 0 ? LED_BTN_RED : i == 1 ? LED_BTN_GREEN : i == 2 ? LED_BTN_YELLOW : LED_BTN_BLUE);
          if (i == 0) digitalWrite(LED_BTN_RED, !ledState);
          else if (i == 1) digitalWrite(LED_BTN_GREEN, !ledState);
          else if (i == 2) digitalWrite(LED_BTN_YELLOW, !ledState);
          else digitalWrite(LED_BTN_BLUE, !ledState);
        }
      }
      
      // Unlock player
      if (lockElapsed >= CHAOSRUN_LOCK_TIME) {
        chaosRunPlayerLocked[i] = false;
        if (i == 0) digitalWrite(LED_BTN_RED, HIGH);
        else if (i == 1) digitalWrite(LED_BTN_GREEN, HIGH);
        else if (i == 2) digitalWrite(LED_BTN_YELLOW, HIGH);
        else digitalWrite(LED_BTN_BLUE, HIGH);
      }
    }
  }

  // Move runner (clockwise or counter-clockwise based on direction)
  if (now - chaosRunLastMove >= chaosRunSpeed) {
    chaosRunLastMove = now;
    // Move in direction (1 = clockwise, -1 = counter-clockwise)
    chaosRunRunnerPos = (chaosRunRunnerPos + chaosRunDirection + RACE_LED_COUNT) % RACE_LED_COUNT;
    chaosRunDrawRing();
  }

  // Check button presses with edge detection
  bool currRed = digitalRead(BTN_RED);
  bool currGreen = digitalRead(BTN_GREEN);
  bool currYellow = digitalRead(BTN_YELLOW);
  bool currBlue = digitalRead(BTN_BLUE);

  // RED player (0)
  if (chaosRunLastBtnRed == HIGH && currRed == LOW && !chaosRunPlayerLocked[0]) {
    chaosRunCheckHit(0);
  }
  chaosRunLastBtnRed = currRed;

  // GREEN player (1)
  if (chaosRunLastBtnGreen == HIGH && currGreen == LOW && !chaosRunPlayerLocked[1]) {
    chaosRunCheckHit(1);
  }
  chaosRunLastBtnGreen = currGreen;

  // YELLOW player (2)
  if (chaosRunLastBtnYellow == HIGH && currYellow == LOW && !chaosRunPlayerLocked[2]) {
    chaosRunCheckHit(2);
  }
  chaosRunLastBtnYellow = currYellow;

  // BLUE player (3)
  if (chaosRunLastBtnBlue == HIGH && currBlue == LOW && !chaosRunPlayerLocked[3]) {
    chaosRunCheckHit(3);
  }
  chaosRunLastBtnBlue = currBlue;

  // Update display
  chaosRunDrawDisplay();

  // Check if game over (after round 50, check for tie)
  if (chaosRunRound >= CHAOSRUN_MAX_ROUNDS) {
    // Find max score
    uint8_t maxScore = chaosRunScore[0];
    for (uint8_t i = 1; i < 4; i++) {
      if (chaosRunScore[i] > maxScore) {
        maxScore = chaosRunScore[i];
      }
    }
    
    // Count how many players have max score
    uint8_t playersWithMaxScore = 0;
    for (uint8_t i = 0; i < 4; i++) {
      if (chaosRunScore[i] == maxScore) {
        playersWithMaxScore++;
      }
    }
    
    // If tie, continue (Sudden Death)
    // If clear winner, game over
    if (playersWithMaxScore == 1) {
      chaosRunState = CHAOSRUN_GAMEOVER;
    }
    // else continue playing (Sudden Death mode)
  }
}

void chaosRunCheckHit(uint8_t player) {
  // Determine hit zone based on round
  bool isHit = false;
  
  if (chaosRunRound <= 3) {
    // First 3 rounds: 3-LED target (buffer zone)
    uint8_t targetLeft = (chaosRunTargetPos - 1 + RACE_LED_COUNT) % RACE_LED_COUNT;
    uint8_t targetRight = (chaosRunTargetPos + 1) % RACE_LED_COUNT;
    isHit = (chaosRunRunnerPos == targetLeft || 
             chaosRunRunnerPos == chaosRunTargetPos || 
             chaosRunRunnerPos == targetRight);
  } else {
    // Round 4+: Single LED target
    isHit = (chaosRunRunnerPos == chaosRunTargetPos);
  }
  
  if (isHit) {
    // HIT!
    chaosRunScore[player]++;
    playTone(BUZZER_PIN, 1500, 100);
    
    // Visual feedback
    raceRing.fill(raceRing.Color(0, 255, 0));
    raceRing.show();
    delay(200);
  } else {
    // MISS! Lock player for 5 seconds
    chaosRunPlayerLocked[player] = true;
    chaosRunLockTime[player] = millis();
    
    // Flash button LED briefly
    for (int i = 0; i < 3; i++) {
      if (player == 0) digitalWrite(LED_BTN_RED, LOW);
      else if (player == 1) digitalWrite(LED_BTN_GREEN, LOW);
      else if (player == 2) digitalWrite(LED_BTN_YELLOW, LOW);
      else digitalWrite(LED_BTN_BLUE, LOW);
      playTone(BUZZER_PIN, 200, 50);
      delay(100);
      if (player == 0) digitalWrite(LED_BTN_RED, HIGH);
      else if (player == 1) digitalWrite(LED_BTN_GREEN, HIGH);
      else if (player == 2) digitalWrite(LED_BTN_YELLOW, HIGH);
      else digitalWrite(LED_BTN_BLUE, HIGH);
      delay(100);
    }
    
    // Turn LED off (locked)
    if (player == 0) digitalWrite(LED_BTN_RED, LOW);
    else if (player == 1) digitalWrite(LED_BTN_GREEN, LOW);
    else if (player == 2) digitalWrite(LED_BTN_YELLOW, LOW);
    else digitalWrite(LED_BTN_BLUE, LOW);
  }
  
  // Start new round after EVERY button press (hit or miss)
  if (chaosRunRound < CHAOSRUN_MAX_ROUNDS) {
    chaosRunNewRound();
  }
}

void chaosRunDrawRing() {
  raceRing.clear();
  
  // Show target based on round
  if (chaosRunRound <= 3) {
    // First 3 rounds: 3-LED buffer zone (GREEN-WHITE-GREEN)
    uint8_t targetLeft = (chaosRunTargetPos - 1 + RACE_LED_COUNT) % RACE_LED_COUNT;
    uint8_t targetRight = (chaosRunTargetPos + 1) % RACE_LED_COUNT;
    raceRing.setPixelColor(targetLeft, raceRing.Color(0, 255, 0));      // Green
    raceRing.setPixelColor(chaosRunTargetPos, raceRing.Color(255, 255, 255));  // White
    raceRing.setPixelColor(targetRight, raceRing.Color(0, 255, 0));     // Green
  } else {
    // Round 4+: Single white LED
    raceRing.setPixelColor(chaosRunTargetPos, raceRing.Color(255, 255, 255));
  }
  
  // Show runner (red) - overwrites target if on same position
  raceRing.setPixelColor(chaosRunRunnerPos, raceRing.Color(255, 0, 0));
  
  raceRing.show();
}

void chaosRunDrawDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  
  // Round info - top
  display.setCursor(0, 0);
  if (chaosRunRound > CHAOSRUN_MAX_ROUNDS) {
    // Sudden Death mode
    display.print("SUDDEN DEATH!");
  } else {
    display.print("Rnd:");
    display.print(chaosRunRound);
    display.print("/");
    display.print(CHAOSRUN_MAX_ROUNDS);
  }
  
  // Scores - larger text
  display.setTextSize(2);
  
  // Red & Green on first row
  display.setCursor(0, 16);
  display.print("R:");
  display.print(chaosRunScore[0]);
  
  display.setCursor(64, 16);
  display.print("G:");
  display.print(chaosRunScore[1]);
  
  // Yellow & Blue on second row
  display.setCursor(0, 38);
  display.print("Y:");
  display.print(chaosRunScore[2]);
  
  display.setCursor(64, 38);
  display.print("B:");
  display.print(chaosRunScore[3]);
  
  display.display();
}

void chaosRunGameOver() {
  // Find winner
  uint8_t winner = 0;
  uint8_t maxScore = chaosRunScore[0];
  for (uint8_t i = 1; i < 4; i++) {
    if (chaosRunScore[i] > maxScore) {
      maxScore = chaosRunScore[i];
      winner = i;
    }
  }
  
  String winnerName;
  if (winner == 0) winnerName = "RED";
  else if (winner == 1) winnerName = "GREEN";
  else if (winner == 2) winnerName = "YELLOW";
  else winnerName = "BLUE";
  
  // Victory animation
  for (int i = 0; i < 5; i++) {
    raceRing.fill(raceRing.Color(0, 255, 0));
    raceRing.show();
    playTone(BUZZER_PIN, 800 + i * 200, 150);
    delay(200);
    raceRing.clear();
    raceRing.show();
    delay(100);
  }
  
  // Display winner
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX(winnerName, 2), 10);
  display.print(winnerName);
  display.setCursor(centerX("WINS!", 2), 35);
  display.print("WINS!");
  display.setTextSize(1);
  display.setCursor(centerX(String(maxScore) + " Points", 1), 55);
  display.print(maxScore);
  display.print(" Points");
  display.display();
  
  playTone(BUZZER_PIN, 1500, 500);
  delay(5000);
  
  // Turn off all button LEDs
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
}

/* ===================== BUILDER IMPLEMENTATION ===================== */

// BUILDER game variables
// Each player builds from center outward - yellow zones move to edges
// ROT player: LED 0-16, center at 8
// GREEN player: LED 18-34, center at 26

uint8_t builderRing[RACE_LED_COUNT];  // Field states

// Use virtual linear positions (0-14) for both runners for exact same speed
// Then map to actual LED positions
int8_t builderRunner1Virtual = 7;  // Virtual position (0-14, center=7)
int8_t builderRunner2Virtual = 7;  // Virtual position (0-14, center=7)
int8_t builderRunner1Dir = 1;   // ROT runner direction (1=right, -1=left)
int8_t builderRunner2Dir = 1;   // GREEN runner direction (1=right, -1=left)

// Actual LED positions (calculated from virtual positions)
uint8_t builderRunner1Pos = 14;
uint8_t builderRunner2Pos = 32;
unsigned long builderLastMove1 = 0;
unsigned long builderLastMove2 = 0;
uint16_t builderSpeed = 150;  // milliseconds per step

// Track build progress (yellow zones move outward)
uint8_t builderPlayer1LeftEdge = 7;   // Current left yellow zone for ROT
uint8_t builderPlayer1RightEdge = 9;  // Current right yellow zone for ROT
uint8_t builderPlayer2LeftEdge = 25;  // Current left yellow zone for GREEN
uint8_t builderPlayer2RightEdge = 27; // Current right yellow zone for GREEN

// Player lock (3 second penalty for wrong press)
bool builderPlayer1Locked = false;
bool builderPlayer2Locked = false;
unsigned long builderPlayer1LockTime = 0;
unsigned long builderPlayer2LockTime = 0;
#define BUILDER_LOCK_TIME 3000  // 3 seconds

// Button edge detection for BUILDER
bool builderLastBtnBlue = HIGH;
bool builderLastBtnGreen = HIGH;

#define BUILDER_BLUE_CENTER 14    // BLUE center at LED 14
#define BUILDER_GREEN_CENTER 32   // GREEN center at LED 32
#define BUILDER_BLUE_START 7      // BLUE field start
#define BUILDER_BLUE_END 21       // BLUE field end (15 fields: 7-21)
#define BUILDER_GREEN_START 25    // GREEN field start
#define BUILDER_GREEN_END 39      // GREEN field end (15 fields: 25-39, wraps around)

// Helper function to check if position is in a field (handles wrap-around for green)
bool builderIsInBlueField(uint8_t pos) {
  return (pos >= BUILDER_BLUE_START && pos <= BUILDER_BLUE_END);
}

bool builderIsInGreenField(uint8_t pos) {
  return (pos >= BUILDER_GREEN_START && pos <= 35) || (pos >= 0 && pos <= (BUILDER_GREEN_END - 36));
}

void builderLoop() {
  handleYellowReset();

  switch (builderState) {
    case BUILDER_INTRO:
      showIntro();
      builderState = BUILDER_IDLE;
      break;
    case BUILDER_IDLE:
      builderIdle();
      break;
    case BUILDER_COUNTDOWN:
      runBuilderCountdown();  // Only RED and GREEN buttons blink
      builderStartGame();
      builderState = BUILDER_GAME;
      break;
    case BUILDER_GAME:
      builderGame();
      break;
    case BUILDER_GAMEOVER:
      builderGameOver();
      builderState = BUILDER_IDLE;
      break;
  }
}

void builderIdle() {
  // Show static playing field (no runners)
  raceRing.clear();

  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    uint32_t color;

    // BLUE player field (LED 7-21)
    if (i == BUILDER_BLUE_CENTER) {
      color = raceRing.Color(0, 0, 255);  // BLUE center
    } else if (builderIsInBlueField(i)) {
      color = raceRing.Color(60, 60, 60);  // WHITE field
    }
    // RED separators (LED 4, 5, 6 and 22, 23, 24)
    else if ((i >= 4 && i <= 6) || (i >= 22 && i <= 24)) {
      color = raceRing.Color(200, 0, 0);  // RED separator
    }
    // GREEN player field (LED 25-35, 0-3)
    else if (i == BUILDER_GREEN_CENTER) {
      color = raceRing.Color(0, 255, 0);  // GREEN center
    } else if (builderIsInGreenField(i)) {
      color = raceRing.Color(60, 60, 60);  // WHITE field
    } else {
      color = raceRing.Color(0, 0, 0);  // Black
    }

    raceRing.setPixelColor(i, color);
  }

  raceRing.show();

  // Blink YELLOW button LED in IDLE
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    static bool blinkState = false;
    blinkState = !blinkState;
    digitalWrite(LED_BTN_YELLOW, blinkState ? HIGH : LOW);
  }
  // Turn off RED, GREEN, and BLUE
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);

  handleMiniLineBrightness();  // Brightness control (5-200)

  // Turn off LED strip (only ring for BUILDER)
  strip.clear();
  strip.show();

  // Display - larger BUILDER text
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("BUILDER", 2), 18);
  display.print("BUILDER");
  display.setTextSize(1);
  display.setCursor(centerX("2 Players", 1), 42);
  display.print("2 Players");
  
  // Brightness display
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    builderState = BUILDER_COUNTDOWN;
}

void builderStartGame() {
  // Initialize ring - all fields WHITE, only centers in player color, yellow build zones next to center
  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    // BLUE player field (LED 7-21, 15 fields)
    if (i == BUILDER_BLUE_CENTER) {
      builderRing[i] = 3;  // BLUE center (start point)
    } else if (builderIsInBlueField(i)) {
      builderRing[i] = 15;  // WHITE (pre-filled field)
    }
    // RED separators (LED 4, 5, 6 and 22, 23, 24)
    else if ((i >= 4 && i <= 6) || (i >= 22 && i <= 24)) {
      builderRing[i] = 100;  // RED separator
    }
    // GREEN player field (LED 25-35, 0-3, 15 fields)
    else if (i == BUILDER_GREEN_CENTER) {
      builderRing[i] = 10;  // GREEN center (start point)
    } else if (builderIsInGreenField(i)) {
      builderRing[i] = 15;  // WHITE (pre-filled field)
    }
  }

  // Set initial yellow build zones (one step from center)
  builderPlayer1LeftEdge = 13;  // Left of BLUE center (14-1)
  builderPlayer1RightEdge = 15; // Right of BLUE center (14+1)
  builderPlayer2LeftEdge = 31;  // Left of GREEN center (32-1)
  builderPlayer2RightEdge = 33; // Right of GREEN center (32+1), wraps to LED 33

  // Mark yellow build zones
  builderRing[builderPlayer1LeftEdge] = 0;   // YELLOW
  builderRing[builderPlayer1RightEdge] = 0;  // YELLOW
  builderRing[builderPlayer2LeftEdge] = 0;   // YELLOW
  builderRing[builderPlayer2RightEdge % RACE_LED_COUNT] = 0;  // YELLOW (handle wrap)

  // Reset runners to center (virtual position 7 out of 0-14), moving right
  builderRunner1Virtual = 7;
  builderRunner2Virtual = 7;
  builderRunner1Dir = 1;
  builderRunner2Dir = 1;

  // Set actual LED positions from virtual
  builderRunner1Pos = BUILDER_BLUE_CENTER;  // LED 14
  builderRunner2Pos = BUILDER_GREEN_CENTER; // LED 32

  builderSpeed = 150;
  // Synchronize both timers to ensure equal speed
  unsigned long startTime = millis();
  builderLastMove1 = startTime;
  builderLastMove2 = startTime;

  builderPlayer1Locked = false;
  builderPlayer2Locked = false;

  // Only BLUE and GREEN button LEDs on (2 players)
  digitalWrite(LED_BTN_BLUE, HIGH);
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);

  // Reset button states
  builderLastBtnBlue = HIGH;
  builderLastBtnGreen = HIGH;

  builderDrawRing();
}

void builderGame() {
  unsigned long now = millis();
  
  // Handle player 1 lock (3 second timeout)
  if (builderPlayer1Locked) {
    unsigned long lockElapsed = now - builderPlayer1LockTime;

    if (lockElapsed < 500) {
      // First 0.5s: Fast blink (error feedback)
      static unsigned long blinkTimer = 0;
      if (now - blinkTimer > 100) {
        blinkTimer = now;
        bool ledState = digitalRead(LED_BTN_BLUE);
        digitalWrite(LED_BTN_BLUE, !ledState);
      }
    } else if (lockElapsed < BUILDER_LOCK_TIME - 500) {
      // 0.5s to 2.5s: OFF (locked)
      digitalWrite(LED_BTN_BLUE, LOW);
    } else if (lockElapsed < BUILDER_LOCK_TIME) {
      // Last 0.5s: Slow blink (warning - almost ready)
      static unsigned long blinkTimer = 0;
      if (now - blinkTimer > 250) {
        blinkTimer = now;
        bool ledState = digitalRead(LED_BTN_BLUE);
        digitalWrite(LED_BTN_BLUE, !ledState);
      }
    } else {
      // Unlock - LED stays ON
      builderPlayer1Locked = false;
      digitalWrite(LED_BTN_BLUE, HIGH);
    }
  }
  
  // Handle player 2 lock (3 second timeout)
  if (builderPlayer2Locked) {
    unsigned long lockElapsed = now - builderPlayer2LockTime;
    
    if (lockElapsed < 500) {
      // First 0.5s: Fast blink (error feedback)
      static unsigned long blinkTimer = 0;
      if (now - blinkTimer > 100) {
        blinkTimer = now;
        bool ledState = digitalRead(LED_BTN_GREEN);
        digitalWrite(LED_BTN_GREEN, !ledState);
      }
    } else if (lockElapsed < BUILDER_LOCK_TIME - 500) {
      // 0.5s to 2.5s: OFF (locked)
      digitalWrite(LED_BTN_GREEN, LOW);
    } else if (lockElapsed < BUILDER_LOCK_TIME) {
      // Last 0.5s: Slow blink (warning - almost ready)
      static unsigned long blinkTimer = 0;
      if (now - blinkTimer > 250) {
        blinkTimer = now;
        bool ledState = digitalRead(LED_BTN_GREEN);
        digitalWrite(LED_BTN_GREEN, !ledState);
      }
    } else {
      // Unlock - LED stays ON
      builderPlayer2Locked = false;
      digitalWrite(LED_BTN_GREEN, HIGH);
    }
  }
  
  // Track if any runner moved this frame
  bool anyRunnerMoved = false;

  // Move BLUE runner using virtual position (0-14) - ONLY if not locked
  if (!builderPlayer1Locked && now - builderLastMove1 >= builderSpeed) {
    builderLastMove1 = now;

    // Move virtual position (IDENTICAL logic for both players)
    builderRunner1Virtual += builderRunner1Dir;

    // Bounce at virtual edges (0 and 14)
    if (builderRunner1Virtual <= 0) {
      builderRunner1Virtual = 0;
      builderRunner1Dir = 1;
    } else if (builderRunner1Virtual >= 14) {
      builderRunner1Virtual = 14;
      builderRunner1Dir = -1;
    }

    // Map virtual position (0-14) to actual LED position (7-21)
    builderRunner1Pos = BUILDER_BLUE_START + builderRunner1Virtual;

    anyRunnerMoved = true;
  }

  // Move GREEN runner using virtual position (0-14) - ONLY if not locked
  // IDENTICAL logic to BLUE for guaranteed same speed
  if (!builderPlayer2Locked && now - builderLastMove2 >= builderSpeed) {
    builderLastMove2 = now;

    // Move virtual position (IDENTICAL logic for both players)
    builderRunner2Virtual += builderRunner2Dir;

    // Bounce at virtual edges (0 and 14) - SAME AS BLUE
    if (builderRunner2Virtual <= 0) {
      builderRunner2Virtual = 0;
      builderRunner2Dir = 1;
    } else if (builderRunner2Virtual >= 14) {
      builderRunner2Virtual = 14;
      builderRunner2Dir = -1;
    }

    // Map virtual position (0-14) to actual LED position (25-35, 0-3)
    // Virtual 0 = LED 25 (left edge)
    // Virtual 7 = LED 32 (center)
    // Virtual 14 = LED 3 (right edge, wraps around)
    if (builderRunner2Virtual <= 10) {
      // Virtual 0-10 maps to LED 25-35
      builderRunner2Pos = 25 + builderRunner2Virtual;
    } else {
      // Virtual 11-14 maps to LED 0-3
      builderRunner2Pos = builderRunner2Virtual - 11;
    }

    anyRunnerMoved = true;
  }

  // Only redraw once if any runner moved
  if (anyRunnerMoved) {
    builderDrawRing();
  }
  
  // Continuously redraw when player locked (for blinking runner)
  static unsigned long lastBlinkUpdate = 0;
  if ((builderPlayer1Locked || builderPlayer2Locked) && now - lastBlinkUpdate >= 50) {
    lastBlinkUpdate = now;
    builderDrawRing();
  }
  
  // Check button presses with edge detection
  bool currBlue = digitalRead(BTN_BLUE);
  bool currGreen = digitalRead(BTN_GREEN);

  // BLUE player (using BLUE button)
  if (builderLastBtnBlue == HIGH && currBlue == LOW && !builderPlayer1Locked) {
    // Check if on YELLOW gap (current build zone)
    if (builderRing[builderRunner1Pos] == 0) {
      // HIT! Fill with BLUE
      builderRing[builderRunner1Pos] = 3;  // BLUE color
      playTone(BUZZER_PIN, 1500, 100);

      // Move yellow zone outward
      if (builderRunner1Pos == builderPlayer1LeftEdge) {
        // Filled left zone - move it one step left
        builderPlayer1LeftEdge--;
        if (builderPlayer1LeftEdge >= BUILDER_BLUE_START) {
          builderRing[builderPlayer1LeftEdge] = 0;  // New yellow zone
        }
      } else if (builderRunner1Pos == builderPlayer1RightEdge) {
        // Filled right zone - move it one step right
        builderPlayer1RightEdge++;
        if (builderPlayer1RightEdge <= BUILDER_BLUE_END) {
          builderRing[builderPlayer1RightEdge] = 0;  // New yellow zone
        }
      }

      builderDrawRing();
    }
    // FAIL if on WHITE field OR own filled field
    else if (builderRing[builderRunner1Pos] == 15 || builderRing[builderRunner1Pos] == 3) {
      // FAIL! Lock player for 3 seconds
      builderPlayer1Locked = true;
      builderPlayer1LockTime = now;
      playTone(BUZZER_PIN, 200, 200);
    }
  }
  builderLastBtnBlue = currBlue;
  
  // GREEN player
  if (builderLastBtnGreen == HIGH && currGreen == LOW && !builderPlayer2Locked) {
    // Check if on YELLOW gap (current build zone)
    if (builderRing[builderRunner2Pos] == 0) {
      // HIT! Fill with GREEN
      builderRing[builderRunner2Pos] = 10;  // GREEN color
      playTone(BUZZER_PIN, 1500, 100);

      // Move yellow zone outward (with wrap-around)
      if (builderRunner2Pos == builderPlayer2LeftEdge) {
        // Filled left zone - move it one step left
        builderPlayer2LeftEdge--;
        if (builderPlayer2LeftEdge < 0) builderPlayer2LeftEdge = 35;  // Wrap around
        if (builderIsInGreenField(builderPlayer2LeftEdge)) {
          builderRing[builderPlayer2LeftEdge] = 0;  // New yellow zone
        }
      } else if (builderRunner2Pos == builderPlayer2RightEdge) {
        // Filled right zone - move it one step right
        builderPlayer2RightEdge++;
        if (builderPlayer2RightEdge > 35) builderPlayer2RightEdge = 0;  // Wrap around
        if (builderIsInGreenField(builderPlayer2RightEdge)) {
          builderRing[builderPlayer2RightEdge] = 0;  // New yellow zone
        }
      }

      builderDrawRing();
    }
    // FAIL if on WHITE field OR own filled field
    else if (builderRing[builderRunner2Pos] == 15 || builderRing[builderRunner2Pos] == 10) {
      // FAIL! Lock player for 3 seconds
      builderPlayer2Locked = true;
      builderPlayer2LockTime = now;
      playTone(BUZZER_PIN, 200, 200);
    }
  }
  builderLastBtnGreen = currGreen;
  
  // Update display
  builderDrawDisplay();
  
  // Check if game over (all 15 fields filled for one player)
  // Count filled fields
  uint8_t player1Filled = 0;
  uint8_t player2Filled = 0;

  for (uint8_t i = BUILDER_BLUE_START; i <= BUILDER_BLUE_END; i++) {
    if (builderRing[i] == 3) player1Filled++;  // BLUE color code
  }
  // Count GREEN with wrap-around
  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    if (builderIsInGreenField(i) && builderRing[i] == 10) player2Filled++;
  }
  
  if (player1Filled >= 15 || player2Filled >= 15) {
    delay(500);
    builderState = BUILDER_GAMEOVER;
  }
}

void builderDrawRing() {
  raceRing.clear();

  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    uint32_t color;

    if (builderRing[i] == 3) {
      // BLUE (filled by blue player)
      color = raceRing.Color(0, 0, 255);
    } else if (builderRing[i] == 10) {
      // GREEN (filled by green player)
      color = raceRing.Color(0, 255, 0);
    } else if (builderRing[i] == 0) {
      // YELLOW (current build zone - press here!)
      color = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B);
    } else if (builderRing[i] == 15) {
      // WHITE (pre-filled playing field)
      color = raceRing.Color(60, 60, 60);  // Dimmer so runner stands out
    } else if (builderRing[i] == 100) {
      // RED (separators between players)
      color = raceRing.Color(200, 0, 0);
    } else {
      // Black (shouldn't happen)
      color = raceRing.Color(0, 0, 0);
    }

    raceRing.setPixelColor(i, color);
  }

  // Draw runners - WHITE and BRIGHT so visible everywhere
  // Blink during fail (200ms cycle)
  bool showRunner1 = true;
  bool showRunner2 = true;

  if (builderPlayer1Locked) {
    // Blink runner during fail
    showRunner1 = (millis() % 200) < 100;
  }

  if (builderPlayer2Locked) {
    // Blink runner during fail
    showRunner2 = (millis() % 200) < 100;
  }

  // BLUE runner - bright WHITE
  if (showRunner1) {
    raceRing.setPixelColor(builderRunner1Pos, raceRing.Color(255, 255, 255));
  }

  // GREEN runner - bright WHITE
  if (showRunner2) {
    raceRing.setPixelColor(builderRunner2Pos, raceRing.Color(255, 255, 255));
  }

  raceRing.show();
}

void builderDrawDisplay() {
  display.clearDisplay();
  display.setTextSize(1);

  // Title
  display.setCursor(centerX("BUILDER", 1), 0);
  display.print("BUILDER");

  // Count filled fields
  uint8_t player1Filled = 0;
  uint8_t player2Filled = 0;

  for (uint8_t i = BUILDER_BLUE_START; i <= BUILDER_BLUE_END; i++) {
    if (builderRing[i] == 3) player1Filled++;  // BLUE color code
  }
  // Count GREEN with wrap-around
  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    if (builderIsInGreenField(i) && builderRing[i] == 10) player2Filled++;
  }

  // Scores - larger text with color names
  display.setTextSize(2);

  // BLUE score
  display.setCursor(0, 20);
  display.print("BLUE:");
  display.print(player1Filled);

  // GREEN score
  display.setCursor(0, 42);
  display.print("GREEN:");
  display.print(player2Filled);

  display.display();
}

void builderGameOver() {
  // Count filled fields to determine winner
  uint8_t player1Filled = 0;
  uint8_t player2Filled = 0;

  for (uint8_t i = BUILDER_BLUE_START; i <= BUILDER_BLUE_END; i++) {
    if (builderRing[i] == 3) player1Filled++;  // BLUE color code
  }
  // Count GREEN with wrap-around
  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    if (builderIsInGreenField(i) && builderRing[i] == 10) player2Filled++;
  }

  // Determine winner
  String winnerName;
  uint32_t winnerColor;

  if (player1Filled >= 15) {
    winnerName = "BLUE";
    winnerColor = raceRing.Color(0, 0, 255);
  } else {
    winnerName = "GREEN";
    winnerColor = raceRing.Color(0, 255, 0);
  }
  
  // Victory animation - blink entire field in winner color
  for (int i = 0; i < 8; i++) {
    // Fill entire ring with winner color
    raceRing.fill(winnerColor);
    raceRing.show();
    playTone(BUZZER_PIN, 800 + i * 100, 150);
    delay(200);
    
    // Clear ring
    raceRing.clear();
    raceRing.show();
    delay(200);
  }
  
  // Show final winner color
  raceRing.fill(winnerColor);
  raceRing.show();
  
  // Display winner
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX(winnerName, 2), 10);
  display.print(winnerName);
  display.setCursor(centerX("WINS!", 2), 35);
  display.print("WINS!");
  
  display.display();
  
  playTone(BUZZER_PIN, 1500, 500);
  delay(5000);
  
  // Turn off button LEDs
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
}

/* ===================== BUILD ALONE IMPLEMENTATION ===================== */

// BUILD ALONE - Single player version of BUILDER
// Goal: Build out from center (LED 14) to fill the entire ring as fast as possible
// Controls: GREEN = fill field, RED = change direction

#define BUILDALONE_CENTER 14
#define BUILDALONE_FIELD_SIZE 36  // Total LEDs in ring

uint8_t buildAloneRing[RACE_LED_COUNT];  // Field states (0=yellow, 10=green filled, 15=white empty)
int8_t buildAloneRunnerPos;
int8_t buildAloneRunnerDir;
uint8_t buildAloneLeftEdge;
uint8_t buildAloneRightEdge;
unsigned long buildAloneSpeed = 150;
unsigned long buildAloneLastMove = 0;
unsigned long buildAloneStartTime = 0;
unsigned long buildAloneEndTime = 0;
bool buildAloneLastBtnGreen = HIGH;
bool buildAloneLastBtnRed = HIGH;
bool buildAloneErrorActive = false;
unsigned long buildAloneErrorStart = 0;

void buildAloneLoop() {
  handleYellowReset();

  switch (buildAloneState) {
    case BUILDALONE_INTRO:
      showIntro();
      buildAloneState = BUILDALONE_IDLE;
      break;
    case BUILDALONE_IDLE:
      buildAloneIdle();
      break;
    case BUILDALONE_COUNTDOWN:
      runBuildAloneCountdown();  // Only RED and GREEN blink
      buildAloneStartGame();
      buildAloneState = BUILDALONE_GAME;
      break;
    case BUILDALONE_GAME:
      buildAloneGame();
      break;
    case BUILDALONE_GAMEOVER:
      buildAloneGameOver();
      buildAloneState = BUILDALONE_IDLE;
      break;
  }
}

void buildAloneIdle() {
  // Load highscore from EEPROM
  static bool highscoreLoaded = false;
  if (!highscoreLoaded) {
    EEPROM.get(EEPROM_BUILDALONE_HIGHSCORE_ADDR, buildAloneHighScore);
    if (buildAloneHighScore == 0xFFFFFFFF || buildAloneHighScore == 0) {
      buildAloneHighScore = 999999;  // Default if never set
    }
    highscoreLoaded = true;
  }

  // Show static center at LED 14 with yellow build zones
  raceRing.clear();
  raceRing.setPixelColor(BUILDALONE_CENTER, raceRing.Color(0, 255, 0));  // GREEN center

  // Show initial yellow build zones (left and right of center)
  uint8_t leftYellow = (BUILDALONE_CENTER - 1 + RACE_LED_COUNT) % RACE_LED_COUNT;  // LED 13
  uint8_t rightYellow = (BUILDALONE_CENTER + 1) % RACE_LED_COUNT;  // LED 15
  raceRing.setPixelColor(leftYellow, raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B));  // YELLOW left
  raceRing.setPixelColor(rightYellow, raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B));  // YELLOW right

  raceRing.show();

  // Brightness control via BLUE (brighter) and RED (darker)
  bool currBlue = digitalRead(BTN_BLUE);
  bool currRed = digitalRead(BTN_RED);
  static bool lastBlue = HIGH;
  static bool lastRed = HIGH;

  if (lastBlue == HIGH && currBlue == LOW) {
    brightness = min(brightness + BRIGHTNESS_STEP, BRIGHTNESS_MAX);
    raceRing.setBrightness(brightness);
    raceRing.show();
  }
  if (lastRed == HIGH && currRed == LOW) {
    brightness = max(brightness - BRIGHTNESS_STEP, BRIGHTNESS_MIN);
    raceRing.setBrightness(brightness);
    raceRing.show();
  }
  lastBlue = currBlue;
  lastRed = currRed;

  // Turn off LED strip
  strip.clear();
  strip.show();

  // Blink YELLOW for start indication
  softBlinkYellow();

  // Turn off other button LEDs
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);

  // Display
  display.clearDisplay();

  // Title - two lines centered
  display.setTextSize(2);
  display.setCursor(centerX("BUILD", 2), 10);
  display.print("BUILD");
  display.setCursor(centerX("ALONE", 2), 28);
  display.print("ALONE");

  // Highscore below
  display.setTextSize(1);
  String timeStr = String(buildAloneHighScore / 1000.0, 1) + "s";
  display.setCursor(centerX(timeStr, 1), 52);
  display.print(timeStr);

  // Brightness display
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    buildAloneState = BUILDALONE_COUNTDOWN;
}

void buildAloneStartGame() {
  // Initialize ring - center GREEN, rest WHITE
  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    if (i == BUILDALONE_CENTER) {
      buildAloneRing[i] = 10;  // GREEN center
    } else {
      buildAloneRing[i] = 15;  // WHITE (empty)
    }
  }

  // Set initial yellow build zones (one step from center)
  buildAloneLeftEdge = BUILDALONE_CENTER - 1;
  buildAloneRightEdge = BUILDALONE_CENTER + 1;
  buildAloneRing[buildAloneLeftEdge] = 0;   // YELLOW
  buildAloneRing[buildAloneRightEdge] = 0;  // YELLOW

  // Random starting direction
  buildAloneRunnerDir = (random(2) == 0) ? -1 : 1;
  buildAloneRunnerPos = BUILDALONE_CENTER;

  buildAloneSpeed = 150;
  buildAloneLastMove = millis();
  buildAloneStartTime = millis();

  // Turn on RED and GREEN LEDs
  digitalWrite(LED_BTN_RED, HIGH);
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);

  buildAloneLastBtnGreen = HIGH;
  buildAloneLastBtnRed = HIGH;

  // Reset error state
  buildAloneErrorActive = false;

  buildAloneDrawRing();
}

void buildAloneGame() {
  unsigned long now = millis();

  // Handle ERROR state - runner stops and blinks red for 1 second
  if (buildAloneErrorActive) {
    if (now - buildAloneErrorStart < 1000) {
      // Blink red during error (100ms cycle)
      buildAloneDrawRingError((now % 200) < 100);
      buildAloneDrawDisplay();
      return;  // Don't process anything else during error
    } else {
      // Error timeout - resume game
      buildAloneErrorActive = false;
    }
  }

  // Move runner (only if not in error)
  if (now - buildAloneLastMove >= buildAloneSpeed) {
    buildAloneLastMove = now;
    buildAloneRunnerPos += buildAloneRunnerDir;

    // Wrap around ring
    if (buildAloneRunnerPos > 35) {
      buildAloneRunnerPos = 0;
    } else if (buildAloneRunnerPos < 0) {
      buildAloneRunnerPos = 35;
    }

    buildAloneDrawRing();
  }

  // Check button presses
  bool currGreen = digitalRead(BTN_GREEN);
  bool currRed = digitalRead(BTN_RED);

  // GREEN button - fill field
  if (buildAloneLastBtnGreen == HIGH && currGreen == LOW) {
    if (buildAloneRing[buildAloneRunnerPos] == 0) {
      // HIT! Fill with GREEN
      buildAloneRing[buildAloneRunnerPos] = 10;
      playTone(BUZZER_PIN, 1500, 100);

      // Move yellow zone outward
      if (buildAloneRunnerPos == buildAloneLeftEdge) {
        // Modulo-Arithmetik für korrekten Wrap-Around (funktioniert mit uint8_t)
        buildAloneLeftEdge = (buildAloneLeftEdge - 1 + 36) % 36;
        // Nur weiße (leere) Felder werden zu gelben Aufbauzonen
        if (buildAloneRing[buildAloneLeftEdge] == 15) {
          buildAloneRing[buildAloneLeftEdge] = 0;
        }
      }
      if (buildAloneRunnerPos == buildAloneRightEdge) {
        // Modulo-Arithmetik für Konsistenz
        buildAloneRightEdge = (buildAloneRightEdge + 1) % 36;
        // Nur weiße (leere) Felder werden zu gelben Aufbauzonen
        if (buildAloneRing[buildAloneRightEdge] == 15) {
          buildAloneRing[buildAloneRightEdge] = 0;
        }
      }

      buildAloneDrawRing();
    } else {
      // ERROR! Not on yellow field
      buildAloneErrorActive = true;
      buildAloneErrorStart = now;
      playTone(BUZZER_PIN, 200, 200);
    }
  }
  buildAloneLastBtnGreen = currGreen;

  // RED button - change direction (only if not in error)
  if (!buildAloneErrorActive && buildAloneLastBtnRed == HIGH && currRed == LOW) {
    buildAloneRunnerDir = -buildAloneRunnerDir;
    playTone(BUZZER_PIN, 800, 50);
  }
  buildAloneLastBtnRed = currRed;

  buildAloneDrawDisplay();

  // Check if game over (all fields filled)
  uint8_t filledCount = 0;
  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    if (buildAloneRing[i] == 10) filledCount++;
  }

  if (filledCount >= RACE_LED_COUNT) {
    buildAloneEndTime = millis();
    delay(500);
    buildAloneState = BUILDALONE_GAMEOVER;
  }
}

void buildAloneDrawRing() {
  raceRing.clear();

  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    uint32_t color;

    if (buildAloneRing[i] == 10) {
      // GREEN (filled)
      color = raceRing.Color(0, 255, 0);
    } else if (buildAloneRing[i] == 0) {
      // YELLOW (build zone)
      color = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B);
    } else if (buildAloneRing[i] == 15) {
      // WHITE (empty) - dimmed so runner stands out
      color = raceRing.Color(20, 20, 20);
    } else {
      color = raceRing.Color(0, 0, 0);
    }

    raceRing.setPixelColor(i, color);
  }

  // Draw runner - bright WHITE
  raceRing.setPixelColor(buildAloneRunnerPos, raceRing.Color(255, 255, 255));

  raceRing.show();
}

void buildAloneDrawRingError(bool showRed) {
  raceRing.clear();

  for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
    uint32_t color;

    if (buildAloneRing[i] == 10) {
      // GREEN (filled)
      color = raceRing.Color(0, 255, 0);
    } else if (buildAloneRing[i] == 0) {
      // YELLOW (build zone)
      color = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B);
    } else if (buildAloneRing[i] == 15) {
      // WHITE (empty) - dimmed so runner stands out
      color = raceRing.Color(20, 20, 20);
    } else {
      color = raceRing.Color(0, 0, 0);
    }

    raceRing.setPixelColor(i, color);
  }

  // Draw runner - blink RED during error
  if (showRed) {
    raceRing.setPixelColor(buildAloneRunnerPos, raceRing.Color(255, 0, 0));
  }

  raceRing.show();
}

void buildAloneDrawDisplay() {
  display.clearDisplay();
  display.setTextSize(1);

  // Title
  display.setCursor(centerX("BUILD ALONE", 1), 0);
  display.print("BUILD ALONE");

  // Elapsed time
  unsigned long elapsed = millis() - buildAloneStartTime;
  display.setTextSize(2);
  String timeStr = String(elapsed / 1000.0, 1) + "s";
  display.setCursor(centerX(timeStr, 2), 25);
  display.print(timeStr);

  display.display();
}

void buildAloneGameOver() {
  unsigned long finalTime = buildAloneEndTime - buildAloneStartTime;

  // Check if new highscore (lower time is better)
  bool newHighscore = false;
  if (finalTime < buildAloneHighScore) {
    buildAloneHighScore = finalTime;
    EEPROM.put(EEPROM_BUILDALONE_HIGHSCORE_ADDR, buildAloneHighScore);
    EEPROM.commit();  // Save to EEPROM permanently
    newHighscore = true;
  }

  // Victory animation
  uint32_t greenColor = raceRing.Color(0, 255, 0);
  for (int i = 0; i < 8; i++) {
    raceRing.fill(greenColor);
    raceRing.show();
    playTone(BUZZER_PIN, 800 + i * 100, 150);
    delay(200);

    raceRing.clear();
    raceRing.show();
    delay(200);
  }

  // Show final color
  raceRing.fill(greenColor);
  raceRing.show();

  // Display result
  display.clearDisplay();
  display.setTextSize(2);

  if (newHighscore) {
    display.setCursor(centerX("NEW", 2), 5);
    display.print("NEW");
    display.setCursor(centerX("RECORD!", 2), 25);
    display.print("RECORD!");
  } else {
    display.setCursor(centerX("DONE!", 2), 15);
    display.print("DONE!");
  }

  display.setTextSize(1);
  String timeStr = String(finalTime / 1000.0, 1) + "s";
  display.setCursor(centerX(timeStr, 1), 50);
  display.print(timeStr);

  display.display();

  playTone(BUZZER_PIN, 1500, 500);
  delay(5000);

  // Turn off button LEDs
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
}

/* ===================== COUNTDOWN GAME IMPLEMENTATION ===================== */

// COUNTDOWN - Time estimation game
// Goal: Estimate a random target time (10-60 seconds) as accurately as possible
// Controls: GREEN = Start/Stop countdown

uint32_t countdownTargetTime = 0;      // Target time in seconds (10-60)
uint32_t countdownElapsedTime = 0;     // Actual elapsed time in milliseconds
unsigned long countdownStartTime = 0;
bool countdownRunning = false;
bool countdownLastBtnGreen = HIGH;

void countdownGameLoop() {
  handleYellowReset();

  switch (countdownGameState) {
    case COUNTDOWNGAME_INTRO:
      showIntro();
      countdownGameState = COUNTDOWNGAME_IDLE;
      break;
    case COUNTDOWNGAME_IDLE:
      countdownGameIdle();
      break;
    case COUNTDOWNGAME_RUNNING:
      countdownGameRunning();
      break;
    case COUNTDOWNGAME_GAMEOVER:
      countdownGameOver();
      countdownGameState = COUNTDOWNGAME_IDLE;
      break;
  }
}

void countdownGameIdle() {
  // Load highscore from EEPROM
  static bool highscoreLoaded = false;
  if (!highscoreLoaded) {
    EEPROM.get(EEPROM_COUNTDOWNGAME_HIGHSCORE_ADDR, countdownBestDiff);
    if (countdownBestDiff == 0xFFFFFFFF) {
      countdownBestDiff = 999999;  // Default if never set
    }
    highscoreLoaded = true;
  }

  // Animated ring - countdown preview (filling circle)
  static unsigned long lastUpdate = 0;
  static float fillProgress = 0;

  if (millis() - lastUpdate >= 50) {
    lastUpdate = millis();
    fillProgress += 0.02;
    if (fillProgress > 1.0) fillProgress = 0;

    raceRing.clear();
    int ledsToFill = (int)(fillProgress * RACE_LED_COUNT);
    for (int i = 0; i < ledsToFill; i++) {
      // Gradient from green to yellow to red
      uint8_t r = map(i, 0, RACE_LED_COUNT - 1, 0, 255);
      uint8_t g = map(i, 0, RACE_LED_COUNT - 1, 255, 100);
      raceRing.setPixelColor(i, raceRing.Color(r, g, 0));
    }
    raceRing.show();
  }

  // Brightness control via BLUE (brighter) and RED (darker)
  bool currBlue = digitalRead(BTN_BLUE);
  bool currRed = digitalRead(BTN_RED);
  static bool lastBlue = HIGH;
  static bool lastRed = HIGH;

  if (lastBlue == HIGH && currBlue == LOW) {
    brightness = min(brightness + BRIGHTNESS_STEP, BRIGHTNESS_MAX);
    raceRing.setBrightness(brightness);
    raceRing.show();
  }
  if (lastRed == HIGH && currRed == LOW) {
    brightness = max(brightness - BRIGHTNESS_STEP, BRIGHTNESS_MIN);
    raceRing.setBrightness(brightness);
    raceRing.show();
  }
  lastBlue = currBlue;
  lastRed = currRed;

  // Turn off LED strip
  strip.clear();
  strip.show();

  // Blink YELLOW for start indication
  softBlinkYellow();

  // Turn off other button LEDs
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);

  // Display
  display.clearDisplay();

  // Title - one line centered
  display.setTextSize(2);
  display.setCursor(centerX("COUNTDOWN", 2), 20);
  display.print("COUNTDOWN");

  // Highscore (best accuracy)
  display.setTextSize(1);
  display.setCursor(centerX("HI-SCORE:", 1), 42);
  display.print("HI-SCORE:");
  String diffStr = String(countdownBestDiff / 1000.0, 2) + "s";
  display.setCursor(centerX(diffStr, 1), 52);
  display.print(diffStr);

  // Brightness display
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  // Start game with YELLOW
  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW) {
    countdownGameStart();
  }
}

void countdownGameStart() {
  // Generate random target time between 1-72 seconds (2 seconds per LED)
  countdownTargetTime = random(1, 73);  // 1 to 72 seconds
  countdownRunning = false;
  countdownElapsedTime = 0;

  countdownLastBtnGreen = HIGH;

  // Turn off all LEDs initially
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);

  countdownGameState = COUNTDOWNGAME_RUNNING;
}

void countdownGameRunning() {
  bool currGreen = digitalRead(BTN_GREEN);

  // GREEN button - start/stop
  if (countdownLastBtnGreen == HIGH && currGreen == LOW) {
    if (!countdownRunning) {
      // Start countdown
      countdownStartTime = millis();
      countdownRunning = true;
      digitalWrite(LED_BTN_GREEN, HIGH);  // Keep GREEN LED on during game
      playTone(BUZZER_PIN, 1000, 100);
    } else {
      // Stop countdown
      countdownElapsedTime = millis() - countdownStartTime;
      countdownRunning = false;
      playTone(BUZZER_PIN, 1500, 200);
      delay(500);
      countdownGameState = COUNTDOWNGAME_GAMEOVER;
    }
  }
  countdownLastBtnGreen = currGreen;

  // Update elapsed time if running
  if (countdownRunning) {
    countdownElapsedTime = millis() - countdownStartTime;
  }

  // Rainbow effect on ring (slow, counter-clockwise)
  if (countdownRunning) {
    static unsigned long lastRainbow = 0;
    static uint16_t rainbowOffset = 0;

    if (millis() - lastRainbow >= 50) {  // Slow rainbow
      lastRainbow = millis();

      for (uint8_t i = 0; i < RACE_LED_COUNT; i++) {
        // Counter-clockwise: subtract offset instead of add
        uint16_t hue = ((65535L / RACE_LED_COUNT) * i) - rainbowOffset;
        raceRing.setPixelColor(i, raceRing.gamma32(raceRing.ColorHSV(hue)));
      }
      raceRing.show();

      rainbowOffset += 200;  // Speed of rainbow rotation
    }
  } else {
    // Before start: blink only GREEN button, show all LEDs up to target
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink >= 500) {
      lastBlink = millis();
      static bool blinkState = false;
      blinkState = !blinkState;
      digitalWrite(LED_BTN_GREEN, blinkState ? HIGH : LOW);
    }
    digitalWrite(LED_BTN_YELLOW, LOW);  // YELLOW stays off in TARGET

    // Show all LEDs up to target time (2 seconds per LED)
    raceRing.clear();
    uint8_t targetLedCount = (countdownTargetTime + 1) / 2;  // 2 seconds per LED
    if (targetLedCount > RACE_LED_COUNT) targetLedCount = RACE_LED_COUNT;

    for (uint8_t i = 0; i < targetLedCount; i++) {
      raceRing.setPixelColor(i, raceRing.Color(0, 255, 0));
    }
    raceRing.show();
  }

  // Keep strip off
  strip.clear();
  strip.show();

  // Display - only show target, no time display during game
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(centerX("TARGET", 1), 10);
  display.print("TARGET");

  // Target time - large
  display.setTextSize(3);
  String targetStr = String(countdownTargetTime) + "s";
  display.setCursor(centerX(targetStr, 3), 30);
  display.print(targetStr);

  display.display();
}

void countdownGameOver() {
  // Calculate difference with sign
  uint32_t targetMs = countdownTargetTime * 1000;
  int32_t signedDiff = (int32_t)countdownElapsedTime - (int32_t)targetMs;
  uint32_t diff = abs(signedDiff);

  // Check if new highscore (smaller difference is better)
  bool newHighscore = false;
  if (diff < countdownBestDiff) {
    countdownBestDiff = diff;
    EEPROM.put(EEPROM_COUNTDOWNGAME_HIGHSCORE_ADDR, countdownBestDiff);
    EEPROM.commit();  // Save to EEPROM permanently
    newHighscore = true;
  }

  // Show ring with all LEDs up to target in GREEN, all LEDs up to result in RED
  raceRing.clear();

  // Calculate LED counts (2 seconds per LED)
  uint8_t targetLedCount = (countdownTargetTime + 1) / 2;
  uint32_t elapsedSeconds = countdownElapsedTime / 1000;
  elapsedSeconds = constrain(elapsedSeconds, 1, 72);
  uint8_t elapsedLedCount = (elapsedSeconds + 1) / 2;

  if (targetLedCount > RACE_LED_COUNT) targetLedCount = RACE_LED_COUNT;
  if (elapsedLedCount > RACE_LED_COUNT) elapsedLedCount = RACE_LED_COUNT;

  // Draw all LEDs up to target in GREEN
  for (uint8_t i = 0; i < targetLedCount; i++) {
    raceRing.setPixelColor(i, raceRing.Color(0, 255, 0));
  }

  // Draw all LEDs up to elapsed time in RED (will overlap GREEN if needed)
  for (uint8_t i = 0; i < elapsedLedCount; i++) {
    raceRing.setPixelColor(i, raceRing.Color(255, 0, 0));
  }

  raceRing.show();

  // Display result
  display.clearDisplay();

  if (newHighscore) {
    display.setTextSize(2);
    display.setCursor(centerX("NEW", 2), 0);
    display.print("NEW");
    display.setCursor(centerX("RECORD!", 2), 18);
    display.print("RECORD!");
  } else {
    display.setTextSize(2);
    display.setCursor(centerX("RESULT", 2), 5);
    display.print("RESULT");
  }

  // Show difference
  display.setTextSize(1);
  display.setCursor(centerX("Difference:", 1), 40);
  display.print("Difference:");

  display.setTextSize(2);
  String sign = (signedDiff >= 0) ? "+" : "-";
  String diffStr = sign + String(diff / 1000.0, 2) + "s";
  display.setCursor(centerX(diffStr, 2), 48);
  display.print(diffStr);

  display.display();

  if (newHighscore) {
    playTone(BUZZER_PIN, 1500, 500);
  } else {
    playTone(BUZZER_PIN, 1000, 300);
  }

  delay(5000);

  // Turn off button LEDs
  digitalWrite(LED_BTN_GREEN, LOW);
}


/* ===================== REAKTION IMPLEMENTATION ===================== */

// REAKTION - Reaction speed game
// Goal: Press the correct colored button as fast as possible
// 60 seconds game time, points for speed, penalties for wrong buttons

#define REAKTION_GAME_TIME 60000  // 60 seconds

enum ReaktionColor {
  COLOR_RED = 0,
  COLOR_GREEN = 1,
  COLOR_BLUE = 2,
  COLOR_YELLOW = 3
};

ReaktionColor reaktionCurrentColor;
unsigned long reaktionColorStartTime = 0;
unsigned long reaktionGameStartTime = 0;
int32_t reaktionScore = 0;
int32_t reaktionHits = 0;  // Count correct hits
bool reaktionColorActive = false;

bool reaktionLastBtnRed = HIGH;
bool reaktionLastBtnGreen = HIGH;
bool reaktionLastBtnBlue = HIGH;
bool reaktionLastBtnYellow = HIGH;

void reaktionLoop() {
  handleYellowReset();

  switch (reaktionState) {
    case REAKTION_INTRO:
      showIntro();
      reaktionState = REAKTION_IDLE;
      break;
    case REAKTION_IDLE:
      reaktionIdle();
      break;
    case REAKTION_COUNTDOWN:
      runSimonMultiCountdown();
      reaktionStartGame();
      reaktionState = REAKTION_GAME;
      break;
    case REAKTION_GAME:
      reaktionGame();
      break;
    case REAKTION_GAMEOVER:
      reaktionGameOver();
      reaktionState = REAKTION_IDLE;
      break;
  }
}

void reaktionIdle() {
  // Load highscore from EEPROM
  static bool highscoreLoaded = false;
  if (!highscoreLoaded) {
    EEPROM.get(EEPROM_REAKTION_HIGHSCORE_ADDR, reaktionHighScore);
    if (reaktionHighScore < 0 || reaktionHighScore > 100000) {
      reaktionHighScore = 0;
    }
    EEPROM.get(EEPROM_REAKTION_HIGHHITS_ADDR, reaktionHighHits);
    if (reaktionHighHits < 0 || reaktionHighHits > 1000) {
      reaktionHighHits = 0;
    }
    highscoreLoaded = true;
  }

  // Animated ring - random flashing LEDs (like the reaction game)
  static unsigned long lastFlash = 0;
  static int flashLED1 = -1;
  static int flashLED2 = -1;
  static int flashLED3 = -1;

  if (millis() - lastFlash >= 300) {
    lastFlash = millis();
    flashLED1 = random(RACE_LED_COUNT);
    flashLED2 = random(RACE_LED_COUNT);
    flashLED3 = random(RACE_LED_COUNT);

    raceRing.clear();
    // Flash 3 random LEDs in different colors
    raceRing.setPixelColor(flashLED1, raceRing.Color(255, 0, 0));    // RED
    raceRing.setPixelColor(flashLED2, raceRing.Color(0, 255, 0));    // GREEN
    raceRing.setPixelColor(flashLED3, raceRing.Color(0, 0, 255));    // BLUE
    raceRing.show();
  }

  // Brightness control
  bool currBlue = digitalRead(BTN_BLUE);
  bool currRed = digitalRead(BTN_RED);
  static bool lastBlue = HIGH;
  static bool lastRed = HIGH;

  if (lastBlue == HIGH && currBlue == LOW) {
    brightness = min(brightness + BRIGHTNESS_STEP, BRIGHTNESS_MAX);
    raceRing.setBrightness(brightness);
  }
  if (lastRed == HIGH && currRed == LOW) {
    brightness = max(brightness - BRIGHTNESS_STEP, BRIGHTNESS_MIN);
    raceRing.setBrightness(brightness);
  }
  lastBlue = currBlue;
  lastRed = currRed;

  strip.clear();
  strip.show();

  softBlinkYellow();

  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("REAKTION", 2), 8);
  display.print("REAKTION");

  display.setTextSize(1);
  display.setCursor(centerX("HI-SCORE:", 1), 26);
  display.print("HI-SCORE:");
  display.setTextSize(2);
  display.setCursor(centerX(String(reaktionHighScore), 2), 38);
  display.print(reaktionHighScore);

  display.setTextSize(1);
  display.setCursor(centerX("TREFFER:", 1), 56);
  display.print("TREFFER:");
  display.print(" ");
  display.print(reaktionHighHits);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    reaktionState = REAKTION_COUNTDOWN;
}

void reaktionStartGame() {
  reaktionScore = 0;
  reaktionHits = 0;  // Reset hit counter
  reaktionGameStartTime = millis();
  reaktionColorActive = false;

  reaktionLastBtnRed = HIGH;
  reaktionLastBtnGreen = HIGH;
  reaktionLastBtnBlue = HIGH;
  reaktionLastBtnYellow = HIGH;

  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);

  reaktionSpawnNewColor();
}

void reaktionSpawnNewColor() {
  reaktionCurrentColor = (ReaktionColor)random(4);
  reaktionColorStartTime = millis();
  reaktionColorActive = true;

  digitalWrite(LED_BTN_RED, (reaktionCurrentColor == COLOR_RED) ? HIGH : LOW);
  digitalWrite(LED_BTN_GREEN, (reaktionCurrentColor == COLOR_GREEN) ? HIGH : LOW);
  digitalWrite(LED_BTN_BLUE, (reaktionCurrentColor == COLOR_BLUE) ? HIGH : LOW);
  digitalWrite(LED_BTN_YELLOW, (reaktionCurrentColor == COLOR_YELLOW) ? HIGH : LOW);

  uint32_t ringColor;
  switch (reaktionCurrentColor) {
    case COLOR_RED: ringColor = raceRing.Color(255, 0, 0); break;
    case COLOR_GREEN: ringColor = raceRing.Color(0, 255, 0); break;
    case COLOR_BLUE: ringColor = raceRing.Color(0, 0, 255); break;
    case COLOR_YELLOW: ringColor = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B); break;
  }
  raceRing.fill(ringColor);
  raceRing.show();
}

void reaktionGame() {
  unsigned long now = millis();
  unsigned long elapsed = now - reaktionGameStartTime;

  if (elapsed >= REAKTION_GAME_TIME) {
    reaktionState = REAKTION_GAMEOVER;
    return;
  }

  bool currRed = digitalRead(BTN_RED);
  bool currGreen = digitalRead(BTN_GREEN);
  bool currBlue = digitalRead(BTN_BLUE);
  bool currYellow = digitalRead(BTN_YELLOW);

  if (reaktionColorActive) {
    unsigned long reactionTime = now - reaktionColorStartTime;

    bool correctPressed = false;
    bool wrongPressed = false;

    if (reaktionLastBtnRed == HIGH && currRed == LOW) {
      if (reaktionCurrentColor == COLOR_RED) correctPressed = true;
      else wrongPressed = true;
    }
    if (reaktionLastBtnGreen == HIGH && currGreen == LOW) {
      if (reaktionCurrentColor == COLOR_GREEN) correctPressed = true;
      else wrongPressed = true;
    }
    if (reaktionLastBtnBlue == HIGH && currBlue == LOW) {
      if (reaktionCurrentColor == COLOR_BLUE) correctPressed = true;
      else wrongPressed = true;
    }
    if (reaktionLastBtnYellow == HIGH && currYellow == LOW) {
      if (reaktionCurrentColor == COLOR_YELLOW) correctPressed = true;
      else wrongPressed = true;
    }

    if (correctPressed) {
      int points = max(10, 100 - (int)(reactionTime / 20));
      reaktionScore += points;
      reaktionHits++;  // Count hit
      playTone(BUZZER_PIN, 1500, 50);

      digitalWrite(LED_BTN_RED, LOW);
      digitalWrite(LED_BTN_GREEN, LOW);
      digitalWrite(LED_BTN_BLUE, LOW);
      digitalWrite(LED_BTN_YELLOW, LOW);
      raceRing.clear();
      raceRing.show();
      delay(100);

      reaktionSpawnNewColor();
    } else if (wrongPressed) {
      reaktionScore -= 50;
      if (reaktionScore < 0) reaktionScore = 0;
      playTone(BUZZER_PIN, 200, 200);
    }
  }

  reaktionLastBtnRed = currRed;
  reaktionLastBtnGreen = currGreen;
  reaktionLastBtnBlue = currBlue;
  reaktionLastBtnYellow = currYellow;

  display.clearDisplay();
  display.setTextSize(1);

  unsigned long timeLeft = (REAKTION_GAME_TIME - elapsed) / 1000;
  display.setCursor(0, 0);
  display.print("TIME: ");
  display.print(timeLeft);
  display.print("s");

  // Show hit count instead of score
  display.setTextSize(1);
  display.setCursor(centerX("TREFFER", 1), 20);
  display.print("TREFFER");

  display.setTextSize(3);
  display.setCursor(centerX(String(reaktionHits), 3), 35);
  display.print(reaktionHits);

  display.display();
}

void reaktionGameOver() {
  bool newHighscore = false;
  if (reaktionScore > reaktionHighScore) {
    reaktionHighScore = reaktionScore;
    EEPROM.put(EEPROM_REAKTION_HIGHSCORE_ADDR, reaktionHighScore);
    newHighscore = true;
  }

  // Save best hits
  if (reaktionHits > reaktionHighHits) {
    reaktionHighHits = reaktionHits;
    EEPROM.put(EEPROM_REAKTION_HIGHHITS_ADDR, reaktionHighHits);
  }

  // Commit all EEPROM changes permanently
  if (newHighscore || reaktionHits > reaktionHighHits) {
    EEPROM.commit();
  }

  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);
  raceRing.clear();
  raceRing.show();

  display.clearDisplay();

  if (newHighscore) {
    display.setTextSize(2);
    display.setCursor(centerX("NEW", 2), 0);
    display.print("NEW");
    display.setCursor(centerX("RECORD!", 2), 18);
    display.print("RECORD!");
  } else {
    display.setTextSize(2);
    display.setCursor(centerX("GAME", 2), 5);
    display.print("GAME");
    display.setCursor(centerX("OVER", 2), 23);
    display.print("OVER");
  }

  display.setTextSize(1);
  display.setCursor(centerX("SCORE:", 1), 32);
  display.print("SCORE:");
  display.setTextSize(1);
  display.setCursor(centerX(String(reaktionScore), 1), 42);
  display.print(reaktionScore);

  display.setTextSize(1);
  display.setCursor(centerX("TREFFER:", 1), 52);
  display.print("TREFFER:");
  display.print(" ");
  display.print(reaktionHits);

  display.display();

  if (newHighscore) {
    playTone(BUZZER_PIN, 1500, 500);
  } else {
    playTone(BUZZER_PIN, 1000, 300);
  }

  delay(5000);
}

/* ===================== SIMON MULTI IMPLEMENTATION ===================== */

// SIMON MULTI - Multiplayer Simon variant
// Players build a sequence together
// Each player must repeat the entire sequence + add their own color
// Wrong sequence = Game Over

#define SIMONMULTI_MAX_SEQUENCE 100

uint8_t simonMultiSequence[SIMONMULTI_MAX_SEQUENCE];
uint8_t simonMultiSequenceLength = 0;
uint8_t simonMultiPlaybackIndex = 0;
uint8_t simonMultiInputIndex = 0;
bool simonMultiWaitingForInput = false;
bool simonMultiPlayingSequence = false;

unsigned long simonMultiLastPlayback = 0;
unsigned long simonMultiInputTimeout = 0;

bool simonMultiLastBtnRed = HIGH;
bool simonMultiLastBtnGreen = HIGH;
bool simonMultiLastBtnBlue = HIGH;
bool simonMultiLastBtnYellow = HIGH;

void simonMultiLoop() {
  handleYellowReset();

  switch (simonMultiState) {
    case SIMONMULTI_INTRO:
      showIntro();
      simonMultiState = SIMONMULTI_IDLE;
      break;
    case SIMONMULTI_IDLE:
      simonMultiIdle();
      break;
    case SIMONMULTI_COUNTDOWN:
      runSimonMultiCountdown();
      simonMultiSequenceLength = 0;
      simonMultiWaitingForInput = true;
      simonMultiInputIndex = 0;
      simonMultiState = SIMONMULTI_GAME;
      break;
    case SIMONMULTI_GAME:
      simonMultiGame();
      break;
    case SIMONMULTI_GAMEOVER:
      simonMultiGameOver();
      simonMultiState = SIMONMULTI_IDLE;
      break;
  }
}

void simonMultiIdle() {
  static bool highscoreLoaded = false;
  if (!highscoreLoaded) {
    EEPROM.get(EEPROM_SIMONMULTI_HIGHSCORE_ADDR, simonMultiHighScore);
    if (simonMultiHighScore > 1000) {
      simonMultiHighScore = 0;
    }
    highscoreLoaded = true;
  }

  // Animated ring - four rotating color quarters (representing 4 buttons)
  static unsigned long lastRotate = 0;
  static float rotationAngle = 0;

  if (millis() - lastRotate >= 50) {
    lastRotate = millis();
    rotationAngle += 0.05;
    if (rotationAngle > TWO_PI) rotationAngle -= TWO_PI;

    raceRing.clear();

    // Divide ring into 4 quarters: RED, GREEN, BLUE, YELLOW
    for (int i = 0; i < RACE_LED_COUNT; i++) {
      float angle = (float)i / RACE_LED_COUNT * TWO_PI + rotationAngle;
      if (angle > TWO_PI) angle -= TWO_PI;

      uint32_t color;
      if (angle < PI / 2) {
        color = raceRing.Color(255, 0, 0);  // RED
      } else if (angle < PI) {
        color = raceRing.Color(0, 255, 0);  // GREEN
      } else if (angle < 3 * PI / 2) {
        color = raceRing.Color(0, 0, 255);  // BLUE
      } else {
        color = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B);  // YELLOW
      }

      raceRing.setPixelColor(i, color);
    }
    raceRing.show();
  }

  // Brightness control
  bool currBlue = digitalRead(BTN_BLUE);
  bool currRed = digitalRead(BTN_RED);
  static bool lastBlue = HIGH;
  static bool lastRed = HIGH;

  if (lastBlue == HIGH && currBlue == LOW) {
    brightness = min(brightness + BRIGHTNESS_STEP, BRIGHTNESS_MAX);
    raceRing.setBrightness(brightness);
  }
  if (lastRed == HIGH && currRed == LOW) {
    brightness = max(brightness - BRIGHTNESS_STEP, BRIGHTNESS_MIN);
    raceRing.setBrightness(brightness);
  }
  lastBlue = currBlue;
  lastRed = currRed;

  strip.clear();
  strip.show();

  softBlinkYellow();

  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("SIMON", 2), 8);
  display.print("SIMON");
  display.setTextSize(1);
  display.setCursor(centerX("MULTI", 1), 26);
  display.print("MULTI");

  display.setTextSize(1);
  display.setCursor(centerX("HI-SCORE:", 1), 34);
  display.print("HI-SCORE:");
  display.setTextSize(2);
  display.setCursor(centerX(String(simonMultiHighScore), 2), 46);
  display.print(simonMultiHighScore);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    simonMultiState = SIMONMULTI_COUNTDOWN;
}

void simonMultiGame() {
  if (simonMultiWaitingForInput) {
    bool currRed = digitalRead(BTN_RED);
    bool currGreen = digitalRead(BTN_GREEN);
    bool currBlue = digitalRead(BTN_BLUE);
    bool currYellow = digitalRead(BTN_YELLOW);

    uint8_t pressedColor = 255;

    if (simonMultiLastBtnRed == HIGH && currRed == LOW) pressedColor = 0;
    if (simonMultiLastBtnGreen == HIGH && currGreen == LOW) pressedColor = 1;
    if (simonMultiLastBtnBlue == HIGH && currBlue == LOW) pressedColor = 2;
    if (simonMultiLastBtnYellow == HIGH && currYellow == LOW) pressedColor = 3;

    if (pressedColor != 255) {
      simonMultiShowColor(pressedColor);
      delay(300);
      simonMultiClearAll();
      delay(200);

      // Check if repeating the sequence or adding new color
      if (simonMultiInputIndex < simonMultiSequenceLength) {
        // Player is repeating the sequence
        if (simonMultiSequence[simonMultiInputIndex] != pressedColor) {
          // Wrong color pressed
          simonMultiState = SIMONMULTI_GAMEOVER;
          return;
        }
        simonMultiInputIndex++;

        // Check if player finished repeating the sequence
        // If so, next input will be their new color
      } else {
        // Player is adding their new color to the sequence
        simonMultiSequence[simonMultiSequenceLength] = pressedColor;
        simonMultiSequenceLength++;

        if (simonMultiSequenceLength >= SIMONMULTI_MAX_SEQUENCE) {
          simonMultiState = SIMONMULTI_GAMEOVER;
          return;
        }

        // Reset for next player
        simonMultiInputIndex = 0;
      }
    }

    simonMultiLastBtnRed = currRed;
    simonMultiLastBtnGreen = currGreen;
    simonMultiLastBtnBlue = currBlue;
    simonMultiLastBtnYellow = currYellow;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(centerX("SIMON MULTI", 1), 0);
  display.print("SIMON MULTI");

  display.setTextSize(2);
  display.setCursor(centerX("LENGTH", 2), 20);
  display.print("LENGTH");
  
  display.setTextSize(3);
  display.setCursor(centerX(String(simonMultiSequenceLength), 3), 40);
  display.print(simonMultiSequenceLength);

  display.display();
}

void simonMultiShowColor(uint8_t color) {
  uint32_t ledColor;
  uint8_t ledPin;

  switch (color) {
    case 0:
      ledColor = raceRing.Color(255, 0, 0);
      ledPin = LED_BTN_RED;
      break;
    case 1:
      ledColor = raceRing.Color(0, 255, 0);
      ledPin = LED_BTN_GREEN;
      break;
    case 2:
      ledColor = raceRing.Color(0, 0, 255);
      ledPin = LED_BTN_BLUE;
      break;
    case 3:
      ledColor = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B);
      ledPin = LED_BTN_YELLOW;
      break;
  }

  digitalWrite(ledPin, HIGH);
  raceRing.fill(ledColor);
  raceRing.show();
  playTone(BUZZER_PIN, 400 + color * 200, 200);
}

void simonMultiClearAll() {
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);
  raceRing.clear();
  raceRing.show();
}

void simonMultiGameOver() {
  bool newHighscore = false;
  if (simonMultiSequenceLength > simonMultiHighScore) {
    simonMultiHighScore = simonMultiSequenceLength;
    EEPROM.put(EEPROM_SIMONMULTI_HIGHSCORE_ADDR, simonMultiHighScore);
    EEPROM.commit();  // Save to EEPROM permanently
    newHighscore = true;
  }

  // GAME OVER warning - red flashing
  simonMultiClearAll();
  for (uint8_t i = 0; i < 5; i++) {
    digitalWrite(LED_BTN_RED, HIGH);
    raceRing.fill(raceRing.Color(255, 0, 0));
    raceRing.show();
    playTone(BUZZER_PIN, 200, 150);

    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(centerX("GAME", 3), 10);
    display.print("GAME");
    display.setCursor(centerX("OVER", 3), 40);
    display.print("OVER");
    display.display();

    delay(200);

    digitalWrite(LED_BTN_RED, LOW);
    raceRing.clear();
    raceRing.show();
    display.clearDisplay();
    display.display();

    delay(200);
  }

  delay(1000);

  // Show correct sequence
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(centerX("CORRECT SEQUENCE:", 1), 28);
  display.print("CORRECT SEQUENCE:");
  display.display();

  delay(1500);

  for (uint8_t i = 0; i < simonMultiSequenceLength; i++) {
    simonMultiShowColor(simonMultiSequence[i]);
    delay(400);
    simonMultiClearAll();
    delay(200);
  }

  delay(1000);

  display.clearDisplay();

  if (newHighscore) {
    display.setTextSize(2);
    display.setCursor(centerX("NEW", 2), 0);
    display.print("NEW");
    display.setCursor(centerX("RECORD!", 2), 18);
    display.print("RECORD!");
  } else {
    display.setTextSize(2);
    display.setCursor(centerX("GAME", 2), 5);
    display.print("GAME");
    display.setCursor(centerX("OVER", 2), 23);
    display.print("OVER");
  }

  display.setTextSize(1);
  display.setCursor(centerX("LENGTH:", 1), 38);
  display.print("LENGTH:");
  display.setTextSize(1);
  display.setCursor(centerX(String(simonMultiSequenceLength), 1), 48);
  display.print(simonMultiSequenceLength);

  display.display();

  if (newHighscore) {
    playTone(BUZZER_PIN, 1500, 500);
  } else {
    playTone(BUZZER_PIN, 800, 300);
  }

  delay(5000);
}

/* ===================== COLOR RUN IMPLEMENTATION ===================== */

// COLOR RUN - Single player color memory game
// Player must press correct position AND correct color
// 50 rounds, speeds up each round
// Wrong press = error display, continue immediately

#define COLORRUN_MAX_ROUNDS 50
uint8_t colorRunRound = 1;
uint8_t colorRunHits = 0;          // Count correct hits (for scoring)
unsigned long colorRunStartTime = 0;
uint8_t colorRunTargetPos = 0;    // Position of standing target (WHITE, with YELLOW buffer in first 3 rounds)
uint8_t colorRunRunnerColor = 0;  // Color of the moving runner
int8_t colorRunRunnerPos = 0;     // Position of moving runner
int8_t colorRunRunnerDir = 1;     // Direction of runner (-1 or 1)
unsigned long colorRunLastMove = 0;
unsigned long colorRunMoveSpeed = 200;  // Start with 200ms per LED

bool colorRunLastBtnRed = HIGH;
bool colorRunLastBtnGreen = HIGH;
bool colorRunLastBtnBlue = HIGH;
bool colorRunLastBtnYellow = HIGH;

bool colorRunShowError = false;
unsigned long colorRunErrorTime = 0;

void colorRunLoop() {
  handleYellowReset();

  switch (colorRunState) {
    case COLORRUN_IDLE:
      colorRunIdle();
      break;
    case COLORRUN_COUNTDOWN:
      runSimonMultiCountdown();
      colorRunStartGame();
      colorRunState = COLORRUN_GAME;
      break;
    case COLORRUN_GAME:
      colorRunGame();
      break;
    case COLORRUN_GAMEOVER:
      colorRunGameOver();
      colorRunState = COLORRUN_IDLE;
      break;
  }
}

void colorRunIdle() {
  // Load highscore from EEPROM
  static bool highscoreLoaded = false;
  if (!highscoreLoaded) {
    EEPROM.get(EEPROM_COLORRUN_HIGHSCORE_ADDR, colorRunHighScore);
    if (colorRunHighScore > 50) {
      colorRunHighScore = 0;
    }
    highscoreLoaded = true;
  }

  // Animated ring - colorful running light (representing the game)
  static unsigned long lastMove = 0;
  static int runnerPos = 0;
  static uint32_t runnerColors[4] = {
    0xFF0000,  // RED
    0x00FF00,  // GREEN
    0x0000FF,  // BLUE
    (COLOR_YELLOW_R << 16) | (COLOR_YELLOW_G << 8) | COLOR_YELLOW_B   // YELLOW (better tone)
  };
  static int currentColor = 0;

  if (millis() - lastMove >= 80) {
    lastMove = millis();
    runnerPos++;
    if (runnerPos >= RACE_LED_COUNT) {
      runnerPos = 0;
      currentColor = (currentColor + 1) % 4;  // Change color each lap
    }

    raceRing.clear();

    // Draw moving colored dot with trail
    raceRing.setPixelColor(runnerPos, runnerColors[currentColor]);

    // Fading trail (3 LEDs behind)
    int trail1 = (runnerPos - 1 + RACE_LED_COUNT) % RACE_LED_COUNT;
    int trail2 = (runnerPos - 2 + RACE_LED_COUNT) % RACE_LED_COUNT;
    int trail3 = (runnerPos - 3 + RACE_LED_COUNT) % RACE_LED_COUNT;

    uint8_t r = (runnerColors[currentColor] >> 16) & 0xFF;
    uint8_t g = (runnerColors[currentColor] >> 8) & 0xFF;
    uint8_t b = runnerColors[currentColor] & 0xFF;

    raceRing.setPixelColor(trail1, raceRing.Color(r * 0.5, g * 0.5, b * 0.5));
    raceRing.setPixelColor(trail2, raceRing.Color(r * 0.25, g * 0.25, b * 0.25));
    raceRing.setPixelColor(trail3, raceRing.Color(r * 0.1, g * 0.1, b * 0.1));

    raceRing.show();
  }

  // Brightness control
  bool currBlue = digitalRead(BTN_BLUE);
  bool currRed = digitalRead(BTN_RED);
  static bool lastBlue = HIGH;
  static bool lastRed = HIGH;

  if (lastBlue == HIGH && currBlue == LOW) {
    brightness = min(brightness + BRIGHTNESS_STEP, BRIGHTNESS_MAX);
    raceRing.setBrightness(brightness);
  }
  if (lastRed == HIGH && currRed == LOW) {
    brightness = max(brightness - BRIGHTNESS_STEP, BRIGHTNESS_MIN);
    raceRing.setBrightness(brightness);
  }
  lastBlue = currBlue;
  lastRed = currRed;

  strip.clear();
  strip.show();

  softBlinkYellow();

  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("COLOR", 2), 8);
  display.print("COLOR");
  display.setCursor(centerX("RUN", 2), 26);
  display.print("RUN");

  display.setTextSize(1);
  display.setCursor(centerX("HI-SCORE:", 1), 46);
  display.print("HI-SCORE:");
  display.print(" ");
  display.print(colorRunHighScore);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    colorRunState = COLORRUN_COUNTDOWN;
}

void colorRunStartGame() {
  colorRunRound = 0;  // Start at 0, will increment to 1 in first round
  colorRunHits = 0;   // Reset hit counter
  colorRunStartTime = millis();
  colorRunMoveSpeed = 200;
  colorRunShowError = false;
  colorRunLastMove = millis();
  colorRunNewRound();
}

void colorRunNewRound() {
  colorRunRound++;

  // Random target position (avoid edges)
  colorRunTargetPos = random(2, RACE_LED_COUNT - 2);

  // Runner starts at opposite side
  colorRunRunnerPos = (colorRunTargetPos + RACE_LED_COUNT / 2) % RACE_LED_COUNT;

  // Random runner color (this determines which button to press!)
  colorRunRunnerColor = random(4);

  // Random direction
  colorRunRunnerDir = (random(2) == 0) ? -1 : 1;

  // Speed up each round
  colorRunMoveSpeed = max(200 - (colorRunRound * 3), 50);

  // Turn on all button LEDs
  digitalWrite(LED_BTN_RED, HIGH);
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_BLUE, HIGH);
  digitalWrite(LED_BTN_YELLOW, HIGH);
}

void colorRunGame() {
  unsigned long now = millis();

  // Show error if active
  if (colorRunShowError) {
    if (now - colorRunErrorTime < 500) {
      // Blink red
      if ((now - colorRunErrorTime) % 200 < 100) {
        raceRing.fill(raceRing.Color(255, 0, 0));
      } else {
        raceRing.clear();
      }
      raceRing.show();

      display.clearDisplay();
      display.setTextSize(3);
      display.setCursor(centerX("ERROR", 3), 20);
      display.print("ERROR");
      display.display();
      return;
    } else {
      colorRunShowError = false;

      // Check if max rounds reached
      if (colorRunRound >= COLORRUN_MAX_ROUNDS) {
        // Game complete after fail!
        colorRunState = COLORRUN_GAMEOVER;
        return;
      }

      // Continue with next round (also after fail!)
      colorRunNewRound();
    }
  }

  // Move runner
  if (now - colorRunLastMove >= colorRunMoveSpeed) {
    colorRunLastMove = now;
    colorRunRunnerPos += colorRunRunnerDir;

    // Wrap around
    if (colorRunRunnerPos < 0) colorRunRunnerPos = RACE_LED_COUNT - 1;
    if (colorRunRunnerPos >= RACE_LED_COUNT) colorRunRunnerPos = 0;
  }

  // Read buttons
  bool currRed = digitalRead(BTN_RED);
  bool currGreen = digitalRead(BTN_GREEN);
  bool currBlue = digitalRead(BTN_BLUE);
  bool currYellow = digitalRead(BTN_YELLOW);

  bool correctPress = false;
  bool wrongPress = false;
  uint8_t pressedColor = 255;

  // Check for button press
  if (colorRunLastBtnRed == HIGH && currRed == LOW) pressedColor = 0;
  if (colorRunLastBtnGreen == HIGH && currGreen == LOW) pressedColor = 1;
  if (colorRunLastBtnBlue == HIGH && currBlue == LOW) pressedColor = 2;
  if (colorRunLastBtnYellow == HIGH && currYellow == LOW) pressedColor = 3;

  colorRunLastBtnRed = currRed;
  colorRunLastBtnGreen = currGreen;
  colorRunLastBtnBlue = currBlue;
  colorRunLastBtnYellow = currYellow;

  if (pressedColor != 255) {
    // Check if runner is on target AND correct color pressed
    bool onTarget = false;

    if (colorRunRound <= 3) {
      // First 3 rounds: 3-LED target zone
      uint8_t targetLeft = (colorRunTargetPos - 1 + RACE_LED_COUNT) % RACE_LED_COUNT;
      uint8_t targetRight = (colorRunTargetPos + 1) % RACE_LED_COUNT;
      onTarget = (colorRunRunnerPos == targetLeft ||
                  colorRunRunnerPos == colorRunTargetPos ||
                  colorRunRunnerPos == targetRight);
    } else {
      // Round 4+: Single LED target
      onTarget = (colorRunRunnerPos == colorRunTargetPos);
    }

    bool correctColor = (pressedColor == colorRunRunnerColor);

    if (onTarget && correctColor) {
      // Correct!
      colorRunHits++;  // Count this hit
      playTone(BUZZER_PIN, 1500, 50);

      if (colorRunRound >= COLORRUN_MAX_ROUNDS) {
        // Game complete!
        colorRunState = COLORRUN_GAMEOVER;
        return;
      }

      colorRunNewRound();
    } else {
      // Wrong!
      playTone(BUZZER_PIN, 200, 100);
      colorRunShowError = true;
      colorRunErrorTime = millis();
    }
  }

  // Draw ring
  raceRing.clear();

  // Draw target based on round
  if (colorRunRound <= 3) {
    // First 3 rounds: YELLOW-WHITE-YELLOW (buffer zone)
    uint8_t targetLeft = (colorRunTargetPos - 1 + RACE_LED_COUNT) % RACE_LED_COUNT;
    uint8_t targetRight = (colorRunTargetPos + 1) % RACE_LED_COUNT;
    raceRing.setPixelColor(targetLeft, raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B));      // YELLOW
    raceRing.setPixelColor(colorRunTargetPos, raceRing.Color(255, 255, 255));  // WHITE
    raceRing.setPixelColor(targetRight, raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B));     // YELLOW
  } else {
    // Round 4+: Single WHITE LED
    raceRing.setPixelColor(colorRunTargetPos, raceRing.Color(255, 255, 255));
  }

  // Draw runner (overwrites target if on same position)
  uint32_t runnerColor;
  switch (colorRunRunnerColor) {
    case 0: runnerColor = raceRing.Color(255, 0, 0); break;    // RED
    case 1: runnerColor = raceRing.Color(0, 255, 0); break;    // GREEN
    case 2: runnerColor = raceRing.Color(0, 0, 255); break;    // BLUE
    case 3: runnerColor = raceRing.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B); break;  // YELLOW
  }
  raceRing.setPixelColor(colorRunRunnerPos, runnerColor);
  raceRing.show();

  // Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(centerX("COLOR RUN", 1), 5);
  display.print("COLOR RUN");

  // Show round number
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print("RND:");
  display.print(colorRunRound);

  // Show hit count (score)
  display.setTextSize(2);
  display.setCursor(centerX("HITS", 2), 35);
  display.print("HITS");
  display.setTextSize(3);
  display.setCursor(centerX(String(colorRunHits), 3), 50);
  display.print(colorRunHits);

  display.display();
}

void colorRunGameOver() {
  // Turn off all LEDs
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);
  raceRing.clear();
  raceRing.show();

  // Calculate final score (use hits, not rounds!)
  uint32_t finalScore = colorRunHits;

  bool newHighscore = false;
  if (finalScore > colorRunHighScore) {
    colorRunHighScore = finalScore;
    EEPROM.put(EEPROM_COLORRUN_HIGHSCORE_ADDR, colorRunHighScore);
    EEPROM.commit();  // Save to EEPROM permanently
    newHighscore = true;
  }

  display.clearDisplay();

  if (newHighscore) {
    display.setTextSize(2);
    display.setCursor(centerX("NEW", 2), 0);
    display.print("NEW");
    display.setCursor(centerX("RECORD!", 2), 18);
    display.print("RECORD!");
  } else {
    display.setTextSize(2);
    display.setCursor(centerX("GAME", 2), 5);
    display.print("GAME");
    display.setCursor(centerX("OVER", 2), 23);
    display.print("OVER");
  }

  display.setTextSize(1);
  display.setCursor(centerX("HITS:", 1), 43);
  display.print("HITS:");
  display.setTextSize(2);
  display.setCursor(centerX(String(finalScore), 2), 51);
  display.print(finalScore);

  display.display();

  if (newHighscore) {
    playTone(BUZZER_PIN, 1500, 500);
  } else {
    playTone(BUZZER_PIN, 800, 300);
  }

  delay(5000);
}


/* ===================== SPEED IMPLEMENTATION ===================== */

// SPEED - Button mashing speed game
// Press green button as fast as possible to complete 5 laps
// Each press moves car forward by 1 LED
// Fastest time wins

#define SPEED_LAPS 5
#define SPEED_START_POS 0
#define SPEED_CAR_START_POS 35  // Start just before finish line

uint8_t speedCarPos = SPEED_CAR_START_POS;
uint32_t speedLapsCompleted = 0;
uint32_t speedTotalPresses = 0;
unsigned long speedStartTime = 0;
unsigned long speedEndTime = 0;
bool speedLastBtnGreen = HIGH;
bool speedStartLineReached = false;  // Track if car reached start line first time

// ENEMYS - Color matching shooter game (ähnlich THE LINE)
#define ENEMYS_DEFENSE_LINE 4  // White line at LED 4 - game over if reached
#define ENEMYS_MAX_ENEMY_LENGTH 150  // Maximum LEDs in enemy snake
#define ENEMYS_INITIAL_SPEED 1200  // ms between enemy moves (ULTRA SLOW)
#define ENEMYS_SPEED_DECREASE 25  // Speed increase per level
#define ENEMYS_MIN_SPEED 250  // Minimum speed
#define ENEMYS_SHOOT_SPEED 3  // ms between shoot moves (ULTRA FAST)

// Enemy snake - eine kontinuierliche LED-Schlange
uint8_t enemysSnakeColors[ENEMYS_MAX_ENEMY_LENGTH];  // Color for each LED (0=RED, 1=GREEN, 2=BLUE)
int enemysSnakeLength = 0;  // Number of LEDs in enemy snake
int enemysSnakeOffset = LED_COUNT - 1;  // Position of first enemy LED (starts at 149)

// Shoot queue on LED ring
#define ENEMYS_QUEUE_SIZE 12  // Max shoots in queue (für volle Ring-Nutzung)
struct EnemysShoot {
  uint8_t colors[3];  // 2 or 3 LEDs
  uint8_t length;     // 2 or 3
};
EnemysShoot enemysShootQueue[ENEMYS_QUEUE_SIZE];
int enemysQueueHead = 0;  // Next shoot to fire
int enemysQueueTail = 0;  // Where to add new shoots
int enemysQueueCount = 0; // Number of shoots in queue

// Fired shoots (moving towards enemies)
struct EnemysFiredShoot {
  int pos;  // Position on strip
  uint8_t colors[4];
  uint8_t length;
  bool active;
};
#define ENEMYS_MAX_SHOOTS 5
EnemysFiredShoot enemysFiredShoots[ENEMYS_MAX_SHOOTS];

uint32_t enemysLevel = 1;
uint32_t enemysScore = 0;
unsigned long enemysSpeed = ENEMYS_INITIAL_SPEED;
unsigned long enemysLastEnemyMove = 0;
unsigned long enemysGameStartTime = 0;  // Spielstart-Zeit
uint8_t enemysColorCount = 2;  // Farben für Shoots (ROT, GRÜN)
uint8_t enemysEnemyColorCount = 2;  // Farben für Enemy-Strang (ROT, GRÜN)
bool enemysLastBtnRed = HIGH;
bool enemysLastBtnGreen = HIGH;
bool enemysLastBtnBlue = HIGH;

void speedLoop() {
  handleYellowReset();

  switch (speedState) {
    case SPEED_INTRO:
      showIntro();
      speedState = SPEED_IDLE;
      break;
    case SPEED_IDLE:
      speedIdle();
      break;
    case SPEED_COUNTDOWN:
      runSpeedCountdown();  // Only green button countdown
      speedStartGame();
      speedState = SPEED_GAME;
      break;
    case SPEED_GAME:
      speedGame();
      break;
    case SPEED_GAMEOVER:
      speedGameOver();
      speedState = SPEED_IDLE;
      break;
  }
}

void speedIdle() {
  // Load highscore from EEPROM
  static bool highscoreLoaded = false;
  if (!highscoreLoaded) {
    EEPROM.get(EEPROM_SPEED_HIGHSCORE_ADDR, speedHighScore);
    if (speedHighScore == 0xFFFFFFFF || speedHighScore == 0 || speedHighScore > 300000) {
      speedHighScore = 999999;  // Default if never set (999.999 seconds)
    }
    highscoreLoaded = true;
  }

  // Animated ring - car slowly running in circles
  static unsigned long lastMove = 0;
  static int carPos = SPEED_CAR_START_POS;

  if (millis() - lastMove >= 150) {  // Slow movement
    lastMove = millis();
    carPos++;
    if (carPos >= RACE_LED_COUNT) {
      carPos = 0;
    }

    raceRing.clear();

    // Start/Finish line at LED 0 - fixed white
    raceRing.setPixelColor(SPEED_START_POS, raceRing.Color(255, 255, 255));

    // Car running slowly - green with trail
    raceRing.setPixelColor(carPos, raceRing.Color(0, 255, 0));

    // Trail
    int trail1 = (carPos - 1 + RACE_LED_COUNT) % RACE_LED_COUNT;
    int trail2 = (carPos - 2 + RACE_LED_COUNT) % RACE_LED_COUNT;
    if (trail1 != SPEED_START_POS) {
      raceRing.setPixelColor(trail1, raceRing.Color(COLOR_YELLOW_R / 3, COLOR_YELLOW_G / 3, 0));
    }
    if (trail2 != SPEED_START_POS) {
      raceRing.setPixelColor(trail2, raceRing.Color(COLOR_YELLOW_R / 6, COLOR_YELLOW_G / 6, 0));
    }

    raceRing.show();
  }

  // Brightness control
  bool currBlue = digitalRead(BTN_BLUE);
  bool currRed = digitalRead(BTN_RED);
  static bool lastBlue = HIGH;
  static bool lastRed = HIGH;

  if (lastBlue == HIGH && currBlue == LOW) {
    brightness = min(brightness + BRIGHTNESS_STEP, BRIGHTNESS_MAX);
    raceRing.setBrightness(brightness);
  }
  if (lastRed == HIGH && currRed == LOW) {
    brightness = max(brightness - BRIGHTNESS_STEP, BRIGHTNESS_MIN);
    raceRing.setBrightness(brightness);
  }
  lastBlue = currBlue;
  lastRed = currRed;

  strip.clear();
  strip.show();

  softBlinkYellow();

  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("SPEED", 2), 8);
  display.print("SPEED");

  display.setTextSize(1);
  display.setCursor(centerX("HI-SCORE:", 1), 30);
  display.print("HI-SCORE:");

  String timeStr;
  if (speedHighScore < 999999) {
    timeStr = String(speedHighScore / 1000.0, 3) + "s";
  } else {
    timeStr = "---";
  }
  display.setCursor(centerX(timeStr, 1), 40);
  display.print(timeStr);

  display.setTextSize(1);
  display.setCursor(centerX("5 LAPS", 1), 52);
  display.print("5 LAPS");

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    speedState = SPEED_COUNTDOWN;
}

void speedStartGame() {
  speedCarPos = SPEED_CAR_START_POS;
  speedLapsCompleted = 0;
  speedTotalPresses = 0;
  speedStartTime = 0;  // Will start on first press
  speedLastBtnGreen = HIGH;
  speedStartLineReached = false;  // Reset flag
}

void speedGame() {
  // Read green button
  bool currGreen = digitalRead(BTN_GREEN);

  // Detect button press (edge detection)
  if (speedLastBtnGreen == HIGH && currGreen == LOW) {
    // Start timer on first press
    if (speedTotalPresses == 0) {
      speedStartTime = millis();
    }

    speedTotalPresses++;

    // Move car forward
    speedCarPos++;
    if (speedCarPos >= RACE_LED_COUNT) {
      speedCarPos = 0;

      // Only count laps after reaching start line the first time
      if (!speedStartLineReached) {
        speedStartLineReached = true;  // First crossing - start counting from here
      } else {
        speedLapsCompleted++;  // Subsequent crossings count as completed laps

        // Check if finished
        if (speedLapsCompleted >= SPEED_LAPS) {
          speedEndTime = millis();
          speedState = SPEED_GAMEOVER;
          return;
        }
      }
    }

    playTone(BUZZER_PIN, 800, 20);  // Quick beep
  }

  speedLastBtnGreen = currGreen;

  // Draw ring
  raceRing.clear();

  // Start/Finish line at LED 0 - white
  raceRing.setPixelColor(SPEED_START_POS, raceRing.Color(255, 255, 255));

  // Car - green with yellow trail
  raceRing.setPixelColor(speedCarPos, raceRing.Color(0, 255, 0));

  // Trail
  int trail1 = (speedCarPos - 1 + RACE_LED_COUNT) % RACE_LED_COUNT;
  int trail2 = (speedCarPos - 2 + RACE_LED_COUNT) % RACE_LED_COUNT;
  if (trail1 != SPEED_START_POS) {
    raceRing.setPixelColor(trail1, raceRing.Color(COLOR_YELLOW_R / 2, COLOR_YELLOW_G / 2, 0));
  }
  if (trail2 != SPEED_START_POS) {
    raceRing.setPixelColor(trail2, raceRing.Color(COLOR_YELLOW_R / 4, COLOR_YELLOW_G / 4, 0));
  }

  raceRing.show();

  strip.clear();
  strip.show();

  // Green button always on during game
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);

  // Display
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("LAP", 2), 0);
  display.print("LAP");

  display.setTextSize(3);
  // Show current lap: if start line not reached yet, show "1/5", otherwise show completed+1
  uint8_t currentLap = speedStartLineReached ? (speedLapsCompleted + 1) : 1;

  display.setCursor(centerX(String(currentLap) + "/" + String(SPEED_LAPS), 3), 20);
  display.print(currentLap);
  display.print("/");
  display.print(SPEED_LAPS);

  // Show elapsed time if started
  if (speedTotalPresses > 0) {
    unsigned long elapsed = millis() - speedStartTime;
    display.setTextSize(1);
    display.setCursor(centerX("TIME:", 1), 44);
    display.print("TIME:");
    String elapsedStr = String(elapsed / 1000.0, 2) + "s";
    display.setCursor(centerX(elapsedStr, 1), 56);
    display.print(elapsedStr);
  }

  display.display();
}

void speedGameOver() {
  unsigned long finalTime = speedEndTime - speedStartTime;

  // Check if new highscore (lower time is better)
  bool newHighscore = false;
  if (finalTime < speedHighScore) {
    speedHighScore = finalTime;
    EEPROM.put(EEPROM_SPEED_HIGHSCORE_ADDR, speedHighScore);
    EEPROM.commit();
    newHighscore = true;
  }

  // Victory animation - chase around ring
  digitalWrite(LED_BTN_GREEN, LOW);

  for (int lap = 0; lap < 3; lap++) {
    for (int i = 0; i < RACE_LED_COUNT; i++) {
      raceRing.clear();
      raceRing.setPixelColor(i, raceRing.Color(0, 255, 0));
      raceRing.show();

      if (i % 6 == 0) playTone(BUZZER_PIN, 800 + lap * 200, 30);
      delay(20);
    }
  }

  // Final flash
  for (int i = 0; i < 5; i++) {
    raceRing.fill(raceRing.Color(0, 255, 0));
    raceRing.show();
    digitalWrite(LED_BTN_GREEN, HIGH);
    playTone(BUZZER_PIN, 1500, 100);
    delay(150);

    raceRing.clear();
    raceRing.show();
    digitalWrite(LED_BTN_GREEN, LOW);
    delay(150);
  }

  digitalWrite(LED_BTN_GREEN, LOW);

  // Display result
  display.clearDisplay();
  display.setTextSize(2);

  if (newHighscore) {
    display.setCursor(centerX("NEW", 2), 0);
    display.print("NEW");
    display.setCursor(centerX("RECORD!", 2), 18);
    display.print("RECORD!");
  } else {
    display.setCursor(centerX("FINISH!", 2), 10);
    display.print("FINISH!");
  }

  display.setTextSize(1);
  display.setCursor(centerX("TIME:", 1), 34);
  display.print("TIME:");

  String timeStr = String(finalTime / 1000.0, 3) + "s";
  display.setTextSize(2);
  display.setCursor(centerX(timeStr, 2), 46);
  display.print(timeStr);

  display.display();

  if (newHighscore) {
    playTone(BUZZER_PIN, 2000, 500);
  }

  delay(5000);
}


/* ===================== ENEMYS IMPLEMENTATION ===================== */

// ENEMYS - Color matching shooter on main LED strip
// Enemies come from LED 149 towards LED 0
// Player has shoots at LED 0-3
// White defense line at LED 6

void enemysLoop() {
  handleYellowReset();

  switch (enemysState) {
    case ENEMYS_INTRO:
      // Skip intro - go directly to IDLE
      enemysState = ENEMYS_IDLE;
      break;
    case ENEMYS_IDLE:
      enemysIdle();
      break;
    case ENEMYS_COUNTDOWN:
      runEnemysCountdown();
      enemysStartGame();
      enemysState = ENEMYS_GAME;
      break;
    case ENEMYS_GAME:
      enemysGame();
      break;
    case ENEMYS_GAMEOVER:
      enemysGameOver();
      enemysState = ENEMYS_IDLE;
      break;
  }
}

// Countdown with RED, GREEN, BLUE buttons
void runEnemysCountdown() {
  for (int i = 3; i >= 1; i--) {
    digitalWrite(LED_BTN_RED, HIGH);
    digitalWrite(LED_BTN_GREEN, HIGH);
    digitalWrite(LED_BTN_BLUE, HIGH);
    digitalWrite(LED_BTN_YELLOW, LOW);

    playTone(BUZZER_PIN, 1200, 150);

    display.clearDisplay();
    display.setTextSize(5);
    display.setCursor(centerX(String(i), 5), 15);
    display.print(i);
    display.display();

    delay(300);

    digitalWrite(LED_BTN_RED, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    digitalWrite(LED_BTN_BLUE, LOW);

    delay(700);
  }

  playTone(BUZZER_PIN, 2200, 400);
  delay(400);
}

void enemysIdle() {
  // Load highscore from EEPROM
  static bool highscoreLoaded = false;
  if (!highscoreLoaded) {
    EEPROM.get(EEPROM_ENEMYS_HIGHSCORE_ADDR, enemysHighScore);
    if (enemysHighScore > 10000) {
      enemysHighScore = 0;
    }
    highscoreLoaded = true;
  }

  // Rainbow on strip
  static unsigned long rainbowTimer = 0;
  if (millis() - rainbowTimer >= 30) {
    rainbowTimer = millis();
    static uint16_t h = 0;
    for (int i = 0; i < LED_COUNT; i++)
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(h + i * 65535L / LED_COUNT)));
    strip.show();
    h += 40;
  }

  // Turn off ring
  raceRing.clear();
  raceRing.show();

  softBlinkYellow();
  handleBrightness();

  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("ENEMYS", 2), 8);
  display.print("ENEMYS");

  display.setTextSize(1);
  display.setCursor(centerX("HI-SCORE:", 1), 30);
  display.print("HI-SCORE:");

  display.setTextSize(2);
  display.setCursor(centerX(String(enemysHighScore), 2), 42);
  display.print(enemysHighScore);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  // Check for mute toggle (2-second green button hold)
  checkMuteToggle();

  // Draw speaker icon
  drawSpeakerIcon();

  display.display();

  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    enemysState = ENEMYS_COUNTDOWN;
}

// Generate random color for shoots (progressive: 2→3→4 Farben)
uint8_t enemysRandomColor() {
  return random(enemysColorCount);
}

// Generate random color for enemy (kann mehr Farben haben als Shoots)
uint8_t enemysRandomEnemyColor() {
  return random(enemysEnemyColorCount);
}

// Generate random shoot and add to queue
void enemysGenerateShoot() {
  if (enemysQueueCount >= ENEMYS_QUEUE_SIZE) return;  // Queue full

  EnemysShoot newShoot;
  // Nur 2er Shoots
  newShoot.length = 2;

  for (uint8_t i = 0; i < newShoot.length; i++) {
    newShoot.colors[i] = enemysRandomColor();
  }

  enemysShootQueue[enemysQueueTail] = newShoot;
  enemysQueueTail = (enemysQueueTail + 1) % ENEMYS_QUEUE_SIZE;
  enemysQueueCount++;
}

// Rotate current shoot (swap/rotate colors)
void enemysFlipShoot() {
  if (enemysQueueCount == 0) return;

  EnemysShoot* currentShoot = &enemysShootQueue[enemysQueueHead];

  if (currentShoot->length == 2) {
    // 2er: Einfach tauschen [0,1] → [1,0]
    uint8_t temp = currentShoot->colors[0];
    currentShoot->colors[0] = currentShoot->colors[1];
    currentShoot->colors[1] = temp;
  } else if (currentShoot->length == 3) {
    // 3er: Rotieren [0,1,2] → [1,2,0]
    uint8_t temp = currentShoot->colors[0];
    currentShoot->colors[0] = currentShoot->colors[1];
    currentShoot->colors[1] = currentShoot->colors[2];
    currentShoot->colors[2] = temp;
  }

  playTone(BUZZER_PIN, 600, 50);
}

// Get color value
uint32_t enemysGetColor(uint8_t colorIndex) {
  switch (colorIndex) {
    case 0: return strip.Color(255, 0, 0);    // RED
    case 1: return strip.Color(0, 255, 0);    // GREEN
    case 2: return strip.Color(0, 0, 255);    // BLUE
    case 3: return strip.Color(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B);  // YELLOW
    default: return strip.Color(255, 255, 255);  // WHITE
  }
}

void enemysStartGame() {
  // WICHTIG: Erst Farben zurücksetzen, DANN Queue füllen!
  enemysLevel = 1;
  enemysScore = 0;
  enemysSpeed = ENEMYS_INITIAL_SPEED;
  enemysLastEnemyMove = millis();
  enemysGameStartTime = millis();  // Spielstart merken
  enemysColorCount = 2;  // Shoot-Farben: ROT, GRÜN (ZURÜCKSETZEN!)
  enemysEnemyColorCount = 2;  // Enemy-Farben: ROT, GRÜN (ZURÜCKSETZEN!)

  // Clear enemy snake
  enemysSnakeLength = 0;
  enemysSnakeOffset = LED_COUNT - 1;  // Start at LED 149

  // Start with some random enemy LEDs (nutzt enemysEnemyColorCount)
  for (int i = 0; i < 20; i++) {
    enemysSnakeColors[enemysSnakeLength++] = enemysRandomEnemyColor();
  }

  // Clear fired shoots
  for (int i = 0; i < ENEMYS_MAX_SHOOTS; i++) {
    enemysFiredShoots[i].active = false;
  }

  // Initialize shoot queue
  enemysQueueHead = 0;
  enemysQueueTail = 0;
  enemysQueueCount = 0;

  // Fill queue with initial shoots (nutzt enemysColorCount)
  for (int i = 0; i < ENEMYS_QUEUE_SIZE; i++) {
    enemysGenerateShoot();
  }

  // Button LEDs on
  digitalWrite(LED_BTN_RED, HIGH);
  digitalWrite(LED_BTN_GREEN, HIGH);
  digitalWrite(LED_BTN_BLUE, HIGH);
  digitalWrite(LED_BTN_YELLOW, LOW);

  enemysLastBtnRed = HIGH;
  enemysLastBtnGreen = HIGH;
  enemysLastBtnBlue = HIGH;
}

void enemysGame() {
  unsigned long now = millis();

  // Kontinuierlich neue Enemy-LEDs hinzufügen (wie bei THE LINE)
  if (enemysSnakeLength < ENEMYS_MAX_ENEMY_LENGTH) {
    enemysSnakeColors[enemysSnakeLength++] = enemysRandomEnemyColor();
  }

  // Move enemy snake towards player
  if (now - enemysLastEnemyMove >= enemysSpeed) {
    enemysLastEnemyMove = now;
    enemysSnakeOffset--;

    // Check if enemy reached defense line
    if (enemysSnakeOffset <= ENEMYS_DEFENSE_LINE) {
      enemysState = ENEMYS_GAMEOVER;
      return;
    }

    // Prüfe ob neue Farben in Schussreichweite sind (vordere 30 LEDs)
    // und schalte sie für Shoots frei
    if (enemysColorCount < enemysEnemyColorCount) {
      // Durchsuche die vorderen 30 LEDs des Enemy-Strangs
      for (int i = 0; i < 30 && i < enemysSnakeLength; i++) {
        uint8_t color = enemysSnakeColors[i];
        // Wenn eine Farbe gefunden wird die >= enemysColorCount ist, schalte sie frei
        if (color >= enemysColorCount) {
          enemysColorCount = color + 1;  // Schalte diese Farbe frei
          playTone(BUZZER_PIN, 1800, 100);  // Hinweis-Ton
          break;
        }
      }
    }
  }

  // Move fired shoots
  static unsigned long lastShootMove = 0;
  if (now - lastShootMove >= ENEMYS_SHOOT_SPEED) {
    lastShootMove = now;

    for (int i = 0; i < ENEMYS_MAX_SHOOTS; i++) {
      if (!enemysFiredShoots[i].active) continue;

      enemysFiredShoots[i].pos++;

      // Check collision with enemy snake
      // Shoot reaches enemy wenn pos >= enemysSnakeOffset
      if (enemysFiredShoots[i].pos >= enemysSnakeOffset) {
        // Match LED für LED von vorne (Shoot-Front trifft Enemy-Front)
        // Shoot: [0]=hinten(LED0), [length-1]=vorne (trifft zuerst)
        // Enemy: [0]=vorne, [1]=weiter hinten...

        int totalRemoved = 0;
        int shootIdx = enemysFiredShoots[i].length - 1;  // Von vorne (höchster Index)
        int enemyIdx = 0;  // Von vorne

        // Match so lange wie Farben übereinstimmen
        while (shootIdx >= 0 && enemyIdx < enemysSnakeLength) {
          uint8_t shootColor = enemysFiredShoots[i].colors[shootIdx];
          uint8_t enemyColor = enemysSnakeColors[enemyIdx];

          if (shootColor == enemyColor) {
            // MATCH! Zähle alle zusammenhängenden LEDs dieser Farbe

            // Zähle zusammenhängende Shoot-LEDs
            int shootMatchCount = 0;
            while (shootIdx >= 0 && enemysFiredShoots[i].colors[shootIdx] == shootColor) {
              shootMatchCount++;
              shootIdx--;
            }

            // Zähle zusammenhängende Enemy-LEDs
            int enemyMatchCount = 0;
            while (enemyIdx < enemysSnakeLength && enemysSnakeColors[enemyIdx] == enemyColor) {
              enemyMatchCount++;
              enemyIdx++;
            }

            // Entferne die gematchten Enemy-LEDs
            // KEIN Array-Verschieben! Nur Offset anpassen und Length reduzieren
            enemysSnakeOffset += enemyMatchCount;  // Front rückt nach hinten (weg von Spieler)
            enemysSnakeLength -= enemyMatchCount;

            // Verschiebe restliche Farben nach vorne im Array
            for (int k = 0; k < enemysSnakeLength; k++) {
              enemysSnakeColors[k] = enemysSnakeColors[k + enemyMatchCount];
            }
            enemyIdx = 0;  // Von vorne neu starten

            totalRemoved += shootMatchCount + enemyMatchCount;
          } else {
            // KEIN MATCH - Blocker gefunden!
            // Übrige Shoot-LEDs werden vorne eingefügt
            break;
          }
        }

        // Übrige Shoot-LEDs? (von Index 0 bis shootIdx)
        int remainingShootLEDs = shootIdx + 1;
        if (remainingShootLEDs > 0) {
          // FAIL: Shoot-LEDs reihen sich VORNE ein
          // Offset muss nach vorne (niedrigere LED-Nummer)
          enemysSnakeOffset -= remainingShootLEDs;

          // Verschiebe bestehende Enemy-Farben nach hinten im Array
          for (int k = enemysSnakeLength - 1; k >= 0; k--) {
            enemysSnakeColors[k + remainingShootLEDs] = enemysSnakeColors[k];
          }

          // Füge Shoot-LEDs vorne ein (in gleicher Reihenfolge)
          for (int k = 0; k < remainingShootLEDs; k++) {
            enemysSnakeColors[k] = enemysFiredShoots[i].colors[k];
          }
          enemysSnakeLength += remainingShootLEDs;

          // Enemy wird länger, wächst nach vorne (Offset verschoben)
        }

        if (totalRemoved > 0) {
          enemysScore += totalRemoved * 10;
          playTone(BUZZER_PIN, 1500, 50);
        } else {
          playTone(BUZZER_PIN, 200, 100);
        }

        enemysFiredShoots[i].active = false;
      }

      // Remove if off screen
      if (enemysFiredShoots[i].pos >= LED_COUNT) {
        enemysFiredShoots[i].active = false;
      }
    }
  }

  // Button handling
  bool currRed = digitalRead(BTN_RED);
  bool currGreen = digitalRead(BTN_GREEN);
  bool currBlue = digitalRead(BTN_BLUE);

  // GRÜN: Rotieren/Tauschen des aktuellen Shoots
  if (enemysLastBtnGreen == HIGH && currGreen == LOW) {
    enemysFlipShoot();
  }

  // BLAU: Neuen Shoot generieren (kostet Punkte!)
  if (enemysLastBtnBlue == HIGH && currBlue == LOW && enemysQueueCount > 0) {
    // Speichere aktuellen Shoot zum Vergleich
    EnemysShoot oldShoot = enemysShootQueue[enemysQueueHead];

    // Entferne aktuellen Shoot aus Queue
    enemysQueueHead = (enemysQueueHead + 1) % ENEMYS_QUEUE_SIZE;
    enemysQueueCount--;

    // Generiere neuen Shoot, der sich vom alten unterscheidet
    int attempts = 0;
    bool isDifferent = false;
    while (attempts < 20 && !isDifferent) {
      enemysGenerateShoot();

      // Prüfe ob neuer Shoot anders ist (Queue tail - 1 ist der neue Shoot)
      int newShootIdx = (enemysQueueTail - 1 + ENEMYS_QUEUE_SIZE) % ENEMYS_QUEUE_SIZE;
      EnemysShoot* newShoot = &enemysShootQueue[newShootIdx];

      // Vergleiche Farben in gleicher Reihenfolge
      isDifferent = false;
      for (uint8_t i = 0; i < oldShoot.length; i++) {
        if (newShoot->colors[i] != oldShoot.colors[i]) {
          isDifferent = true;
          break;
        }
      }

      // Wenn gleich, entferne den neuen Shoot wieder
      if (!isDifferent) {
        enemysQueueTail = (enemysQueueTail - 1 + ENEMYS_QUEUE_SIZE) % ENEMYS_QUEUE_SIZE;
        enemysQueueCount--;
      }

      attempts++;
    }

    // Punkteabzug (z.B. -20 Punkte)
    if (enemysScore >= 20) {
      enemysScore -= 20;
    } else {
      enemysScore = 0;
    }

    playTone(BUZZER_PIN, 400, 100);  // Tiefer Ton = Strafe
  }

  // Shoot with RED
  if (enemysLastBtnRed == HIGH && currRed == LOW && enemysQueueCount > 0) {
    // Fire the current shoot (from queue head)
    EnemysShoot* currentShoot = &enemysShootQueue[enemysQueueHead];

    // Find free shoot slot
    for (int i = 0; i < ENEMYS_MAX_SHOOTS; i++) {
      if (!enemysFiredShoots[i].active) {
        enemysFiredShoots[i].pos = 3;  // Start after LED 0-3
        enemysFiredShoots[i].length = currentShoot->length;
        for (uint8_t j = 0; j < currentShoot->length; j++) {
          enemysFiredShoots[i].colors[j] = currentShoot->colors[j];
        }
        enemysFiredShoots[i].active = true;
        playTone(BUZZER_PIN, 1200, 50);

        // Remove from queue and add new shoot
        enemysQueueHead = (enemysQueueHead + 1) % ENEMYS_QUEUE_SIZE;
        enemysQueueCount--;
        enemysGenerateShoot();
        break;
      }
    }
  }

  enemysLastBtnRed = currRed;
  enemysLastBtnGreen = currGreen;
  enemysLastBtnBlue = currBlue;

  // Level up (nur für Speed-Erhöhung)
  if (enemysScore >= enemysLevel * 50) {
    enemysLevel++;

    // Speed erhöhen
    if (enemysSpeed > ENEMYS_MIN_SPEED) {
      enemysSpeed -= ENEMYS_SPEED_DECREASE;
    }

    playTone(BUZZER_PIN, 2000, 200);
  }

  // Dynamische Farbfreischaltung basierend auf Zeit UND Punkten
  unsigned long gameTime = (now - enemysGameStartTime) / 1000;  // Sekunden

  // BLAU: Nach 180 Sekunden (3 Minuten) ODER 500 Punkten
  if (enemysEnemyColorCount == 2 && (gameTime >= 180 || enemysScore >= 500)) {
    enemysEnemyColorCount = 3;
    playTone(BUZZER_PIN, 1600, 150);  // Neue Farbe im Enemy!
  }

  // GELB: Nach 360 Sekunden (6 Minuten) ODER 1000 Punkten
  if (enemysEnemyColorCount == 3 && (gameTime >= 360 || enemysScore >= 1000)) {
    enemysEnemyColorCount = 4;
    playTone(BUZZER_PIN, 1600, 150);  // Neue Farbe im Enemy!
  }

  // Draw to strip
  strip.clear();

  // Draw defense line (white)
  strip.setPixelColor(ENEMYS_DEFENSE_LINE, strip.Color(255, 255, 255));

  // Draw current shoot at bottom (LED 0-2, ready to fire)
  if (enemysQueueCount > 0) {
    EnemysShoot* currentShoot = &enemysShootQueue[enemysQueueHead];
    for (uint8_t i = 0; i < currentShoot->length; i++) {
      strip.setPixelColor(i, enemysGetColor(currentShoot->colors[i]));
    }
  }

  // Draw fired shoots
  for (int i = 0; i < ENEMYS_MAX_SHOOTS; i++) {
    if (enemysFiredShoots[i].active) {
      for (uint8_t j = 0; j < enemysFiredShoots[i].length; j++) {
        int ledPos = enemysFiredShoots[i].pos + j;
        if (ledPos >= 0 && ledPos < LED_COUNT) {
          strip.setPixelColor(ledPos, enemysGetColor(enemysFiredShoots[i].colors[j]));
        }
      }
    }
  }

  // Draw enemy snake (kontinuierlich von enemysSnakeOffset aus)
  for (int i = 0; i < enemysSnakeLength; i++) {
    int ledPos = enemysSnakeOffset + i;
    if (ledPos >= 0 && ledPos < LED_COUNT) {
      strip.setPixelColor(ledPos, enemysGetColor(enemysSnakeColors[i]));
    }
  }

  strip.show();

  // Turn off ring (keine Warteschlangen-Anzeige)
  raceRing.clear();
  raceRing.show();

  // Display score
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LVL:");
  display.print(enemysLevel);

  display.setTextSize(2);
  display.setCursor(centerX("SCORE", 2), 20);
  display.print("SCORE");

  display.setTextSize(3);
  display.setCursor(centerX(String(enemysScore), 3), 40);
  display.print(enemysScore);

  display.display();
}

void enemysGameOver() {
  // Check for highscore
  bool newHighscore = false;
  if (enemysScore > enemysHighScore) {
    enemysHighScore = enemysScore;
    EEPROM.put(EEPROM_ENEMYS_HIGHSCORE_ADDR, enemysHighScore);
    EEPROM.commit();
    newHighscore = true;
  }

  // Game over animation - red flash
  for (int i = 0; i < 5; i++) {
    strip.fill(strip.Color(255, 0, 0));
    strip.show();
    digitalWrite(LED_BTN_RED, HIGH);
    playTone(BUZZER_PIN, 200, 150);
    delay(200);

    strip.clear();
    strip.show();
    digitalWrite(LED_BTN_RED, LOW);
    delay(200);
  }

  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);

  // Display result
  display.clearDisplay();
  display.setTextSize(2);

  if (newHighscore) {
    display.setCursor(centerX("NEW", 2), 0);
    display.print("NEW");
    display.setCursor(centerX("RECORD!", 2), 18);
    display.print("RECORD!");
  } else {
    display.setCursor(centerX("GAME", 2), 0);
    display.print("GAME");
    display.setCursor(centerX("OVER!", 2), 18);
    display.print("OVER!");
  }

  display.setTextSize(1);
  display.setCursor(centerX("SCORE:", 1), 34);
  display.print("SCORE:");

  display.setTextSize(2);
  display.setCursor(centerX(String(enemysScore), 2), 46);
  display.print(enemysScore);

  display.display();

  if (newHighscore) {
    playTone(BUZZER_PIN, 2000, 500);
  }

  delay(5000);
}


/* ===================== REFLEX RACE IMPLEMENTATION ===================== */

// Forward declarations
void reflexRaceLoop();
void reflexRaceIdle();
void reflexRaceStartGame();
void reflexRaceGame();
void reflexRaceGameOver();
void reflexRaceSpawnTarget();

// State enum
enum ReflexRaceState {
  REFLEXRACE_INTRO,
  REFLEXRACE_IDLE,
  REFLEXRACE_COUNTDOWN,
  REFLEXRACE_GAME,
  REFLEXRACE_GAMEOVER
};
ReflexRaceState reflexRaceState = REFLEXRACE_INTRO;

// Game constants
#define REFLEXRACE_GAME_TIME 60000        // 60 seconds
#define REFLEXRACE_ZONE1_END 49           // RED zone: 0-49
#define REFLEXRACE_ZONE2_START 50         // GREEN zone: 50-99
#define REFLEXRACE_ZONE2_END 99
#define REFLEXRACE_ZONE3_START 100        // BLUE zone: 100-149
#define REFLEXRACE_INITIAL_DISPLAY_TIME 1000  // LED visible for 1 second
#define REFLEXRACE_MIN_DISPLAY_TIME 300       // Minimum 300ms

// Game variables
uint32_t reflexRaceScore = 0;
uint32_t reflexRaceHighScore = 0;
uint32_t reflexRaceHits = 0;
unsigned long reflexRaceGameStartTime = 0;
unsigned long reflexRaceTargetSpawnTime = 0;
unsigned long reflexRaceDisplayTime = REFLEXRACE_INITIAL_DISPLAY_TIME;
int reflexRaceCurrentLED = -1;          // Which LED is active
int reflexRaceCurrentZone = -1;         // 0=RED, 1=GREEN, 2=BLUE
bool reflexRaceTargetActive = false;

// Button states for edge detection
bool reflexRaceLastBtnRed = HIGH;
bool reflexRaceLastBtnGreen = HIGH;
bool reflexRaceLastBtnBlue = HIGH;

void reflexRaceLoop() {
  handleYellowReset();

  switch (reflexRaceState) {
    case REFLEXRACE_INTRO:
      showIntro();
      reflexRaceState = REFLEXRACE_IDLE;
      break;

    case REFLEXRACE_IDLE:
      reflexRaceIdle();
      break;

    case REFLEXRACE_COUNTDOWN:
      runSimonMultiCountdown();  // 3-2-1 countdown with all buttons
      reflexRaceStartGame();
      reflexRaceState = REFLEXRACE_GAME;
      break;

    case REFLEXRACE_GAME:
      reflexRaceGame();
      break;

    case REFLEXRACE_GAMEOVER:
      reflexRaceGameOver();
      reflexRaceState = REFLEXRACE_IDLE;
      break;
  }
}

void reflexRaceIdle() {
  // Load highscore once
  static bool highscoreLoaded = false;
  if (!highscoreLoaded) {
    EEPROM.get(EEPROM_REFLEXRACE_HIGHSCORE_ADDR, reflexRaceHighScore);
    if (reflexRaceHighScore > 100000) reflexRaceHighScore = 0;
    highscoreLoaded = true;
  }

  // Animated ring - 3 random flashing LEDs
  static unsigned long lastFlash = 0;
  if (millis() - lastFlash >= 200) {
    lastFlash = millis();
    raceRing.clear();
    raceRing.setPixelColor(random(36), raceRing.Color(255, 0, 0));    // RED
    raceRing.setPixelColor(random(36), raceRing.Color(0, 255, 0));    // GREEN
    raceRing.setPixelColor(random(36), raceRing.Color(0, 0, 255));    // BLUE
    raceRing.show();
  }

  // Brightness control
  bool currBlue = digitalRead(BTN_BLUE);
  bool currRed = digitalRead(BTN_RED);
  static bool lastBlue = HIGH, lastRed = HIGH;

  if (lastBlue == HIGH && currBlue == LOW) {
    brightness = min(brightness + BRIGHTNESS_STEP, BRIGHTNESS_MAX);
    raceRing.setBrightness(brightness);
  }
  if (lastRed == HIGH && currRed == LOW) {
    brightness = max(brightness - BRIGHTNESS_STEP, BRIGHTNESS_MIN);
    raceRing.setBrightness(brightness);
  }
  lastBlue = currBlue;
  lastRed = currRed;

  // Clear strip
  strip.clear();
  strip.show();

  softBlinkYellow();
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);

  // Display
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX("REFLEX", 2), 8);
  display.print("REFLEX");
  display.setCursor(centerX("RACE", 2), 24);
  display.print("RACE");

  display.setTextSize(1);
  display.setCursor(centerX("HI-SCORE:", 1), 36);
  display.print("HI-SCORE:");
  display.setTextSize(2);
  display.setCursor(centerX(String(reflexRaceHighScore), 2), 48);
  display.print(reflexRaceHighScore);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BR:");
  display.print(brightness);

  checkMuteToggle();
  drawSpeakerIcon();
  display.display();

  // Yellow to start
  if (!ignoreYellowUntilRelease && digitalRead(BTN_YELLOW) == LOW)
    reflexRaceState = REFLEXRACE_COUNTDOWN;
}

void reflexRaceStartGame() {
  reflexRaceScore = 0;
  reflexRaceHits = 0;
  reflexRaceGameStartTime = millis();
  reflexRaceDisplayTime = REFLEXRACE_INITIAL_DISPLAY_TIME;
  reflexRaceTargetActive = false;
  reflexRaceCurrentLED = -1;
  reflexRaceCurrentZone = -1;

  // Reset button states
  reflexRaceLastBtnRed = HIGH;
  reflexRaceLastBtnGreen = HIGH;
  reflexRaceLastBtnBlue = HIGH;

  // Turn off button LEDs
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);

  // Spawn first target
  reflexRaceSpawnTarget();
}

void reflexRaceSpawnTarget() {
  // Choose random LED
  reflexRaceCurrentLED = random(LED_COUNT);

  // Determine zone
  if (reflexRaceCurrentLED <= REFLEXRACE_ZONE1_END) {
    reflexRaceCurrentZone = 0;  // RED
    strip.setPixelColor(reflexRaceCurrentLED, strip.Color(255, 0, 0));
    digitalWrite(LED_BTN_RED, HIGH);
  } else if (reflexRaceCurrentLED <= REFLEXRACE_ZONE2_END) {
    reflexRaceCurrentZone = 1;  // GREEN
    strip.setPixelColor(reflexRaceCurrentLED, strip.Color(0, 255, 0));
    digitalWrite(LED_BTN_GREEN, HIGH);
  } else {
    reflexRaceCurrentZone = 2;  // BLUE
    strip.setPixelColor(reflexRaceCurrentLED, strip.Color(0, 0, 255));
    digitalWrite(LED_BTN_BLUE, HIGH);
  }

  strip.show();
  reflexRaceTargetSpawnTime = millis();
  reflexRaceTargetActive = true;
}

void reflexRaceGame() {
  unsigned long now = millis();
  unsigned long elapsed = now - reflexRaceGameStartTime;

  // Check time limit
  if (elapsed >= REFLEXRACE_GAME_TIME) {
    reflexRaceState = REFLEXRACE_GAMEOVER;
    return;
  }

  // Update ring countdown (36 LEDs = 60 seconds, 1 LED = 1.67s)
  unsigned long timeLeft = REFLEXRACE_GAME_TIME - elapsed;
  int ledsToShow = (int)((timeLeft * RACE_LED_COUNT) / REFLEXRACE_GAME_TIME);
  raceRing.clear();
  for (int i = 0; i < ledsToShow; i++) {
    raceRing.setPixelColor(i, raceRing.Color(255, 255, 0));  // Yellow
  }
  raceRing.show();

  // Check if target expired
  if (reflexRaceTargetActive && (now - reflexRaceTargetSpawnTime >= reflexRaceDisplayTime)) {
    // Target missed - penalty
    reflexRaceScore = (reflexRaceScore > 20) ? reflexRaceScore - 20 : 0;
    playTone(BUZZER_PIN, 200, 100);

    // Turn off LEDs
    strip.clear();
    strip.show();
    digitalWrite(LED_BTN_RED, LOW);
    digitalWrite(LED_BTN_GREEN, LOW);
    digitalWrite(LED_BTN_BLUE, LOW);

    // Spawn new target
    delay(200);
    reflexRaceSpawnTarget();
  }

  // Read buttons
  bool currRed = digitalRead(BTN_RED);
  bool currGreen = digitalRead(BTN_GREEN);
  bool currBlue = digitalRead(BTN_BLUE);

  if (reflexRaceTargetActive) {
    unsigned long reactionTime = now - reflexRaceTargetSpawnTime;
    bool correctPressed = false;
    bool wrongPressed = false;

    // Edge detection
    if (reflexRaceLastBtnRed == HIGH && currRed == LOW) {
      if (reflexRaceCurrentZone == 0) correctPressed = true;
      else wrongPressed = true;
    }
    if (reflexRaceLastBtnGreen == HIGH && currGreen == LOW) {
      if (reflexRaceCurrentZone == 1) correctPressed = true;
      else wrongPressed = true;
    }
    if (reflexRaceLastBtnBlue == HIGH && currBlue == LOW) {
      if (reflexRaceCurrentZone == 2) correctPressed = true;
      else wrongPressed = true;
    }

    if (correctPressed) {
      // Calculate points (faster = more points)
      int points = max(10, (int)(100 - (reactionTime / 10)));
      reflexRaceScore += points;
      reflexRaceHits++;
      playTone(BUZZER_PIN, 1500, 50);

      // Progressive difficulty (reduce display time)
      if (reflexRaceHits % 5 == 0) {
        reflexRaceDisplayTime = max(REFLEXRACE_MIN_DISPLAY_TIME,
                                    reflexRaceDisplayTime - 50);
      }

      // Clear and spawn next
      strip.clear();
      strip.show();
      digitalWrite(LED_BTN_RED, LOW);
      digitalWrite(LED_BTN_GREEN, LOW);
      digitalWrite(LED_BTN_BLUE, LOW);
      delay(100);
      reflexRaceSpawnTarget();

    } else if (wrongPressed) {
      // Wrong button - penalty
      reflexRaceScore = (reflexRaceScore > 50) ? reflexRaceScore - 50 : 0;
      playTone(BUZZER_PIN, 200, 200);
    }
  }

  // Update button states
  reflexRaceLastBtnRed = currRed;
  reflexRaceLastBtnGreen = currGreen;
  reflexRaceLastBtnBlue = currBlue;

  // Display HUD
  display.clearDisplay();
  display.setTextSize(1);

  unsigned long timeLeftSec = timeLeft / 1000;
  display.setCursor(0, 0);
  display.print("TIME: ");
  display.print(timeLeftSec);
  display.print("s");

  display.setCursor(centerX("SCORE", 1), 20);
  display.print("SCORE");
  display.setTextSize(3);
  display.setCursor(centerX(String(reflexRaceScore), 3), 35);
  display.print(reflexRaceScore);

  display.display();
}

void reflexRaceGameOver() {
  bool newHighscore = false;
  if (reflexRaceScore > reflexRaceHighScore) {
    reflexRaceHighScore = reflexRaceScore;
    EEPROM.put(EEPROM_REFLEXRACE_HIGHSCORE_ADDR, reflexRaceHighScore);
    EEPROM.commit();
    newHighscore = true;
  }

  // Turn off all LEDs
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);
  strip.clear();
  strip.show();
  raceRing.clear();
  raceRing.show();

  // Display results
  display.clearDisplay();

  if (newHighscore) {
    display.setTextSize(2);
    display.setCursor(centerX("NEW", 2), 0);
    display.print("NEW");
    display.setCursor(centerX("RECORD!", 2), 18);
    display.print("RECORD!");
  } else {
    display.setTextSize(2);
    display.setCursor(centerX("GAME", 2), 5);
    display.print("GAME");
    display.setCursor(centerX("OVER", 2), 23);
    display.print("OVER");
  }

  display.setTextSize(1);
  display.setCursor(centerX("SCORE:", 1), 32);
  display.print("SCORE:");
  display.setTextSize(1);
  display.setCursor(centerX(String(reflexRaceScore), 1), 42);
  display.print(reflexRaceScore);

  display.setTextSize(1);
  display.setCursor(centerX("TREFFER:", 1), 52);
  display.print("TREFFER:");
  display.print(" ");
  display.print(reflexRaceHits);

  display.display();

  // Victory sound
  if (newHighscore) {
    playTone(BUZZER_PIN, 1500, 500);
  } else {
    playTone(BUZZER_PIN, 800, 300);
  }

  delay(5000);  // Show for 5 seconds
}


/* ===================== CLOCK IMPLEMENTATION ===================== */

// Clock uses the LED Ring (GPIO14, 36 LEDs) as clock face
// LED 0 = 12:00/24:00 position
// 36 LEDs for clock: each LED = 10 seconds or ~1.67 minutes
// Hour markers every 3 LEDs (36/12 = 3 LEDs per hour)

void clockLoop() {
  // Check for any button press to exit clock mode
  if (digitalRead(BTN_RED) == LOW || digitalRead(BTN_GREEN) == LOW ||
      digitalRead(BTN_BLUE) == LOW || digitalRead(BTN_YELLOW) == LOW) {
    // Reset idle timer and stay in current game
    lastIdleActivity = millis();
    // Brief delay for debounce
    delay(200);
    return;
  }

  switch (clockState) {
    case CLOCK_INTRO:
      // Show "UHR" text briefly without sound (like other games)
      display.clearDisplay();
      display.setTextSize(3);
      display.setCursor(centerX("UHR", 3), 20);
      display.print("UHR");
      display.display();
      delay(500);  // Brief display
      clockState = CLOCK_DISPLAY;
      break;
    case CLOCK_DISPLAY:
      clockDisplay();
      break;
  }
}

void clockDisplay() {
  // Get current time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Time not available
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Zeit nicht verfuegbar");
    display.setCursor(0, 20);
    display.print("WiFi pruefen!");
    display.display();

    raceRing.clear();
    raceRing.show();
    strip.clear();
    strip.show();
    delay(1000);
    return;
  }

  // Get milliseconds for smooth second hand
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int milliseconds = tv.tv_usec / 1000;  // 0-999

  // Draw clock on LED ring (GPIO14) with smooth seconds
  clockDrawRing(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, milliseconds);

  // Draw time on OLED display (without seconds)
  display.clearDisplay();

  // Large time display (24h format, no seconds)
  display.setTextSize(3);
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  display.setCursor(centerX(String(timeStr), 3), 8);
  display.print(timeStr);

  // Date below
  display.setTextSize(1);
  char dateStr[12];
  sprintf(dateStr, "%02d.%02d.%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  display.setCursor(centerX(String(dateStr), 1), 42);
  display.print(dateStr);

  // Day of week
  const char* days[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
  display.setCursor(centerX(String(days[timeinfo.tm_wday]), 1), 54);
  display.print(days[timeinfo.tm_wday]);

  display.display();

  // Turn off main strip
  strip.clear();
  strip.show();

  // Turn off all button LEDs (especially yellow)
  digitalWrite(LED_BTN_RED, LOW);
  digitalWrite(LED_BTN_GREEN, LOW);
  digitalWrite(LED_BTN_BLUE, LOW);
  digitalWrite(LED_BTN_YELLOW, LOW);

  // Short delay for smooth animation (20ms = 50fps)
  delay(20);
}

void clockDrawRing(int hours, int minutes, int seconds, int milliseconds) {
  raceRing.clear();

  // Ring has 36 LEDs
  // LED 0 = 12:00 position (top)
  // Hour markers: every 3 LEDs (36/12 = 3)

  // Convert 24h to 12h for hour hand position (display stays 24h)
  int hours12 = hours % 12;

  // Hour markers (every 3 LEDs) - dim yellow/orange
  // LED 0 = 12 Uhr, LED 3 = 1 Uhr, LED 6 = 2 Uhr, etc.
  for (int i = 0; i < RACE_LED_COUNT; i += 3) {
    raceRing.setPixelColor(i, raceRing.Color(40, 30, 0));  // Dim yellow/orange
  }

  // Smooth second hand position (white) - use milliseconds for fluid movement
  // Total milliseconds in current minute: seconds * 1000 + milliseconds
  long totalMs = (long)seconds * 1000 + milliseconds;
  // Map 0-59999 ms to 0-35 LEDs (multiply first to avoid precision loss)
  int secondLed = (int)((totalMs * RACE_LED_COUNT) / 60000L);
  raceRing.setPixelColor(secondLed, raceRing.Color(255, 255, 255));  // White

  // Minute hand position (blue) - map 0-59 minutes to 0-35 LEDs
  int minuteLed = (minutes * RACE_LED_COUNT) / 60;
  raceRing.setPixelColor(minuteLed, raceRing.Color(0, 0, 255));  // Blue

  // Hour hand position (red) - each hour = 3 LEDs, plus partial for minutes
  // hours12 * 3 gives base position, + (minutes * 3) / 60 for smooth movement
  int hourLed = (hours12 * 3 + (minutes * 3) / 60) % RACE_LED_COUNT;
  raceRing.setPixelColor(hourLed, raceRing.Color(255, 0, 0));  // Red

  // If hands overlap, show combined colors
  // Second + Minute overlap = Cyan
  if (secondLed == minuteLed) {
    raceRing.setPixelColor(secondLed, raceRing.Color(0, 255, 255));  // Cyan
  }
  // Second + Hour overlap = Magenta
  if (secondLed == hourLed) {
    raceRing.setPixelColor(secondLed, raceRing.Color(255, 0, 255));  // Magenta
  }
  // Minute + Hour overlap = Purple
  if (minuteLed == hourLed && minuteLed != secondLed) {
    raceRing.setPixelColor(minuteLed, raceRing.Color(128, 0, 255));  // Purple
  }
  // All three overlap = White
  if (secondLed == minuteLed && minuteLed == hourLed) {
    raceRing.setPixelColor(secondLed, raceRing.Color(255, 255, 255));  // White
  }

  raceRing.show();
}
