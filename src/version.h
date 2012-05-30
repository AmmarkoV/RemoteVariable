#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "30";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.05";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 4;
	static const long BUILD = 412;
	static const long REVISION = 2314;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 549;
	#define RC_FILEVERSION 1,4,412,2314
	#define RC_FILEVERSION_STRING "1, 4, 412, 2314\0"
	static const char FULLVERSION_STRING[] = "1.4.412.2314";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 12;
	

#endif //VERSION_H
