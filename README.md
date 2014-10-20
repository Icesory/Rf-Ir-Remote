Rf-Ir-Remote
============

A Arduino based RF to IR Remote

Im zuge meiner Hausautomatisierung überkam mich der Wunsch auch meinen Fernseher, ja eigentlich 
alle meine Heimkino Geräte über mein Handy steuern zukönnen.

Die Basis meiner Hausautomatisierung bildet ein Raspberry-PI mit der absolut genialen Software pimatic.
pimatic wird von Oliver Schneider als Open Source Project entwickelt.

LINKS

Die RF-IR-Remote empfängt 433MHz AM Signal, decodiert sie und Sendet dann den entsprechenden Infrarot Code.
Für den Empfang benötigt man einen 433MHz Empfänger. Diese bekommt man recht günstig bei ebay.
Zum Senden braucht man Infrarot LEDs. Im Moment verwende ich zwei aus einer alten Fernbedienung. Damit der Arduino da nicht überlastet wird sollte ein Transistor verwendet werden.

Im folgenden ist der Schaltplan dargestellt.
BILD

Die Software habe ich mir größten Teils zusammen gehamstert.
Als erstes benötigt ihr die RFControl library von Oliver Schneider. https://github.com/pimatic/RFControl
Als zweites wird die IR library von Ken Shirriff benötigt. https://github.com/shirriff/Arduino-IRremote
