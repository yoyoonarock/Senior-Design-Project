import smbus
import time
import MySQLdb

bus = smbus.SMBus(1)

db = MySQLdb.connect("localhost", "root", "ece445", "locks")
cursor = db.cursor()

# This is the address we setup in the Arduino Program
address = 0x04

while True:

	# Receive data from the Arduino to query
	UID = bus.read_byte_data(address, 4)
	lockstate = bus.read_byte(address)

	# Query database to see if the arduino is already serving a phone
	sql = "SELECT phoneID FROM lockdat WHERE lockID = '%s'" % (address)
	cursor.execute(sql)
	dbUID = cursor.fetchone()

	if dbUID: # This means the lock is in use.

		if (dbUID == UID): # This means the phone matches.

			# Update the database with UNLOCKED; wipe the phoneID
			sql = "UPDATE lockdat SET phoneID = 0, lockState = 0 WHERE lockID = '%s'" % (address)
			cursor.execute()

			# Send a signal back to the Arduino
			bus.write_byte(address, 1)

		else: ### This means the lock is being used by another phone.
			
			# Send a signal back to the Arduino
			bus.write_byte(address, 0)

	else: # This means the lock is not in use. Update the database with locked bike

		# Update the database with the phoneID and LOCKED
		sql = """UPDATE lockdat SET phoneID = '%s', lockState = 1 WHERE lockID = '%s'""" % (UID, address)
		cursor.execute()

		# Send a signal back to the Arduino
		bus.write_byte(address, 1)