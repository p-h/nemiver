/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4; -*- */

/*Copyright (c) 2005-2006 Dodji Seketeli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef __NMV_LIBXML_UTILS_H__
#define __NMV_LIBXML_UTILS_H__

#include "nmv-api-macros.h"
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include <libxml/xmlmemory.h>
#include "nmv-safe-ptr-utils.h"
#include "nmv-i-input-stream.h"

namespace nemiver {
namespace common {
namespace libxmlutils {

struct XMLTextReaderRef {

    void
    operator () (xmlTextReader* a_ptr) {if (a_ptr){}}

};//end XMLReaderRef

struct XMLTextReaderUnref {

    bool
    operator () (xmlTextReader* a_ptr)
    {
        if (a_ptr) {
            xmlFreeTextReader (a_ptr);
        }
        return true;
    }

};//end XMLReaderRef

struct XMLXPathContextRef {
    void
    operator () (xmlXPathContext *a_ptr) {if (a_ptr){}}
};//end XPathContextRef

struct XMLXPathContextUnref {
    bool
    operator () (xmlXPathContext *a_ptr)
    {
        if (a_ptr) {
            xmlXPathFreeContext (a_ptr);
        }
        return true;
    }
};//end XPathContextRef

struct XMLXPathObjectRef {
    void
    operator () (xmlXPathObject *a_ptr) {if (a_ptr) {}}
};//end XMLXPathObjectRef

struct XMLXPathObjectUnref {
    bool
    operator () (xmlXPathObject *a_ptr)
    {
        if (a_ptr) {
            xmlXPathFreeObject (a_ptr);
        }
        return true;
    }
};//end XMLXPathObjectRef

struct XMLCharRef {
    void
    operator () (xmlChar *a_ptr) {if (a_ptr) {}}
};//end XMLCharRef

struct XMLCharUnref {
    bool
    operator () (xmlChar *a_ptr)
    {
        if (a_ptr) {
            xmlFree (a_ptr);
        }
        return true;
    }
};//end XMLCharRef

typedef SafePtr <xmlTextReader, XMLTextReaderRef, XMLTextReaderUnref>
XMLTextReaderSafePtr;

typedef SafePtr<xmlXPathContext, XMLXPathContextRef,
XMLXPathContextUnref> XMLXPathContextSafePtr;

typedef SafePtr<xmlXPathObject, XMLXPathObjectRef, XMLXPathObjectUnref>
XMLXPathObjectSafePtr;

typedef SafePtr<xmlChar, XMLCharRef, XMLCharUnref> XMLCharSafePtr;

//****************************************************
//helpers to use xmlTextReader with our own IO system
//*****************************************************

struct ReaderIOContext {
    IInputStream &m_istream;

    ReaderIOContext (IInputStream &a_istream):
            m_istream (a_istream)
    {}
};

int NEMIVER_API reader_io_read_callback (ReaderIOContext *a_read_context,
                                     char * a_buf,
                                     int a_len);

int NEMIVER_API reader_io_close_callback (ReaderIOContext *a_read_context);

bool NEMIVER_API goto_next_element_node (XMLTextReaderSafePtr &a_reader);

bool NEMIVER_API goto_next_element_node_and_check (XMLTextReaderSafePtr &a_reader,
                                               const char* a_element_name);

bool NEMIVER_API search_next_element_node (XMLTextReaderSafePtr &a_reader,
                                       const char *a_element_name);

bool NEMIVER_API goto_next_text_node (XMLTextReaderSafePtr &a_reader);

bool NEMIVER_API read_next_and_check_node (XMLTextReaderSafePtr &a_reader,
                                   xmlReaderTypes a_node_type_to_be);

bool NEMIVER_API is_empty_element (XMLTextReaderSafePtr &a_reader);

}//end namespace libxmlutils
}//end namespace common
}//end namespace nemiver

#endif //__NMV_LIBXML_SAFE_PTR_H__

