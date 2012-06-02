#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "02";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.06";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 6;
	static const long BUILD = 615;
	static const long REVISION = 3400;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 786;
	#define RC_FILEVERSION 1,6,615,3400
	#define RC_FILEVERSION_STRING "1, 6, 615, 3400\0"
	static const char FULLVERSION_STRING[] = "1.6.615.3400";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 15;
	

#endif //VERSION_H
