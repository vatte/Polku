import serial
from numpy import mean
from numpy import inf
import time

class Scale:
	def __init__(self):
		#self.port = serial.Serial("/dev/tty.usbmodemfd121", 57600)
		self.port = serial.Serial("/dev/tty.usbmodem441", 57600)
		#self.port = serial.Serial("/dev/tty.usbmodemfd1221", 57600)

		#constant variables:
		self.nScales = 6
		self.nPrevValues = 50
		self.minCalibratedScaleVal = 0.15
		self.maxIdDiff = 0.1
		self.nPrevIds = 15
		self.nIds = 3
		self.colors = [ [120, 0, 0], [0, 120, 0], [0, 0, 120], [60, 60, 0], [60, 0, 60], [0, 60, 60] ]
		
		#class variables:
		#self.iCurrVal = 0
		self.iCurrId = 0
		self.allScales = [ [0] * self.nPrevValues for i in range(self.nScales) ]
		self.calibratedScales = [ [0] * self.nPrevValues for i in range(self.nScales) ]
		self.filteredScales = [ [0] * self.nPrevValues for i in range(self.nScales) ]
		#self.allIds = [inf] * self.nPrevIds
		
		self.allIds = [i*0.1+0.2 for i in range(self.nPrevIds)] 

		self.idsAtScales = [-1] * self.nScales
		#calibration variables:
		self.calibratingScale = -1
		self.minScales = [0] * self.nScales
		self.calibrationScales = [1] * self.nScales
		self.calibrated = False
			
	
	def updateData ( self, incomingData ):
		for (index, value) in enumerate(incomingData):
			self.allScales[index].pop(0)
			self.allScales[index].append(value)
			self.calibratedScales[index].pop(0)
			self.calibratedScales[index].append( self.getCalibratedData(index, value) )
			
			if(self.calibrated):
				id = self.getId( index )
				self.idsAtScales[index] = id
				#print(id, self.allIds[id], end="\t")
		#print()
			
	def getId (self, index ):
		if ( self.idsAtScales[index] == -1 and min( self.calibratedScales[index] ) >= self.minCalibratedScaleVal ):
			print("getting id")
			id = -1
			calibratedValue = self.calibratedScales[index][-1]
			mindiff = inf
			for (i, idValue) in enumerate(self.allIds):
				diff = abs(calibratedValue - idValue)
				if ( diff < self.maxIdDiff and diff < mindiff):
					mindiff = diff
					id = i
			#if (id == -1):
				#self.allIds[self.iCurrId] = calibratedValue
				#id = self.iCurrId
				#self.iCurrId = (self.iCurrId + 1) % self.nPrevIds
			#else:
				#self.allIds[id] = calibratedValue
			return (id % self.nIds)
		elif ( min(self.calibratedScales[index]) < self.minCalibratedScaleVal ):
                        return -1
		else:
			return self.idsAtScales[index]
				
	def getCalibratedData( self, index, value ):
		return float(value - self.minScales[index]) / float(self.calibrationScales[index] - self.minScales[index])
					

	def parseSerialData(self):
		valArr = []
		try:
			string = self.port.readline().decode('ascii')
			strArr = string.split(',')
			for subStr in strArr:
				val = int(subStr)
				valArr.append(val)
		except KeyboardInterrupt:
			raise
		except:
			print ("didn't parse value")
		return valArr

	def update (self):
		newValues = self.parseSerialData()
		if len(newValues) == self.nScales:
			self.updateData(newValues)
		if not self.calibratingScale == -1:
			self.calibrate()
			
	def changeColor ( self, index, color ):
		msg = "I {0} {1} {2} {3}".format(index, color[0], color[1], color[2])
		#print (msg)
		self.port.write(msg.encode())
               
		
	def startCalibration ( self ):
		self.calibratingScale = 0
		self.minScales = [0] * self.nScales
		self.changeColor(self.calibratingScale, [255, 0, 0])
	
	def calibrate ( self ):
		if self.minScales[self.calibratingScale] == 0:
			print ("Calibrating minimum")
			self.minScales[self.calibratingScale] = max(self.allScales[self.calibratingScale])
		elif min(self.allScales[self.calibratingScale]) > 2 * self.minScales[self.calibratingScale]:
		# note, "2 * " in above function could be "other things"
			print ("Calibrating")
			self.calibrationScales[self.calibratingScale] = max(self.allScales[self.calibratingScale])
			self.changeColor(self.calibratingScale, [0, 0, 0])
			self.calibratingScale += 1
			if self.calibratingScale == self.nScales:
				self.calibratingScale = -1
				print (self.calibrationScales)
				print (self.minScales)
				self.calibrated = True
				self.saveCalibrationFile("calibration.txt")
			else:
				self.changeColor(self.calibratingScale, [255, 0, 0])
			
	def saveCalibrationFile( self, filename ):
		outFile = open(filename, 'w')
		for i in range(self.nScales):
			outFile.write(str(self.calibrationScales[i]))
			outFile.write(" ")
		outFile.write('\n')
		for i in range(self.nScales):
			outFile.write(str(self.minScales[i]))
			outFile.write(" ")
		outFile.close()
	
	def loadCalibrationFile( self, filename ):
		inFile = open(filename, 'r')
		str = inFile.readline()
		str2 = inFile.readline()
		strArr = str.split(' ')
		strArr2 = str2.split(' ')
		for i in range(self.nScales):
			self.calibrationScales[i] = int(strArr[i])
			self.minScales[i] = int(strArr2[i])
		self.calibrated = True
			
			
			
			
			
			
			
			
			
			
