from email import message
from re import X
import paho.mqtt.client as mqtt
import sensor_log  # LOG IMPORT
from commandLine_interface import CLI


# import RoboCar


class Mission_Handler:

    def __init__(self, Distance, Turning, Dust_Sensor, Airquality_Sensor):
        self.__Distance = Distance
        self.__Turning = Turning
        self.__Dust_Sensor = Dust_Sensor
        self.__Airquality_Sensor = Airquality_Sensor

    def getMission(self):
        message = ""
        to_array = list(message)
        return to_array

    def addMission(self):
        self.__Distance = input(print("Enter the distance: "))
        self.__Turning = input(print("Enter the turning: "))
        self.__Dust_Sensor = input(print("Enter if you want dust-sensor data: "))
        self.__Airquality_Sensor = input(print("Enter if you want airquality-sensor data: "))


class MQTT_Handler:
   # message = Mission_Handler.getMission()



    def __init__(self):
        self.mission = CLI()
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

        self.dust_sensor_log = sensor_log.Log('dust')
        self.air_sensor_log = sensor_log.Log('air')




    def connect(self, ip, port, ka):
        self.client.connect(ip, port, ka)

    def loop(self):
        self.client.loop_forever()




    def on_connect(self, client, userdata, flags, rc):
        self.mission.interface()
        commands = self.mission.getCommands()
        print(commands)

        print("Connected with result code " + str(rc))

        self.client.subscribe([("RoboCar/Sensor/Air", 0), ("RoboCar/Sensor/Dust", 0)])
        #client.subscribe([("gateway/a555b555c555d555/rx", 0), ("gateway/new topic/rx", 0)])
        self.client.publish("RoboCar/Mission420/", commands)

    def on_message(self, client, userdata, msg):
        print(msg.topic + " " + str(msg.payload))

        #data = msg.payload.decode("utf-8")
        #print(data)
        print(msg.topic == 'RoboCar/Sensor/Air')
        if msg.topic == 'RoboCar/Sensor/Air':
            airquality = msg.payload
            print(airquality)
            self.air_sensor_log.log(airquality)
            self.air_sensor_log.close()

        if msg.topic == 'RoboCar/Sensor/Dust':
            dust = msg.payload
            print(dust)
            self.dust_sensor_log.log(dust)
            self.dust_sensor_log.close()




mqttH = MQTT_Handler()
mqttH.connect("10.120.0.94", 1883, 60)
#print('lol1')

mqttH.loop() #frozes
#print('lol')
