import os
import datetime

LOG_FOLDER = 'logs'


class Log:
    def __init__(self, name):
        cwd = os.getcwd()
        if not os.path.exists(cwd+"/"+LOG_FOLDER+"/"):
            os.mkdir(cwd+"/"+LOG_FOLDER+"/")
        self.path=cwd+"/"+LOG_FOLDER +"/"+ name+"/"
        if not os.path.exists(self.path):
            os.mkdir(self.path)





    def log(self, msg):
        self.x = datetime.datetime.now()
        self.file = open(self.path + str(self.x.day) + "." + str(self.x.month) + "." + "." + str(self.x.year) + ".txt", "a")
        self.file.write(self.getTime()+str(msg)+"\n")
        #self.file.close()

    def getTime(self):
        x = datetime.datetime.now()
        return "["+x.strftime("%X")+"] "
    def close(self):
        self.file.close()

#testing
#sensor_dust = Log('dust')
#sensor_air = Log('air')

#sensor_dust.log(700)
#sensor_air.log("Sensor value: 500")
