#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define RFID_SDA_PIN 6
#define RDIF_RST_PIN 9
#define RFID_SCK_PIN 13
#define BUZZER_PIN 8
#define LED_GREEN_PIN 5
#define LED_RED_PIN 7
#define DHT_PIN 2 
#define DHT_TYPE DHT11 

bool open_system;

MFRC522 mfrc522(RFID_SDA_PIN, RDIF_RST_PIN);  // Create MFRC522 instance.
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
DHT dht(DHT_PIN, DHT_TYPE);

bool card_detection(){
  return mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
}

bool is_desired_tag(String tag){
  return tag == "13957C1A";
}

String get_card_id(){
  Serial.print("UID tag :");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase(); // Convert content to uppercase
  return content;
}

void effect_desired_tag(){
  Serial.println("Authorized access!");
  lcd.setCursor(3, 0);
  lcd.print("Authorized");
  lcd.setCursor(4, 1);
  lcd.print("access!");

  tone(BUZZER_PIN, 1200);
  digitalWrite(LED_GREEN_PIN, HIGH);
  delay(300);
  noTone(BUZZER_PIN);
  delay(1000);
  digitalWrite(LED_GREEN_PIN, LOW);
  lcd.clear();
}

void effect_denied_tag(){
  Serial.println("Denied access!");
  lcd.setCursor(1, 0);
  lcd.print("Denied access!");

  tone(BUZZER_PIN, 800);
  digitalWrite(LED_RED_PIN, HIGH);
  delay(700);
  noTone(BUZZER_PIN);
  delay(1000);
  digitalWrite(LED_RED_PIN, LOW);
  lcd.clear();
}

void read_temperature_humidity(float *humidity, float *temperature){
  *humidity = dht.readHumidity();
  *temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(*humidity) || isnan(*temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print temperature and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(*humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(*temperature);
  Serial.println(" *C");
}

void setup() {
  Serial.begin(9600);  // Initialize serial communications with the PC
  SPI.begin();          // Init SPI bus
  SPI.setClockDivider(SPI_CLOCK_DIV4); // Set clock speed (optional)
  mfrc522.PCD_Init();   // Init MFRC522 card

  lcd.init();
  lcd.backlight();

  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);

  open_system = false;
}


void loop() {

  // Check for the presence of a new card
  if (card_detection()) {
    // Show UID on serial monitor
    String tag = get_card_id();
    // Check if the read RFID tag matches the desired tag
    if (is_desired_tag(tag)) {
      
      effect_desired_tag();

        if(card_detection()){
          open_system = false;

          if(is_desired_tag(get_card_id())){
              lcd.setCursor(5, 0);
              lcd.print("Have a");
              lcd.setCursor(3, 1);
              lcd.print("nice day!");
              delay(3000);
              lcd.clear();
          }
        }

      }

    } else {
      effect_denied_tag();
    }
    
  }
  // Delay before scanning for the next card
  delay(3000);