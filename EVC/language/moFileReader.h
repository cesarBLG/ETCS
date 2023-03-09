/*
 * moFileReader - A simple .mo-File-Reader
 * Copyright (C) 2009 Domenico Gentner (scorcher24@gmail.com)
 * All rights reserved.                          
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. The names of its contributors may not be used to endorse or promote 
 *      products derived from this software without specific prior written 
 *      permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __MOFILEREADER_H_INCLUDED__
#define __MOFILEREADER_H_INCLUDED__

#include <deque>
#include <map>
#include <fstream>
#include <cstring> // this is for memset when compiling with gcc.
#include <string>
#include <sstream>

#ifndef __MOFILECONFIG_H_INCLUDED__
#   include "moFileConfig.h"
#endif

/** \mainpage moFileReaderSDK
  * 
  *
  * <h2>Compilation with Visual C++ (Express and better)</h2>
  *
  * We provide a project for Visual C++ 2008. You can select 3 Types of Compilation:
  *
  * <h3>Executable (Release or Debug)</h3>
  *
  * This will compile the code as an executable which can lookup strings from every .mo-File you load with it.
  * This can be handy if you want to have a peek into a file or test something etc. I recommend building the 
  * release-executable only if you just want to use it.
  *
  * <h3>Dynamic Loaded Library ( ReleaseDLL )</h3>
  *
  * This may be overkill, but perhaps you like it modular. This Configuration will create a dll and an import-library.
  * Do not forget to link against the import-library and please define MOFILE_IMPORT in your preprocessor-settings, 
  *  otherwise you will receive a bunch of linker-errors. 
  * You will find all files in the directory "lib" in the Solutions Directory.
  *
  * <h3>Static Library ( ReleaseLIB )</h3>
  *
  * This will compile the code as a static library with no Entry-Point. This is the recommended usage. 
  * But please do not forget to link against moFileReader.static.lib. Otherwise you will receive linker-errors.
  *
  * <h2>Compilation via cmake</h2>
  *
  * - Make sure you have cmake installed and in your path. If not, go to http://www.cmake.org and get it. 
  * - Switch to a Shell or commandline
  * - Run cmake in $INSTALLDIR\\build. See cmake --help for possible generators. Here are the available Options:
  *     - COMPILE_DLL Setting this to ON will compile the library as a shared module. By default, a static library is built.
  *     .
  *   Example:
  *   \code
  *   cmake -G"MinGW Makefiles" -DCOMPILE_DLL=ON
  *   cmake -G"Visual Studio 9 2008"
  *   // etc
  *   \endcode
  *
  * cmake will compile the library and moReader[.exe]-binary, which can do lookups in moFiles and export moFiles as HTML.
  * See moReader[.exe] --help for details. 
  * You will find the libraries in %%projectdir%%/lib and the binary in %%projectdir%%/bin
  *
  * <h2>Compilation via provided batch-files (woe32 only)</h2>
  *
  * - Call compile_vc.bat or compile_mingw.bat with one of these options:
  *     - DLL - Compiles as dynamic loaded module.
  *     - LIB - Compiles as static library.
  *     - EXE - Compiles the executable.
  *
  * <h2>None of those?</h2>
  *
  * The last option is to simply add moFileReader.cpp, moFileReader.h and moFileConfig.h to your project. Thats all you have to do. 
  * You can safely exclude mo.cpp, since this file keeps the entry-points of the .exe and .dll only.
  *
  * <h2>Usage</h2>
  *
  * This is moFileReader, a simple gettext-replacement. The usage of this library is, hopefully, fairly simple:
  * \code
  *
  * // Instanciate the class
  * moFileLib::moFileReader reader;
  *
  * // Load a .mo-File.
  * if ( reader.ReadFile("myTranslationFile.mo") != moFileLib::moFileReader::EC_SUCCESS )
  * {
  *    // Error Handling
  * }
  *
  * // Now, you can lookup the strings you stored in the .mo-File:
  * std::cout << reader.Lookup("MyTranslationString") << std::endl;  
  *
  * \endcode
  * Thats all! This small code has no dependencies, except the C/C++-runtime of your compiler,
  * so it should work on all machines where a C++-runtime is provided.
  *
  * \note We do not yet support .mo-Files with reversed magic-numbers, since I don't have
  *       a file to test it and I hate to release stuff I wasn't able to test. 
  *       If you can provide such a file with reversed bytes, please compile %%projectdir%%/bin/i18n/de/moTest.po with
  *       gettext or poEdit and send it to scorcher24 [at] gmail [dot] com.
  *
  * <h2>Changelog</h2>
  * 
  * - Version 0.1.2
  *     - Generic improvements to the documentation.
  *     - Generic improvements to the code
  *     - Fixed a bug in mo.cpp which caused the application not to print the help
  *       message if only --export or --lookup where missing.
  *     - Added -h, --help and -? to moReader[.exe]. It will print the help-screen.
  *     - Added --version and -v to moReader[.exe]. It will print some informations about the program.
  *     - Added --license to moReader[.exe]. This will print its license. 
  *     - --export gives now a feedback about success or failure.   
  *     - The HTML-Dump-Method outputs now the whole table from the empty msgid in a nice html-table, not only a few hardcoded. 
  *     - I had an issue-report that the Error-Constants can collide with foreign code under certain conditions, 
  *       so I added a patch which renamed the error-constants to more compatible names. 
  *
  * - Version 0.1.1
  *     - Added the ability to export mo's as HTML.
  *     - Fixed a bug causing a crash when passing an invalid value to moFileReader::Lookup().
  *     - Added a new file, moFileConfig.h, holding the macros for the project.
  *     - Added the ability to be configured by cmake.
  *     - Added some more inline-functions, which really enhance the singleton.
  *
  * - Version 0.1.0
  *     - Initial Version and release to http://googlecode.com
  *
  *
  * <h2>Credits</h2>
  *
  * Gettext is part of the GNU-Tools and (C) by the <a href="http://fsf.org">Free Software Foundation</a>.\n
  * Visual C++ Express is a registered Trademark of Microsoft, One Microsoft Way, Redmond, USA.\n
  * moFileReader is using NSIS for creating the setup-package. \n
  * All other Trademarks are property of their respective owners. \n
  * \n
  * Thanks for using this piece of OpenSource-Software.\n
  * If you (dis)like it or have suggestions, questions, patches etc, please don't hesitate to write to my email-adress: scorcher24 [at] gmail [dot] com.
  * Submit patches and/or bugs on http://mofilereader.googlecode.com. You must register with googlemail to sign in.
  * Send your flames, dumb comments etc to /dev/null, thank you. 
  */



/*
    About Warning 4251:
    http://support.microsoft.com/default.aspx?scid=KB;EN-US;16.

    I am aware of this warning and know how to deal with it. 
    To avoid that derived projects are influenced by this warning
    I have deactivated it for your convinience.
    Note: This warning only occurs, when using this code as a DLL.
*/
#if defined(_MSC_VER) && ( defined(_EXPORT) || defined(MOFILE_IMPORT) )
#   pragma warning (disable:4251)
#endif /* _MSC_VER */


/** \namespace moFileLib
  * \brief This is the only namespace of this small sourcecode.
  */
MO_BEGIN_NAMESPACE

const std::string g_css = \
"\
body {\
    background-color: black;\
    color: silver;\
}\
table {\
width: 80%;}\
th {\
background-color: orange;\
color: black;\
}\
hr { color: red;width: 80%; size: 5px; }\
a:link{color: gold;}\
a:visited{color: grey;}\
a:hover{color:blue;}\
.copyleft{\
    font-size: 12px; \
    text-align: center;\
}\
";

/**
  * \brief Keeps the Description of translated and original strings.
  *
  *
  * To load a String from the file, we need its offset and its length.
  * This struct helps us grouping this information.
  */
struct moTranslationPairInformation
{
    /// \brief Constructor
    moTranslationPairInformation() 
        : m_orLength(0), m_orOffset(0), 
          m_trLength(0), m_trOffset(0)
    {}

    /// \brief Length of the Original String
    int m_orLength;

    /// \brief Offset of the Original String (absolute)
    int m_orOffset;

    /// \brief Length of the Translated String
    int m_trLength;

    /// \brief Offset of the Translated String (absolute)
    int m_trOffset;
};

/**
  * \brief Describes the "Header" of a .mo-File.
  *
  * 
  * The File info keeps the header of a .mo-file and 
  * a list of the string-descriptions.
  * The typedef is for the type of the string-list.
  * The constructor ensures, that all members get a nice
  * initial value.
  */     
struct moFileInfo
{
    /// \brief Type for the list of all Translation-Pair-Descriptions.
    typedef std::deque<moTranslationPairInformation> moTranslationPairList;

    /// \brief Constructor
    moFileInfo()
        : m_magicNumber(0), m_fileVersion(0), m_numStrings(0),
          m_offsetOriginal(0), m_offsetTranslation(0), m_sizeHashtable(0), 
          m_offsetHashtable(0), m_reversed(false)
    {}

    /// \brief The Magic Number, compare it to g_MagicNumber.
    long m_magicNumber;          
    
    /// \brief The File Version, 0 atm according to the manpage.
    int m_fileVersion;          

    /// \brief Number of Strings in the .mo-file.
    int m_numStrings;           

    /// \brief Offset of the Table of the Original Strings
    int m_offsetOriginal;       

    /// \brief Offset of the Table of the Translated Strings
    int m_offsetTranslation;    

    /// \brief Size of 1 Entry in the Hashtable.
    int m_sizeHashtable;       

    /// \brief The Offset of the Hashtable.
    int m_offsetHashtable;      

    /** \brief Tells you if the bytes are reversed
      * \note When this is true, the bytes are reversed and the Magic number is like g_MagicReversed
      */
    bool m_reversed;            

    /// \brief A list containing offset and length of the strings in the file.
    moTranslationPairList m_translationPairInformation;   
};

/** 
  * \brief This class is a gettext-replacement.
  *
  *
  * The usage is quite simple:\n
  * Tell the class which .mo-file it shall load via 
  * moFileReader::ReadFile(). The method will attempt to load
  * the file, all translations will be stored in memory.
  * Afterwards you can lookup the strings with moFileReader::Lookup() just
  * like you would do with gettext.
  * Additionally, you can call moFileReader::ReadFile() for as much files as you 
  * like. But please be aware, that if there are duplicated keys (original strings),
  * that they will replace each other in the lookup-table. There is no check done, if a 
  * key already exists.
  *
  * \note If you add "Lookup" to the keywords of the gettext-parser (like poEdit),
  * it will recognize the Strings loaded with an instance of this class.
  * \note I strongly recommend poEdit from Vaclav Slavik for editing .po-Files,
  *       get it at http://poedit.net for various systems :).
  */
class MOEXPORT moFileReader
{
protected:
    /// \brief Type for the map which holds the translation-pairs later.
    typedef std::map<std::string, std::string> moLookupList;

public:

    /// \brief The Magic Number describes the endianess of bytes on the system.   
    static const long MagicNumber   = 0x950412DE;

    /// \brief If the Magic Number is Reversed, we need to swap the bytes.     
    static const long MagicReversed = 0xDE120495;

    /// \brief The possible errorcodes for methods of this class
    enum eErrorCode
    {
        /// \brief Indicated success
        EC_SUCCESS = 0,

        /// \brief Indicates an error
        EC_ERROR,

        /// \brief The given File was not found.
        EC_FILENOTFOUND,

        /// \brief The file is invalid.
        EC_FILEINVALID,

        /// \brief Empty Lookup-Table (returned by ExportAsHTML())
        EC_TABLEEMPTY,

        /// \brief The magic number did not match 
        EC_MAGICNUMBER_NOMATCH, 

        /**
          * \brief The magic number is reversed.
          * \note This is an error until the class supports it.
          */
        EC_MAGICNUMBER_REVERSED,           
    };

    /** \brief Reads a .mo-file
     * \param[in] _filename The path to the file to load.
     * \return SUCCESS on success or one of the other error-codes in eErrorCode on error.
     *
     * This is the core-feature. This method loads the .mo-file and stores
     * all translation-pairs in a map. You can access this map via the method
     * moFileReader::Lookup().
     */
    virtual moFileReader::eErrorCode ParseData(std::string data);

    /** \brief Reads a .mo-file
      * \param[in] _filename The path to the file to load.
      * \return SUCCESS on success or one of the other error-codes in eErrorCode on error.
      *
      * This is the core-feature. This method loads the .mo-file and stores
      * all translation-pairs in a map. You can access this map via the method
      * moFileReader::Lookup().
      */
    virtual eErrorCode ReadFile(const char* filename);

    /** \brief Returns the searched translation or returns the input.
      * \param[in,out] id The id of the translation to search for.
      * \return The value you passed in via _id or the translated string.
      */
    virtual std::string Lookup( const char* id ) const;

    /// \brief Returns the Error Description.
    virtual const std::string& GetErrorDescription() const;

    /// \brief Empties the Lookup-Table.
    virtual void ClearTable();
    
    /** \brief Returns the Number of Entries in our Lookup-Table.
      * \note The mo-File-table always contains an empty msgid, which contains informations
      *       about the tranlsation-project. So the real number of strings is always minus 1.
      */
    virtual unsigned int GetNumStrings() const;

    /** \brief Exports the whole content of the .mo-File as .html
      * \param[in] infile The .mo-File to export. 
      * \param[in] filename Where to store the .html-file. If empty, the path and filename of the _infile with .html appended. 
      * \param[in,out] css The css-script for the visual style of the 
      *                     file, in case you don't like mine ;).
      * \see g_css for the possible and used css-values.
      */
    static eErrorCode ExportAsHTML(const std::string infile, const std::string filename = "", const std::string css = g_css );

protected:
    /// \brief Keeps the last error as String.
    std::string m_error;

    /** \brief Swap the endianness of a 4 byte WORD.
      * \param[in] in The value to swap.
      * \return The swapped value.
      */
    unsigned long SwapBytes(unsigned long in);    

private:
    // Holds the lookup-table
    moLookupList m_lookup;

    void MakeHtmlConform(std::string& _inout);
    bool GetPoEditorString(const char* _buffer, std::string& _name, std::string& _value);
    void Trim(std::string& _in);
};

/** \brief Convience Class
  *
  *
  * This class derives from moFileReader and builds a singleton to access its methods
  * in a global manner. 
  * \note This class is a Singleton. Please access it via moFileReaderSingleton::GetInstance()
  * or use the provided wrappers:\n
  * - moReadMoFile()
  * - _()
  * - moFileClearTable()  
  * - moFileGetErrorDescription()
  * - moFileGetNumStrings();
  */
class MOEXPORT moFileReaderSingleton : public moFileReader
{
private:
    // Private Contructor and Copy-Constructor to avoid 
    // that this class is instanced.
    moFileReaderSingleton();
    moFileReaderSingleton(const moFileReaderSingleton&);
    moFileReaderSingleton& operator=(const moFileReaderSingleton&);
    
public:
    /** \brief Singleton-Accessor. 
      * \return A static instance of moFileReaderSingleton.
      */
    static moFileReaderSingleton& GetInstance();      
};

/** \brief Reads the .mo-File.
  * \param[in] _filename The path to the file to use.
  * \see moFileReader::ReadFile() for details.
  */
inline moFileReader::eErrorCode moReadMoFile(const char* _filename)
{
    moFileReader::eErrorCode r = moFileReaderSingleton::GetInstance().ReadFile(_filename);
    return r;
}

/** \brief Looks for the spec. string to translate.
  * \param[in] id The string-id to search.
  * \return The translation if found, otherwise it returns id.
  */
inline std::string _(const char* id)
{
    std::string r = moFileReaderSingleton::GetInstance().Lookup(id);
    return r;
}

/// \brief Resets the Lookup-Table.
inline void moFileClearTable()
{
    moFileReaderSingleton::GetInstance().ClearTable();
}

/// \brief Returns the last known error as string or an empty class.
inline std::string moFileGetErrorDescription() 
{
    std::string r = moFileReaderSingleton::GetInstance().GetErrorDescription();
    return r;
}

/// \brief Returns the number of entries loaded from the .mo-File.
inline int moFileGetNumStrings()
{
    int r = moFileReaderSingleton::GetInstance().GetNumStrings();
    return r;
}

#if defined(_MSC_VER)
#   pragma warning (default:4251)
#endif /* _MSC_VER */

MO_END_NAMESPACE

#endif /* __MOFILEREADER_H_INCLUDED__ */
