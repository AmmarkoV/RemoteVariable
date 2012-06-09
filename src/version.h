#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "09";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.06";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 10;
	static const long BUILD = 1032;
	static const long REVISION = 5715;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1282;
	#define RC_FILEVERSION 1,10,1032,5715
	#define RC_FILEVERSION_STRING "1, 10, 1032, 5715\0"
	static const char FULLVERSION_STRING[] = "1.10.1032.5715";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 32;
	

#endif //VERSION_H
