#define LED_RED            (PIN_PD0)
#define LED_YELLOW         (PIN_PD1)
#define LED_GREEN          (PIN_PD2)
#define LED_BLUE           (PIN_PD3)

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
}

void loop() {
  digitalWrite(LED_RED, LOW);
  delay(100);
  digitalWrite(LED_YELLOW, LOW);
  delay(100);
  digitalWrite(LED_GREEN, LOW);
  delay(100);
  digitalWrite(LED_BLUE, LOW);
  delay(100);
  
  digitalWrite(LED_RED, HIGH);
  delay(100);
  digitalWrite(LED_YELLOW, HIGH);
  delay(100);
  digitalWrite(LED_GREEN, HIGH);
  delay(100);
  digitalWrite(LED_BLUE, HIGH);
  delay(100);
}