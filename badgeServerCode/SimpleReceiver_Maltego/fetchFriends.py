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
badgeNum = sys.argv[1]
c = conn.cursor()
c.execute("SELECT Seen,BadgeNum1,BadgeNum2 from relationships WHERE (BadgeNum1 = ? OR BadgeNum2 = ?)",[badgeNum,badgeNum]);
rows = c.fetchall();
for r in rows:
	epoch = int(r[0]);
	firstSeen = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime(epoch))
	newBadge = r[1]; 
	if(r[1] == badgeNum):
		newBadge = r[2];
	b = me.addEntity("zacon.Badge",newBadge);
	b.addAdditionalFields("link#maltego.link.label","link#maltego.link.label",True,"Seen at:" + firstSeen)
	
me.returnOutput()
