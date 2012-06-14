#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "15";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.06";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 0;
	static const long BUILD = 1163;
	static const long REVISION = 6454;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1461;
	#define RC_FILEVERSION 2,0,1163,6454
	#define RC_FILEVERSION_STRING "2, 0, 1163, 6454\0"
	static const char FULLVERSION_STRING[] = "2.0.1163.6454";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 63;
	

#endif //VERSION_H
