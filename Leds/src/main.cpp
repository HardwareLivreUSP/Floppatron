#include <WiFi.h>
#include <Wire.h>
#include <FastLED.h>
#include <SSD1306Wire.h>
#include <esp_now.h>

#define LED_PIN     16
#define NUM_LEDS    500
#define NUM_BUTTONS 12

SSD1306Wire display(0x3c, 5, 4);
CRGB leds[NUM_LEDS];
uint8_t buttonState[NUM_BUTTONS];

// hsv colors up to 256
uint8_t buttonColors[NUM_BUTTONS][3] = {
  {  0, 255, 255}, // red
  { 32, 255, 255}, // orange
  { 64, 255, 255}, // yellow
  { 96, 255, 255}, // green
  {128, 255, 255}, // cyan
  {160, 255, 255}, // blue
  {192, 255, 255}, // purple
  {224, 255, 255}, // pink
  {  0,   0, 255}, // white
  {  0,   0, 128}, // grey
  {  0,   0,  64}, // dark grey
  {  0,   0,   0}  // black
};

typedef struct _note {
    bool state;
    int button_N;
    unsigned int channel;
} note;

note myNote;

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

    buttonState[myNote.button_N] = myNote.state;
} 

void setup() {
	// setup serial
	Serial.begin(115200);
	Serial.println("Serial ok");
	// setup display
	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 0, "Display ok");
	display.display();
	// setup leds
	FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
	// setup wifi
	WiFi.mode(WIFI_STA);
	Serial.print("MAC: ");
	Serial.println(WiFi.macAddress());
	// setup espnow to receive data
	if (esp_now_init() != ESP_OK) {
		Serial.println("Error initializing ESP-NOW");
		return;
	}
	esp_now_register_recv_cb(onDataRecv);

	Serial.println("Setup finished");
}

void loop() {
	// delay(10);
	for (int i = NUM_LEDS-1; i > 0; i--) {
		leds[i].r = leds[i-1].r;
		leds[i].g = leds[i-1].g;
		leds[i].b = leds[i-1].b;
	}
	// printa cores do leds[1]
	Serial.print("leds[1]: ");
	Serial.print(leds[1].r);
	Serial.print(" ");
	Serial.print(leds[1].g);
	Serial.print(" ");
	Serial.println(leds[1].b);

	// sum colors of all active buttons
	uint8_t h = 0, s = 0, v = 0;
	for (int i = 0; i < NUM_BUTTONS; i++) {
		if (buttonState[i] == 1) {
			Serial.print("Button ");
			Serial.print(i);
			Serial.print(" Color: ");
			Serial.print(buttonColors[i][0]);
			Serial.print(" ");
			Serial.print(buttonColors[i][1]);
			Serial.print(" ");
			Serial.println(buttonColors[i][2]);

			h += buttonColors[i][0];
			s += buttonColors[i][1];
			v += buttonColors[i][2];
		}
	}
	leds[0] = CHSV(h, s, v);
  	FastLED.show();
}
