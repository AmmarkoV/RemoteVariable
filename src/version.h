#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "04";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.06";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 6;
	static const long BUILD = 685;
	static const long REVISION = 3793;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 876;
	#define RC_FILEVERSION 1,6,685,3793
	#define RC_FILEVERSION_STRING "1, 6, 685, 3793\0"
	static const char FULLVERSION_STRING[] = "1.6.685.3793";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 85;
	

#endif //VERSION_H
