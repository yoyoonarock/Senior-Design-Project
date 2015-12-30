This is the code for my senior design project that I worked on with two other classmates. In a single sentence, we created a bike lock that can be electronically locked and unlocked using a smartphone or a RFID card. The code that I have provided involves the communication between an Arduino and a Raspberry Pi. The Arduino serves to receive a phone signal and send signals to a motor, locking or unlocking it. The Raspberry Pi acts as a server and a database that stores the lock state of every lock.

There are two files:

arduino.ino
raspberrypi.py

When the Arduino receives a phone signal, it will create a unique ID for that phone, and send that to the Raspberry Pi. The Raspberry Pi then queries the database for this phone ID, and sends back a verification bit to the Arduino depending on whether or not the query was successful. The Arduino will then lock or unlock the lock or do nothing based on the verification bit.

For more information on our project, you can go to https://courses.engr.illinois.edu/ece445/projects.asp, choose the Fall 2015 term, and select Group 26.