#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "13";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.06";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 0;
	static const long BUILD = 1122;
	static const long REVISION = 6224;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1402;
	#define RC_FILEVERSION 2,0,1122,6224
	#define RC_FILEVERSION_STRING "2, 0, 1122, 6224\0"
	static const char FULLVERSION_STRING[] = "2.0.1122.6224";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 22;
	

#endif //VERSION_H
