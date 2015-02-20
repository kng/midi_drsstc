
//#include <ardumidi.h>

#define DBG 1

#define MAXKEYS 4       // number of simultaneous oscillators running
#define NOTES 128       // number of notes starting from A4
#define STARTNOTE 69    //A4 = 69 (440 Hz)
#define STARTFREQ 440.0
#define SRATE 44100     // approx 22.7ns/sample @ 44.1kHz

#define MAXON 120       // max on time in us
#define MINON 50        // min on time in us
#define HOLDOFF 150     // holdoff after a pulse has turned off

enum states {
  idling,
  new_on,
  note_on,
  note_off
};

typedef struct {
  byte key;
  states stat;
  short osc;
  byte vol;
} synthesizer;

typedef struct {
  unsigned short frq;
  byte vol;
} synthkeys;

synthesizer synth[MAXKEYS];
synthkeys keys[NOTES];

const byte ledPin = 13;

void setup(){
#ifdef DBG
  Serial.begin(115200);      // MIDI 31250, hairless 115200
#else
  Serial.begin(31250);
#endif

  pinMode(ledPin, OUTPUT);
  noInterrupts();           // disable all interrupts

#ifndef DBG
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = F_CPU / SRATE; // compare match register (cpu clock)16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS10);    // 1 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
#endif

  DDRB = 1;    // debug output on PB0
  DDRD = 0xFF;

  unsigned char x;
  float f;
  for (x = 0; x < NOTES; x++) {    // precalculate the frequencies and pulse length
    f = STARTFREQ * pow(2, (x - STARTNOTE) / 12.0);
    //float f = 440.0 * pow(2, (x - 69) / 12.0);
    keys[x].frq = 65536 * f / SRATE;
    f = (float)(1+MAXON-MINON) / NOTES;  // ramp the volume over MINON to MAXON
    keys[x].vol = MINON + (f * x);
  }
  interrupts();             // enable all interrupts

#ifdef DBG
  for(x = 0; x < NOTES; x++){
    Serial.print("key: ");
    Serial.print(x);
    Serial.print(", frq: ");
    Serial.print(keys[x].frq);
    Serial.print(", vol: ");
    Serial.println(keys[x].vol);
  }
#endif
}

void loop(){

#ifdef DBG
  unsigned char x;
  for(x = 0; x < MAXKEYS; x++){
    Serial.print("oscillator: ");
    Serial.print(x);
    Serial.print(", key: ");
    Serial.print(keys[x].key);
    Serial.print(", frq: ");
    Serial.print(keys[x].frq);
    Serial.print(", vol: ");
    Serial.println("buu");
  }
  delay(1000);
#endif
  /*  while (midi_message_available() > 0) {
    MidiMessage m = read_midi_message();
    if (m.command == MIDI_NOTE_ON && m.param1 >= STARTNOTE && m.param1 < STARTNOTE - NOTES) { // make sure the key is within our acceptable range
      for(unsigned char x=0; x < MAXKEYS; x++){
        if(synth[x].stat == idling){   // find first availible slot
          synth[x].key = m.param1;  // copy the note
          synth[x].osc = keys[m.param1].frq;
          synth[x].vol = keys[m.param1].vol;
          synth[x].stat = new_on;
          break;
        }
      }
    }
    else if (m.command == MIDI_NOTE_OFF) {
      for(unsigned char x=0; x < MAXKEYS; x++){
        if(synth[x].key == m.param1){   // find active note
          synth[x].stat = note_off;
          break;
        }
      }
    }
  }*/
}

ISR(TIMER1_COMPA_vect) {
  PORTB = 1;
  for (unsigned char x = 0; x < MAXKEYS; x++) {
    if(synth[x].stat == new_on){
      synth[x].stat == note_on;
      // start oscillator
    }
    if(synth[x].stat == note_off){
      synth[x].stat == idling;
    }
  }
  PORTB = 0;
}


