#include <WiFi.h>
#include <Wire.h>
#include <FastLED.h>
#include <SSD1306Wire.h>
#include <esp_now.h>

#define LED_PIN     16
#define NUM_LEDS    20

SSD1306Wire display(0x3c, 5, 4);
CRGB leds[NUM_LEDS];

typedef struct _note {
    bool state;
    int button_N;
    unsigned int channel;
} note;

note myNote;

void streamColor(int a, int b, int c) {
	
}

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    Serial.print("Bytes received: ");
    memcpy(&myNote, incomingData, sizeof(myNote));
    Serial.println(len);
    Serial.print("State: ");
    Serial.println(myNote.state);
    Serial.print("Button_N: ");
    Serial.println(myNote.button_N);
    Serial.print("Channel: ");
    Serial.println(myNote.channel);
    Serial.println();

    display.clear();
    display.drawString(0, 0, "State: " + String(myNote.state));
    display.drawString(0, 10, "Button_N: " + String(myNote.button_N));
    display.drawString(0, 20, "Channel: " + String(myNote.channel));
    display.display();
} 

void setup() {
	// setup serial
	Serial.begin(115200);
	Serial.println("Hello World");
	// setup display
	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 0, "Hello World");
	display.display();
	// setup leds
	FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
	// setup wifi
	WiFi.mode(WIFI_STA);
	Serial.println(WiFi.macAddress());
	// setup espnow to receive data
	if (esp_now_init() != ESP_OK) {
		Serial.println("Error initializing ESP-NOW");
		return;
	}
	esp_now_register_recv_cb(onDataRecv);

	Serial.println("Setup done");
}

void loop() {
	delay(200);
	for (int i = NUM_LEDS-1; i > 0; i--) {
		swap(leds[i], leds[i-1]);
	}
	leds[0] = CRGB(255, 0, 0);
  	FastLED.show();
}
