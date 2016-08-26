/*********************************************************************
* Software License Agreement (AGPL-3 License)
*
* CertiViBE Test Software
* Based on OpenViBE V1.1.0, Copyright (C) Inria, 2006-2015
* Copyright (C) Inria, 2015-2017,V1.0
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License version 3,
* as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

namespace OpenViBE
{
	namespace Kernel
	{
		/**
		 * \class ErrorType
		 * \author Charles Garraud (Inria)
		 * \date 2016-07-13
		 * \brief List of error types
		 * \ingroup Group_Error
		 * \ingroup Group_Kernel
		 */
		enum class ErrorType
		{
			// Misc
			NoErrorFound = 0, /*!< Used when no error occurs */
			Unknown = 1, /*!< Unknown error */
			Internal = 2, /*!< Used mainly for high-level errors generated by low-level errors */
			NotImplemented = 3, /*!< Used when the body of an API call is not yet implemented */
			ExceptionCaught = 4, /*!< Used to report an error related to a c++ expection caught in the system */
			BadCall = 5, /*!< Used when an API call is incompatible with the current state */
			BadProcessing = 6, /*!< Used when a processing/computation failed */
			BadVersion = 7, /*!< Used for plugin or api version mismatch */
			// Value setting
			BadValue = 10, /*!< Used when an entity is given an invalid value */
			BadArgument = 11, /*!< Used when a function is fed with invalid arguments */
			BadCast = 12, /*!< Used when a cast failed */
			OutOfBound = 13, /*!< Used for out of range value (specific case of BadValue/BadArguments) */
			Overflow = 14, /*!< Used to report arithmetic overflow */
			BadParsing = 15, /*!< Used to report data parsing error */
			// Alloc
			BadAlloc = 20, /*!< Used when an allocation failed */
			OutOfMemory = 21, /*!< Used when the system is running out of memory (specific case of BadAlloc) */
			// Resource
			ResourceNotFound = 30, /*!< Used to report that a resource (logical or physical) was not found */
			BadResourceCreation = 31, /*!< Used to report that a resource (logical or physical) could not be created */
			// IO
			BadFileRead = 40, /*!< Used when opening a file failed */
			BadFileWrite = 41, /*!< Used when writing to a file failed */
			BadFileParsing = 42, /*!< Used when reading succeeded but parsing of data within the file failed */
			BadNetworkConnection = 43, /*!< Used when network connection failed */
			// Config
			BadConfig = 50, /*!< Used when a given configuration state is erroneous */
			BadInput = 51, /*!< Used when an input (box, algorithm) is missing, empty or has a wrong value */
			BadOutput = 52, /*!< Used when an output (box, algorithm) is missing, empty or has a wrong value */
			BadTrigger = 53, /*!< Used when an algorithm trigger has a wrong value */
			BadSetting = 54 /*!< Used when a box setting has a wrong value */
		};
	}
}
