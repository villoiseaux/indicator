// Analyse comparator signal
// processing in µs on uint32_t integer
// 4 294 967 296 µs
// 4 294.967296 sec
// 1:11:34 so after 1 hour needs to reboot
// Related to https://knowledge.pinon-hebert.fr/index.php/Comparateur_Num%C3%A9rique



const int clk = 32;//32;
const int dat = 33;

const int clk_out = 23; // to monitor clock signal
const int frame_out = 22; /// frame detection
const int dat_out=21; // data decode

#define TRUE 1
#define FALSE 0
#define REVERSE     // Defined if input signal needs to be inverted (needed if level adaptation done with NPN)

// ANALYSE THE CLOCK SIGNAL

uint32_t event_fall;   // timestamp of the last falling edge
uint32_t time_low;    // duration of last low level (between falling and rising edge)

uint32_t event_rise;  // tlmestamp of the last rising edge
uint32_t time_high;   // duration of the last high levet (between rising and fallin edge)

uint32_t event_change;
uint32_t period;



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
  period=ti-event_change;
  event_change=ti;
  if(getClkState()){
    digitalWrite(clk_out,LOW);
    return clk_rise();
  }else{
    digitalWrite(clk_out,HIGH);
    return clk_fall();
  }
}

void printMeasure(){
  for (int i=0; i<24; i++){
    Serial.print(measure[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  pinMode(clk, INPUT_PULLUP);
  pinMode(dat,INPUT_PULLUP);
  pinMode(clk_out,OUTPUT);
  pinMode(frame_out,OUTPUT);
  pinMode(dat_out,OUTPUT);
  digitalWrite(frame_out,LOW);
  digitalWrite(dat_out,HIGH);
  
  Serial.println("\nInit interuption (wait for 10s)");
  for (int i=0; i<10; i++){
    Serial.print(".");
    delay (1000);
  }
  Serial.println();

  state=getClkState();
  attachInterrupt(clk,clk_CHANGE,CHANGE);
  Serial.println("Sync");
  while (time_high<100000){

  }
  bit=0;
  Serial.println(getClkState());
  delay(5000);
  Serial.println("Ready");
}

void loop() {
  delay(10);
  printMeasure();
}
