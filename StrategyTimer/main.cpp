#include <iostream>
#include <stdexcept>

#include "timer.h"
#include "logger.h"

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "en_US.utf8");

	int minutes = 0;
	// If there are any arguments
	if (argc > 1)
	{
		// Try converting minutes to int, if it fails, make it 60
		try 
		{
			minutes = std::stoi(argv[1]);

			// if minutes variable is negative, make it positive
			if (minutes < 0)
			{
				minutes -= 2 * minutes;
			}
		}
		catch (std::exception& ex)
		{
			minutes = 60;
		}
	}
	// If there are no args
	else
	{
		minutes = 60;
	}

	try
	{
		Timer timer;
		timer.StartTimer(minutes);
	}
	catch (std::runtime_error& rEx)
	{
		std::cout << "Error: " << rEx.what() << std::endl;
		logmsg(rEx.what(), LogLevel::ERRORLEVEL);
		//LOG(ERROR) << rEx.what();
	}
}
