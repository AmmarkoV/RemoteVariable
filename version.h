#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "06";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2010";
	static const double UBUNTU_VERSION_STYLE = 10.05;
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 82;
	static const long REVISION = 467;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 126;
	#define RC_FILEVERSION 1,0,82,467
	#define RC_FILEVERSION_STRING "1, 0, 82, 467\0"
	static const char FULLVERSION_STRING[] = "1.0.82.467";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 82;
	

#endif //VERSION_h
