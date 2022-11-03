#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Scheduler.h>
#include <Task.h>

#define LENGTH(_Array) (sizeof(_Array) / sizeof(_Array[0]))

// Please set the pins inside each channel instatiation below
// These are defined as legacy for the hardcoded song
#define C0_PIN_STEP D5
#define C0_PIN_DIR D7

#define MAX_STEPS 100
#define NUM_BUTTONS 12

#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978
#define REST      1

typedef struct _note {
    bool state;
    int button_N;
    unsigned int channel;
} note;

note myNote;

// Underworld melody
int underworld_melody[] = {
    NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
    NOTE_AS3, NOTE_AS4, 0,
    0,
    NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
    NOTE_AS3, NOTE_AS4, 0,
    0,
    NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
    NOTE_DS3, NOTE_DS4, 0,
    0,
    NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
    NOTE_DS3, NOTE_DS4, 0,
    0, NOTE_DS4, NOTE_CS4, NOTE_D4,
    NOTE_CS4, NOTE_DS4,
    NOTE_DS4, NOTE_GS3,
    NOTE_G3, NOTE_CS4,
    NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
    NOTE_GS4, NOTE_DS4, NOTE_B3,
    NOTE_AS3, NOTE_A3, NOTE_GS3,
    0, 0, 0};
// Underwolrd tempo
int underworld_tempo[] = {
    12, 12, 12, 12,
    12, 12, 6,
    3,
    12, 12, 12, 12,
    12, 12, 6,
    3,
    12, 12, 12, 12,
    12, 12, 6,
    3,
    12, 12, 12, 12,
    12, 12, 6,
    6, 18, 18, 18,
    6, 6,
    6, 6,
    6, 6,
    18, 18, 18, 18, 18, 18,
    10, 10, 10,
    10, 10, 10,
    3, 3, 3};


uint16_t notes[NUM_BUTTONS] = {NOTE_C3,
                               NOTE_CS3,
                               NOTE_D3,
                               NOTE_DS3,
                               NOTE_E3,
                               NOTE_F3,
                               NOTE_FS3,
                               NOTE_G3,
                               NOTE_GS3,
                               NOTE_A3,
                               NOTE_AS3,
                               NOTE_B3};

void set_dir(int pin_dir, int dir) {
    digitalWrite(pin_dir, dir == 1 ? HIGH : LOW);
};

void step(int pin_step, int pin_dir) {
    static int count = 0;
    static int last_step = LOW;
    static int dir = 1;

    if (count >= MAX_STEPS || count < 0) {
        dir *= -1;
        set_dir(pin_dir, dir);
    }
    last_step = last_step ? LOW : HIGH;
    digitalWrite(pin_step, last_step);
    count += dir;
}

void floppy_tone(float freq) {
    int steps = freq;
    float period = 1000 / freq;  // milisegundos

    for (int i = 0; i < steps; i++) {
        step(C0_PIN_STEP, C0_PIN_DIR);
        delay(period);
    }
}

void floppy_tone_tempo(float freq, float duration) {
    float sleeptime;
    int cycles;
    sleeptime = ((1.0 / freq) * 1000);
    cycles = (duration / (sleeptime + 3));
    sleeptime = sleeptime * 1000;
    if ((duration > 0) and (freq >= 30) and (freq <= 500) and (cycles > 0)) {
        for (int c = 1; c <= cycles; c++) {
            step(C0_PIN_STEP, C0_PIN_DIR);
            delayMicroseconds(sleeptime);
            yield();
        }
    } else {
        delay(duration);
    }
}

void play_melody() {
    Serial.println(" 'Mario Theme'");
    int size = sizeof(underworld_melody) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {
        int noteDuration = 1000 / underworld_tempo[thisNote];
        Serial.println(String(underworld_melody[thisNote]) + " " + String(noteDuration));
        floppy_tone_tempo(underworld_melody[thisNote] * 0.3, noteDuration);
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
    }
}


void play_notes() {
    floppy_tone_tempo(NOTE_C3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_CS3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_D3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_DS3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_E3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_F3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_FS3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_G3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_GS3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_A3, 2000);
    delay(100);
    floppy_tone_tempo(NOTE_AS3, 2000);
    delay(100);
}

class ChannelPlayer : public Task {
 public:
    int pin_step;
    int pin_dir;
    note* current_note;
    ChannelPlayer(int init_pin_step, int init_pin_dir){
        pin_step = init_pin_step;
        pin_dir = init_pin_dir;
    };
 protected:
  void setup() {
    Serial.println("setup ran");
    current_note = (note*) malloc(sizeof(myNote));
    current_note->button_N = -1;
    current_note->channel = 0;
    current_note->state = 0;

    pinMode(pin_step, OUTPUT);
    pinMode(pin_dir, OUTPUT);
  }

  void loop() {
    int sleeptime = (((1000.0*1000.0) / notes[current_note->button_N]));
    if (current_note->state && false){
        Serial.println("loop N "+String(current_note->button_N));
        Serial.println("loop notes[button_N] "+String(notes[current_note->button_N]));
        Serial.println("loop sleep "+String(sleeptime));
        Serial.println("loop float "+String((1000.0 / notes[current_note->button_N])));
        Serial.println("loop addrN "+String((unsigned int)&current_note->button_N));
    }
    

    while (current_note->state) {
        step(pin_step, pin_dir);
        delay(sleeptime); // this should now be an async delay that yields to ESP8266Scheduler 
        yield(); // this should yield to the 8266 chip, allowing espnow to magically work
    }
  }

};

ChannelPlayer c0(D5, D7);

// we can't create the objects directly inside the array, or else all hell breaks loose 
ChannelPlayer* channels[] = {
    &c0
};

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    memcpy(&myNote, incomingData, sizeof(myNote));
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.print("State: ");
    Serial.println(myNote.state);
    Serial.print("Button_N: ");
    Serial.println(myNote.button_N);
    Serial.print("Channel: ");
    Serial.println(myNote.channel);
    Serial.println();

    // Sanity check
    if (myNote.channel >= LENGTH(channels)){
        Serial.println("INVALID CHANNEL!");
    }
    else {
        note* notePtr = channels[myNote.channel]->current_note;
        Serial.println("note addr "+String((unsigned int)notePtr));
        Serial.println("note state "+String((unsigned int)notePtr->state));
        Serial.println("note N "+String((unsigned int)notePtr->button_N));
        Serial.println("note addr N "+String((unsigned int)&notePtr->button_N));
        notePtr->button_N = myNote.button_N;
        notePtr->state = myNote.state;
        notePtr->channel = myNote.channel;

        //memcpy(&notePtr, &myNote, sizeof(myNote));
    }
    
}


void setup() {
    Serial.begin(115200);
    Serial.println("Start");
    WiFi.mode(WIFI_STA);
    // play_melody();
    Serial.println();
    Serial.print("Endereco MAC: ");
    Serial.println(WiFi.macAddress());
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    } else
        Serial.println("Tudo certo");

    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);

    for (unsigned int i = 0; i < LENGTH(channels); i++){
        Scheduler.start(channels[i]);
        Serial.println("Starting channel "+String(i));
    }
    Scheduler.begin();
}

void loop() {
    // _fplay_hz_key();
    // play_notes();
}
