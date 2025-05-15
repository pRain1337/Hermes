#include "Prints.h"

#include <iostream>
#include <map>
#include <string>

// Status codes used for communication between client and smm
std::map<uint64_t, std::string> hermes_statuscodes =
{
  { HERMES_STATUS_INVALID_COMMAND, "Invalid command parameter was passed"  },
  { HERMES_STATUS_INVALID_DATA_PTR, "Invalid data pointer was passed"  },
  { HERMES_STATUS_INVALID_RES_PTR, "Invalid result pointer was passed"  },
  { HERMES_STATUS_INVALID_IDENTIFIERS, "Invalid identifiers were passed"  },
  { HERMES_STATUS_FAIL_FIND_PROC, "Failed finding the process"  },
  { HERMES_STATUS_FAIL_FIND_DIRBASE, "Failed getting dirbase"  },
  { HERMES_STATUS_FAIL_FIND_MOD, "Failed finding the module"  },
  { HERMES_STATUS_FAIL_READ_MOD_BASE, "Failed reading module base"  },
  { HERMES_STATUS_FAIL_READ_MOD_SIZE, "Failed reading module size"  },
  { HERMES_STATUS_PROCNAME_TOO_LONG, "Process name size is too big"  },
  { HERMES_STATUS_MODNAME_TOO_LONG, "Module name size is too big"  },
  { HERMES_STATUS_FAIL_VIRT_READ, "Failed reading virtual address"  },
  { HERMES_STATUS_FAIL_VIRT_WRITE, "Failed writing virtual address"  },
  { HERMES_STATUS_INVALID_DATA_SOURCE, "Invalid data source was passed"  },
  { HERMES_STATUS_INVALID_DATA_DIRBASE, "Invalid data dirbase was passed"  },
  { HERMES_STATUS_INVALID_DATA_SIZE, "Invalid data size was passed"  },
  { HERMES_STATUS_INVALID_DATA_DEST, "Invalid data destination was passed"  },
  { HERMES_STATUS_REQ_TOO_LARGE, "Requested size is too big"  },
  { HERMES_STATUS_FAIL_SBUFF_VIRTW, "Failed reading source buffer for virtual write"  },
  { HERMES_STATUS_FAIL_ADDR_TRANSLATION, "Failed translating address"  },
  { HERMES_STATUS_FAIL_SBUFF_PHYSW, "Failed reading source buffer for physical write"  },
  { HERMES_STATUS_INVALID_MOD_BASE, "Invalid module base"  },
  { HERMES_STATUS_INVALID_MOD_SIZE, "Invalid module size"  },
  { HERMES_STATUS_FAIL_PHYS_READ, "Failed reading physical address"  },
  { HERMES_STATUS_FAIL_PHYS_WRITE, "Failed writing physical address"  }
};

/*
 *   Function for printing out the various supported commands.
 *   Asks for the following parameter:
 *       - dummy, can be left empty. Is just needed because all the other function in the
 *                function map have a parameter.
 *
*/
void printCommands(uint64_t dummy)
{
    std::cout << "Commands: " << std::endl;
    std::cout << " gd		- gets dirbase from the supplied process" << std::endl;
    std::cout << " gmd		- gets data from the supplied modulename" << std::endl;
    std::cout << " gm		- returns all the modules from the supplied process" << std::endl;
    std::cout << " vr		- reads the supplied virtual address" << std::endl;
    std::cout << " vw		- writes to supplied virtual address" << std::endl;
    std::cout << " pr		- reads the supplied physical address" << std::endl;
    std::cout << " pw		- writes to supplied physical address" << std::endl;
    std::cout << " vtp		- converts virtual to physical" << std::endl;
    std::cout << " dm		- dumps the supplied modulename / size" << std::endl;
    std::cout << " st		- run speed test" << std::endl;
    std::cout << " exit		- ends the Hermes process " << std::endl;
    std::cout << " help		- displays this again" << std::endl;
}

/*
 *   Function for printing out a memory buffer.
 *   Asks for the following two parameters:
 *       - buf, the actual buffer which should be printed out
 *       - size, the size of the buffer that should be printed out
 *
 *   The Output will be formatted nicely to make it easier to interpret.
 *
*/
void printBuffer(unsigned char *buf, int size)
{
    for (int i = 0; i < size; i += 0x10)
    {
        std::cout << std::endl << std::hex << std::setw(4) << std::setfill('0') << (int)i << ": ";

        for (int j = 0; j < 0x10; j++)
        {
            if (i + j < size)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(uint8_t)buf[i + j] << " ";
            }
            else
            {
                // Fill the rest with ??
                std::cout << "??" << " ";
            }
        }

        std::cout << " : ";

        for (int j = 0; j < 0x10; j++)
        {
            if (i + j < size)
            {
                if (buf[i + j] >= 0x21 && buf[i + j] <= 0x7e)
                {
                    std::cout << (buf[i + j]) << " ";
                }
            }
        }
    }
    std::cout << std::endl << std::endl;
}

/*
 *   Function for printing out the error message from the error code.
 *   Asks for the following parameter:
 *       - errorcode, the code which was returned by SMM
 *
 *   Unknown errors should not happen.
 *
*/
void printError(uint64_t errorcode)
{
    try
    {
        std::cout << hermes_statuscodes.at(errorcode) << " (0x" << errorcode << ")" << std::endl;
    }
    catch (...)
    {
        // Invalid status code received, print unknown
        std::cout << "Unknown error occured (0x" << errorcode << ")" << std::endl;
    }
}