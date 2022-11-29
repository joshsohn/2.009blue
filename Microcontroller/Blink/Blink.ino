void setup() {
  // put your setup code here, to run once:
  pinMode(A3, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(A3, HIGH);
  delay(100);
  digitalWrite(A3, LOW);
  delay(50);
} 
