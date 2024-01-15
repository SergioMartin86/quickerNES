#pragma once

/* Copyright (C) 2005-2006 Shay Green. Permission is hereby granted, free of
charge, to any person obtaining a copy of this software module and associated
documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the
following conditions: The above copyright notice and this permission notice
shall be included in all copies or substantial portions of the Software. THE
SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Abstract file access interfaces

#include "Data_Reader.h"
#include <cstring>

// Supports writing
class Data_Writer {
public:
	Data_Writer() { }
	virtual ~Data_Writer() { }
	
	// Write 'n' bytes. NULL on success, otherwise error string.
	virtual const char *write( const void*, long ) { return 0; }

private:
	// noncopyable
	Data_Writer( const Data_Writer& );
	Data_Writer& operator = ( const Data_Writer& );
};

// Write data to memory
class Mem_Writer : public Data_Writer {
	char* data_;
	size_t size_;
	size_t allocated;
	enum { expanding, fixed, ignore_excess } mode;
public:
	// Keep all written data in expanding block of memory
	Mem_Writer()
	{
		data_ = 0;
		size_ = 0;
		allocated = 0;
		mode = expanding;
  }

	// Write to fixed-size block of memory. If ignore_excess is false, returns
	// error if more than 'size' data is written, otherwise ignores any excess.
	Mem_Writer( void* p, long s, int b )
	{
		data_ = (char*) p;
		size_ = 0;
		allocated = s;
		mode = b ? ignore_excess : fixed;
	}

	const char * write( const void* p, long s )
	{
		long remain = allocated - size_;
		if ( s > remain )
		{
			if ( mode == fixed )
				return "Tried to write more data than expected";
			
			if ( mode == ignore_excess )
			{
				s = remain;
			}
			else // expanding
			{
				long new_allocated = size_ + s;
				new_allocated += (new_allocated >> 1) + 2048;
				void* p = realloc( data_, new_allocated );
				if ( !p )
					return "Out of memory";
				data_ = (char*) p;
				allocated = new_allocated;
			}
		}
		
		memcpy( data_ + size_, p, s );
		size_ += s;
		
		return 0;
	}
		
	// Pointer to beginning of written data
	char* data() { return data_; }
	
	// Number of bytes written
	size_t size() const { return size_; }
	
	~Mem_Writer()
	{
	 if ( ( mode == expanding ) && data_ ) free( data_ );
  }  
};

// Dry writer to get the state size
class Dry_Writer : public Data_Writer {
	long size_;

public:

	Dry_Writer()
	{
		size_ = 0;
	}

	~Dry_Writer()
	{
	}

	const char *write( const void* p, long s )
	{
		size_ += s;
		return 0;
	}

	
	// Pointer to beginning of written data
	char* data() { return NULL; }
	
	// Number of bytes written
	long size() const { return size_; }
};


// Auto_File to use in place of Data_Reader&/Data_Writer&, allowing a normal
// file path to be used in addition to a Data_Reader/Data_Writer.

class Auto_File_Reader {
public:
	Auto_File_Reader()                      : data(  0 ), path( 0 ) { }
	Auto_File_Reader( Data_Reader& r )      : data( &r ), path( 0 ) { }
	Auto_File_Reader( Auto_File_Reader const& );
	Auto_File_Reader& operator = ( Auto_File_Reader const& );

	const char* open()
	{
		return 0;
	}

	~Auto_File_Reader()
	{
		if ( path )
			delete data;
	}

	int operator ! () const { return !data; }
	Data_Reader* operator -> () const { return  data; }
	Data_Reader& operator *  () const { return *data; }
private:
	/* mutable */ Data_Reader* data;
	const char* path;
};

class Auto_File_Writer {
public:
	Auto_File_Writer()                      : data(  0 ), path( 0 ) { }
	Auto_File_Writer( Data_Writer& r )      : data( &r ), path( 0 ) { }
	Auto_File_Writer( Auto_File_Writer const& );
	Auto_File_Writer& operator = ( Auto_File_Writer const& );
	
	~Auto_File_Writer()
	{
	}
	
	const char* open()
	{
		return 0;
	}

	const char* open_comp( int level = -1 )
	{
		return 0;
	}
	
	int operator ! () const { return !data; }
	Data_Writer* operator -> () const { return  data; }
	Data_Writer& operator *  () const { return *data; }
private:
	/* mutable */ Data_Writer* data;
	const char* path;
};

inline Auto_File_Reader& Auto_File_Reader::operator = ( Auto_File_Reader const& r )
{
	data = r.data;
	path = r.path;
	((Auto_File_Reader*) &r)->data = 0;
	return *this;
}
inline Auto_File_Reader::Auto_File_Reader( Auto_File_Reader const& r ) { *this = r; }

inline Auto_File_Writer& Auto_File_Writer::operator = ( Auto_File_Writer const& r )
{
	data = r.data;
	path = r.path;
	((Auto_File_Writer*) &r)->data = 0;
	return *this;
}
inline Auto_File_Writer::Auto_File_Writer( Auto_File_Writer const& r ) { *this = r; }
