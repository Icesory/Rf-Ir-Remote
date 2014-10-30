#include <RFControl.h>
#include <IRremote.h>

//#define DEBUG_LED
/*
SAMSUNG IR Remote
REMOTE: BN59-00861A
DEVICE: TV UE46
PROTOCOL: SAMSUNG
*/
#define SAMSUNG_BITS        32
#define SAMSUNG_POWER       0xE0E040BF
#define SAMSUNG_volume_UP   0xE0E0E01F
#define SAMSUNG_volume_DOWN 0xE0E0D02F
#define SAMSUNG_prog_UP     0xE0E048B7
#define SAMSUNG_prog_DOWN   0xE0E008F7
#define SAMSUNG_OK          0xE0E016E9
#define SAMSUNG_SOURCE      0xE0E0807F
#define SAMSUNG_arrow_UP    0xE0E006F9
#define SAMSUNG_arrow_DOWN  0xE0E08679
#define SAMSUNG_arrow_LEFT  0xE0E0A659
#define SAMSUNG_arrow_RIGHT 0xE0E046B9
#define SAMSUNG_GUIDE       0xE0E0F20D
#define SAMSUNG_MUTE        0xE0E0F00F
#define SAMSUNG_RETURN      0xE0E01AE5
#define SAMSUNG_TV          0xE0E0D827
#define SAMSUNG_ONE         0xE0E020DF
#define SAMSUNG_TWO         0xE0E0A05F
#define SAMSUNG_THREE       0xE0E0609F
#define SAMSUNG_FOUR        0xE0E010EF
#define SAMSUNG_FIVE        0xE0E0906F
#define SAMSUNG_SIX         0xE0E050AF
#define SAMSUNG_SEVEN       0xE0E030CF
#define SAMSUNG_EIGHT       0xE0E0B04F
#define SAMSUNG_NINE        0xE0E0708F
#define SAMSUNG_ZERO        0xE0E08877
const unsigned long samsung_commands[] = {SAMSUNG_POWER, SAMSUNG_POWER, SAMSUNG_volume_DOWN, SAMSUNG_volume_UP};
const int samsung_commands_lenght = 4;
const unsigned long SAMSUNG_REMOTE_ID = 1234567;
/*
PIONEER IR Remote
REMOTE: AXD7596
DEVICE: AVR VSX-950
PROTOCOL: NEC
*/
#define PIONEER_BITS        32
#define PIONEER_POWER       0xA55A38C7
#define PIONEER_volume_UP   0xA55A50AF
#define PIONEER_volume_DOWN 0xA55AD02F
#define PIONEER_MUTE        0xA55A48B7
#define PIONEER_source_BD   0xA55A3AC5 
#define PIONEER_source_DVD  0xA55AA15E
#define PIONEER_source_DVR  0xA55A916E
#define PIONEER_source_HDMI 0xA55A6A95
#define PIONEER_source_TV   0xA55A30CF
#define PIONEER_source_CD   0xA55A32CD
#define PIONEER_source_NETR 0xA55A3AC5
#define PIONEER_source_ADAP 0xA55A3AC5
#define PIONEER_source_IPOD 0xA55A7986
#define PIONEER_source_TUNE 0xA55AE21D
#define PIONEER_source_VID  0xA55A6897
#define PIONEER_source_L    0xA55A6A95
#define PIONEER_source_R    0xA55AAA55
#define PIONEER_OK          0xA55A21DE
#define PIONEER_arrow_UP    0xA55A01FE
#define PIONEER_arrow_DOWN  0xA55A817E
#define PIONEER_arrow_LEFT  0xA55AC13E
#define PIONEER_arrow_RIGHT 0xA55A41BE
#define PIONEER_CHLEVEL     0xA55A6A95
#define PIONEER_AUTO_ALC    0xA55A59A6
#define PIONEER_STEREO      0xA55AF906
#define PIONEER_STANDART    0xA55A7A85
#define PIONEER_ADV_SURR    0xA55AF906
#define PIONEER_AUDIO_PARA  0xA55A5AA5
#define PIONEER_VIDEO_PARA  0xA55ADA25
#define PIONEER_HOME        0xA55AD926
#define PIONEER_RETURN      0xA55ABA45
const unsigned long pioneer_commands[] = {PIONEER_POWER, PIONEER_POWER, PIONEER_volume_DOWN, PIONEER_volume_UP};
const int pioneer_commands_lenght = 4;
const unsigned long PIONEER_REMOTE_ID = 1231238;

IRsend irsend;

decode_results results;
const unsigned int buckets_ref[] = {320, 1550, 2750, 10350, 0, 0, 0, 0};
const unsigned int pulse_count = 132;
const unsigned long intervall = 30000;
unsigned long time_old = 0;
unsigned long time_old_led = 0;
unsigned long vcc = 5000;
boolean led = false;

void setup() {
  Serial.begin(9600);
  RFControl::startReceiving(0);
  pinMode(13, OUTPUT);
}

void loop() {
  if(RFControl::hasData()) {
    #ifdef DEBUG_LED
    digitalWrite(13, HIGH);
    #endif
    unsigned int *timings;
    unsigned int timings_size;
    RFControl::getRaw(&timings, &timings_size);
    unsigned int buckets[8];
    RFControl::compressTimingsAndSortBuckets(buckets, timings, timings_size);
    unsigned long id = 0;
    byte state = 0;
    byte unit = 0;
    boolean detected = decodeProtocol(timings_size, timings, &buckets[0], &id, &state, &unit);
    if(detected){     
      if(id == SAMSUNG_REMOTE_ID){
        int i = unit*2 + state;
        if(i < samsung_commands_lenght){
          irsend.sendSAMSUNG(samsung_commands[i], SAMSUNG_BITS);
        }
      } else if (id == PIONEER_REMOTE_ID){
        int i = unit*2 + state;
        if(i < pioneer_commands_lenght){
          irsend.sendNEC(pioneer_commands[i], PIONEER_BITS);
        }
      }
      
    }
    RFControl::continueReceiving();
    #ifdef DEBUG_LED
    digitalWrite(13, LOW);
    #endif
  }
  unsigned long time = millis();
  if(time > time_old){
    time_old = time + intervall;
    vcc = vccVoltage();
  }
  if(vcc < 3500){
    if(time > time_old_led){
      led = !led;
      digitalWrite(13, led);
      time_old_led = time + 500;
    } 
  }
}


boolean decodeProtocol(unsigned int timings_size, unsigned int *timings, unsigned int buckets[], unsigned long int *id, byte *state, byte *unit){
  if(timings_size != pulse_count)
    return false;

  for(int j = 0; j < 8; j++){
    unsigned int delta = buckets_ref[j]/4 + buckets_ref[j]/8;
    if (buckets[j]!= 0 && (buckets_ref[j] - delta > buckets[j] || buckets_ref[j] + delta < buckets[j])){
      return false;
    } 
  }

  byte decode_timings[32];
  for(int i= 0;i<32;i++){
    decode_timings[i]=0; 
  }
  int j = 2;
  for(int i = 0; i < 32; i++){
    if(timings[j+1] == 1)
      decode_timings[i] = 1;
    j += 4;
  }
  id[0] = binaryToLong(&decode_timings[0], 0, 25);
  state[0] = decode_timings[27];
  unit[0] = binaryToByte(&decode_timings[0], 28, 31);
  return true;
}

unsigned long int binaryToLong(byte *data, byte b, byte e){
  unsigned long int number = 0;
  while (b <= e){
    number = number << 1;
    number = number + data[b];
    b++;
  }
  return number;
}

byte binaryToByte(byte *data, byte b, byte e){
  byte number = 0;
  while (b <= e){
    number = number << 1;
    number = number + data[b];
    b++;
  }
  return number;
}


// code.google.com/p/tinkerit/wiki/SecretVoltmeter
long vccVoltage() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}
