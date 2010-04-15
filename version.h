#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "15";
	static const char MONTH[] = "04";
	static const char YEAR[] = "2010";
	static const double UBUNTU_VERSION_STYLE = 10.04;
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 63;
	static const long REVISION = 357;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 106;
	#define RC_FILEVERSION 1,0,63,357
	#define RC_FILEVERSION_STRING "1, 0, 63, 357\0"
	static const char FULLVERSION_STRING[] = "1.0.63.357";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 63;
	

#endif //VERSION_h
