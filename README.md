Rf-Ir-Remote
============

A Arduino based RF to IR Remote

![Overview](picture1.jpg)

-------------

Im zuge meiner Hausautomatisierung überkam mich der Wunsch auch meinen Fernseher, ja eigentlich 
alle meine Heimkino Geräte über mein Handy steuern zukönnen.

Die Basis meiner Hausautomatisierung bildet ein Raspberry-PI mit der absolut genialen Software pimatic.
pimatic wird von Oliver Schneider als Open Source Project entwickelt.

LINKS

Die RF-IR-Remote empfängt 433MHz AM Signal, decodiert sie und Sendet dann den entsprechenden Infrarot Code.
Für den Empfang benötigt man einen 433MHz Empfänger. Diese bekommt man recht günstig bei ebay.
Zum Senden braucht man Infrarot LEDs. Im Moment verwende ich zwei aus einer alten Fernbedienung. Damit der Arduino da nicht überlastet wird sollte ein Transistor verwendet werden.

Im folgenden ist der Schaltplan dargestellt.

Die Software habe ich mir größten Teils zusammen gehamstert.

[Als erstes benötigt ihr die RFControl library von Oliver Schneider.](https://github.com/pimatic/RFControl)

[Als zweites wird die IR library von Ken Shirriff benötigt.](https://github.com/shirriff/Arduino-IRremote)
Die zwei libraries habe ich auch in mein Repository kopiert. Mit diesen version funktioniert der Code defintiv.

Im moment befindet sich das projekt noch ganz am Anfang. Das bedeut, es wir im laufe der Zeit noch einige änderungen geben. Die Rf-IR-Remote soll eine eigene Library werden.

Der folgende abschnitt zeigt, wie man die IR Befehle an einen RF Befehl knüpft.

-------------

### RF zu IR
Verwendet wird im moment das neue Kaku-Protokol. In diesem wird eine ID, eine Unit und ein State übertragen.
Die ID ist der Name der Fernbedienung. 
Sie ist eine 26bit große Zahl und kann demnach einen wert zwischen 0 und 67´108´863 annehmen. Ich empfehle einfach mal einen bereich zwischen einer und zehn millionen.

Die Unit ist der angesprochene Actuator.
Sie ist eine 4bit Zahl und kann demnach einen wert zwischen 0 und 15 annemhen.

Der state bestimmt für einen Actuator den einzunhemen Zustand.
Es ist nur 1bit und kann demnach nur 0 oder 1 sein.

Die Rf-IR-Remote bindet eine Fernbedienung an eine bestimmte ID. Man pro ID nur 16 Befehle senden kann sieht das für eine Moderne Fernbedienung etwas mau aus. Aber benötigen wir die kontrolle über den Status?
Nein. Ein IR Befehl wird gesendet oder eben ein anderer. So können wir mit hilfe des state 32 IR Befehle auf eine RF-
ID legen.

Ein IR Befehel setzt sich als aus der ID, der Unit und dem State zusammen.
In der Software wird als erste geprüft ob die empfange ID zu einer IR-Fernbedienung passt.
Danach wird der auszuführende Befehl nach folgender Formel berechent

###### IR_pos = Unit * 2 + state 

Die IR Befehle sind in einem const Array gespeichert. Aus dem Array wird nun die Stelle IR_pos ausgelesen und an die Sendefunktion übergeben.

-------------

### Das Array füllen

Die bekannten IR Codes sind als sogenannte präprozessor defines gespeichert. Diese belegen keinen Speicherplatz auf dem Arduino, wie defines fuktionieren könnt ihr im Internet nachlesen (link folgt).
Da ich einen Samsung Tv besitze bezieht sich das Beispiel dadrauf.
```Arduino
/*
SAMSUNG IR Remote
REMOTE: BN59-00861A
DEVICE: TV UE46
PROTOCOL: SAMSUNG
*/
#define SAMSUNG_BITS 32
#define SAMSUNG_POWER 0xE0E040BF
#define SAMSUNG_volume_UP 0xE0E0E01F
#define SAMSUNG_volume_DOWN 0xE0E0D02F
...
#define SAMSUNG_EIGHT 0xE0E0B04F
#define SAMSUNG_NINE 0xE0E0708F
#define SAMSUNG_ZERO 0xE0E08877
const unsigned long samsung_commands[] = {SAMSUNG_POWER, SAMSUNG_volume_DOWN, SAMSUNG_volume_UP};
const int samsung_commands_lenght = 3;
```
Das array samsung_commands wird mit den defines gefüllt. Dabei ist der erste Command die stelle 0 und ab dann dann aufsteigend. Wichtig ist immer die samsung_commands_lenght anzugeben.
Auch die SAMSUNG_REMOTE_ID muss konfiguriert werden. Es ist die RF ID.
Ihr könnt maximal 32 Befehle in ein Array speichern. Wenn ihr mehr Befehle benötig müsst ihr eine zweite Fernbedieung anlegen.
Wenn nun ein RF Signal empfangen wurde wird geprüft ob die ID passt. Wenn ja dann wird die Position berechnet und der equivalente IR Befehl gesendet.
Dabei gilt nach oben stehnder Formel
(Unit=0,State=0) => IR_pos=0
(Unit=0,State=1) => IR_pos=1
(Unit=1,State=0) => IR_pos=2
(Unit=1,State=1) => IR_pos=3
(Unit=2,State=0) => IR_pos=4
usẃ.

-------------

### Eine neue Fernbedienung anlegen

Die Fernbedienungen sind in dem Folgenden Abschnitt definiert.

```Arduino
boolean detected = decodeProtocol(timings_size, timings, &buckets[0], &id, &state, &unit);

if(detected){
  if(id == SAMSUNG_REMOTE_ID){
    int i = unit*2 + state;
    if(i < samsung_commands_lenght){
      irsend.sendSAMSUNG(samsung_commands[i], SAMSUNG_BITS);
    }
  } 
}

RFControl::continueReceiving();
```
Um nun eine weitere Fenbedienung zudefinieren wird ein neuer else if zweig eingebaut.

```Arduino
boolean detected = decodeProtocol(timings_size, timings, &buckets[0], &id, &state, &unit);

if(detected){
  if(id == SAMSUNG_REMOTE_ID){
    int i = unit*2 + state;
    if(i < samsung_commands_lenght){
      irsend.sendSAMSUNG(samsung_commands[i], SAMSUNG_BITS);
    }
  }  
  else if (id == PIONEER_REMOTE_ID){
    int i = unit*2 + state;
    if(i < pioneer_commands_lenght){
      irsend.sendNEC(pioneer_commands[i], PIONEER_BITS);
    }
  }
}

RFControl::continueReceiving();
```
Ihr bebötigt eine neu ID, ein neues Array und Array länge.
Zudem dem müsst ihr wissen, welches Protokol euer Gerät versteht. Samsung hat ein eigenes, Pioneer verwendet NEC.
