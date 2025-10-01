// Precompiled headers
#include "pch.h"

// Project headers
#include "FileReader.h"


//================================================================================
// Method: FileReader
// Description: Constructor with file path input for reading Bitcoin block data
//              files. Verifies that the input path points to a file with the
//              expected ".dat" extension. Opens the file and stores its size.
//================================================================================
FileReader::FileReader(IN const std::wstring& path)
    : kFileSize(std::filesystem::file_size(path))
{
    const std::filesystem::path kBlockPath = std::filesystem::path(path);
    const std::filesystem::path kBlockExtension = kBlockPath.extension();
    constexpr const char* kExpectedExtension = ".dat";

    if (kBlockExtension != kExpectedExtension)
        throw std::runtime_error("Input file must be *.dat. Instead received: " + kBlockExtension.string());

    m_file.open(path, std::ios::binary);
    if (!m_file.is_open())
        throw std::runtime_error("Failed to open the: " + kBlockPath.string());
}

//================================================================================
// Method: ~FileReader
// Description: Destructor to close the file.
//================================================================================
FileReader::~FileReader()
{
    if (m_file.is_open())
        m_file.close();
}

//================================================================================
// Method: ReadBytes
// Description: Reads a specified number of bytes from the file into the output
//              buffer. Handles possible errros from the reading.
//================================================================================
bool FileReader::readBytes(IN const size_t bytes, OUT std::vector<uint8_t>& out)
{
    if (bytes == 0)
        return true;

    out.clear();
    out.resize(bytes);
    out.shrink_to_fit();
    m_file.read(reinterpret_cast<char*>(out.data()), bytes);

    const auto readCount = m_file.gcount();
    if (readCount == bytes)
        return true;

    const bool kIsEof = m_file.eof();
    if (kIsEof)
    {
        out.resize(readCount);
        std::vector<uint8_t>(out).swap(out);
        return true;
    }

    return false;
}

//================================================================================
// Method: IsEof
// Description: Checks whether the file stream has reached the end of file. 
//              Returns true if EOF encountered, and false otherwise
//================================================================================
bool FileReader::isEof()
{
    const bool kIsEof = m_file.eof();
    const bool kIsNextEof = m_file.peek() == EOF;
    return kIsEof || kIsNextEof;
}

//================================================================================
// Method: SetReadPosition
// Description: Moves the file read pointer to the specified absolute position via
//              seekg.
//================================================================================
bool FileReader::setReadPosition(IN size_t position)
{
    return m_file.seekg(position) ? true : false;
}

//================================================================================
// Method: GetReadPosition
// Description: Returns the current absolute position of the read pointer in the
//              file via tellg.
//================================================================================
size_t FileReader::getReadPosition()
{
    return m_file.tellg();
}

//================================================================================
// Method: GetFileSize
// Description: Returns the total size of the m_file in bytes.
//================================================================================
size_t FileReader::getFileSize() const
{
    return kFileSize;
}
