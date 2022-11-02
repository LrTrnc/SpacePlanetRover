
class CLI:
    def __init__(self):
        self.__first = True
        self.__inputting = True
        self.__stringy = ""
        self.__going = True
        self.__commands = ''

    def interface (self):
        while self.__going == True:
            input("Welcome to mission control, press the any-key to continue..")
            self.__inputting=True
            while self.__inputting == True:

                if self.__first == True:

                    print("\nPossible commands are: Forward, backward, pivot left, pivot right, turn left, turn right.\n for driving forward and backwards type input as centimeters and for turning type input as degrees (0 for indefinite)  ::  \n When done type stop")
                    self.__command = str(input("\ninput first command  ::  "))
                    if (self.__command != 'stop' and self.__command != 'end'):
                        self.__duration = str(input("for driving forward and backwards type input as centimeters and for turning type input as degrees (0 for indefinite)  ::  "))
                    self.__first = False


                elif self.__first == False:
                    self.__command = str(input("input next command  ::"))
                    if (self.__command != 'stop' and self.__command != 'end'):
                        self.__duration = str(input("for driving forward and backwards type input as centimeters and for turning type input as degrees (0 for indefinite)  ::  "))


                if self.__command == 'forward' or self.__command == 'f' : #forward
                    print(self.__command, self.__duration)
                    self.__commands += 'f'
                    self.__commands +=  self.__duration
                    self.__commands += ' '


                    #mqtt_publish(f"f{ self.__duration} ")
                    #else:
                    #mqtt_publish("f ")

                elif self.__command == 'backward' or self.__command == 'b': #backward
                    print(self.__command,  self.__duration)
                    self.__commands += 'b'
                    self.__commands +=  self.__duration
                    self.__commands += ' '
                    #mqtt_publish(f"b{ self.__duration} ")
                #else:
                    #mqtt_publish("b ")

                elif self.__command == 'r': #pivot right

                    print(self.__command,  self.__duration)
                    self.__commands += 'r'
                    self.__commands +=  self.__duration
                    self.__commands += ' '

                elif self.__command == 'l': #pivot left

                    print(self.__command,  self.__duration)
                    self.__commands += 'l'
                    self.__commands +=  self.__duration
                    self.__commands += ' '

                elif self.__command == 'a': #use sensors

                    print(self.__command,  self.__duration)
                    self.__commands += 'a'
                    self.__commands +=  self.__duration
                    self.__commands += ' '

                elif self.__command == 'w':  # use sensors

                    print(self.__command, self.__duration)
                    self.__commands += 'w'
                    self.__commands += self.__duration
                    self.__commands += ' '


                elif self.__command == 'stop': #end of transmission
                    self.__inputting = False
                    self.__going = False
                    self.__commands += 's'
                    print(self.__commands)



                elif self.__command == 'end':
                    self.__inputting = False
                    self.__going = False
                    exit()
                    #mqtt_publish("s")
            #mqtt_publish("s")

    def getCommands(self):
        return self.__commands