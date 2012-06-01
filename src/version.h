#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "02";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.06";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 5;
	static const long BUILD = 526;
	static const long REVISION = 2920;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 684;
	#define RC_FILEVERSION 1,5,526,2920
	#define RC_FILEVERSION_STRING "1, 5, 526, 2920\0"
	static const char FULLVERSION_STRING[] = "1.5.526.2920";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 26;
	

#endif //VERSION_H
