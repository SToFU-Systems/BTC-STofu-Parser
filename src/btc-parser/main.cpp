/* TODO
* 1. Parser features
*   - ScriptPubKey tokenization for Receiver public address
* 2. Refactoring & Optimizations
*   - (!) Parsing takes too much memory. Find out why.
*   - Utils: All LeUint8To<X> have identical loop, only difference is static_cast type.
*   - ReadBytes output type: change from vector to array (?)
*   - FileReader: assert (bytes == 0) (?)
*   - Most methods that return bool and output as argument can be changed to return std::optional<output>
*/

// Precompiled headers
#include "pch.h"

// Project headers
#include "BlockParser.h"
#include "Common.h"
#include "Config.h"
#include "ConsoleBlockPrinter.h"
#include "DataTypes.h"
#include "Utils.h"

Config config;

int wmain(int argc, wchar_t* argv[])
{
    // argv : <json_path>
    const bool kConfigInitResult = config.init(argc, argv);
    if (!kConfigInitResult)
        return static_cast<int>(ReturnTypes::COMMAND_ARGUMENTS_ERROR);

    const std::wstring kBlockDirectory = config.getBlockPath();
    const std::wstring kXorPath = config.getXorPath();

    try
    {
        std::vector<std::wstring> blocksPath = getBlockFilesInDirectory(kBlockDirectory);

        for (const std::wstring& kBlockPath : blocksPath)
        {
            std::wcout << "File: " << kBlockPath << "\n";
            BlockParser parser(kBlockPath, kXorPath);
            std::vector<Block> blocks;
            const bool kParseResult = parser.parseAllBlocks(blocks);
            if (!kParseResult)
            {
                std::cout << "[Block parsing failed. Aborting...]";
                return static_cast<int>(ReturnTypes::BLOCK_PARSING_FAILURE);
            }
            
            for (const Block& block : blocks)
            {
                std::cout << "\n===Block===\n";
                ConsoleBlockPrinter printer(block, std::cout);
                printer.printBlock();
            }
            std::cout << "\n";
        }
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "[Runtime] " << e.what() << "\n";
        return static_cast<int>(ReturnTypes::RUNTIME_EXCEPTION);
    }
    catch(const std::exception& e)
    {
        std::cout << "[Exception] " << e.what() << "\n";
        return static_cast<int>(ReturnTypes::EXCEPTION);
    }

    return static_cast<int>(ReturnTypes::SUCCESS);
}
