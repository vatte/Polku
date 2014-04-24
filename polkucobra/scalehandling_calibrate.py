from Scale import Scale
import time
from random import randint
from threading import Thread

from pythonosc import udp_client
from pythonosc import osc_message_builder
from pythonosc import osc_server
from pythonosc import dispatcher



client = udp_client.UDPClient("127.0.0.1", 8103)


scale = Scale()
timer = time.time()
calibrationStarted = False

def parseLightChange(index, r, g, b):
	scale.changeColor(index, [r, g, b])

dispatcher = dispatcher.Dispatcher()
dispatcher.map("/changeLight", parseLightChange)

server = osc_server.ThreadingOSCUDPServer(("127.0.0.1", 8102), dispatcher)
print("ready to serve")
serverThread = Thread(target = server.serve_forever)
print("readier")
serverThread.daemon = True
serverThread.start()
print("started")
#serverThread.run()
print("serving")


while True:
	
	if(not calibrationStarted and time.time() - timer > 5):
		timer = time.time()
		#scale.changeColor(randint(0,5), [randint(0,255), randint(0,255), randint(0,255)])
		calibrationStarted = True
		scale.startCalibration()
		#scale.loadCalibrationFile("calibration.txt")
	scale.update()
	
	oscMsg = osc_message_builder.OscMessageBuilder(address = "/scales")
	
	for i in range(scale.nScales):
		val =  scale.calibratedScales[i][-1]
		#val = scale.idsAtScales[i]
		oscMsg.add_arg(scale.idsAtScales[i])
		#print("{1}\t{0:.2f}".format(val, scale.idsAtScales[i]), end = "\t")
		#print (val, end="\t")
		#a = scale.isStable(i, 10)
		#print scale.isStable(i), "\t",
	#print()
	try:
		msg = oscMsg.build()
		client.send(msg)
	except:
		print("Failed to build oscmessage")
