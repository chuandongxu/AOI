// Copyright (C) 2010, SafeNet, Inc. All rights reserved.

#include "dog_api_cpp.h"
#include "errorprinter.h"
#include <iostream>
#include <QDebug>

using namespace std;

ErrorPrinter::ErrorPrinter()
{
	errorMap.insert(StatusMap::value_type(DOG_STATUS_OK, "Operation completed successfully"));
	errorMap.insert(StatusMap::value_type(DOG_MEM_RANGE, "Invalid memory address"));
	errorMap.insert(StatusMap::value_type(DOG_INSUF_MEM, "Memory allocation failed"));
	errorMap.insert(StatusMap::value_type(DOG_TMOF, "Too many open Features"));
	errorMap.insert(StatusMap::value_type(DOG_ACCESS_DENIED, "Access denied"));
	errorMap.insert(StatusMap::value_type(DOG_NOT_FOUND, "SuperDog not found"));
	errorMap.insert(StatusMap::value_type(DOG_TOO_SHORT, "Encryption/decryption length too short"));
	errorMap.insert(StatusMap::value_type(DOG_INV_HND, "Invalid handle"));
	errorMap.insert(StatusMap::value_type(DOG_INV_FILEID, "Invalid file ID / memory descriptor"));
	errorMap.insert(StatusMap::value_type(DOG_INV_FORMAT, "Unrecognized info format"));
	errorMap.insert(StatusMap::value_type(DOG_KEYID_NOT_FOUND, "SuperDog with specified ID was not found"));
	errorMap.insert(StatusMap::value_type(DOG_INV_UPDATE_DATA, "Update data consistency check failed"));
	errorMap.insert(StatusMap::value_type(DOG_INV_UPDATE_NOTSUPP, "Update not supported by SuperDog"));
	errorMap.insert(StatusMap::value_type(DOG_INV_UPDATE_CNTR, "Update counter mismatch"));
	errorMap.insert(StatusMap::value_type(DOG_INV_VCODE, "Invalid Vendor Code"));
	errorMap.insert(StatusMap::value_type(DOG_INV_TIME, "Invalid date/time"));
	errorMap.insert(StatusMap::value_type(DOG_NO_ACK_SPACE, "Acknowledge data requested by the update, however the ack_data input parameter is NULL"));
	errorMap.insert(StatusMap::value_type(DOG_TS_DETECTED, "Terminal services (remote terminal) detected"));
	errorMap.insert(StatusMap::value_type(DOG_UNKNOWN_ALG, "Unknown algorithm"));
	errorMap.insert(StatusMap::value_type(DOG_INV_SIG, "Signature check failed"));
	errorMap.insert(StatusMap::value_type(DOG_FEATURE_NOT_FOUND, "Feature not found" ));
	errorMap.insert(StatusMap::value_type(DOG_LOCAL_COMM_ERR, "Communication error between API and local SuperDog License Manager"));
	errorMap.insert(StatusMap::value_type(DOG_UNKNOWN_VCODE, "Vendor Code not recognized by API)" ));
	errorMap.insert(StatusMap::value_type(DOG_INV_SPEC, "Invalid XML spec" ));
	errorMap.insert(StatusMap::value_type(DOG_INV_SCOPE, "Invalid XML scope" ));
	errorMap.insert(StatusMap::value_type(DOG_TOO_MANY_KEYS, "Too many SuperDog connected" ));
	errorMap.insert(StatusMap::value_type(DOG_BROKEN_SESSION, "Broken session" ));
	errorMap.insert(StatusMap::value_type(DOG_FEATURE_EXPIRED, "The feature is expired" ));
	errorMap.insert(StatusMap::value_type(DOG_OLD_LM, "SuperDog License Manager version too old" ));
	errorMap.insert(StatusMap::value_type(DOG_DEVICE_ERR, "USB error occurred when communicating with a SuperDog" ));
	errorMap.insert(StatusMap::value_type(DOG_TIME_ERR, "System time has been tampered" ));
	errorMap.insert(StatusMap::value_type(DOG_SCHAN_ERR, "Secure channel communication error" ));
	errorMap.insert(StatusMap::value_type(DOG_SCOPE_RESULTS_EMPTY, "No feature matching scope found" ));
	errorMap.insert(StatusMap::value_type(DOG_UPDATE_TOO_OLD, "Update was already installed" ));
	errorMap.insert(StatusMap::value_type(DOG_UPDATE_TOO_NEW, "Another update must be installed first" ));

	errorMap.insert(StatusMap::value_type(DOG_NO_API_DYLIB, "A required API dynamic library was not found" ));
	errorMap.insert(StatusMap::value_type(DOG_INV_API_DYLIB, "The found and assigned API dynamic library could not verified" ));
	errorMap.insert(StatusMap::value_type(DOG_INVALID_OBJECT, "Object incorrectly initialized" ));
	errorMap.insert(StatusMap::value_type(DOG_INVALID_PARAMETER, "Invalid function parameter" ));
	errorMap.insert(StatusMap::value_type(DOG_ALREADY_LOGGED_IN, "Logging in twice to the same object" ));
	errorMap.insert(StatusMap::value_type(DOG_ALREADY_LOGGED_OUT, "Logging out twice from the same object" ));
	
	errorMap.insert(StatusMap::value_type(DOG_OPERATION_FAILED, "Incorrect use of system or platform" ));

	errorMap.insert(StatusMap::value_type(DOG_NOT_IMPL, "Requested function not implemented" ));
	errorMap.insert(StatusMap::value_type(DOG_INT_ERR, "Internal API error" ));
}

void ErrorPrinter::printError(dogStatus status)
{
	StatusMap::const_iterator error = errorMap.find(status);
	if(error == errorMap.end())
		qDebug() <<"unknown error: " << status;
	else
		qDebug () << error->second;
}

const char* ErrorPrinter::getError(dogStatus status)
{
	StatusMap::const_iterator error = errorMap.find(status);
	if(error == errorMap.end())
		return "Unknown error";
	else
		return error->second;
}




 