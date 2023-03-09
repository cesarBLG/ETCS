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
#ifndef __MOFILECONFIG_H_INCLUDED__
#define __MOFILECONFIG_H_INCLUDED__

//-------------------------------------------------------------
// Defines an export-macro when compiling as dll on woe32.
//-------------------------------------------------------------
#if defined(MOFILE_EXPORTS) && defined (WIN32)
#   define MOEXPORT __declspec(dllexport) 
#elif defined (MOFILE_IMPORT) && defined(WIN32) 
#   define MOEXPORT __declspec(dllimport) 
#else
#   define MOEXPORT
#endif

//-------------------------------------------------------------
// Path-Seperators are different on other OS.
//-------------------------------------------------------------
#ifdef WIN32
#   define moPATHSEP std::string("\\")
#else
#   define moPATHSEP std::string("/")
#endif

//-------------------------------------------------------------
// Defines the beginning of the namespace moFileLib.
//-------------------------------------------------------------
#define MO_BEGIN_NAMESPACE namespace moFileLib{

//-------------------------------------------------------------
// Ends the current namespace.
//-------------------------------------------------------------
#define MO_END_NAMESPACE   }



#endif /* __MOFILECONFIG_H_INCLUDED__ */


