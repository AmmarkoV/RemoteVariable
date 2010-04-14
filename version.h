#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "12";
	static const char MONTH[] = "04";
	static const char YEAR[] = "2010";
	static const double UBUNTU_VERSION_STYLE = 10.04;
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 51;
	static const long REVISION = 295;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 93;
	#define RC_FILEVERSION 1,0,51,295
	#define RC_FILEVERSION_STRING "1, 0, 51, 295\0"
	static const char FULLVERSION_STRING[] = "1.0.51.295";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 51;
	

#endif //VERSION_h
