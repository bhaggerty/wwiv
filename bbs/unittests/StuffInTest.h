/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.0x                         */
/*             Copyright (C)2007, WWIV Software Services                  */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/*                                                                        */
/**************************************************************************/
#ifndef __INCLUDED_STUFF_IN_TEST_H__
#define __INCLUDED_STUFF_IN_TEST_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/TestCase.h"
#include "cppunit/ui/text/TestRunner.h"
#include "cppunit/TestCaller.h"

class StuffInTest : public CppUnit::TestCase {
   CPPUNIT_TEST_SUITE( StuffInTest );
   CPPUNIT_TEST( testSimpleCase );
   CPPUNIT_TEST( testEmpty );
   CPPUNIT_TEST( testAllNumbers );
   CPPUNIT_TEST( testAllDropFiles );
   CPPUNIT_TEST( testPortAndNode );
   CPPUNIT_TEST( testSpeeds );
   CPPUNIT_TEST_SUITE_END( );
public:
   virtual void tearDown();

protected:
    void testSimpleCase();
    void testEmpty();
    void testAllNumbers();
    void testAllDropFiles();
    void testPortAndNode();
    void testSpeeds();



private:
    const std::string t(const std::string name);
};

#endif