#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "31";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.05";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 4;
	static const long BUILD = 452;
	static const long REVISION = 2546;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 601;
	#define RC_FILEVERSION 1,4,452,2546
	#define RC_FILEVERSION_STRING "1, 4, 452, 2546\0"
	static const char FULLVERSION_STRING[] = "1.4.452.2546";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 52;
	

#endif //VERSION_H
