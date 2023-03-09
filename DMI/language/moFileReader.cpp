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
#include "moFileReader.h"
#include <iostream>
#include <cstdlib>

MO_BEGIN_NAMESPACE

unsigned long moFileReader::SwapBytes(unsigned long in) 
{
    unsigned long b0 = (in >> 0) & 0xff;
    unsigned long b1 = (in >> 8) & 0xff;
    unsigned long b2 = (in >> 16) & 0xff;
    unsigned long b3 = (in >> 24) & 0xff;

    return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

const std::string& moFileReader::GetErrorDescription() const
{
    return m_error;
}

void moFileReader::ClearTable()
{
    m_lookup.clear();
}

unsigned int moFileReader::GetNumStrings() const
{
    return m_lookup.size();
}

std::string moFileReader::Lookup( const char* id ) const
{
    if ( m_lookup.size() <= 0) return id;
    moLookupList::const_iterator iterator = m_lookup.find(id);    

    if ( iterator == m_lookup.end() )
    {
        return id;
    }
    return iterator->second;
}

moFileReader::eErrorCode moFileReader::ParseData(std::string data)
{
    // Creating a file-description.
    moFileInfo moInfo;
    
    // Reference to the List inside moInfo.
    moFileInfo::moTranslationPairList& TransPairInfo = moInfo.m_translationPairInformation;
    
    // Opening the file.
    std::stringstream stream(data);
    
    // Read in all the 4 bytes of fire-magic, offsets and stuff...
    stream.read((char*)&moInfo.m_magicNumber, 4);
    stream.read((char*)&moInfo.m_fileVersion, 4);
    stream.read((char*)&moInfo.m_numStrings, 4);
    stream.read((char*)&moInfo.m_offsetOriginal, 4);
    stream.read((char*)&moInfo.m_offsetTranslation, 4);
    stream.read((char*)&moInfo.m_sizeHashtable, 4);
    stream.read((char*)&moInfo.m_offsetHashtable, 4);
    
    if ( stream.bad() )
    {
        m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
        printf("%s", m_error.c_str());
        return moFileReader::EC_FILEINVALID;
    }
    
    // Checking the Magic Number
    if ( MagicNumber != moInfo.m_magicNumber )
    {
        if ( MagicReversed != moInfo.m_magicNumber )
        {
            m_error = "The Magic Number does not match in all cases!";
            printf("%s", m_error.c_str());
            return moFileReader::EC_MAGICNUMBER_NOMATCH;
        }
        else
        {
            moInfo.m_reversed = true;
            m_error = "Magic Number is reversed. We do not support this yet!";
            return moFileReader::EC_MAGICNUMBER_REVERSED;
        }
    }
    
    // Now we search all Length & Offsets of the original strings
    for ( int i = 0; i < moInfo.m_numStrings; i++ )
    {
        moTranslationPairInformation _str;
        stream.read((char*)&_str.m_orLength, 4);
        stream.read((char*)&_str.m_orOffset, 4);
        if ( stream.bad() )
        {
            m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            printf("%s", m_error.c_str());
            return moFileReader::EC_FILEINVALID;
        }
        
        TransPairInfo.push_back(_str);
    }
    
    // Get all Lengths & Offsets of the translated strings
    // Be aware: The Descriptors already exist in our list, so we just mod. refs from the deque.
    for ( int i = 0; i < moInfo.m_numStrings; i++ )
    {
        moTranslationPairInformation& _str = TransPairInfo[i];
        stream.read((char*)&_str.m_trLength, 4);
        stream.read((char*)&_str.m_trOffset, 4);
        if ( stream.bad() )
        {
            m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            printf("%s", m_error.c_str());
            return moFileReader::EC_FILEINVALID;
        }
    }
    
    // Normally you would read the hash-table here, but we don't use it. :)
    
    // Now to the interesting part, we read the strings-pairs now
    for ( int i = 0; i < moInfo.m_numStrings; i++)
    {
        // We need a length of +1 to catch the trailing \0.
        int orLength = TransPairInfo[i].m_orLength+1;
        int trLength = TransPairInfo[i].m_trLength+1;
        
        int orOffset = TransPairInfo[i].m_orOffset;
        int trOffset = TransPairInfo[i].m_trOffset;
        
        // Original
        char* original  = new char[orLength];
        memset(original, 0, sizeof(char)*orLength);
        
        stream.seekg(orOffset);
        stream.read(original, orLength);
        
        if ( stream.bad() )
        {
            m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            printf("%s", m_error.c_str());
            return moFileReader::EC_FILEINVALID;
        }
        
        // Translation
        char* translation = new char[trLength];
        memset(translation, 0, sizeof(char)*trLength);
        
        stream.seekg(trOffset);
        stream.read(translation, trLength);
        
        if ( stream.bad() )
        {
            m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            printf("%s", m_error.c_str());
            return moFileReader::EC_FILEINVALID;
        }
        
        // Store it in the map.
        m_lookup[std::string(original)] = std::string(translation);
        
        // Cleanup...
        delete original;
        delete translation;
    }
    
    // Done :)
    return moFileReader::EC_SUCCESS;
}

moFileReader::eErrorCode moFileReader::ReadFile( const char* filename )
{    
    // Creating a file-description.
    moFileInfo moInfo;

    // Reference to the List inside moInfo.
    moFileInfo::moTranslationPairList& TransPairInfo = moInfo.m_translationPairInformation;

    // Opening the file.
    std::ifstream stream( filename, std::ios_base::binary | std::ios_base::in );
    if ( !stream.is_open() )
    {
        m_error = std::string("Cannot open File ") + std::string(filename);
        return moFileReader::EC_FILENOTFOUND;
    }

    // Read in all the 4 bytes of fire-magic, offsets and stuff...
    stream.read((char*)&moInfo.m_magicNumber, 4);
    stream.read((char*)&moInfo.m_fileVersion, 4);
    stream.read((char*)&moInfo.m_numStrings, 4);
    stream.read((char*)&moInfo.m_offsetOriginal, 4);
    stream.read((char*)&moInfo.m_offsetTranslation, 4);
    stream.read((char*)&moInfo.m_sizeHashtable, 4);
    stream.read((char*)&moInfo.m_offsetHashtable, 4);

    if ( stream.bad() )
    {
        stream.close();
        m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
        return moFileReader::EC_FILEINVALID;
    }

    // Checking the Magic Number
    if ( MagicNumber != moInfo.m_magicNumber )
    {
        if ( MagicReversed != moInfo.m_magicNumber )
        {
            m_error = "The Magic Number does not match in all cases!";
            return moFileReader::EC_MAGICNUMBER_NOMATCH;
        }
        else
        {
            moInfo.m_reversed = true;
            m_error = "Magic Number is reversed. We do not support this yet!";
            return moFileReader::EC_MAGICNUMBER_REVERSED;
        }
    }  
    
    // Now we search all Length & Offsets of the original strings
    for ( int i = 0; i < moInfo.m_numStrings; i++ )
    {
        moTranslationPairInformation _str;        
        stream.read((char*)&_str.m_orLength, 4);
        stream.read((char*)&_str.m_orOffset, 4);
        if ( stream.bad() )
        {
            stream.close();
            m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            return moFileReader::EC_FILEINVALID;
        }

        TransPairInfo.push_back(_str);
    }

    // Get all Lengths & Offsets of the translated strings
    // Be aware: The Descriptors already exist in our list, so we just mod. refs from the deque.
    for ( int i = 0; i < moInfo.m_numStrings; i++ )
    {
        moTranslationPairInformation& _str = TransPairInfo[i];
        stream.read((char*)&_str.m_trLength, 4);
        stream.read((char*)&_str.m_trOffset, 4);
        if ( stream.bad() )
        {
            stream.close();
            m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            return moFileReader::EC_FILEINVALID;
        }
    }

    // Normally you would read the hash-table here, but we don't use it. :)

    // Now to the interesting part, we read the strings-pairs now
    for ( int i = 0; i < moInfo.m_numStrings; i++)
    {
        // We need a length of +1 to catch the trailing \0.
        int orLength = TransPairInfo[i].m_orLength+1;
        int trLength = TransPairInfo[i].m_trLength+1;

        int orOffset = TransPairInfo[i].m_orOffset;
        int trOffset = TransPairInfo[i].m_trOffset;

        // Original
        char* original  = new char[orLength];
        memset(original, 0, sizeof(char)*orLength);

        stream.seekg(orOffset);
        stream.read(original, orLength);

        if ( stream.bad() )
        {
            m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            return moFileReader::EC_FILEINVALID;
        }
        
        // Translation
        char* translation = new char[trLength];
        memset(translation, 0, sizeof(char)*trLength);

        stream.seekg(trOffset);
        stream.read(translation, trLength);

        if ( stream.bad() )
        {
            m_error = "Stream bad during reading. The .mo-file seems to be invalid or has bad descriptions!";
            return moFileReader::EC_FILEINVALID;
        }

        // Store it in the map.    
        m_lookup[std::string(original)] = std::string(translation);

        // Cleanup...
        delete original;
        delete translation;
    }

    // Done :)
    stream.close();
    return moFileReader::EC_SUCCESS;
}



moFileReader::eErrorCode moFileReader::ExportAsHTML(std::string infile, std::string filename, std::string css )
{
    // Read the file
    moFileReader reader;
    moFileReader::eErrorCode r = reader.ReadFile(infile.c_str()) ;
    if ( r != moFileReader::EC_SUCCESS )
    {
        return r;
    }
    if ( reader.m_lookup.empty() )
    {
        return moFileReader::EC_TABLEEMPTY;
    }    

    // Beautify Output
    std::string fname;
    unsigned int pos = infile.find_last_of(moPATHSEP);
    if ( pos != std::string::npos )
    {
        fname = infile.substr( pos+1, infile.length() );
    }
    else
    {
        fname = infile;
    }

    // if there is no filename given, we set it to the .mo + html, e.g. test.mo.html
    std::string htmlfile(filename);
    if (htmlfile.empty())
    {
        htmlfile = infile + std::string(".html");
    }   

    // Ok, now prepare output.
    std::ofstream stream(htmlfile.c_str());
    if ( stream.is_open() ) 
    {
        stream << "<!DOCTYPE HTML PUBLIC \"- //W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">" << std::endl;
        stream << "<html><head><style type=\"text/css\">\n" << std::endl;
        stream << css << std::endl;
        stream << "</style>" << std::endl;
        stream << "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">" << std::endl;
        stream << "<title>Dump of " << fname << "</title></head>" << std::endl;
        stream << "<body>" << std::endl;
        stream << "<center>" <<std::endl;
        stream << "<h1>" << fname << "</h1>" << std::endl;
        stream << "<table border=\"1\"><th colspan=\"2\">Project Info</th>" << std::endl;

        std::stringstream parsee;
        parsee << reader.Lookup("");
     
        while ( !parsee.eof() )
        {
            char buffer[1024];
            parsee.getline(buffer, 1024);
            std::string name;
            std::string value;

            reader.GetPoEditorString( buffer, name, value );
            if ( !(name.empty() || value.empty()) )
            {
                stream << "<tr><td>" << name << "</td><td>" <<  value << "</td></tr>" << std::endl;
            }
        }
        stream << "</table>" << std::endl;
        stream << "<hr noshade/>" << std::endl;

        // Now output the content
        stream << "<table border=\"1\"><th colspan=\"2\">Content</th>" << std::endl;        
        for ( moLookupList::const_iterator it = reader.m_lookup.begin(); 
             it != reader.m_lookup.end(); it++)
        {
            if ( it->first != "" ) // Skip the empty msgid, its the table we handled above.
            {
                stream << "<tr><td>" << it->first << "</td><td>" <<  it->second << "</td></tr>" << std::endl;
            }
        }
        stream << "</table><br/>" << std::endl;

        stream << "</center>" << std::endl;
        stream << "<div class=\"copyleft\">File generated by <a href=\"http://mofilereader.googlecode.com\" target=\"_blank\">moFileReaderSDK</a></div>" << std::endl;        
        stream << "</body></html>" << std::endl;
        stream.close();
    }
    else
    {
        return moFileReader::EC_FILENOTFOUND;
    }
   
    return moFileReader::EC_SUCCESS;
}


// Removes spaces from front and end. 
void moFileReader::Trim(std::string& in)
{
    while ( in[0] == ' ' )
    {
        in = in.substr(1, in.length() );
    }
    while( in[in.length()] == ' ' )
    {
        in = in.substr(0, in.length() - 1 );
    }
}

// Extracts a value-pair from the po-edit-information
bool moFileReader::GetPoEditorString(const char* buffer, std::string& name, std::string& value)
{
    std::string line(buffer);
    size_t first = line.find_first_of(":");

    if ( first != std::string::npos )
    {
        name   = line.substr( 0, first );
        value  = line.substr( first + 1, line.length() );

        // Replace <> with () for Html-Conformity.
        MakeHtmlConform(value);
        MakeHtmlConform(name);

        // Remove spaces from front and end.
        Trim(value);
        Trim(name);

        return true;
    }
    return false;
}

// Replaces < with ( to satisfy html-rules.
void moFileReader::MakeHtmlConform(std::string& inout)
{
    std::string temp = inout;
    for ( unsigned int i = 0; i < temp.length(); i++)
    {
        if ( temp[i] == '>')
        {
            inout.replace(i, 1, ")");
        }
        if ( temp[i] == '<' )
        {
            inout.replace(i, 1, "(");
        }
    }    
}






moFileReaderSingleton& moFileReaderSingleton::GetInstance()
{
    static moFileReaderSingleton theoneandonly;
    return theoneandonly;
} 


moFileReaderSingleton::moFileReaderSingleton(const moFileReaderSingleton& )
{
}

moFileReaderSingleton::moFileReaderSingleton()
{
}

moFileReaderSingleton& moFileReaderSingleton::operator=(const moFileReaderSingleton&)
{
    return *this;
}



MO_END_NAMESPACE
