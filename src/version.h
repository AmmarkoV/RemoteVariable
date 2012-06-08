#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "08";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.06";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 9;
	static const long BUILD = 987;
	static const long REVISION = 5481;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1221;
	#define RC_FILEVERSION 1,9,987,5481
	#define RC_FILEVERSION_STRING "1, 9, 987, 5481\0"
	static const char FULLVERSION_STRING[] = "1.9.987.5481";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 87;
	

#endif //VERSION_H
