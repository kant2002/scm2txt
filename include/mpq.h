#include <StormLib.h>
#if USE_BOOST_STREAMS
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#endif

#if USE_BOOST_STREAMS
class mpq_file_source {
public:
	typedef char        char_type;
	typedef boost::iostreams::seekable_device_tag  category;

	mpq_file_source(HANDLE hArchive, const char* fileName)
		: hFile(NULL)
	{
		if (!SFileOpenFileEx(hArchive, fileName, SFILE_OPEN_FROM_MPQ, &hFile))
		{
			hFile = NULL;
		}
	}

	void close()
	{
		if (hFile != NULL)
		{
			SFileCloseFile(hFile);
			hFile = NULL;
		}
	}

	bool isValid() const { return hFile != NULL; }

	std::streamsize read(char* s, std::streamsize n)
	{
		// Read up to n characters from the underlying data source
		// into the buffer s, returning the number of characters
		// read; return -1 to indicate EOF
		DWORD dwRead = 0;
		if (!SFileReadFile(hFile, s, n, &dwRead, NULL))
		{
			if (dwRead == 0)
			{
				return -1;
			}

			return dwRead;
		}

		return dwRead;
	}
	std::streamsize write(const char* s, std::streamsize n)
	{
		// Write up to n characters to the underlying 
		// data sink into the buffer s, returning the 
		// number of characters written
		return 0;
	}

	boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
	{
		// Seek to position off and return the new stream 
		// position. The argument way indicates how off is
		// interpretted:
		//    - std::ios_base::beg indicates an offset from the 
		//      sequence beginning 
		//    - std::ios_base::cur indicates an offset from the 
		//      current character position 
		//    - std::ios_base::end indicates an offset from the 
		//      sequence end 
		DWORD seekMethod = 0;
		if (way == std::ios_base::beg)
		{
			seekMethod = 0;
		}
		else if (way == std::ios_base::cur)
		{
			seekMethod = 1;
		}
		else if (way == std::ios_base::end)
		{
			seekMethod = 2;
		}

		int64_t position = static_cast<int64_t>(off);
		LONG hPosition = 0;
		return SFileSetFilePointer(hFile, static_cast<LONG>(position), &hPosition, seekMethod);
	}


	/* Other members */
private:
	HANDLE hFile;
};

typedef boost::iostreams::stream<mpq_file_source> mpq_file_stream;
#else
class mpq_file_streambuf : public std::streambuf {
public:
	mpq_file_streambuf(HANDLE hArchive, const char* fileName)
		: hFile(NULL)
	{
		if (!SFileOpenFileEx(hArchive, fileName, SFILE_OPEN_FROM_MPQ, &hFile))
		{
			hFile = NULL;
		}
	}

	bool isValid() const { return hFile != NULL; }

protected:
	virtual int underflow()
	{
		DWORD dwRead = 0;
		if (!SFileReadFile(hFile, &ch, 1, &dwRead, NULL))
		{
			if (dwRead == 0)
			{
				return traits_type::eof();
			}
		}

		setg(&ch, &ch, &ch + 1);
		return dwRead;
	}
	pos_type seekoff(off_type off,
		std::ios_base::seekdir dir,
		std::ios_base::openmode which = std::ios_base::in)
	{
		DWORD seekMethod = 0;
		if (dir == std::ios_base::beg)
		{
			seekMethod = 0;
		}
		else if (dir == std::ios_base::cur)
		{
			seekMethod = 1;
		}
		else if (dir == std::ios_base::end)
		{
			seekMethod = 2;
		}

		int64_t position = static_cast<int64_t>(off);
		LONG hPosition = 0;
		return SFileSetFilePointer(hFile, static_cast<LONG>(position), &hPosition, seekMethod);
	}

private:
	char ch;
	HANDLE hFile;
};

class mpq_file_istream : public std::istream {
public:
	mpq_file_istream(HANDLE hArchive, const char* fileName)
		: std::istream(&_buf), _buf(hArchive, fileName)
	{
	}

	mpq_file_streambuf* component()
	{
		return &_buf;
	}
private:
	mpq_file_streambuf _buf;
};

typedef mpq_file_istream mpq_file_stream;
#endif