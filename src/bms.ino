#include <PinChangeInterrupt.h>
#include <PinChangeInterruptBoards.h>
#include <PinChangeInterruptPins.h>
#include <PinChangeInterruptSettings.h>

#define CONTACTOR_PIN 8
#define CHARGER_PIN 9
#define BMS_PIN 7
#define KEY_PIN 6
#define AC_PIN 5

typedef enum {
  STANDBY,
  DISCHARGING,
  EMPTY,
  CHARGING,
} bms_state_t;

typedef enum {
  BMS_OPENED,
  BMS_CLOSED,
  AC_UNPLUGGED,
  AC_PLUGIN,
  KEY_ON,
  KEY_OFF,
} bms_event_t;

bms_state_t state;
bms_state_t nextState;

void setup() {
  // set up IO pin modes
  pinMode(CONTACTOR_PIN, OUTPUT);   // First and second relay from the left
  pinMode(CHARGER_PIN, OUTPUT);

  pinMode(AC_PIN, INPUT_PULLUP);     // Digital inputs 6, 5, 4 on relay board
  pinMode(KEY_PIN, INPUT_PULLUP);
  pinMode(BMS_PIN, INPUT_PULLUP);

  // set up interrupts
  attachPCINT(digitalPinToPCINT(BMS_PIN), bmsChange, CHANGE);
  attachPCINT(digitalPinToPCINT(AC_PIN), acChange, CHANGE);
  attachPCINT(digitalPinToPCINT(KEY_PIN), keyChange, CHANGE);

  // set up initial state
  state = DISCHARGING;
  nextState = state;

  Serial.begin(115200);
  Serial.write("ready");
}

void loop()
{
  // set outputs for each state
  switch(state) {
    case STANDBY:
      Serial.println("STANDBY");
      digitalWrite(CONTACTOR_PIN, LOW);
      digitalWrite(CHARGER_PIN, LOW);
      break;
    case DISCHARGING:
      Serial.println("DISCHARGING");
      digitalWrite(CONTACTOR_PIN, HIGH);
      digitalWrite(CHARGER_PIN, LOW);
      break;
    case EMPTY:
      Serial.println("EMPTY");
      digitalWrite(CONTACTOR_PIN, LOW);
      digitalWrite(CHARGER_PIN, LOW);
      break;
    case CHARGING:
      Serial.println("CHARGING");
      digitalWrite(CONTACTOR_PIN, LOW);
      digitalWrite(CHARGER_PIN, HIGH);
      break;
    default: // Unknown state, something is wrong, shut everything down
      digitalWrite(CONTACTOR_PIN, LOW);
      digitalWrite(CHARGER_PIN, LOW);
      break;
  }

  state = nextState;
  delay(1000);  // delay so we don't clatter the contactor
}

void handleBMSEvent(uint8_t event) {
  switch(state) {
    case DISCHARGING:
      switch (event) {
        case BMS_OPENED:
          nextState = EMPTY;
          break;
        case BMS_CLOSED:
          nextState = DISCHARGING;
          break;
        case AC_UNPLUGGED:
          nextState = DISCHARGING;
          break;
        case AC_PLUGIN:
          nextState = CHARGING;
          break;
        case KEY_ON:
          nextState = DISCHARGING;
          break;
        case KEY_OFF:
          nextState = STANDBY;
          break;
      }
      break;
    case EMPTY:
      switch (event) {
        case BMS_OPENED:
        nextState = EMPTY;
          break;
        case BMS_CLOSED:
          nextState = STANDBY;
          break;
        case AC_UNPLUGGED:
          nextState = digitalRead(BMS_PIN) == LOW ? STANDBY : EMPTY;
          break;
        case AC_PLUGIN:
          nextState = CHARGING;
          break;
        case KEY_ON:
          nextState = EMPTY;
          break;
        case KEY_OFF:
          nextState = EMPTY;
          break;
      }
      break;
    case CHARGING:
      switch (event) {
        case BMS_OPENED:
          nextState = STANDBY;
          break;
        case BMS_CLOSED:
          nextState = CHARGING;
          break;
        case AC_UNPLUGGED:
          nextState = digitalRead(BMS_PIN) == LOW ? STANDBY : EMPTY;
          break;
        case AC_PLUGIN:
          nextState = CHARGING;
          break;
        case KEY_ON:
          nextState = CHARGING;
          break;
        case KEY_OFF:
          nextState = CHARGING;
          break;
      }
      break;
    case STANDBY:
      switch (event) {
        case BMS_OPENED:
          nextState = EMPTY;
          break;
        case BMS_CLOSED:
          nextState = STANDBY;
          break;
        case AC_UNPLUGGED:
          nextState = STANDBY;
          break;
        case AC_PLUGIN:
          nextState = CHARGING;
          break;
        case KEY_ON:
          nextState = DISCHARGING;
          break;
        case KEY_OFF:
          nextState = STANDBY;
          break;
      }
      break;
  }
}

void bmsChange() {
  noInterrupts();
  uint8_t bmsEdge = getPinChangeInterruptTrigger(digitalPinToPCINT(BMS_PIN));
  handleBMSEvent(bmsEdge == RISING ? BMS_OPENED : BMS_CLOSED);
  delay(50);
  interrupts();
}

void acChange() {
  noInterrupts();
  uint8_t acEdge = getPinChangeInterruptTrigger(digitalPinToPCINT(AC_PIN));
  handleBMSEvent(acEdge == RISING ? AC_UNPLUGGED : AC_PLUGIN);
  delay(50);
  interrupts();
}

void keyChange() {
  noInterrupts();
  uint8_t keyEdge = getPinChangeInterruptTrigger(digitalPinToPCINT(KEY_PIN));
  handleBMSEvent(keyEdge == RISING ? KEY_OFF : KEY_ON);
  delay(50);
  interrupts();
}
