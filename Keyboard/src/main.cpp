#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Bounce2.h>
#include <vector>
#include <algorithm>
#define NUM_BUTTONS 12
#define NUM_DEVICES 2

const uint8_t BUTTON_PINS[NUM_BUTTONS] = {32, 5, 33, 18, 25, 26, 23, 27, 19, 14, 22, 12};
Bounce * buttons = new Bounce[NUM_BUTTONS];

int button_channels[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

uint8_t broadcastAddress[NUM_DEVICES][6] = {{0xC8, 0x2B, 0x96, 0x30, 0x4F, 0xCE}, {0x3C, 0x71, 0xBF, 0xC2, 0xB5, 0xFC}};

bool reverseComp (int i,int j) { return (i>j); }


typedef struct _note {
  bool state;
  int button_N;
  unsigned int channel;
} note;

note myNote;

int playing_channels = 0;

std::vector<int> channel_stack;

esp_now_peer_info_t peerInfo[NUM_DEVICES];

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void sendNote (int button_n, bool state) {
  myNote.button_N = button_n;
  myNote.state = state;
  if (state){
    std::sort(channel_stack.begin(), channel_stack.end(), reverseComp);
    button_channels[button_n] = channel_stack.back();
    channel_stack.pop_back();
    myNote.channel = button_channels[button_n];
  }
  else{
    myNote.channel = button_channels[button_n];
    button_channels[button_n] = -1;
    channel_stack.push_back(myNote.channel);
  }
  Serial.println("State: " + String(state) + " button_n: "+ String(button_n) + " channel: "+ String(myNote.channel));
  for (int i = 0; i < NUM_DEVICES; i++) {
	note copy = myNote;
  	esp_err_t result = esp_now_send(broadcastAddress[i], (uint8_t *) &copy, sizeof(copy));
  	if (result == ESP_OK) Serial.println("Sent with success");
  	else Serial.println("Error sending the data");
  }
}


void instanceButtons() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP);       //setup the bounce instance for the current button
    buttons[i].interval(25);                               // interval in ms
  }
}

void readButtons() {
 for (int i = 0; i < NUM_BUTTONS; i++)  {
    buttons[i].update();
    if ( buttons[i].fell() ) sendNote (i, true);
    else if (buttons[i].rose()) sendNote (i, false);
    // Serial.print(String(buttons[i].read()) + " ");
  }
  // Serial.println();
;}



void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("Start 32");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else Serial.println("Initializing ESP-NOW, ok");

  esp_now_register_send_cb(OnDataSent);
  
  for (int i = 0; i < NUM_DEVICES; i++) { 
  	memcpy(peerInfo[i].peer_addr, broadcastAddress[i], 6);
  	peerInfo[i].channel = 0;  
  	peerInfo[i].encrypt = false;
  	if (esp_now_add_peer(&peerInfo[i]) != ESP_OK){
    		Serial.println("Failed to add peer");
    		return;
  	}
  	else Serial.println("Added peer with success");
  }
  for (int i = 12; i>=0; i--){
	channel_stack.push_back(i);
    	std::sort(channel_stack.begin(), channel_stack.end(), reverseComp);
  }

  instanceButtons();
}


 
void loop() {
  readButtons();
}
