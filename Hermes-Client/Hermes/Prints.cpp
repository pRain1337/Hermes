#include "Prints.h"

/*
 *   Funtetm.
 *   et:
 *       - etread
 *       - Size, et read
 *
 *   ettet.
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
	std::cout << " exit		- ends the atlas process " << std::endl;
	std::cout << " help		- displays this again" << std::endl;
}

/*
 *   Funtetm.
 *   et:
 *       - etread
 *       - Size, et read
 *
 *   ettet.
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
 *   Function .
 *   Asks :
 *       -
 *       -
 *
 *   .
 *
*/
void printError(uint64_t errorcode)
{
	switch (errorcode)
	{
	case (HERMES_STATUS_INVALID_COMMAND):
		std::cout << "Invalid command parameter was passed (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_INVALID_DATA_PTR):
		std::cout << "Invalid data pointer was passed (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_INVALID_RES_PTR):
		std::cout << "Invalid result pointer was passed (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_INVALID_IDENTIFIERS):
		std::cout << "Invalid identifiers were passed (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_FIND_PROC):
		std::cout << "Failed finding the process (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_FIND_DIRBASE):
		std::cout << "Failed getting dirbase (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_FIND_MOD):
		std::cout << "Failed finding the module (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_READ_MOD_BASE):
		std::cout << "Failed reading module base (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_READ_MOD_SIZE):
		std::cout << "Failed reading module size (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_PROCNAME_TOO_LONG):
		std::cout << "Process name size is too big (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_MODNAME_TOO_LONG):
		std::cout << "Module name size is too big (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_VIRT_READ):
		std::cout << "Failed reading virtual address (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_VIRT_WRITE):
		std::cout << "Failed writing virtual address (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_INVALID_DATA_SOURCE):
		std::cout << "Invalid data source was passed (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_INVALID_DATA_DIRBASE):
		std::cout << "Invalid data dirbase was passed (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_INVALID_DATA_SIZE):
		std::cout << "Invalid data size was passed (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_INVALID_DATA_DEST):
		std::cout << "Invalid data destination was passed (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_REQ_TOO_LARGE):
		std::cout << "Requested size is too big (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_SBUFF_VIRTW):
		std::cout << "Failed reading source buffer for virtual write (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_ADDR_TRANSLATION):
		std::cout << "Failed translating address (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_SBUFF_PHYSW):
		std::cout << "Failed reading source buffer for physical write (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_INVALID_MOD_BASE):
		std::cout << "Invalid module base (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_INVALID_MOD_SIZE):
		std::cout << "Invalid module size (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_PHYS_READ):
		std::cout << "Failed reading physical address (0x" << errorcode << ")" << std::endl;
		break;

	case (HERMES_STATUS_FAIL_PHYS_WRITE):
		std::cout << "Failed writing physical address (0x" << errorcode << ")" << std::endl;
		break;

	default:
		std::cout << "Unknown error occured (0x" << errorcode << ")" << std::endl;
		break;
	}
}