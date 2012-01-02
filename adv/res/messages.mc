;#ifndef __MESSAGES_H__
;#define __MESSAGES_H__
;


SeverityNames = (
	Success=0x0:STATUS_SEVERITY_SUCCESS
	Warning=0x1:STATUS_SEVERITY_WARNING
	Error=0x2:STATUS_SEVERITY_ERROR
)
			  
;////////////////////////////////////////
;// Eventlog categories
;//
;// These always have to be the first entries in a message file
;//

MessageId       = 1
SymbolicName    = CAT_ADV_DLL
Severity		= Success
Language        = English
adv.dll
.

;////////////////////////////////////////
;// Events: Status
;//

MessageId       = +1
SymbolicName    = STATUS_OK
Severity		= Success
Language        = English
%1: function call finished successfully.
.

MessageId       = +1
SymbolicName    = STATUS_WARNING
Severity		= Success
Language        = English
%1: function call finished with some problems.
Please check the event log for details.
.

MessageId       = +1
SymbolicName    = STATUS_FAIL
Severity		= Success
Language        = English
%1: function call failed after unrecoverable error(s).
Please check the event log for detailed error messages.
.

MessageId       = +1
SymbolicName    = STATUS_GENERIC
Severity		= Success
Language        = English
%1
.

MessageId       = +1
SymbolicName    = ERROR_UNKNOWN
Severity		= Error
Language        = English
%1: Unknown error occured during the function call.
.

;////////////////////////////////////////
;// Events: Information
;//

MessageId       = +1
SymbolicName    = INFO_STARTED
Severity		= Success
Language        = English
Adv.dll module started.
.

MessageId       = +1
SymbolicName    = INFO_CONFIG_SET
Severity		= Success
Language        = English
Configuration setting stored in registry: %1
.

MessageId       = +1
SymbolicName    = INFO_TIME
Severity		= Success
Language        = English
%1: function completed in %2 seconds.
.

;////////////////////////////////////////
;// Errors & Warnings
;//

MessageId       = +1
SymbolicName    = ERROR_GENERIC
Severity		= Error
Language        = English
%1
.

MessageId       = +1
SymbolicName    = WARNING_GENERIC
Severity		= Warning
Language        = English
%1
.

MessageId       = +1
SymbolicName    = ERROR_INTERNAL
Severity		= Error
Language        = English
Internal error: %1
.

MessageId       = +1
SymbolicName    = ERROR_COM
Severity		= Error
Language        = English
COM system error.%n
COM Error Message: %1
.

MessageId       = +1
SymbolicName    = WARNING_DEPRECATED
Severity		= Warning
Language        = English
Function %1 is deprecated.
.

MessageId       = +1
SymbolicName    = ERROR_FILE_XML
Severity		= Error
Language        = English
SIM file not found or corrupt.%n
File path: %2%n
COM Error Message: %1
.

MessageId       = +1
SymbolicName    = ERROR_FILE_NOTFOUND
Severity		= Error
Language        = English
SIM file not found or corrupt.%n
File path: %1
.

MessageId       = +1
SymbolicName    = ERROR_FILE_READ
Severity		= Error
Language        = English
An error occured while reading from SIM file.%n
File path: %1%n
This file may be incomplete or corrupt.
.

MessageId       = +1
SymbolicName    = ERROR_FILE_FORMAT
Severity		= Error
Language        = English
The file provided has not been recognised as the SIM file format.%n
File path: %1
.

MessageId       = +1
SymbolicName    = ERROR_FILE_VERSION
Severity		= Error
Language        = English
Unsupported SIM file version.%n
File path: %1
.

MessageId       = +1
SymbolicName    = ERROR_FILE_DECKS
Severity		= Error
Language        = English
Wrong number of decks found in SIM file. Only single and double deckers are supported.%n
File path: %1
.

MessageId       = +1
SymbolicName    = ERROR_PROJECT
Severity		= Error
Language        = English
Corrupt project information.
It may be caused by missing, improper or corrupt data,
or by database version mismatch. This error concerns the Advisuo_Console database.
.

MessageId       = +1
SymbolicName    = ERROR_BUILDING
Severity		= Error
Language        = English
Corrupt building structure.
It may be caused by missing, improper or corrupt data,
database version mismatch,
as well as lack of lift shafts or storeys defined in the building.
This error concerns the Advisuo_Console database.
.

MessageId       = +1
SymbolicName    = ERROR_DATA_NOT_FOUND
Severity		= Error
Language        = English
Data not found.
It may be caused by missing, improper or corrupt data,
database version mismatch,
or may be a consequence of prior errors.
This error concerns the Advisuo_Visualisation database.
.

MessageId       = +1
SymbolicName    = ERROR_UNRECOGNISED_STRING
Severity		= Error
Language        = English
Unrecognised %1: %2.
.

MessageId       = +1
SymbolicName    = ERROR_DBCONN
Severity		= Error
Language        = English
Database connection problem.%n
Connection string: %1
.

MessageId       = +1
SymbolicName    = ERROR_DB
Severity		= Error
Language        = English
Database error: %1%n
Message: %2%n
Source: %3%n
Description: %4
.

MessageId       = +1
SymbolicName    = ERROR_FILE_INCONSISTENT_FLOORS
Severity		= Error
Language        = English
SIM file data inconsistent with the stored building structure: number or parameters of floors vary
.

MessageId       = +1
SymbolicName    = ERROR_FILE_INCONSISTENT_LIFTS
Severity		= Error
Language        = English
SIM file data inconsistent with the stored building structure: number or parameters of lifts vary
.

MessageId       = +1
SymbolicName    = ERROR_FILE_INCONSISTENT_DECKS
Severity		= Error
Language        = English
SIM file data inconsistent with the stored building structure: number lift decks vary
.

MessageId       = +1
SymbolicName    = WARNING_PASSENGER_FLOOR
Severity		= Warning
Language        = English
Passenger getting %1 on a wrong floor.%n
Passenger Id:%2%n
Time: %3%n
Floor: %4%n
Lift: %5%n
Deck: %6
.

MessageId       = +1
SymbolicName    = ERROR_PASSENGER_FLOOR
Severity		= Error
Language        = English
Passenger getting %1 on a wrong floor.%n
Passenger Id:%2%n
Time: %3%n
Floor: %4%n
Lift: %5%n
This event would impose non existing deck number: %6
.

MessageId       = +1
SymbolicName    = ERROR_IFC_PRJ
Severity		= Error
Language        = English
Error creating IFC file: cannot build a %1.
.

MessageId       = +1
SymbolicName    = WARNING_PASSENGER_1
Severity		= Warning
Language        = English
Passenger getting %1 while door opening.%n
Passenger Id:%2%n
Time: %3%n
Floor: %4%n
Lift: %5%n
Deck: %6
.

MessageId       = +1
SymbolicName    = WARNING_PASSENGER_2
Severity		= Warning
Language        = English
Passenger getting %1 before door fully open.%n
Passenger Id:%2%n
Time: %3%n
Floor: %4%n
Lift: %5%n
Deck: %6
.

MessageId       = +1
SymbolicName    = WARNING_PASSENGER_3
Severity		= Warning
Language        = English
Passenger getting %1 while door closing.%n
Passenger Id:%2%n
Time: %3%n
Floor: %4%n
Lift: %5%n
Deck: %6
.

MessageId       = +1
SymbolicName    = WARNING_PASSENGER_4
Severity		= Warning
Language        = English
Passenger getting %1 after door closed.%n
Passenger Id:%2%n
Time: %3%n
Floor: %4%n
Lift: %5%n
Deck: %6
.

MessageId       = +1
SymbolicName    = ERROR_CONVERSION
Severity		= Error
Language        = English
Bad data conversion.
.

;
;#endif  //__MESSAGES_H__
;
