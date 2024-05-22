#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Servo.h>

#define RFID_SDA_PIN 6
#define RDIF_RST_PIN 9
#define RFID_SCK_PIN 13
#define BUZZER_PIN 8
#define LED_GREEN_PIN 5
#define LED_RED_PIN 7
#define DHT_PIN 2
#define DHT_TYPE DHT11
#define SERVO_PIN 9

bool open_system;

MFRC522 mfrc522(RFID_SDA_PIN, RDIF_RST_PIN); // Create MFRC522 instance.
LiquidCrystal_I2C lcd(0x27, 16, 2);          // Set the LCD address to 0x27 for a 16 chars and 2 line display
DHT dht(DHT_PIN, DHT_TYPE);
Servo servo;

unsigned long servoStartTime = 0;
unsigned long displayStartTime = 0;
const unsigned long servoInterval = 15; // Interval between servo movements in milliseconds
const unsigned long displayInterval = 750; // Interval between display updates in milliseconds

bool card_detection()
{
    return mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
}

bool is_desired_tag(String tag)
{
    return tag == "13957C1A";
}

String get_card_id()
{
    Serial.print("UID tag :");
    String content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase(); // Convert content to uppercase
    return content;
}

void effect_desired_tag()
{
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

void effect_denied_tag()
{
    Serial.println("Denied access!");
    lcd.clear();
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

void read_temperature_humidity(float *humidity, float *temperature)
{
    *humidity = dht.readHumidity();
    *temperature = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(*humidity) || isnan(*temperature))
    {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
}

void open_servo_motor()
{
    unsigned long currentTime = millis();
    if (currentTime - servoStartTime >= servoInterval)
    {
        static int angle = 0;
        if (angle <= 180)
        {
            servo.write(angle); // Write the angle to the servo
            angle++;
        }
        else
        {
            angle = 0;
        }
        servoStartTime = currentTime;
    }
}

void display_information_circular(String info)
{
    unsigned long currentTime = millis();
    if (currentTime - displayStartTime >= displayInterval)
    {
        static int index = 0;
        lcd.clear();
        int textLength = info.length();
        for (int j = 0; j < 16; j++)
        {
            lcd.setCursor(j, 0);
            lcd.print(info[(index + j) % textLength]);
        }
        index++;
        displayStartTime = currentTime;
    }
}

void setup()
{
    Serial.begin(9600);         // Initialize serial communications with the PC
    SPI.begin();                 // Init SPI bus
    SPI.setClockDivider(SPI_CLOCK_DIV4); // Set clock speed (optional)
    mfrc522.PCD_Init();          // Init MFRC522 card

    lcd.init();
    lcd.backlight();

    dht.begin();

    servo.attach(SERVO_PIN);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);

    open_system = false;
}

void loop()
{
    // Check for the presence of a new card
    if (card_detection())
    {
        // Show UID on serial monitor
        String tag = get_card_id();
        // Check if the read RFID tag matches the desired tag
        if (is_desired_tag(tag))
        {
            effect_desired_tag();

            open_system = true;
            while (open_system)
            {
                float humidity;
                float temperature;
                read_temperature_humidity(&humidity, &temperature);

                // Concatenate the strings
                String info = "  Humidity: " + String(humidity) + "%" + "  Temperature: " + String(temperature) + " C";

                if(temperature >= 30)
                  open_servo_motor();
                  
                display_information_circular(info);

                if (card_detection() || !open_system)
                {
                    open_system = false;
                    if (is_desired_tag(get_card_id()))
                    {
                        lcd.clear();
                        lcd.setCursor(5, 0);
                        lcd.print("Have a");
                        lcd.setCursor(3, 1);
                        lcd.print("nice day!");
                        delay(3000);
                        lcd.clear();
                    }
                }
            }
        }
        else
        {
            effect_denied_tag();
        }
    }
}