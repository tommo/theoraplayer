/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#include <stdio.h>
#include <sys/stat.h>
#include <memory.h>
#include "TheoraDataSource.h"
#include "TheoraException.h"
#include "TheoraVideoManager.h"
#include "TheoraUtil.h"

TheoraDataSource::~TheoraDataSource()
{

}

TheoraFileDataSource::TheoraFileDataSource(std::string filename)
{
	mFilename = filename;
	mFilePtr = NULL;
}

TheoraFileDataSource::~TheoraFileDataSource()
{
	if (mFilePtr)
	{
		fclose(mFilePtr);
		mFilePtr = NULL;
	}
}

void TheoraFileDataSource::openFile()
{
	if (mFilePtr == NULL)
	{
		mFilePtr = fopen(mFilename.c_str(), "rb");
		if (!mFilePtr)
        {
            std::string msg = "Can't open video file: " + mFilename;
            th_writelog(msg);
            throw TheoraGenericException(msg);
        }
		struct stat s;
		fstat(fileno(mFilePtr), &s);
		mSize = (uint64_t) s.st_size;
	}
}

int TheoraFileDataSource::read(void* output, int nBytes)
{
	if (mFilePtr == NULL) openFile();
	uint64_t n = fread(output, 1, nBytes, mFilePtr);
	return (int) n;
}

void TheoraFileDataSource::seek(uint64_t byte_index)
{
	if (mFilePtr == NULL) openFile();
	fpos_t fpos = byte_index;
	fsetpos(mFilePtr, &fpos);
}

uint64_t TheoraFileDataSource::size()
{
	if (mFilePtr == NULL) openFile();
	return mSize;
}

uint64_t TheoraFileDataSource::tell()
{
	if (mFilePtr == NULL) return 0;
	fpos_t pos;
	fgetpos(mFilePtr, &pos);
	return (uint64_t) pos;
}

TheoraMemoryFileDataSource::TheoraMemoryFileDataSource(std::string filename) :
	mReadPointer(0),
	mData(0)
{
	mFilename=filename;
	FILE* f = fopen(filename.c_str(),"rb");
	if (!f) throw TheoraGenericException("Can't open video file: "+filename);
	struct stat s;
	fstat(fileno(f), &s);
	mSize = (uint64_t) s.st_size;
	mData = new unsigned char[mSize];
	if (mSize < UINT_MAX)
	{
		fread(mData, 1, (size_t) mSize, f);
	}
	else
	{
		for (uint64_t offset = 0; offset < mSize; offset += UINT_MAX)
		{
			if (mSize - offset >= UINT_MAX)
			{
				fread(mData + offset, 1, UINT_MAX, f);
			}
			else
			{
				fread(mData + offset, 1, (size_t) (mSize - offset), f);
			}
		}
	}
	fclose(f);
}

TheoraMemoryFileDataSource::TheoraMemoryFileDataSource(unsigned char* data, long size, const std::string& filename)
{
	mFilename = filename;
	mData = data;
	mSize = size;
	mReadPointer = 0;
}

TheoraMemoryFileDataSource::~TheoraMemoryFileDataSource()
{
	if (mData) delete [] mData;
}

int TheoraMemoryFileDataSource::read(void* output, int nBytes)
{
	int n = (int) ((mReadPointer+nBytes <= mSize) ? nBytes : mSize - mReadPointer);
	if (!n) return 0;
	memcpy(output, mData + mReadPointer, n);
	mReadPointer += n;
	return n;
}

void TheoraMemoryFileDataSource::seek(uint64_t byte_index)
{
	mReadPointer = byte_index;
}

uint64_t TheoraMemoryFileDataSource::size()
{
	return mSize;
}

uint64_t TheoraMemoryFileDataSource::tell()
{
	return mReadPointer;
}
