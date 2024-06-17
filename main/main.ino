// Analyse comparator signal
// processing in µs on uint32_t integer
// 4 294 967 296 µs
// 4 294.967296 sec
// 1:11:34 so after 1 hour needs to reboot
// Related to https://knowledge.pinon-hebert.fr/index.php/Comparateur_Num%C3%A9rique



const int clk = 32;
const int dat = 33;
const int frame_out = 22; /// frame detection
const int dat_out=21; // data decode

#define REVERSE     // Defined if input signal needs to be inverted (needed if level adaptation done with NPN)

// ANALYSE THE CLOCK SIGNAL
uint32_t event_fall;   // timestamp of the last falling edge
uint32_t time_low;    // duration of last low level (between falling and rising edge)

uint32_t event_rise;  // tlmestamp of the last rising edge
uint32_t time_high;   // duration of the last high levet (between rising and fallin edge)

int bit=-1;
uint8_t bitFrame[24];
uint8_t measure[24];

int state;

inline int getClkState(){ 
  #ifdef REVERSE
    return (!digitalRead(clk));
  #else
    return (digitalRead(clk));
  #endif
}

inline int getDatState(){ 
  #ifdef REVERSE
    return (!digitalRead(dat));
  #else
    return (digitalRead(dat));
  #endif
}

void IRAM_ATTR clk_rise(){
  state=HIGH;
  event_rise=micros();
  int value=getDatState();
  time_low=event_rise-event_fall;
  if (bit>=0){
    bitFrame[bit]=value;
    bit++;
    bit%=24;
  }
  if (bit==0){
    memcpy(measure,bitFrame,24);
  }
  digitalWrite(frame_out,(bit==0));
  digitalWrite(dat_out,value); 
}

void IRAM_ATTR clk_fall(){
  state=LOW;
  event_fall=micros();
  time_high=event_fall-event_rise;  
}

void IRAM_ATTR clk_CHANGE() {
  uint32_t ti=micros();
  if(getClkState()){
    return clk_rise();
  }else{
    return clk_fall();
  }
}

void printMeasure(){
  int i;
  // Décode mesured value
  int val=0;
  for (i=11; i>=0; i--){
    val<<=1;
    val+=measure[i];
  }
  if (measure[20]==HIGH){
    val=-val;
  }
  // convert regarding the system (metric vs US)
  float fval;
  if (measure[23]==HIGH){
    fval=float(val)/2000.0;
    Serial.println(fval,4);
  }else{
    fval=float(val)/100.0;
    Serial.println(fval,2);
  }
}

void setup() {
  
  Serial.begin(115200);
  pinMode(clk, INPUT_PULLUP);
  pinMode(dat,INPUT_PULLUP);
  pinMode(frame_out,OUTPUT);
  pinMode(dat_out,OUTPUT);
  digitalWrite(frame_out,LOW);
  digitalWrite(dat_out,HIGH);
  
  Serial.println("\nInit interuption");

  state=getClkState();
  attachInterrupt(clk,clk_CHANGE,CHANGE);
  Serial.println("Sync");
  while (time_high<100000){

  }
  bit=0;
  Serial.println(getClkState());
  Serial.println("Ready");
}

void loop() {
  delay(100);
  printMeasure();
}
