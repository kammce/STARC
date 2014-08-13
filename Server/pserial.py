import sys
import json
import datetime

#NOTE: NEVER TESTED!!!

US_CHAR = 0x1F;

try:
	args = sys.argv[1]
	incomming = args[0].split(us+"",2);
	if(incomming[0] == "GetTime"):
		now = datetime.datetime.now()
		time = "%d:%d" % (now.hour, now.minute)
		print time
	elif(incomming[0] == "FoundPerson"):
		now = datetime.datetime.now()
		print "Cool Story Bro!"
except Exception, e:
	print "INVALID"
	exit()
	raise e

exit()