#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "20";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.05";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 1;
	static const long BUILD = 168;
	static const long REVISION = 974;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 248;
	#define RC_FILEVERSION 1,1,168,974
	#define RC_FILEVERSION_STRING "1, 1, 168, 974\0"
	static const char FULLVERSION_STRING[] = "1.1.168.974";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 68;
	

#endif //VERSION_H
