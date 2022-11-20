#define BUZZER_PIN A0
void setup() {
  // put your setup code here, to run once:
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(6,HIGH);
  tone(BUZZER_PIN, 4000);
  delay(100);
  noTone(BUZZER_PIN);
  delay(20);
  digitalWrite(6,LOW);
  tone(BUZZER_PIN, 1300);
  delay(100);
  noTone(BUZZER_PIN);
  delay(20);
}
