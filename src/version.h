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
	static const long MINOR = 7;
	static const long BUILD = 714;
	static const long REVISION = 3955;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 909;
	#define RC_FILEVERSION 1,7,714,3955
	#define RC_FILEVERSION_STRING "1, 7, 714, 3955\0"
	static const char FULLVERSION_STRING[] = "1.7.714.3955";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 14;
	

#endif //VERSION_H
