#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "14";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2010";
	static const double UBUNTU_VERSION_STYLE = 10.05;
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 1;
	static const long BUILD = 120;
	static const long REVISION = 667;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 169;
	#define RC_FILEVERSION 1,1,120,667
	#define RC_FILEVERSION_STRING "1, 1, 120, 667\0"
	static const char FULLVERSION_STRING[] = "1.1.120.667";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 20;
	

#endif //VERSION_h
