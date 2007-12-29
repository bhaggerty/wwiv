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
#if defined ( _DEBUG )

#include "wwiv.h"
#include "WOutStreamBuffer.h"
#include "WStringUtils.h"
#include "XferTest.h"

using std::cout;
using std::endl;
using std::ostringstream;
using std::string;

CPPUNIT_TEST_SUITE_REGISTRATION( XferTest );


void XferTest::testOkfn()
{
    CPPUNIT_ASSERT( !okfn("") );
    CPPUNIT_ASSERT( okfn("foo") );
}


#endif
