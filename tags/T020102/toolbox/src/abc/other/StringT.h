/* $Id: StringT.h,v 1.10 2002-01-27 18:20:10 paklein Exp $ */
/* created: paklein (08/01/1996)                                          */

#ifndef _STRING_T_H_
#define _STRING_T_H_

/* Environmental */
#include "Environment.h"

/* base class */
#include "ArrayT.h"

/* forward declarations */
#include "ios_fwd_decl.h"

class StringT: public ArrayT<char>
{
public:

	/** constructors */
	StringT(void);
	StringT(const StringT& string);
	StringT(const char* string);
	explicit StringT(int length);
	
	/** type conversion operator - allows use of StringT in all const char*
	 * ANSI C functions. */
	operator const char*() const;
	operator char*() const;
	
	/** input initializer */
	friend istream& operator>>(istream& in, StringT& string);
	friend ostream& operator<<(ostream& out, const StringT& string);

	/** assignment operator */
	StringT& operator=(const char* string);
	StringT& operator=(const StringT& string);

	/** copy what fits into the current string length. returns new string length */
	int CopyIn(const char* string);

	/** make string empty */
	void Clear(void);

	/** return the string length. the string length is the number of characters
	 * before the first '\0' character, i.e., the ANSI C strlen() function */
	int StringLength(void) const;

	/** equality operators */
	int operator==(const StringT& rhs) const;
	int operator==(const char* string) const;
	int operator==(char* string) const { return operator==((const char *) string); };
	friend int operator==(const char* str_lhs, const StringT& str_rhs);

	/** inequality operators */
	int operator!=(const StringT& rhs) const;
	int operator!=(const char* string) const;
	int operator!=(char* string) const { return operator!=((const char*) string); };
	friend int operator!=(const char* str_lhs, const StringT& str_rhs);

	/** convert all to uppercase */
	const StringT& ToUpper(void);

	/** read a line from the input stream, where a line is the next
	 * kFileNameLength characters or fewer characters terminated
	 * by a newline. */
	void GetLineFromStream(istream& in);

	/** drop the last ".xxx" extension to the string */
	StringT& Root(char marker = '.');
	StringT& Root(const char* s, char marker = '.');
	
	/** returns the last ".xxx" extension to the string */
	StringT& Suffix(char marker = '.');
	StringT& Suffix(const char* s, char marker = '.');

	/** returns the path part of the full path to a file - drops the file
	 * from the full path to a file, keeping the directory separator */
	StringT& FilePath(void);
	StringT& FilePath(const char* s);

	/** append characters to the string */
	StringT& Append(const char* s);
	StringT& Append(const char* s1, const char* s2);
	StringT& Append(const char* s1, const char* s2, const char* s3);
	StringT& Append(char c);
	
	/** append an integer - width specifies the minimum number of digits
	 * that will be appended, padded by zeroes if number has fewer
	 * digits than width */
	StringT& Append(int number, int width = 0);
	StringT& Append(const char* s, int number, int width = 0);

	/** insert characters at the beginning of the string */
	StringT& Prepend(const char* s);
	StringT& Prepend(const char* s1, const char* s2);
	
	/** drop n characters from the string from the start (n > 0) or
	 * from the end (n < 0) */
	StringT& Drop(int n);

	/** delete characters from the string from start to end, inclusive. */
	StringT& Delete(int start, int end);

	/** delete the specified character from the string. */
	StringT& Delete(int position) { return Delete(position, position); };
	
	/** take n characters from the source from the start (n > 0) or
	 * from the end (n < 0) */
	StringT& Take(const StringT& source, int n);

	/** copy a section of the source string */
	StringT& Take(const StringT& source, int start, int end);
	
	/** copy the first word from the source and return number of characters scanned */
	StringT& FirstWord(const StringT& source, int& count, bool C_word_only);

	/** drop leading white space */
	StringT& DropLeadingSpace(void);

	/** drop trailing white space */
	StringT& DropTrailingSpace(void);

	/** convert string to native, relative file path */
	void ToNativePathName(void);
	void ToMacOSPath(void);			
	void ToWinNTPath(void);			
	void ToUNIXPath(void);
	static char DirectorySeparator(void);	

	/** print ASCII codes */
	void PrintCodes(ostream& out) const;

	/** version number comparison - returns 0 if the versions numbers are
	 * the same, -1 if v1 is older than v2, 1 if v1 is newer than v2 */
	static int versioncmp(const char* v1, const char* v2);
	
	/** extract double. perform type conversion on the tail of the string
	 * \param key last character before the start of the tail
	 * \param value conversion of tail, returns 0.0 if key not found
	 * \return true if key found, else returns false */
	bool Tail(char key, double& value) const;

	/** extract integer. perform type conversion on the tail of the string
	 * \param key last character before the start of the tail
	 * \param value conversion of tail, returns 0 if key not found
	 * \return true if key found, else returns false */
	bool Tail(char key, int& value) const;

private:
	
	/** to NT or UNIX path - from and to are the delimiting characters */
	void ToNTorUNIX(char from, char to);

	/** deep copy of string */
	void CopyString(const char* string);
	
	/** returns the character string corresponding to the number */
	void IntegerToString(int number, char* string) const;
};

/* inlines */

/* constructors */
inline StringT::StringT(void) { operator=("\0"); }
inline StringT::StringT(const StringT& string): ArrayT<char>(string) { }
inline StringT::StringT(const char* string) { operator=(string); }

/* type conversion operator - allows use of StringT in all const char*
* ANSI C functions */
inline StringT::operator const char*() const { return Pointer(); }
inline StringT::operator char*() const { return Pointer(); }

/* assignment operator */
inline StringT& StringT::operator=(const StringT& string)
{
	return operator=(string.Pointer());
}

/* equality operator */
inline int StringT::operator==(const StringT& rhs) const
{
	return operator==(rhs.Pointer());
}

inline int StringT::operator!=(const StringT& rhs) const
{
	return operator!=(rhs.Pointer());
}

/* string length */
inline int StringT::StringLength(void) const { return strlen(*this); }

inline char StringT::DirectorySeparator(void)
{
#ifdef _MACOS_
#ifdef __MACH__
	return '/';
#else
	return ':';
#endif
#elif defined(_WINNT_)
	return '\\';
#endif

	/* UNIX by default */
	return '/';
}

#endif /* _STRING_T_H_ */