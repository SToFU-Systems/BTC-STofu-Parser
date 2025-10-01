#pragma once

// Standard Library headers
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

// Project headers
#include "Common.h"


//================================================================================
// Class: FileReader
// Description: std::ifstream wrapper for reading bitcoin *.dat files.
//================================================================================
class FileReader
{
public:
    //================================================================================
    // Method: FileReader
    // Description: Constructor with file path input for reading Bitcoin block data
    //              files. Verifies that the input path points to a file with the
    //              expected ".dat" extension. Opens the file and stores its size.
    //================================================================================
    explicit FileReader(IN const std::wstring& path);


    /* Deleted */
    FileReader(const FileReader&) = delete;
    FileReader(FileReader&&) noexcept = delete;
    FileReader& operator=(const FileReader&) = delete;
    FileReader& operator=(FileReader&&) noexcept = delete;


    //================================================================================
    // Method: ~FileReader
    // Description: Destructor to close the file.
    //================================================================================
    ~FileReader();


    //================================================================================
    // Method: ReadBytes
    // Description: Reads a specified number of bytes from the file into the output
    //              buffer. Handles possible errros from the reading.
    //================================================================================
    [[nodiscard]] bool readBytes(IN const size_t bytes, OUT std::vector<uint8_t>& out);


    //================================================================================
    // Method: IsEof
    // Description: Checks whether the file stream has reached the end of file. 
    //              Returns true if EOF encountered, and false otherwise
    //================================================================================
    [[nodiscard]] bool isEof();


    //================================================================================
    // Method: SetReadPosition
    // Description: Moves the file read pointer to the specified absolute position via
    //              seekg.
    //================================================================================
    [[nodiscard]] bool setReadPosition(IN size_t position);


    //================================================================================
    // Method: GetReadPosition
    // Description: Returns the current absolute position of the read pointer in the
    //              file via tellg.
    //================================================================================
    size_t getReadPosition();


    //================================================================================
    // Method: GetFileSize
    // Description: Returns the total size of the m_file in bytes.
    //================================================================================
    size_t getFileSize() const;

private:
    const size_t kFileSize;
    std::ifstream m_file;
};
