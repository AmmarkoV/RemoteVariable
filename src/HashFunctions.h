#ifndef HASHFUNCTIONS_H_INCLUDED
#define HASHFUNCTIONS_H_INCLUDED

unsigned long hash(unsigned char *str);
unsigned long sdbm(unsigned char *str);
unsigned long hash1(unsigned char *str);

unsigned long rvhash(void *str , unsigned int bytescount);

#endif // HASHFUNCTIONS_H_INCLUDED
