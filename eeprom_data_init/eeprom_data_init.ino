#include <EEPROM.h>

//VALORI DA SCRIVERE
unsigned int tOneshot = 1000;
unsigned int delayHDR = 400; //tempo tra uno scatto hdr e l'altro nella serie
unsigned int tfocus = 300; //tempo pressione messa a fuoco
unsigned int tbetween = 30; //tempo tra uno scatto time lapse e l'altro
bool wakeUp = 0; //usare il tasto messa a fuoco?
unsigned int tshot = 350;
unsigned int nshots = 1;

void setup() {
  Serial.begin(9600);

  //WRITING VALUES

  EEPROM.put( 0, tshot );
  Serial.print("Written tshot! Value: ");
  Serial.println(tshot, DEC);

  EEPROM.put( 2, tfocus );
  Serial.print("Written tfocus! Value: ");
  Serial.println(tfocus, DEC);

  EEPROM.put( 4, wakeUp );
  Serial.print("Written wakeUp! Value:");
  Serial.println(wakeUp, DEC);

  EEPROM.put( 6, tOneshot );
  Serial.print("Written tOneshot! Value: ");
  Serial.println(tOneshot, DEC);

  EEPROM.put( 8, nshots );
  Serial.print("Written nshots! Value: ");
  Serial.println(nshots, DEC);

  EEPROM.put( 10, delayHDR );
  Serial.print("Written delayHDR! Value: ");
  Serial.println(delayHDR, DEC);

  EEPROM.put( 12, tbetween );
  Serial.print("Written tbetween! Value: ");
  Serial.println(tbetween, DEC);

  //READING VALUES
  unsigned int tOneshotread; //tempo pressione scatto oneshot
  unsigned int delayHDRread; //tempo tra uno scatto hdr e l'altro nella serie
  unsigned int tfocusread; //tempo pressione messa a fuoco
  unsigned int tbetweenread; //tempo tra uno scatto time lapse e l'altro
  bool wakeUpread; //usare il tasto messa a fuoco?
  unsigned int tshotread;
  unsigned int nshotsread;

  Serial.println();

  EEPROM.get(0, tshotread); //read saved value in eeprom (time of shutter)
  Serial.print("tshotread value read: ");
  Serial.println(tshotread, DEC);

  EEPROM.get(2, tfocusread);
  Serial.print("tfocusread value read: ");
  Serial.println(tfocusread, DEC);

  EEPROM.get(4, wakeUpread); //read saved value in eeprom (enable focus)
  Serial.print("wakeUpread value read: ");
  Serial.println(wakeUpread, DEC);

  EEPROM.get(6, tOneshotread);
  Serial.print("tOneshotread value read: ");
  Serial.println(tOneshotread, DEC);

  EEPROM.get(8, nshotsread);
  Serial.print("nshotsread value read: ");
  Serial.println(nshotsread, DEC);

  EEPROM.get(10, delayHDRread);
  Serial.print("delayHDRread value read: ");
  Serial.println(delayHDRread, DEC);

  EEPROM.get(12, tbetweenread);
  Serial.print("tbetweenread value read: ");
  Serial.println(tbetweenread, DEC);

}

void loop() {
  /* Empty loop */
}
