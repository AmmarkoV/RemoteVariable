#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "23";
	static const char MONTH[] = "03";
	static const char YEAR[] = "2010";
	static const double UBUNTU_VERSION_STYLE = 10.03;
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 5;
	static const long REVISION = 32;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 44;
	#define RC_FILEVERSION 1,0,5,32
	#define RC_FILEVERSION_STRING "1, 0, 5, 32\0"
	static const char FULLVERSION_STRING[] = "1.0.5.32";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 5;
	

#endif //VERSION_h
