/* $Id: main.cpp,v 1.2 2002-06-03 22:20:43 jzimmer Exp $ */
#include <iostream>
#include "ExceptionCodes.h"
#include "PeriodicTableT.h"
#include "StringT.h"

int main()
{

	try {
		cout << "Starting up Bravais program..." << endl;
		PeriodicTableT BobTable;
		cout << BobTable[1].GetName() << "\n";

	}	
	catch (int ErrorCode) {
		cout << "\n\n Exiting due to error . . . ";
		switch (ErrorCode) {
			case eBadInputValue:
				cout << " Bad Input Value\n";
				break;
			case eOutOfRange:
				cout << " Out of Range\n";
				break;
			case eSizeMismatch:
				cout << " Size Mismatch\n";
				break;
			case eOutOfMemory:
				cout << " Out of Memory\n";
				break;
			case eDatabaseFail:
				cout << " Error with database\n";
				break;
		}
		cout << "\n\n Game Over\n\n";
	}

	cout << "Exiting Bravais program.\n";

	return 0;
}	
