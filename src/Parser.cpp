//!
//! @file 			Parser.cpp
//! @author 		Geoffrey Hunter <gbmhunter@gmail.com> (www.cladlab.com)
//! @created		2014/04/03
//! @last-modified 	2014/05/27
//! @brief			CSV file parser, which can both decode and encode CSV files.
//! @details
//!

// System libraries
#include <iostream>
#include <fstream>
#include <string>		// std::string, std::stod()
#include <cstring>		// stderror()
#include <stdexcept>	// For throwing std::runtime_error exceptions

// User libraries
#include "lib/logger-cpp/api/LoggerApi.hpp"

// User source
#include "../include/Logger.hpp"
#include "../include/Parser.hpp"
#include "../include/Config.hpp"


namespace CsvCpp
{

	Parser::Parser(std::string fieldDelimiter, std::string recordDelimiter)
	{
		// Call shared code
		this->SharedConstructorCode(fieldDelimiter, recordDelimiter);
	}

	Parser::Parser()
	{
		// Call shared code, passing in defaults
		this->SharedConstructorCode(config_DEFAULT_FIELD_DELIMITER, config_DEFAULT_RECORD_DELIMITER);
	}

	void Parser::SharedConstructorCode(std::string fieldDelimiter, std::string recordDelimiter)
	{
		//InitLogger();

		// Default delimiter for CSV files
		this->recordDelimiter = recordDelimiter;

		// Default field delimiter for CSV files
		this->fieldDelimiter = fieldDelimiter;
	}

	void Parser::SetFilename(std::string filename)
	{
		this->filename = filename;
	}

	CsvRecord Parser::ReadRecord()
	{
		std::ifstream myIfStream(this->filename, std::ifstream::in);

		if ( (myIfStream.rdstate() & std::ifstream::failbit ) != 0 )
			errorMsg << "Error opening 'test.csv'." << std::endl;

		std::string csvLine;

		// Read a line from the CSV file
		std::getline(myIfStream, csvLine);

		std::istreambuf_iterator<char> eos;
		std::string theEntireFile(std::istreambuf_iterator<char>(myIfStream), eos);

		debugMsg << "The entire file:" << theEntireFile.c_str() << std::endl;

		debugMsg << "csvLine = " << csvLine << std::endl;

		// Return a single row from the CSV file
		return this->RecordStringToRecord(csvLine);

	}

	CsvTable Parser::ReadEntireFile(std::string fileName)
	{
		CsvTable csvTable;

		// Setup input filestream on the filename provided
		std::ifstream myIfStream(fileName, std::ifstream::in);

		if ( (myIfStream.rdstate() & std::ifstream::failbit ) != 0 )
		{
			//errorMsg << "Error opening '" << fileName << "'" << std::endl;
			//throw "Couldn't open file.\r\n";
			std::string tempString = "Couldn't open '" + fileName + "'.";
			throw std::runtime_error(tempString);
		}

		// Read the entire file stream into a string so we can peform
		// multi-character searches on it
		std::istreambuf_iterator<char> eos;
		std::string theEntireFile(std::istreambuf_iterator<char>(myIfStream), eos);

		// Stream has been read, so we can now close
		myIfStream.close();

		debugMsg << "The entire file:" << theEntireFile << std::endl;

		//std::size_t found;

		while(1)
		{
			debugMsg << "Next iteration of processing loop.\r\n";
			std::string csvLine;
			// Read a line from the CSV file.

			// Search for the terminating character(s)
			std::size_t found = theEntireFile.find(this->recordDelimiter);

			if(found == std::string::npos)
			{
				// Terminating character not found, so read entire string
				// (must be last line)
				debugMsg << "Line delimiter not found, must be last line." << std::endl;
				csvLine = theEntireFile.substr(0, theEntireFile.size());
				// Removes line from the stream
				theEntireFile.erase(0, theEntireFile.size());
			}
			else
			{
				debugMsg << "Line delimiter found at '" << found << "'." << std::endl;
				csvLine = theEntireFile.substr(0, (unsigned long int)found);
				// Removes line from the stream (including the line delimiters)
				theEntireFile.erase(0, found + this->recordDelimiter.size());
			}

			debugMsg << "csvLine = " << csvLine << std::endl;
			debugMsg << "csvLine size() = " << csvLine.size() << std::endl;

			// This stops ExtractElementsToRow from running if the last valid
			// row has a new line character at the end
			if (csvLine.empty())
			{
				debugMsg << "csvLine empty." << std::endl;
				break;
			}

			// Return a single row from the CSV file
			csvTable.AddRecord(this->RecordStringToRecord(csvLine));

		}

		return csvTable;

	}

	CsvTable Parser::ReadEntireFile()
	{
		// Call base function with file being the one stored
		// in the variable "filename" (set with SetFilename()).
		return this->ReadEntireFile(this->filename);
	}

	void Parser::CheckOStream(std::ostream* ostream)
	{
		if((*ostream).fail())
		{
			if ((*ostream).eof())
			{
				//errorMsg << "Unexpected end of file." << std::endl;
				throw std::runtime_error("Unexpected end of file.");
			}
			else if (errno)
			{
				//errorMsg << std::string(strerror(errno)) << std::endl;
				throw std::runtime_error(strerror(errno));
			}
			else
			{
				//errorMsg << "Unknown file error." << std::endl;
				throw std::runtime_error("Unknown file error.");
			}
		}
	}

	void Parser::CreateCsvFile(const CsvTable* csvTable, std::string fileName)
	{
		// Note: This function is overloaded.

		debugMsg << "Entered '" << __FUNCTION__ << "'." << std::endl;
		debugMsg << "Num. records = " << csvTable->NumRecords() << "." << std::endl;

		// Create output stream to file
		std::ofstream outputFile(fileName);
		this->CheckOStream(&outputFile);

		std::string result;

		// Iterate through the CSV table
		uint32_t x, y;

		for(x = 0; x < csvTable->NumRecords(); x++)
		{
			debugMsg << "Num. fields = " << (*csvTable)[x].NumFields() << "." << std::endl;
			for(y = 0; y < ((*csvTable)[x].NumFields()); y++)
			{
				debugMsg << "Writing '" << (*csvTable)[x][y] << "' to file." << std::endl;
				outputFile << (*csvTable)[x][y];
				this->CheckOStream(&outputFile);

				// Add a field delimiter as long as this IS NOT
				// the last field in the record
				if(y != (*csvTable)[x].NumFields() - 1)
				{
					outputFile << this->fieldDelimiter;
					this->CheckOStream(&outputFile);
				}

			}
			// Record delimiter is added to the end of every record,
			// including the last record in the file.
			debugMsg << "Writing record delimiter." << std::endl;
			outputFile << this->recordDelimiter;
			this->CheckOStream(&outputFile);
		}

		// Close the output file
		outputFile.close();
		this->CheckOStream(&outputFile);

	}

	void Parser::CreateCsvFile(const CsvTable* csvTable)
	{
		// Call the base function, providing the filename
		this->CreateCsvFile(csvTable, this->filename);
	}

	CsvRecord Parser::RecordStringToRecord(std::string csvLine)
	{
		debugMsg << "Entered '" << __FUNCTION__ << "'." << std::endl;

		int lastPosOfFieldDelimiter = 0;
		CsvRecord csvRecord;

		// Used to change the algorithm the first time it runs through
		bool firstTime = true;

		while(1)
		{
			// Find the next occurrence of the field delimiter
			int nextPosOfFieldDelimiter;
			if(firstTime)
			{
				nextPosOfFieldDelimiter = csvLine.find(this->fieldDelimiter, lastPosOfFieldDelimiter);
				firstTime = false;
			}
			else
				nextPosOfFieldDelimiter = csvLine.find(this->fieldDelimiter, lastPosOfFieldDelimiter + 1);

			int x = 0;
			std::string field;

			// Check to see if delimiter was found
			if(nextPosOfFieldDelimiter == (int)std::string::npos)
			{
				debugMsg << "Delimiter was not found, must be last field in record." << std::endl;
				// Delimiter was not found, must be last field in record,
				// so add all remaining characters
				for(x = lastPosOfFieldDelimiter + 1; x < (int)csvLine.length(); x++)
				{
					field += csvLine[x];
				}
			}
			else
			{
				// Next delimiter in row was found
				debugMsg << "Delimiter was found, must not be last field in record." << std::endl;
				// Check to see wether we are at the first element or not
				if(lastPosOfFieldDelimiter == 0)
				{
					for(x = lastPosOfFieldDelimiter; x < nextPosOfFieldDelimiter; x++)
					{
						field += csvLine[x];
					}
				}
				else
				{
					for(x = lastPosOfFieldDelimiter + 1; x < nextPosOfFieldDelimiter; x++)
					{
						field += csvLine[x];
					}
				}
			}

			csvRecord.AddField(field);
			debugMsg << "field = " << field << std::endl;

			if(nextPosOfFieldDelimiter == (int)std::string::npos)
			{
				debugMsg << "End of record reached." << std::endl;
				break;
			}

			lastPosOfFieldDelimiter = nextPosOfFieldDelimiter;
		}

		return csvRecord;
	}

	Parser::Status Parser::GetStatus(const CsvTable* csvTable)
	{
		debugMsg << "Entered Parser::GetStatus()." << std::endl;
		// Create a status object, which we will fill in with information.
		Parser::Status status;

		// Set status.isWellformed to true, then any test may set to false if
		// relevant
		status.isWellformed.reset(true);


		//========== ALL RECORDS HAVE EQUAL NUMBER OF FIELDS TEST =========//

		// Check if the table has records (a blank table would cause an exception to be thrown)
		if(csvTable->NumRecords() > 0)
		{
			uint32_t prevNumFields = (*csvTable)[0].NumFields();
			// Iterate through all records, except first ([0]), since
			// we read that above.
			uint32_t x;
			for(x = 1; x < (*csvTable).NumRecords(); x++)
			{
				debugMsg << "prevNumFields = '" << prevNumFields << "'." << std::endl;
				debugMsg << "numFields = '" << (*csvTable)[x].NumFields() << "'." << std::endl;
				if((*csvTable)[x].NumFields() != prevNumFields)
				{
					// Unequal number of fields detected!
					status.allRecordsHaveEqualNumFields.reset(false);
					// This is NOT a wellformed CSV table!
					status.isWellformed.reset(false);
					debugMsg << "All records do not have an equal number of fields." << std::endl;
					// We have discovered a record which doesn't have the same number
					// of fields as the previous one, so no need to continue searching through table
					break;
				}
				prevNumFields = (*csvTable)[x].NumFields();
			}

			// Now set to result to true, if it hasn't already been set to false
			if(!status.allRecordsHaveEqualNumFields)
			{
				status.allRecordsHaveEqualNumFields.reset(true);
				debugMsg << "All records have an equal number of fields." << std::endl;
			}
		}

		//============= POPULATING NUM RECORDS AND NUM FIELD VARIABLES =============//

		// Get the number of records.
		status.numRecords.reset(csvTable->NumRecords());

		// Check to see if there were no records
		if(*status.numRecords == 0)
		{
			// This is NOT a wellformed CSV table!
			status.isWellformed.reset(false);
		}

		// Make sure it is valid before trying to access it
		if(status.allRecordsHaveEqualNumFields)
		{
			if(*(status.allRecordsHaveEqualNumFields))
			{
				// Since all records have the same number of fields, we can use any record
				// to find the number of fields.
				status.numFields.reset((*csvTable)[0].NumFields());
			}
		}

		//============= CHECK IF ALL FIELDS ARE NUMERAL =============//

		// Set to true, and then if we find any one field which isn't numeral,
		// set to false
		status.allFieldsNumeral.reset(true);

		// Iterate through all fields in the CsvTable
		for(uint32_t x = 0; x < (*csvTable).NumRecords(); x++)
		{
			for(uint32_t y = 0; y < (*csvTable)[x].NumFields(); y++)
			{
				//std::cout << "csvTable[" << x << "][" << y << "] = " << csvTable[x][y] << std::endl;
				std::size_t pos;
				try
				{
					double result = std::stod((*csvTable)[x][y], &pos);
				}
				catch(std::invalid_argument& e)
				{
					// Non-numeral found in field!
					status.allFieldsNumeral.reset(false);
					// Finish test, no need to keep checking
					goto END_OF_NUMERAL_CHECK;
				}
				catch(std::out_of_range& e)
				{
					// Non-numeral found in field!
					status.allFieldsNumeral.reset(false);
					// Finish test, no need to keep checking
					goto END_OF_NUMERAL_CHECK;
				}

				// Even if exception wasn't thrown, the "number" could still
				// have non-numerals at the end of it
				if(pos != (*csvTable)[x][y].size())
				{
					// Non-numeral found in field!
					status.allFieldsNumeral.reset(false);
					// Finish test, no need to keep checking
					goto END_OF_NUMERAL_CHECK;
				}
			}
		}

		END_OF_NUMERAL_CHECK:

		//======================== FINISHED =========================//



		debugMsg << "Exiting Parser::GetStatus()..." << std::endl;

		// All tests complete, return the status
		return status;
	}

}

// EOF
