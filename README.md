# SpacePlanetRover
The idea is that this vehicle will be controllable in a human unfriendly environment. There it  should be able to enter a polluted area and 
measure different properties in the atmosphere  for categorizing. Having a camera onboard provides vision to the mission control. 

We developed this machine with 2 sensors: SGP40 Sensor - the air quality sensor and the SM-UART-04L Laser Dust Sensor. Moreover, the car contains an 
esp32 with a digital camera, and a Raspberry Pie which is used outside of the vehicle as the MQTT broker.

It is vital that a sequence of commands for the rover is handled by all software developed since the delay of transmitted signals will be between 5
and 20 minutes.  

MOVEMENT AND GETTING THE DATA FROM THE SENSORS
In order to move the vehicle and get the data, a command line must be entered as "[movement][centimeters][space]'s'". For example, if you want vehicle to go forward 10 cm, backward 8 cm, and give 5 lines of data you must type "f10 b8 a5 s".

 
