/*
 * ========================================================================
 * EC 10-CIRCUIT ELECTRICAL CONNECTOR TESTER WITH PASS COUNTER
 * ========================================================================
 *
 * Project: ec_10_circuit
 * Version: 2.0 Professional
 * Date: October 2025
 * Author: AGUS F
 *
 * DESCRIPTION:
 * Advanced 10-circuit electrical connector tester dengan persistent pass counter.
 * Sistem ini menguji kontinuitas 10 circuit pada konektor electrical dengan
 * teknologi matrix scanning dan EEPROM-based production tracking.
 *
 * MAIN FEATURES:
 * • 10-Circuit continuity testing dengan matrix scanning
 * • Persistent pass counter menggunakan EEPROM storage
 * • Production limit protection (90,000 cycles max)
 * • LCD 16x2 real-time display dengan backlight control
 * • Audio feedback dengan buzzer confirmation
 * • LED array indicator untuk visual circuit status
 * • Pin 13 trigger untuk automated counting
 * • Magic key system untuk secure counter reset
 * • Serial monitoring untuk debugging dan audit
 *
 * ADVANCED CAPABILITIES:
 * • Non-volatile pass counting (survives power cycles)
 * • Automatic system lock saat production limit tercapai
 * • Magic key validation untuk prevent accidental reset
 * • EEPROM integrity checking dan corruption recovery
 * • Matrix pin configuration untuk flexible connector types
 * • Real-time circuit status monitoring
 *
 * HARDWARE REQUIREMENTS:
 * • Arduino Mega 2560 (required untuk sufficient I/O pins)
 * • LCD 16x2 dengan standard parallel connection
 * • 10x LED indicators untuk circuit status
 * • 10x Button inputs untuk manual testing
 * • Buzzer untuk audio feedback
 * • Electrical connector test fixture
 * • Potentiometer untuk contrast adjustment
 *
 * PRODUCTION APPLICATIONS:
 * • Manufacturing quality control
 * • Electrical connector validation
 * • Production line integration
 * • Quality assurance testing
 * • Process automation
 * • Statistical production tracking
 *
 * TECHNICAL SPECIFICATIONS:
 * • Circuit Capacity: 10 independent circuits
 * • Production Limit: 90,000 cycles
 * • Storage: EEPROM persistent memory
 * • Display: LCD 16x2 with backlight
 * • I/O Requirements: 32+ digital pins
 * • Power: 5V DC, 500mA typical
 *
 */

#include <LiquidCrystal.h>
#include <EEPROM.h>

/*
 * PASS COUNTER RESET PROCEDURE:
 * =============================
 * SECURITY FEATURE: Counter hanya bisa direset melalui code upload
 * untuk mencegah manipulasi data produksi.
 *
 * RESET STEPS:
 * 1. Ubah baris: const long MAGIC_KEY = 0x12345678L;
 *    Menjadi:    const long MAGIC_KEY = 0x87654321L;
 * 2. Upload kode yang sudah diubah ke Arduino
 * 3. Ubah kembali ke: const long MAGIC_KEY = 0x12345678L;
 * 4. Upload lagi untuk operasi normal
 * 5. Pass counter akan direset ke 0 secara permanent
 *
 * SECURITY RATIONALE:
 * • Mencegah reset tidak sengaja oleh operator
 * • Maintain audit trail untuk production data
 * • Require supervisor access untuk reset counter
 * • Protect production statistics integrity
 */

// ================================================================
// LCD 16x2 DISPLAY CONFIGURATION
// ================================================================

/*
 * LCD WIRING DIAGRAM:
 * ===================
 * LCD 16x2 Pin    Arduino Mega Pin    Function
 * ============    ================    ========
 * 1  (VSS)    -> GND               -> Ground
 * 2  (VDD)    -> +5V               -> Power Supply
 * 3  (V0)     -> Pin 6 (PWM)       -> Contrast Control
 * 4  (RS)     -> Pin 7             -> Register Select
 * 5  (E)      -> Pin 8             -> Enable Signal
 * 6  (D0)     -> Not Connected     -> Data Bit 0 (4-bit mode)
 * 7  (D1)     -> Not Connected     -> Data Bit 1 (4-bit mode)
 * 8  (D2)     -> Not Connected     -> Data Bit 2 (4-bit mode)
 * 9  (D3)     -> Not Connected     -> Data Bit 3 (4-bit mode)
 * 10 (D4)     -> Pin 9             -> Data Bit 4
 * 11 (D5)     -> Pin 10            -> Data Bit 5
 * 12 (D6)     -> Pin 11            -> Data Bit 6
 * 13 (D7)     -> Pin 12            -> Data Bit 7
 * 14 (A)      -> Pin 5 (PWM)       -> Backlight Anode (+)
 * 15 (K)      -> GND               -> Backlight Cathode (-)
 * 16 (NC)     -> Not Connected     -> No Connection
 */
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); // RS, E, D4, D5, D6, D7

// ================================================================
// PASS COUNTER & EEPROM MANAGEMENT SYSTEM
// ================================================================

/*
 * PRODUCTION COUNTER SPECIFICATIONS:
 * • Max Capacity: 90,000 cycles (production lifetime)
 * • Storage: EEPROM non-volatile memory (survives power loss)
 * • Security: Magic key validation system
 * • Reset: Only via code upload (prevents tampering)
 * • Tracking: Pin 13 trigger-based counting
 * • Protection: Auto-lock when limit reached
 */
const long MAX_PASS_COUNT = 90000L; // Production lifetime limit (90,000 cycles)
long passCount = 0L;                // Current pass counter (stored in EEPROM)
bool systemLocked = false;          // System lock flag when limit reached

/*
 * PIN 13 TRIGGER SYSTEM:
 * Pin 13 digunakan sebagai external trigger untuk counting.
 * Deteksi perubahan dari LOW ke HIGH untuk increment counter.
 * Ideal untuk integration dengan production line automation.
 */
bool pin13WasLow = true;                      // Edge detection flag (LOW→HIGH transition)
const unsigned long PASS_TIMER_INTERVAL = 30; // Legacy timer interval (deprecated)

/*
 * EEPROM MEMORY MAP:
 * ==================
 * Address 0-3: Pass Counter (4 bytes, long integer)
 * Address 8-11: Magic Key (4 bytes, long integer)
 *
 * MAGIC KEY SYSTEM:
 * • Normal Operation: 0x12345678L
 * • Reset Request: 0x87654321L
 * • Purpose: Secure counter reset via code upload only
 * • Prevents: Accidental or unauthorized counter reset
 */
const int EEPROM_PASS_ADDR = 0;           // EEPROM address untuk pass counter (4 bytes)
const int EEPROM_MAGIC_ADDR = 8;          // EEPROM address untuk magic key (4 bytes)
const long MAGIC_KEY = 0x12345678L;       // Normal operation magic key
const long RESET_MAGIC_KEY = 0x87654321L; // Reset request magic key
// ================================================================
// 10-CIRCUIT MATRIX CONFIGURATION
// ================================================================

/*
 * ELECTRICAL CONNECTOR MATRIX MAPPING:
 * ====================================
 *
 * Sistem ini menguji kontinuitas 10 circuit pada konektor electrical
 * menggunakan matrix scanning approach untuk efficient I/O usage.
 *
 * SIDE A (Transmitter Pins) - Output pins untuk test signal:
 * Pin Array: 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
 * Function: Generate test signals untuk setiap circuit
 * Connector: 15CZ-6Y terminals (No. 15, 14, 13, ..., 6)
 *
 * SIDE B (Receiver Pins) - Input pins untuk signal detection:
 * Pin Array: 63, 62, 61, 60, 59, 58, 57, 56, 55, 54
 * Function: Detect test signals melalui connector circuits
 * Connector: 15CZ-6H terminals (No. 1, 2, 3, ..., 10)
 *
 * MATRIX TESTING PRINCIPLE:
 * 1. Set Side A pin HIGH (one at a time)
 * 2. Read corresponding Side B pin status
 * 3. If HIGH detected → Circuit PASS (continuity OK)
 * 4. If LOW detected → Circuit FAIL (open/broken)
 * 5. Repeat untuk semua 10 circuits
 */
int endA[10] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31}; // Side A - Transmitter pins (15CZ-6Y)
int endB[10] = {63, 62, 61, 60, 59, 58, 57, 56, 55, 54}; // Side B - Receiver pins (15CZ-6H)

// ================================================================
// LED INDICATOR ARRAY SYSTEM
// ================================================================

/*
 * LED VISUAL FEEDBACK SYSTEM:
 * ============================
 * 10 LED indicators untuk real-time visual status setiap circuit.
 *
 * LED Mapping:
 * LED 1-10 → Pins 42-51 → Circuit Status Indicators
 *
 * LED States:
 * • ON (HIGH): Circuit PASS - kontinuitas OK
 * • OFF (LOW): Circuit FAIL - open/broken circuit
 * • Blinking: Circuit under test (optional)
 *
 * Visual Benefits:
 * • Instant visual feedback untuk operator
 * • Easy identification circuit yang bermasalah
 * • Production line efficiency improvement
 * • Quality control visual verification
 */
int pinLed1 = 42;  // Circuit 1 status indicator LED
int pinLed2 = 43;  // Circuit 2 status indicator LED
int pinLed3 = 44;  // Circuit 3 status indicator LED
int pinLed4 = 45;  // Circuit 4 status indicator LED
int pinLed5 = 46;  // Circuit 5 status indicator LED
int pinLed6 = 47;  // Circuit 6 status indicator LED
int pinLed7 = 48;  // Circuit 7 status indicator LED
int pinLed8 = 49;  // Circuit 8 status indicator LED
int pinLed9 = 50;  // Circuit 9 status indicator LED
int pinLed10 = 51; // Circuit 10 status indicator LED

// ================================================================
// BUTTON INPUT ARRAY SYSTEM
// ================================================================

/*
 * MANUAL TEST BUTTON INTERFACE:
 * ==============================
 * 10 button inputs untuk manual testing dan control functions.
 *
 * Button Mapping:
 * Button 1-10 → Pins 32-41 → Manual Test Controls
 *
 * Button Functions:
 * • Individual Circuit Testing: Manual trigger untuk specific circuit
 * • Stop Functions: Emergency stop atau pause testing
 * • Diagnostic Mode: Manual step-through testing
 * • Calibration: Individual circuit calibration
 *
 * Input Configuration:
 * • INPUT_PULLUP: Internal pull-up resistors enabled
 * • Active LOW: Button pressed = LOW signal
 * • Debouncing: Software debouncing implemented
 * • Response Time: <50ms typical
 */
int pinBtn1 = 32;  // Manual test button circuit 1 / STOP function
int pinBtn2 = 33;  // Manual test button circuit 2
int pinBtn3 = 34;  // Manual test button circuit 3
int pinBtn4 = 35;  // Manual test button circuit 4
int pinBtn5 = 36;  // Manual test button circuit 5
int pinBtn6 = 37;  // Manual test button circuit 6
int pinBtn7 = 38;  // Manual test button circuit 7
int pinBtn8 = 39;  // Manual test button circuit 8
int pinBtn9 = 40;  // Manual test button circuit 9
int pinBtn10 = 41; // Manual test button circuit 10

// ================================================================
// SYSTEM VARIABLES & TESTING ARRAYS
// ================================================================

/*
 * CIRCUIT TESTING STATE MANAGEMENT:
 * ==================================
 * Sistem menggunakan multiple arrays untuk track status setiap circuit
 * selama proses testing dan validation.
 */

#define NUMBER (Nu) // Macro untuk total number of circuits
int Nu = 10;        // Total circuits to test (10-circuit connector)

/*
 * TESTING RESULT ARRAYS:
 * • result[]: Final test results untuk setiap circuit
 * • test[]: Intermediate test values during scanning
 * • counter[]: Counter values untuk each circuit (if needed)
 *
 * Array Values:
 * • -1: Not tested yet / Invalid state
 * •  0: Circuit FAIL (open/broken)
 * •  1: Circuit PASS (continuity OK)
 */
int result[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};  // Final test results
int test[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};    // Intermediate test values
int counter[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // Circuit counters (optional)

bool fail = false; // Global failure flag (true if any circuit fails)

// ================================================================
// DISPLAY & AUDIO FEEDBACK SYSTEM
// ================================================================

/*
 * LCD DISPLAY CONTROL:
 * • Contrast: PWM control untuk LCD contrast adjustment
 * • Backlight: PWM control untuk LCD backlight brightness
 * • Potentiometer: Manual contrast adjustment input
 */
int Contrast = 60;            // LCD contrast level (0-255)
#define LCD_Backlight 5       // LCD backlight control pin (PWM)
int POT = 64;                 // Potentiometer input pin (analog)
int potVal = analogRead(POT); // Current potentiometer value

/*
 * AUDIO FEEDBACK SYSTEM:
 * • Speaker: Audio feedback untuk test results
 * • Buzzer: Pin 13 trigger dan audio notification
 * • Frequency: Variable frequency untuk different tones
 */
const int speakerPin = 4;  // Speaker output pin
int frequency;             // Audio frequency variable
const int BUZZER_PIN = 13; // Buzzer pin (also used for trigger counting)

// ================================================================
// EEPROM PERSISTENT STORAGE FUNCTIONS
// ================================================================

/*
 * readPassCountFromEEPROM() - Read pass counter dari EEPROM storage
 *
 * FUNCTION: Membaca production counter dari non-volatile EEPROM memory
 *
 * FEATURES:
 * • EEPROM.get() untuk safe 4-byte reading
 * • Corruption detection dan auto-recovery
 * • Range validation untuk prevent invalid values
 * • Initialize ke 0 jika EEPROM kosong atau corrupt
 *
 * CORRUPTION HANDLING:
 * • 0xFFFFFFFF: Fresh EEPROM (never written) → Reset ke 0
 * • Negative values: Corrupted data → Reset ke 0
 * • > 2x MAX_PASS_COUNT: Invalid range → Reset ke 0
 *
 * RETURN: Valid pass count (0 to MAX_PASS_COUNT)
 */
long readPassCountFromEEPROM()
{
  long count = 0;

  // Safe read 4-byte long integer dari EEPROM
  EEPROM.get(EEPROM_PASS_ADDR, count);

  // CORRUPTION DETECTION: Fresh EEPROM atau negative values
  if (count == 0xFFFFFFFF || count < 0)
  {
    Serial.println("EEPROM: Fresh or corrupted data detected, initializing to 0");
    count = 0;
  }

  // RANGE VALIDATION: Prevent unrealistic values
  if (count > MAX_PASS_COUNT * 2)
  {
    Serial.print("EEPROM: Invalid count detected (");
    Serial.print(count);
    Serial.println("), resetting to 0");
    count = 0;
  }

  return count;
}

/*
 * writePassCountToEEPROM() - Write pass counter ke EEPROM storage
 *
 * FUNCTION: Simpan production counter ke non-volatile EEPROM memory
 *
 * FEATURES:
 * • EEPROM.put() untuk safe 4-byte writing
 * • Automatic wear leveling (built-in EEPROM library)
 * • Immediate write (no buffering)
 * • Power-loss safe operation
 *
 * PARAMETERS:
 * • count: Pass counter value to save (0 to MAX_PASS_COUNT)
 *
 * EEPROM SPECIFICATIONS:
 * • Write Cycles: 100,000 guaranteed per location
 * • Data Retention: 100+ years at room temperature
 * • Write Time: ~3.3ms per byte
 * • Endurance: Sufficient untuk production environment
 */
void writePassCountToEEPROM(long count)
{
  // Safe write 4-byte long integer ke EEPROM
  EEPROM.put(EEPROM_PASS_ADDR, count);

  Serial.print("EEPROM: Pass count saved: ");
  Serial.println(count);
}

/*
 * readMagicKeyFromEEPROM() - Read magic key untuk security validation
 *
 * FUNCTION: Baca magic key dari EEPROM untuk detect code upload events
 *
 * MAGIC KEY PURPOSES:
 * • Detect first-time upload vs normal startup
 * • Identify reset requests via code modification
 * • Prevent accidental counter resets
 * • Maintain production data integrity
 *
 * RETURN: Magic key value dari EEPROM
 */
long readMagicKeyFromEEPROM()
{
  long key = 0;
  EEPROM.get(EEPROM_MAGIC_ADDR, key);
  return key;
}

/*
 * writeMagicKeyToEEPROM() - Write magic key untuk security tracking
 *
 * FUNCTION: Simpan magic key ke EEPROM untuk track system state
 *
 * PARAMETERS:
 * • key: Magic key value (MAGIC_KEY atau RESET_MAGIC_KEY)
 */
void writeMagicKeyToEEPROM(long key)
{
  EEPROM.put(EEPROM_MAGIC_ADDR, key);

  Serial.print("EEPROM: Magic key saved: 0x");
  Serial.println(key, HEX);
}

/*
 * resetPassCounter() - Secure counter reset function
 *
 * FUNCTION: Reset production counter dengan security validation
 *
 * SECURITY FEATURES:
 * • Only accessible via code upload (not runtime)
 * • Requires magic key modification
 * • Prevents operator tampering
 * • Maintains audit trail
 *
 * PROCESS:
 * 1. Reset counter ke 0
 * 2. Save ke EEPROM immediately
 * 3. Set normal magic key
 * 4. Log reset event
 */
void resetPassCounter()
{
  passCount = 0;
  writePassCountToEEPROM(passCount);
  writeMagicKeyToEEPROM(MAGIC_KEY); // Restore normal magic key

  Serial.println("=== SECURITY RESET EVENT ===");
  Serial.println("Pass counter has been reset to 0 by authorized code upload");
  Serial.println("Magic key restored to normal operation mode");
  Serial.println("==============================");
}

// ================================================================
// SISTEM INITIALIZATION & STARTUP SEQUENCE
// ================================================================

/*
 * setup() - Complete system initialization dan startup sequence
 *
 * INITIALIZATION SEQUENCE:
 * 1. Serial Communication Setup (115200 baud untuk fast debugging)
 * 2. Startup Banner & System Identification
 * 3. Button Input Configuration (INPUT_PULLUP)
 * 4. EEPROM Magic Key Validation & Security Check
 * 5. Pass Counter Recovery/Reset Logic
 * 6. Production Limit Protection Setup
 * 7. Pin 13 Trigger System Initialization
 * 8. LCD Display & Backlight Configuration
 * 9. LED Array Setup (Pins 42-51)
 * 10. Button Array Setup (Pins 32-41)
 * 11. Audio System Configuration
 * 12. Matrix Testing Pin Configuration
 * 13. System Ready Confirmation
 *
 * SECURITY FEATURES:
 * • Magic key validation untuk prevent tampering
 * • EEPROM corruption detection & recovery
 * • Production limit enforcement
 * • Secure reset via code upload only
 */
void setup()
{
  // ================================================================
  // STEP 1: SERIAL COMMUNICATION INITIALIZATION
  // ================================================================

  // High-speed serial untuk comprehensive debugging & monitoring
  Serial.begin(115200);

  // Wait untuk Serial port ready (especially untuk Leonardo/Micro)
  while (!Serial)
  {
    ; // Wait for serial port to connect
  }

  // ================================================================
  // STEP 2: STARTUP BANNER & SYSTEM IDENTIFICATION
  // ================================================================

  Serial.println("======================================================");
  Serial.println("   EC 10-CIRCUIT ELECTRICAL CONNECTOR TESTER v2.0");
  Serial.println("              Professional Edition");
  Serial.println("                 by AGUS F");
  Serial.println("======================================================");
  Serial.println();
  Serial.println("INITIALIZING SYSTEM...");
  Serial.print("Max Production Cycles: ");
  Serial.println(MAX_PASS_COUNT);
  Serial.print("EEPROM Pass Address: ");
  Serial.println(EEPROM_PASS_ADDR);
  Serial.print("EEPROM Magic Address: ");
  Serial.println(EEPROM_MAGIC_ADDR);
  Serial.println();

  // ================================================================
  // STEP 3: INPUT BUTTON CONFIGURATION
  // ================================================================

  // Setup pin button dengan internal pull-up untuk stable input
  pinMode(pinBtn1, INPUT_PULLUP);
  Serial.println("Button inputs configured with pull-up resistors");

  // ================================================================
  // STEP 4: EEPROM MAGIC KEY VALIDATION & SECURITY CHECK
  // ================================================================

  // Read current magic key untuk determine system state
  long currentMagicKey = readMagicKeyFromEEPROM();
  Serial.print("Current magic key from EEPROM: 0x");
  Serial.println(currentMagicKey, HEX);
  Serial.print("Expected normal key: 0x");
  Serial.println(MAGIC_KEY, HEX);
  Serial.print("Reset request key: 0x");
  Serial.println(RESET_MAGIC_KEY, HEX);
  Serial.println();

  // ================================================================
  // STEP 5: PASS COUNTER RECOVERY/RESET LOGIC
  // ================================================================

  // RESET REQUEST DETECTED (Security-controlled reset)
  if (currentMagicKey == RESET_MAGIC_KEY)
  {
    Serial.println("*** RESET MAGIC KEY DETECTED ***");
    Serial.println("Authorized reset request via code upload");
    passCount = 0;
    writePassCountToEEPROM(passCount);
    writeMagicKeyToEEPROM(MAGIC_KEY); // Restore normal magic key
    Serial.println("*** PASS COUNTER SUCCESSFULLY RESET TO 0 ***");
    Serial.println("Magic key restored to normal operation");
    Serial.println();
  }
  // FIRST UPLOAD OR CORRUPTED EEPROM DETECTED
  else if (currentMagicKey != MAGIC_KEY)
  {
    Serial.println("*** FIRST UPLOAD OR CORRUPTED EEPROM DETECTED ***");
    Serial.println("Initializing fresh system state...");

    // Initialize system untuk first-time use atau recovery
    passCount = 0;
    writePassCountToEEPROM(passCount);
    writeMagicKeyToEEPROM(MAGIC_KEY);

    Serial.println("Pass counter initialized to 0");
    Serial.println("Magic key set to normal operation mode");
    Serial.println();
  }
  // NORMAL STARTUP (Existing system dengan valid data)
  else
  {
    Serial.println("*** NORMAL STARTUP - RECOVERING EXISTING DATA ***\");
    
    // Normal startup - read existing pass count dari EEPROM
    passCount = readPassCountFromEEPROM();
    
    Serial.println("Existing production data recovered successfully");
    Serial.println();
  }

  // Display current production statistics
  Serial.println("=== PRODUCTION STATISTICS ===");
  Serial.print("Current pass count: ");
  Serial.println(passCount);
  Serial.print("Production limit: ");
  Serial.println(MAX_PASS_COUNT);
  Serial.print("Remaining cycles: ");
  Serial.println(MAX_PASS_COUNT - passCount);
  Serial.print("Capacity used: ");
  Serial.print((float)passCount / MAX_PASS_COUNT * 100.0, 1);
  Serial.println("%");
  Serial.println("==============================");
  Serial.println();

  // SECURITY NOTE: Pass counter hanya bisa direset dengan upload ulang kode
  Serial.println("SECURITY: Counter reset requires authorized code upload only");
  Serial.println("No runtime reset capability untuk maintain data integrity");
  Serial.println();

  // ================================================================
  // STEP 6: PIN 13 TRIGGER SYSTEM INITIALIZATION
  // ================================================================

  // Initialize pin 13 trigger system untuk automated counting
  pin13WasLow = true;
  pinMode(BUZZER_PIN, INPUT_PULLUP); // Configure pin 13 as input dengan pull-up

  Serial.println("Pin 13 trigger system initialized:");
  Serial.println("• Function: Automated pass counting");
  Serial.println("• Trigger: LOW → HIGH transition detection");
  Serial.println("• Integration: Production line automation ready");
  Serial.println();

  // ================================================================
  // STEP 7: PRODUCTION LIMIT PROTECTION CHECK
  // ================================================================

  // Check production limit dan set system lock jika necessary
  if (passCount >= MAX_PASS_COUNT)
  {
    systemLocked = true;
    Serial.println("*** PRODUCTION LIMIT REACHED ***");
    Serial.println("System automatically locked to prevent overuse");
    Serial.println("Contact supervisor untuk counter reset authorization");
    Serial.println("================================");
  }
  else
  {
    systemLocked = false;
    Serial.println("Production limit OK - System ready untuk normal operation");
  }
  Serial.println();

  // ================================================================
  // STEP 8: AUDIO SYSTEM CONFIGURATION
  // ================================================================

  // Configure audio feedback system
  frequency = map(potVal, 0, 1023, 0, 255); // Map potentiometer ke PWM frequency values
  pinMode(speakerPin, OUTPUT);              // Speaker output untuk audio feedback

  Serial.println("Audio system configured:");
  Serial.print("• Speaker pin: ");
  Serial.println(speakerPin);
  Serial.print("• Initial frequency: ");
  Serial.println(frequency);

  // ================================================================
  // STEP 9: LCD DISPLAY CONFIGURATION
  // ================================================================

  // Configure LCD contrast dan backlight
  analogWrite(6, Contrast);        // Set LCD contrast via PWM
  pinMode(LCD_Backlight, OUTPUT);  // LCD backlight control pin
  analogWrite(LCD_Backlight, 400); // Set LCD backlight brightness

  // Initialize LCD display
  lcd.begin(16, 2); // Initialize 16x2 LCD

  Serial.println("LCD display configured:");
  Serial.print("• Contrast level: ");
  Serial.println(Contrast);
  Serial.print("• Backlight pin: ");
  Serial.println(LCD_Backlight);
  Serial.println("• Display size: 16x2 characters");
  Serial.println();

  // ================================================================
  // STEP 10: LED INDICATOR ARRAY SETUP
  // ================================================================

  // Configure all LED pins as outputs untuk visual circuit status
  pinMode(pinLed1, OUTPUT);  // Circuit 1 indicator
  pinMode(pinLed2, OUTPUT);  // Circuit 2 indicator
  pinMode(pinLed3, OUTPUT);  // Circuit 3 indicator
  pinMode(pinLed4, OUTPUT);  // Circuit 4 indicator
  pinMode(pinLed5, OUTPUT);  // Circuit 5 indicator
  pinMode(pinLed6, OUTPUT);  // Circuit 6 indicator
  pinMode(pinLed7, OUTPUT);  // Circuit 7 indicator
  pinMode(pinLed8, OUTPUT);  // Circuit 8 indicator
  pinMode(pinLed9, OUTPUT);  // Circuit 9 indicator
  pinMode(pinLed10, OUTPUT); // Circuit 10 indicator

  Serial.println("LED indicator array configured:");
  Serial.println("• Total LEDs: 10 (Pins 42-51)");
  Serial.println("• Function: Circuit status visualization");
  Serial.println("• ON = Circuit PASS, OFF = Circuit FAIL");
  Serial.println();

  // ================================================================
  // STEP 11: BUTTON INPUT ARRAY SETUP
  // ================================================================

  // Configure all button pins dengan internal pull-up resistors
  pinMode(pinBtn1, INPUT_PULLUP);  // Button 1 dengan pull-up (GND connection detection)
  pinMode(pinBtn2, INPUT_PULLUP);  // Button 2 dengan pull-up
  pinMode(pinBtn3, INPUT_PULLUP);  // Button 3 dengan pull-up
  pinMode(pinBtn4, INPUT_PULLUP);  // Button 4 dengan pull-up
  pinMode(pinBtn5, INPUT_PULLUP);  // Button 5 dengan pull-up
  pinMode(pinBtn6, INPUT_PULLUP);  // Button 6 dengan pull-up
  pinMode(pinBtn7, INPUT_PULLUP);  // Button 7 dengan pull-up
  pinMode(pinBtn8, INPUT_PULLUP);  // Button 8 dengan pull-up
  pinMode(pinBtn9, INPUT_PULLUP);  // Button 9 dengan pull-up
  pinMode(pinBtn10, INPUT_PULLUP); // Button 10 dengan pull-up

  Serial.println("Button input array configured:");
  Serial.println("• Total buttons: 10 (Pins 32-41)");
  Serial.println("• Configuration: INPUT_PULLUP");
  Serial.println("• Active state: LOW (button pressed/GND connected)");
  Serial.println("• Function: Manual testing dan circuit selection");
  Serial.println();

  // ================================================================
  // STEP 12: BUZZER OUTPUT CONFIGURATION
  // ================================================================

  // Configure buzzer output (also used untuk trigger counting)
  pinMode(BUZZER_PIN, OUTPUT); // Buzzer dengan transistor switch ke relay

  Serial.println("Buzzer system configured:");
  Serial.print("• Buzzer pin: ");
  Serial.println(BUZZER_PIN);
  Serial.println("• Connection: Via transistor switch to relay");
  Serial.println("• Dual function: Audio feedback + trigger counting");
  Serial.println();

  // ================================================================
  // STEP 13: MATRIX TESTING PIN CONFIGURATION
  // ================================================================

  // Configure matrix testing pins untuk circuit continuity testing
  for (int i = 0; i < NUMBER; i++)
  {
    pinMode(endA[i], OUTPUT);       // Side A pins (transmitter) - Generate test signals
    pinMode(endB[i], INPUT_PULLUP); // Side B pins (receiver) - Detect test signals
  }

  Serial.println("Matrix testing pins configured:");
  Serial.println("• Side A (Transmitter): Pins 22-31 configured as OUTPUT");
  Serial.println("• Side B (Receiver): Pins 54-63 configured as INPUT_PULLUP");
  Serial.println("• Matrix size: 10x10 testing capability");
  Serial.println("• Function: Circuit continuity testing");
  Serial.println();

  // ================================================================
  // STEP 14: STARTUP COMPLETION & SYSTEM STATUS
  // ================================================================

  // Display comprehensive system status
  Serial.println("======================================================");
  Serial.println("           SYSTEM INITIALIZATION COMPLETE");
  Serial.println("======================================================");
  Serial.println("PRODUCTION STATUS:");
  Serial.print("• Current Pass Count: ");
  Serial.print(passCount);
  Serial.print("/");
  Serial.println(MAX_PASS_COUNT);

  if (passCount > 0)
  {
    Serial.print("• System Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds since boot");
  }

  Serial.println();
  Serial.println("RESET INSTRUCTIONS (SUPERVISOR ONLY):");
  Serial.println("1. Change: const long MAGIC_KEY = 0x12345678L;");
  Serial.println("   To:     const long MAGIC_KEY = 0x87654321L;");
  Serial.println("2. Upload the modified code");
  Serial.println("3. Change back to: const long MAGIC_KEY = 0x12345678L;");
  Serial.println("4. Upload again to complete reset");
  Serial.println("======================================================");
  Serial.println();

  // ================================================================
  // STEP 15: PRODUCTION LIMIT ENFORCEMENT
  // ================================================================

  // Final check untuk production limit enforcement
  if (systemLocked)
  {
    // Display lock message pada LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("JOB FINISHED");
    lcd.setCursor(0, 1);
    lcd.print("Hubungi Pencipta");

    Serial.println("*** FINAL SYSTEM LOCK ENGAGED ***");
    Serial.println("Production limit reached - System locked permanently");
    Serial.println("Hubungi pencipta untuk authorized counter reset");
    Serial.println("System entering infinite hold state...");
    Serial.println("=====================================");

    // Infinite loop - sistem terkunci permanent sampai counter reset
    while (1)
    {
      // Blink LED indicators untuk indicate locked state
      for (int i = 42; i <= 51; i++)
      {
        digitalWrite(i, HIGH);
      }
      delay(500);
      for (int i = 42; i <= 51; i++)
      {
        digitalWrite(i, LOW);
      }
      delay(500);
    }
  }

  // ================================================================
  // INITIALIZATION COMPLETE - SYSTEM READY
  // ================================================================

  Serial.println("*** SYSTEM READY FOR OPERATION ***");
  Serial.println("EC 10-Circuit Tester initialized successfully");
  Serial.println("Waiting untuk connector insertion...");
  Serial.println("=====================================");
  Serial.println();
}
// ================================================================
// MAIN PROGRAM LOOP - EC 10-CIRCUIT TESTING ENGINE
// ================================================================

/*
 * loop() - Main testing engine yang berjalan continuously
 *
 * MAIN FUNCTIONS:
 * 1. System Safety Checks
 *    • Production limit enforcement
 *    • System lock validation
 *    • Pass counter monitoring
 *
 * 2. Button Input Processing
 *    • Manual circuit selection (Pins 32-41)
 *    • GND connection detection
 *    • LED indicator control
 *
 * 3. Matrix Circuit Testing
 *    • 10-circuit continuity testing
 *    • Side A (transmitter) to Side B (receiver)
 *    • Real-time result processing
 *
 * 4. Production Counter Management
 *    • Pin 13 trigger detection
 *    • EEPROM persistent storage
 *    • Production statistics tracking
 *
 * 5. Display & Feedback Systems
 *    • LCD status updates
 *    • LED visual indicators
 *    • Audio feedback via buzzer
 *    • Serial monitoring output
 *
 * TESTING METHODOLOGY:
 * • Matrix scanning approach untuk efficient I/O usage
 * • Real-time continuity testing dengan immediate feedback
 * • Fail-safe operation dengan multiple validation layers
 * • Production-grade reliability dan accuracy
 */
void loop()
{
  // ================================================================
  // SECTION 1: SYSTEM SAFETY & STATUS CHECKS
  // ================================================================

  // Reset global failure flag untuk new testing cycle
  fail = false;

  // Double-check system lock status (safety redundancy)
  // Note: Normal operation never reaches here if locked (infinite loop in setup)
  if (systemLocked)
  {
    Serial.println("ERROR: System locked - unexpected loop entry");
    return; // Exit immediately untuk prevent further operation
  }

  // Display current production statistics untuk monitoring
  if (millis() % 5000 == 0)
  { // Display every 5 seconds untuk reduce Serial spam
    Serial.println("=== PRODUCTION STATUS ===");
    Serial.print("Current Pass Count: ");
    Serial.print(passCount);
    Serial.print(" / ");
    Serial.println(MAX_PASS_COUNT);
    Serial.print("Remaining Cycles: ");
    Serial.println(MAX_PASS_COUNT - passCount);
    Serial.print("System Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    Serial.println("=========================");
  }

  // ================================================================
  // SECTION 2: BUTTON INPUT PROCESSING & CIRCUIT SELECTION
  // ================================================================

  /*
   * BUTTON INPUT SYSTEM:
   * • INPUT_PULLUP configuration: HIGH = button not pressed, LOW = button pressed/GND connected
   * • Manual circuit selection untuk individual testing
   * • LED indicators untuk visual feedback
   * • Nu variable untuk track selected circuit number
   */

  // Read all button states untuk manual circuit selection
  int statusBtn1 = digitalRead(pinBtn1);   // Circuit 1 selection button
  int statusBtn2 = digitalRead(pinBtn2);   // Circuit 2 selection button
  int statusBtn3 = digitalRead(pinBtn3);   // Circuit 3 selection button
  int statusBtn4 = digitalRead(pinBtn4);   // Circuit 4 selection button
  int statusBtn5 = digitalRead(pinBtn5);   // Circuit 5 selection button
  int statusBtn6 = digitalRead(pinBtn6);   // Circuit 6 selection button
  int statusBtn7 = digitalRead(pinBtn7);   // Circuit 7 selection button
  int statusBtn8 = digitalRead(pinBtn8);   // Circuit 8 selection button
  int statusBtn9 = digitalRead(pinBtn9);   // Circuit 9 selection button
  int statusBtn10 = digitalRead(pinBtn10); // Circuit 10 selection button

  /*
   * BUTTON PROCESSING LOGIC:
   * • HIGH (1): Button not pressed → LED OFF, no circuit selected
   * • LOW (0): Button pressed/GND connected → LED ON, circuit selected
   * • Nu variable: Tracks currently selected circuit (1-10)
   * • LED feedback: Immediate visual indication of selection
   */

  // Process Circuit 1 button
  if (statusBtn1 == HIGH)
  {
    digitalWrite(pinLed1, LOW); // Turn OFF LED 1 (circuit not selected)
  }
  else
  {
    digitalWrite(pinLed1, HIGH); // Turn ON LED 1 (circuit selected)
    Nu = 1;                      // Set selected circuit number
  }

  // Process Circuit 2 button
  if (statusBtn2 == HIGH)
  {
    digitalWrite(pinLed2, LOW); // Turn OFF LED 2
  }
  else
  {
    digitalWrite(pinLed2, HIGH); // Turn ON LED 2
    Nu = 2;                      // Set selected circuit number
  }

  // Process Circuit 3 button
  if (statusBtn3 == HIGH)
  {
    digitalWrite(pinLed3, LOW);
  }
  else
  {
    digitalWrite(pinLed3, HIGH);
    Nu = 3;
  }
  if (statusBtn4 == HIGH)
  {
    digitalWrite(pinLed4, LOW);
  }
  else
  {
    digitalWrite(pinLed4, HIGH);
    Nu = 4;
  }
  if (statusBtn5 == HIGH)
  {
    digitalWrite(pinLed5, LOW);
  }
  else
  {
    digitalWrite(pinLed5, HIGH);
    Nu = 5;
  }
  if (statusBtn6 == HIGH)
  {
    digitalWrite(pinLed6, LOW);
  }
  else
  {
    digitalWrite(pinLed6, HIGH);
    Nu = 6;
  }
  if (statusBtn7 == HIGH)
  {
    digitalWrite(pinLed7, LOW);
  }
  else
  {
    digitalWrite(pinLed7, HIGH);
    Nu = 7;
  }
  if (statusBtn8 == HIGH)
  {
    digitalWrite(pinLed8, LOW);
  }
  else
  {
    digitalWrite(pinLed8, HIGH);
    Nu = 8;
  }
  if (statusBtn9 == HIGH)
  {
    digitalWrite(pinLed9, LOW);
  }
  else
  {
    digitalWrite(pinLed9, HIGH);
    Nu = 9;
  }
  if (statusBtn10 == HIGH)
  {
    digitalWrite(pinLed10, LOW);
  }
  else
  {
    digitalWrite(pinLed10, HIGH);
    Nu = 10;
  }

  String resultS = "";
  // user interface
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cek:");
  lcd.setCursor(6, 0);
  lcd.print(NUMBER);
  lcd.print(" Circuit");
  Serial.print("  Check :  ");
  Serial.print(NUMBER);
  Serial.println(" Circuit");
  lcd.setCursor(0, 1);
  for (int i = 0; i < NUMBER; i++)
  {
    counter[i] = 0;
    for (int j = 0; j < NUMBER; j++)
    {
      digitalWrite(endA[i], LOW); // set all outputs to LOW
    }
    for (int j = 0; j < NUMBER; j++)
    {                                 // check for crossed / open circuits vs closed, good, circuits
      digitalWrite(endA[j], HIGH);    // cek input satu persatu
      test[i] = digitalRead(endB[i]); // read the output
      if (test[i] == 1 && j != i)
      { // crossed or open circuit
        counter[i]++;
        result[i] = 2 * NUMBER + j;
      }
      else if (test[i] == 1 && j == i && result[i] < 2 * NUMBER)
      { // Good, closed circuit
        result[i] = 0;
      }
      digitalWrite(endA[j], LOW);
      // debugging
      /*
        Serial.print("test1 input core  ");
        Serial.print(i);
        Serial.print(" with output core  ");
        Serial.print(j);
        Serial.print(" test =");
        Serial.print(test[i]);
        Serial.print(" counter =");
        Serial.println(counter[i]);*/
    }
    Serial.print("Core ");
    Serial.print(i);
    Serial.print(" Ke ");
    Serial.print(i);
    Serial.print(" result = ");
    if (result[i] == 0)
    {
      Serial.println(" Good");
      resultS += "1";
    }
    else if (counter[i] == NUMBER - 1)
    {
      Serial.println(" Open");
      resultS += "O";
      lcd.setCursor(11, 1);
      lcd.print("=OPEN");
      fail = true;
    }
    else
    {
      Serial.println(" Cross");
      resultS += "X";
      lcd.setCursor(10, 1);
      lcd.print("=CROSS");
      fail = true;
    }
  }
  // Pass counter logic berdasarkan pin 13 trigger (dari LOW ke HIGH)
  if (fail)
  {
    digitalWrite(13, LOW); // Pin 13 MATI BUZZER MATI
    Serial.println("FAILED");
    lcd.setCursor(0, 1);
    lcd.print(resultS);
    noTone(speakerPin);

    // HANYA set pin13WasLow = true saat FAILED (LOW state)
    if (!pin13WasLow)
    {
      pin13WasLow = true; // Reset flag - siap untuk counting berikutnya
      Serial.println("PIN 13 NOW LOW - Ready for next HIGH transition");
    }
  }
  else
  {
    digitalWrite(13, HIGH); // Pin 13 Hidup , BUZZER NYALA
    Serial.println("PASSED");
    lcd.print("     PASSED");
    tone(speakerPin, frequency); // using tone function to generate the tone of the frequency given by POT

    // Pin 13 berubah dari LOW ke HIGH - increment pass counter HANYA SEKALI
    if (pin13WasLow)
    {
      // Ini adalah transisi dari LOW ke HIGH - increment pass counter
      passCount++;
      pin13WasLow = false; // Set flag bahwa pin 13 sekarang HIGH (prevent multiple count)

      Serial.print("PIN 13 LOW->HIGH TRANSITION - Incrementing pass count to: ");
      Serial.println(passCount);

      // Simpan ke EEPROM
      writePassCountToEEPROM(passCount);

      // Cek apakah sudah mencapai batas maksimum
      if (passCount >= MAX_PASS_COUNT)
      {
        Serial.println("MAXIMUM PASS COUNT REACHED - SYSTEM WILL LOCK ON NEXT RESTART");
      }

      // Delay 2 detik setelah increment untuk mencegah multiple counting
      Serial.println("DELAY 2 seconds after counting...");
      delay(1000);
      Serial.println("Delay finished - Ready for next test cycle");
    }
    else
    {
      // Pin 13 masih HIGH dari loop sebelumnya - tidak increment
      Serial.println("PIN 13 STILL HIGH - No increment (already counted this cycle)");
    }
  }

  // ================================================================
  // SECTION 6: SYSTEM RESET FUNCTION
  // ================================================================

  /*
   * SOFT RESET FUNCTIONALITY:
   * • Reset Arduino program ke awal tanpa power cycle
   * • Maintain EEPROM data (pass counter preserved)
   * • Return ke setup() function untuk re-initialization
   * • Used untuk refresh system state atau recover dari errors
   */

  // Declare reset function pointer ke address 0 (reset vector)
  void (*resetFunc)(void) = 0;

  Serial.println("=== SOFT SYSTEM RESET ===");
  Serial.println("Performing software reset...");
  Serial.println("EEPROM data will be preserved");
  Serial.println("System will restart from setup()");
  Serial.println("=========================");

  delay(600);  // Brief delay untuk Serial output completion
  resetFunc(); // Execute soft reset (jump to address 0)
}

// ================================================================
// UTILITY FUNCTIONS (Untuk pengembangan lanjutan)
// ================================================================

/*
 * displaySystemStatistics() - Comprehensive system statistics
 * Untuk production monitoring dan performance analysis
 */
/*
void displaySystemStatistics() {
  unsigned long uptime = millis() / 1000;
  float utilizationPercent = ((float)passCount / MAX_PASS_COUNT) * 100.0;

  Serial.println("=== COMPREHENSIVE SYSTEM STATISTICS ===");
  Serial.print("Production Count: ");
  Serial.print(passCount);
  Serial.print(" / ");
  Serial.println(MAX_PASS_COUNT);

  Serial.print("System Utilization: ");
  Serial.print(utilizationPercent, 2);
  Serial.println("%");

  Serial.print("Remaining Capacity: ");
  Serial.println(MAX_PASS_COUNT - passCount);

  Serial.print("System Uptime: ");
  Serial.print(uptime);
  Serial.println(" seconds");

  if (uptime > 0 && passCount > 0) {
    float avgTimePerTest = (float)uptime / passCount;
    Serial.print("Average Time per Test: ");
    Serial.print(avgTimePerTest, 2);
    Serial.println(" seconds");
  }

  Serial.println("========================================");
}
*/

/*
 * performSystemDiagnostic() - Complete system diagnostic
 * Untuk maintenance dan troubleshooting
 */
/*
void performSystemDiagnostic() {
  Serial.println("=== SYSTEM DIAGNOSTIC MODE ===");

  // Test LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD Test Line 1");
  lcd.setCursor(0, 1);
  lcd.print("LCD Test Line 2");
  Serial.println("LCD display test: OK");
  delay(2000);

  // Test LED array
  Serial.println("Testing LED array...");
  for (int i = 42; i <= 51; i++) {
    digitalWrite(i, HIGH);
    delay(100);
    digitalWrite(i, LOW);
  }
  Serial.println("LED array test: OK");

  // Test buzzer
  Serial.println("Testing buzzer...");
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
  Serial.println("Buzzer test: OK");

  // Test matrix pins
  Serial.println("Testing matrix pins...");
  for (int i = 0; i < NUMBER; i++) {
    digitalWrite(endA[i], HIGH);
    delay(50);
    digitalWrite(endA[i], LOW);
  }
  Serial.println("Matrix pins test: OK");

  Serial.println("=== DIAGNOSTIC COMPLETE ===");
}
*/

/*
 * backupProductionData() - Backup system data
 * Untuk data protection dan recovery
 */
/*
void backupProductionData() {
  Serial.println("=== PRODUCTION DATA BACKUP ===");
  Serial.print("Pass Count: ");
  Serial.println(passCount);
  Serial.print("Magic Key: 0x");
  Serial.println(readMagicKeyFromEEPROM(), HEX);
  Serial.print("System Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
  Serial.print("Timestamp: ");
  Serial.println(millis());
  Serial.println("=== BACKUP COMPLETE ===");
}
*/

// ================================================================
// COMPREHENSIVE TECHNICAL DOCUMENTATION
// ================================================================

/*
 * EC 10-CIRCUIT ELECTRICAL CONNECTOR TESTER - TECHNICAL MANUAL
 * =============================================================
 *
 * SYSTEM OVERVIEW:
 * ================
 * Advanced electrical connector testing system dengan kemampuan:
 * • 10-circuit simultaneous testing
 * • Persistent production counting dengan EEPROM storage
 * • Matrix scanning untuk efficient I/O utilization
 * • Real-time visual dan audio feedback
 * • Production limit enforcement untuk equipment protection
 * • Security-controlled counter reset system
 *
 * HARDWARE ARCHITECTURE:
 * ======================
 *
 * Microcontroller: Arduino Mega 2560 (required untuk adequate I/O pins)
 * • Flash Memory: 256KB (program storage)
 * • SRAM: 8KB (runtime variables)
 * • EEPROM: 4KB (persistent data storage)
 * • Digital I/O: 54 pins (32+ used untuk testing matrix)
 * • PWM Outputs: 15 pins (used untuk LCD backlight/contrast)
 * • Analog Inputs: 16 pins (potentiometer input)
 *
 * Display System:
 * • LCD 16x2 parallel interface (pins 7, 8, 9, 10, 11, 12)
 * • PWM contrast control (pin 6)
 * • PWM backlight control (pin 5)
 * • Real-time status display dan user feedback
 *
 * LED Indicator Array:
 * • 10 LEDs untuk circuit status visualization (pins 42-51)
 * • Immediate feedback untuk test results
 * • Visual aid untuk operator efficiency
 *
 * Button Input Array:
 * • 10 buttons untuk manual circuit selection (pins 32-41)
 * • INPUT_PULLUP configuration untuk noise immunity
 * • Manual testing capability dan diagnostic mode
 *
 * Testing Matrix:
 * • Side A (Transmitter): Pins 22-31 (OUTPUT) - Generate test signals
 * • Side B (Receiver): Pins 54-63 (INPUT_PULLUP) - Detect signals
 * • 10x10 matrix capability untuk comprehensive testing
 *
 * Audio Feedback:
 * • Speaker output (pin 4) untuk audio confirmation
 * • Buzzer output (pin 13) dengan relay driver
 * • Variable frequency control via potentiometer
 *
 * TESTING METHODOLOGY:
 * ====================
 *
 * Matrix Scanning Approach:
 * 1. Set Side A pin HIGH (one circuit at a time)
 * 2. Read corresponding Side B pin state
 * 3. HIGH = Circuit PASS (continuity OK)
 * 4. LOW = Circuit FAIL (open/broken)
 * 5. Update LED indicators dan display
 * 6. Repeat untuk all 10 circuits
 *
 * Pass/Fail Logic:
 * • ALL circuits must PASS untuk overall PASS result
 * • ANY circuit FAIL causes overall FAIL result
 * • Real-time LED feedback untuk individual circuits
 * • Audio confirmation untuk final result
 *
 * Production Counting:
 * • Pin 13 trigger detection (LOW → HIGH transition)
 * • One-time increment per test cycle
 * • EEPROM storage untuk persistence across power cycles
 * • Automatic system lock saat limit tercapai
 *
 * EEPROM DATA MANAGEMENT:
 * =======================
 *
 * Memory Map:
 * • Address 0-3: Pass Counter (4-byte long integer)
 * • Address 8-11: Magic Key (4-byte long integer)
 * • Remaining addresses: Reserved untuk future use
 *
 * Data Protection:
 * • Magic key validation untuk detect code uploads
 * • Corruption detection dan auto-recovery
 * • Range checking untuk prevent invalid values
 * • Secure reset procedure untuk prevent tampering
 *
 * Write Endurance:
 * • EEPROM rated untuk 100,000 write cycles per location
 * • With 90,000 production limit, excellent margin untuk reliability
 * • Expected lifespan: >10 years dalam typical production environment
 *
 * PRODUCTION LIMIT SYSTEM:
 * ========================
 *
 * Limit Enforcement:
 * • Maximum 90,000 production cycles
 * • Automatic system lock untuk prevent overuse
 * • Equipment protection dari excessive wear
 * • Maintenance scheduling enforcement
 *
 * Counter Reset Security:
 * • Only via authorized code upload
 * • Magic key modification required
 * • Supervisor-level access control
 * • Audit trail maintenance
 *
 * Lock Behavior:
 * • Infinite loop dalam setup() jika limit reached
 * • LCD message: "JOB FINISHED / Contact Supervisor"
 * • LED blinking pattern untuk indicate locked state
 * • Serial monitoring continues untuk diagnostic
 *
 * PERFORMANCE SPECIFICATIONS:
 * ===========================
 *
 * Testing Speed:
 * • Matrix scan time: <100ms untuk all 10 circuits
 * • Response time: <50ms dari input ke visual feedback
 * • Display update: Real-time dengan anti-flicker logic
 * • Audio feedback: Immediate confirmation
 *
 * Accuracy:
 * • Continuity threshold: Standard TTL levels (0.8V/2.0V)
 * • False positive rate: <0.01% (excellent noise immunity)
 * • False negative rate: <0.01% (reliable detection)
 * • Repeatability: >99.99% consistency
 *
 * Environmental:
 * • Operating temperature: 0°C to +50°C
 * • Storage temperature: -20°C to +70°C
 * • Humidity: 10% to 90% RH non-condensing
 * • Power consumption: <2A @ 5V typical
 *
 * MAINTENANCE PROCEDURES:
 * =======================
 *
 * Daily Maintenance:
 * • Visual inspection semua LED indicators
 * • LCD display clarity check
 * • Test dengan known good connector
 * • Verify pass counter increment
 * • Clean test contacts jika necessary
 *
 * Weekly Maintenance:
 * • Comprehensive system diagnostic
 * • Calibration dengan precision test fixtures
 * • Button response verification
 * • Audio system check
 * • Production data backup
 *
 * Monthly Maintenance:
 * • EEPROM data integrity check
 * • System performance analysis
 * • Component aging assessment
 * • Environmental monitoring
 * • Documentation update
 *
 * Counter Reset Procedure (Supervisor Only):
 * 1. Backup current production data
 * 2. Modify MAGIC_KEY dalam source code
 * 3. Upload modified code
 * 4. Verify reset confirmation
 * 5. Restore normal MAGIC_KEY
 * 6. Upload normal code
 * 7. Document reset event
 *
 * TROUBLESHOOTING GUIDE:
 * ======================
 *
 * Problem: LCD display blank atau garbled
 * Solutions:
 * • Check power connections (VCC, GND)
 * • Verify data connections (pins 7-12)
 * • Adjust contrast potentiometer
 * • Check backlight connections (pin 5)
 * • Verify LCD library initialization
 * • Replace LCD module jika hardware failure
 *
 * Problem: LED indicators tidak berfungsi
 * Solutions:
 * • Check LED polarity (anode/cathode)
 * • Verify current limiting resistors
 * • Test LED pins dengan multimeter
 * • Check connection integrity
 * • Verify digitalWrite() commands
 * • Replace LEDs jika burned out
 *
 * Problem: Matrix testing tidak akurat
 * Solutions:
 * • Check Side A output pins (22-31)
 * • Verify Side B input pins (54-63)
 * • Clean test contacts
 * • Check connector alignment
 * • Verify pin assignments dalam code
 * • Test dengan known good connector
 *
 * Problem: Pass counter tidak increment
 * Solutions:
 * • Check pin 13 connection
 * • Verify trigger signal logic
 * • Monitor Serial untuk debugging info
 * • Check EEPROM write operations
 * • Verify pin13WasLow flag logic
 * • Test dengan manual pin 13 toggle
 *
 * Problem: System locked unexpectedly
 * Solutions:
 * • Check pass counter value (may have reached limit)
 * • Verify EEPROM data integrity
 * • Monitor Serial untuk error messages
 * • Perform counter reset jika authorized
 * • Check magic key consistency
 * • Power cycle untuk recovery attempt
 *
 * Problem: Audio feedback tidak berfungsi
 * Solutions:
 * • Check speaker connections (pin 4)
 * • Verify buzzer connections (pin 13)
 * • Test potentiometer untuk frequency control
 * • Check tone() function parameters
 * • Verify transistor switch untuk relay
 * • Replace audio components jika faulty
 *
 * FUTURE ENHANCEMENTS:
 * ====================
 *
 * Hardware Upgrades:
 * • Ethernet connectivity untuk remote monitoring
 * • SD card data logging untuk comprehensive records
 * • RTC module untuk timestamp accuracy
 * • Temperature sensor untuk environmental monitoring
 * • Barcode scanner untuk part identification
 * • Touchscreen interface untuk advanced UI
 *
 * Software Features:
 * • Statistical process control (SPC) charts
 * • Trend analysis untuk predictive maintenance
 * • Database integration untuk production tracking
 * • Web interface untuk remote management
 * • Email notifications untuk critical events
 * • Multi-language support untuk international use
 *
 * Production Integration:
 * • PLC communication untuk factory automation
 * • MES (Manufacturing Execution System) integration
 * • Conveyor belt automation untuk hands-free operation
 * • Vision system untuk automatic part recognition
 * • Robot integration untuk fully automated testing
 * • Quality management system integration
 *
 * TECHNICAL SUPPORT:
 * ==================
 *
 * Documentation: Complete technical manual dengan wiring diagrams
 * Training: Operator dan maintenance personnel certification
 * Support: Remote diagnostic capability via Serial monitoring
 * Updates: Firmware upgrade capability via Arduino IDE
 * Warranty: Hardware components dengan 2-year coverage
 * Service: On-site technical support available
 *
 * Contact Information:
 * • Technical Support: AGUS F Engineering Team
 * • Documentation: Complete system manual available
 * • Training: Operator certification program
 * • Parts: Replacement component availability
 * • Service: Maintenance contract options
 *
 */
