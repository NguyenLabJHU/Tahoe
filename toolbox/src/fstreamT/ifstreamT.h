/* $Id: ifstreamT.h,v 1.9 2002-04-08 16:15:06 paklein Exp $ */
/* created: paklein (03/03/1999) */

#ifndef _IFSTREAM_T_H_
#define _IFSTREAM_T_H_

#include "Environment.h"

/* base class */
#include "ios_fwd_decl.h"
#include <fstream.h>
#include <stddef.h>

/* direct members */
#include "StringT.h"

/* should be defined by I/O */
#ifndef streamsize
#define streamsize int
#endif

/** input file stream with extended capabilities */
class ifstreamT: public ifstream
{
public:

	/** \name constructors */
	/*@{*/
	ifstreamT(void);
	ifstreamT(const char* file_name);
	ifstreamT(char marker);
	ifstreamT(char marker, const char* file_name);
	/*@}*/

	/** \name opening stream */
	/*@{*/
	void open(const char* file_name);
	int open(const char* prompt, const char* skipname,
		const char* defaultname = NULL);
	int is_open(void);
	/*@}*/
	
	/** close stream */
	void close(void);

	/** \name comment marker */
	/*@{*/
	void set_marker(char marker);
	void clear_marker(void);
	char comment_marker(void) const;
	int skip_comments(void) const;
	/*@}*/
	
	/** put a character back in the stream */
	istream& putback(char a);
	
	/** return the next character (skipping whitespace and comments)
	 * without removing it from the stream */
	char next_char(void);
	
	/** get the next line from stream. Ignores comment lines */
	istream& getline(char* s, streamsize n, char delimiter = '\n');
	
	/** \return the filename or NULL if no file is open */
	const char* filename(void) const;
	
	/** set file name string, but not change the stream */
	void set_filename(const char* name);

	/** adjusting stream position
	 * \return the actual number of rewound lines */
	int rewind(int num_lines = 1);

	/** advance to the end of the line (or next 255 characters) */
	void clear_line(void);

	/** advances passed comments */
	void do_skip_comments(void);

	/** extraction of streams */
	ifstreamT& operator>>(bool& a);

	/** stream search. read lines from the stream looking for key
	 * \param key search string
	 * \param line entire line from string containing key
	 * \return true if key found, else false */
	bool FindString(const char* key, StringT& line);

private:

	/** open stream with prompt
	 * \return 1 if successful */
	int OpenWithPrompt(const char* prompt, const char* skipname,
		const char* defaultname);
	
private:

	/** \name comment marker */
	/*@{*/
	int  fSkipComments;
	char fMarker;
	/*@}*/
	
	/** the filename */
	StringT fFileName;
};

/* inlines */
inline char ifstreamT::comment_marker(void) const { return fMarker; }
inline int ifstreamT::skip_comments(void) const { return fSkipComments; }
inline const char* ifstreamT::filename(void) const { return fFileName; }

inline void ifstreamT::set_marker(char marker)
{
	fSkipComments = 1;
	fMarker = marker;
}

inline void ifstreamT::clear_marker(void) { fSkipComments = 0; }

/* get the next line from stream. Ignores comment lines */
inline istream& ifstreamT::getline(char* s, streamsize n, char delimiter)
{
	/* advance */
	do_skip_comments();
		
	/* ANSI */
	ifstream::getline(s, n, delimiter);

	/* advance */
	do_skip_comments();

	return *this;
}

/* extraction operator */
template <class TYPE> ifstreamT& operator>>(ifstreamT& str, TYPE& data);
template <class TYPE>
ifstreamT& operator>>(ifstreamT& str, TYPE& data)
{
	/* advance */
	str.do_skip_comments();
		
	/* ANSI */
	ifstream& ifstr = str;
	ifstr >> data;

	/* advance */
	str.do_skip_comments();
	
	return str;
};

#endif /* _IFSTREAM_X_H_ */
