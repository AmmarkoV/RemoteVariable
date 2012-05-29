#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "30";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.05";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 3;
	static const long BUILD = 379;
	static const long REVISION = 2136;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 515;
	#define RC_FILEVERSION 1,3,379,2136
	#define RC_FILEVERSION_STRING "1, 3, 379, 2136\0"
	static const char FULLVERSION_STRING[] = "1.3.379.2136";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 79;
	

#endif //VERSION_H
