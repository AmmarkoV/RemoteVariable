#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "05";
	static const char MONTH[] = "08";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.08";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 1;
	static const long BUILD = 1216;
	static const long REVISION = 6746;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1521;
	#define RC_FILEVERSION 2,1,1216,6746
	#define RC_FILEVERSION_STRING "2, 1, 1216, 6746\0"
	static const char FULLVERSION_STRING[] = "2.1.1216.6746";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 16;
	

#endif //VERSION_H
