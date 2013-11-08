# -*- coding: utf-8 -*-

import sys
import time
import sqlite3
from MaltegoTransform import *

#Setup Vars
sqlite_database = '../ZaConV-Badges.s3db'

#DB and MaltegoLib setup
conn = sqlite3.connect(sqlite_database)
me = MaltegoTransform()

c = conn.cursor()
c.execute("SELECT Seen,BadgeNumber from badges");
rows = c.fetchall();
for r in rows:
	b = me.addEntity("zacon.Badge",r[1]);
	epoch = int(r[0]);
	firstSeen = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime(epoch))
	b.addAdditionalFields("link#maltego.link.label","link#maltego.link.label",True,"First seen at:" + firstSeen)
	
me.returnOutput()
