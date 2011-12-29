#ifndef __MESSAGES_H__
#define __MESSAGES_H__

////////////////////////////////////////
// Eventlog categories
//
// These always have to be the first entries in a message file
//
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x1
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_ERROR            0x2


//
// MessageId: CAT_ADV_DLL
//
// MessageText:
//
// adv.dll
//
#define CAT_ADV_DLL                      0x00000001L

////////////////////////////////////////
// Events: Status
//
//
// MessageId: STATUS_OK
//
// MessageText:
//
// %1: function call finished successfully.
//
#define STATUS_OK                        0x00000002L

//
// MessageId: STATUS_WARNING
//
// MessageText:
//
// %1: function call finished with some problems.
// Please check the event log for details.
//
#define STATUS_WARNING                   0x00000003L

//
// MessageId: STATUS_FAIL
//
// MessageText:
//
// %1: function call failed after unrecoverable error(s).
// Please check the event log for detailed error messages.
//
#define STATUS_FAIL                      0x00000004L

//
// MessageId: STATUS_GENERIC
//
// MessageText:
//
// %1
//
#define STATUS_GENERIC                   0x00000005L

//
// MessageId: ERROR_UNKNOWN
//
// MessageText:
//
// %1: Unknown error occured during the function call.
//
#define ERROR_UNKNOWN                    0x80000006L

////////////////////////////////////////
// Events: Information
//
//
// MessageId: INFO_STARTED
//
// MessageText:
//
// Adv.dll module started.
//
#define INFO_STARTED                     0x00000007L

//
// MessageId: INFO_CONFIG_SET
//
// MessageText:
//
// Configuration setting stored in registry: %1
//
#define INFO_CONFIG_SET                  0x00000008L

//
// MessageId: INFO_TIME
//
// MessageText:
//
// %1: function completed in %2 seconds.
//
#define INFO_TIME                        0x00000009L

////////////////////////////////////////
// Errors & Warnings
//
//
// MessageId: ERROR_GENERIC
//
// MessageText:
//
// %1
//
#define ERROR_GENERIC                    0x8000000AL

//
// MessageId: WARNING_GENERIC
//
// MessageText:
//
// %1
//
#define WARNING_GENERIC                  0x4000000BL

//
// MessageId: ERROR_INTERNAL
//
// MessageText:
//
// Internal error: %1
//
#define ERROR_INTERNAL                   0x8000000CL

//
// MessageId: ERROR_COM
//
// MessageText:
//
// COM system error.%n
// COM Error Message: %1
//
#define ERROR_COM                        0x8000000DL

//
// MessageId: WARNING_DEPRECATED
//
// MessageText:
//
// Function %1 is deprecated.
//
#define WARNING_DEPRECATED               0x4000000EL

//
// MessageId: ERROR_FILE_XML
//
// MessageText:
//
// SIM file not found or corrupt.%n
// File path: %2%n
// COM Error Message: %1
//
#define ERROR_FILE_XML                   0x8000000FL

//
// MessageId: ERROR_FILE_NOTFOUND
//
// MessageText:
//
// SIM file not found or corrupt.%n
// File path: %1
//
#define ERROR_FILE_NOTFOUND              0x80000010L

//
// MessageId: ERROR_FILE_READ
//
// MessageText:
//
// An error occured while reading from SIM file.%n
// File path: %1%n
// This file may be incomplete or corrupt.
//
#define ERROR_FILE_READ                  0x80000011L

//
// MessageId: ERROR_FILE_FORMAT
//
// MessageText:
//
// The file provided has not been recognised as the SIM file format.%n
// File path: %1
//
#define ERROR_FILE_FORMAT                0x80000012L

//
// MessageId: ERROR_FILE_VERSION
//
// MessageText:
//
// Unsupported SIM file version.%n
// File path: %1
//
#define ERROR_FILE_VERSION               0x80000013L

//
// MessageId: ERROR_FILE_DECKS
//
// MessageText:
//
// Wrong number of decks found in SIM file. Only single and double deckers are supported.%n
// File path: %1
//
#define ERROR_FILE_DECKS                 0x80000014L

//
// MessageId: ERROR_PROJECT
//
// MessageText:
//
// Corrupt project information.
// It may be caused by missing, improper or corrupt data,
// or by database version mismatch. This error concerns the Advisuo_Console database.
//
#define ERROR_PROJECT                    0x80000015L

//
// MessageId: ERROR_BUILDING
//
// MessageText:
//
// Corrupt building structure.
// It may be caused by missing, improper or corrupt data,
// database version mismatch,
// as well as lack of lift shafts or storeys defined in the building.
// This error concerns the Advisuo_Console database.
//
#define ERROR_BUILDING                   0x80000016L

//
// MessageId: ERROR_DATA_NOT_FOUND
//
// MessageText:
//
// Data not found.
// It may be caused by missing, improper or corrupt data,
// database version mismatch,
// or may be a consequence of prior errors.
// This error concerns the Advisuo_Visualisation database.
//
#define ERROR_DATA_NOT_FOUND             0x80000017L

//
// MessageId: ERROR_UNRECOGNISED_STRING
//
// MessageText:
//
// Unrecognised %1: %2.
//
#define ERROR_UNRECOGNISED_STRING        0x80000018L

//
// MessageId: ERROR_DBCONN
//
// MessageText:
//
// Database connection problem.%n
// Connection string: %1
//
#define ERROR_DBCONN                     0x80000019L

//
// MessageId: ERROR_DB
//
// MessageText:
//
// Database error: %1%n
// Message: %2%n
// Source: %3%n
// Description: %4
//
#define ERROR_DB                         0x8000001AL

//
// MessageId: ERROR_FILE_INCONSISTENT_FLOORS
//
// MessageText:
//
// SIM file data inconsistent with the stored building structure: number or parameters of floors vary
//
#define ERROR_FILE_INCONSISTENT_FLOORS   0x8000001BL

//
// MessageId: ERROR_FILE_INCONSISTENT_LIFTS
//
// MessageText:
//
// SIM file data inconsistent with the stored building structure: number or parameters of lifts vary
//
#define ERROR_FILE_INCONSISTENT_LIFTS    0x8000001CL

//
// MessageId: ERROR_FILE_INCONSISTENT_DECKS
//
// MessageText:
//
// SIM file data inconsistent with the stored building structure: number lift decks vary
//
#define ERROR_FILE_INCONSISTENT_DECKS    0x8000001DL

//
// MessageId: WARNING_PASSENGER_FLOOR
//
// MessageText:
//
// Passenger getting %1 on a wrong floor.%n
// Passenger Id:%2%n
// Time: %3%n
// Floor: %4%n
// Lift: %5%n
// Deck: %6
//
#define WARNING_PASSENGER_FLOOR          0x4000001EL

//
// MessageId: ERROR_PASSENGER_FLOOR
//
// MessageText:
//
// Passenger getting %1 on a wrong floor.%n
// Passenger Id:%2%n
// Time: %3%n
// Floor: %4%n
// Lift: %5%n
// This event would impose non existing deck number: %6
//
#define ERROR_PASSENGER_FLOOR            0x8000001FL

//
// MessageId: ERROR_IFC_PRJ
//
// MessageText:
//
// Error creating IFC file: cannot build a %1.
//
#define ERROR_IFC_PRJ                    0x80000020L

//
// MessageId: WARNING_PASSENGER_1
//
// MessageText:
//
// Passenger getting %1 while door opening.%n
// Passenger Id:%2%n
// Time: %3%n
// Floor: %4%n
// Lift: %5%n
// Deck: %6
//
#define WARNING_PASSENGER_1              0x40000021L

//
// MessageId: WARNING_PASSENGER_2
//
// MessageText:
//
// Passenger getting %1 before door fully open.%n
// Passenger Id:%2%n
// Time: %3%n
// Floor: %4%n
// Lift: %5%n
// Deck: %6
//
#define WARNING_PASSENGER_2              0x40000022L

//
// MessageId: WARNING_PASSENGER_3
//
// MessageText:
//
// Passenger getting %1 while door closing.%n
// Passenger Id:%2%n
// Time: %3%n
// Floor: %4%n
// Lift: %5%n
// Deck: %6
//
#define WARNING_PASSENGER_3              0x40000023L

//
// MessageId: WARNING_PASSENGER_4
//
// MessageText:
//
// Passenger getting %1 after door closed.%n
// Passenger Id:%2%n
// Time: %3%n
// Floor: %4%n
// Lift: %5%n
// Deck: %6
//
#define WARNING_PASSENGER_4              0x40000024L


#endif  //__MESSAGES_H__

