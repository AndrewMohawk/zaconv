# -*- coding: utf-8 -*-

import sys
import time
import serial
import sqlite3

#Setup Vars
arduino_port = 'COM29'
arduino_baud = 19200
sqlite_database = '../ZaConV-Badges.s3db'

#DB and Serial Connection
conn = sqlite3.connect(sqlite_database)
c = conn.cursor()
arduino = serial.Serial(port=arduino_port,baudrate=arduino_baud,timeout=2);


#Parse Data
def ParseData(data):
	CommandType = data[0:1];
	#print("Command type is:" + CommandType);
	if(CommandType == "S"):
		#This is just a badge announcing itself
		badgeNumber = data[1:];
		print "Seen Badge:" + badgeNumber;
		epoch_time = int(time.time())
		c.execute("INSERT OR IGNORE INTO `badges` (Seen,BadgeNumber) VALUES(?,?)",[epoch_time,badgeNumber]);
		conn.commit();
	if(CommandType == "R"):
		#This is a relationship that is in the format of <badgenum1>:<badgenum2>
		midPos = data.find(":");
		if(midPos != -1):
			badgeNum1 = data[1:midPos];
			badgeNum2 = data[midPos+1:];
			epoch_time = int(time.time())
			c.execute("INSERT OR IGNORE INTO `relationships` (Seen,BadgeNum1,BadgeNum2) VALUES(?,?,?)",[epoch_time,badgeNum1,badgeNum2]);
		conn.commit();
	

#Read data
while 1:
  data = arduino.readline().strip()
  if (data is not ""):
	print("Seen '" + data + "'");
	epoch_time = int(time.time())
	c.execute("INSERT INTO `raw_data` (Seen,Data) VALUES(?,?)",[epoch_time,data]);
	conn.commit();
	ParseData(data);
	
	
conn.close();