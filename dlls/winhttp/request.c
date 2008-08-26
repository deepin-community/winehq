/*
 * Copyright 2008 Hans Leidekker for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "wine/port.h"
#include "wine/debug.h"

#include <stdarg.h>
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#include "windef.h"
#include "winbase.h"
#include "winnls.h"
#include "winhttp.h"

#include "winhttp_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(winhttp);

static const WCHAR attr_accept[] = {'A','c','c','e','p','t',0};
static const WCHAR attr_accept_charset[] = {'A','c','c','e','p','t','-','C','h','a','r','s','e','t', 0};
static const WCHAR attr_accept_encoding[] = {'A','c','c','e','p','t','-','E','n','c','o','d','i','n','g',0};
static const WCHAR attr_accept_language[] = {'A','c','c','e','p','t','-','L','a','n','g','u','a','g','e',0};
static const WCHAR attr_accept_ranges[] = {'A','c','c','e','p','t','-','R','a','n','g','e','s',0};
static const WCHAR attr_age[] = {'A','g','e',0};
static const WCHAR attr_allow[] = {'A','l','l','o','w',0};
static const WCHAR attr_authorization[] = {'A','u','t','h','o','r','i','z','a','t','i','o','n',0};
static const WCHAR attr_cache_control[] = {'C','a','c','h','e','-','C','o','n','t','r','o','l',0};
static const WCHAR attr_connection[] = {'C','o','n','n','e','c','t','i','o','n',0};
static const WCHAR attr_content_base[] = {'C','o','n','t','e','n','t','-','B','a','s','e',0};
static const WCHAR attr_content_encoding[] = {'C','o','n','t','e','n','t','-','E','n','c','o','d','i','n','g',0};
static const WCHAR attr_content_id[] = {'C','o','n','t','e','n','t','-','I','D',0};
static const WCHAR attr_content_language[] = {'C','o','n','t','e','n','t','-','L','a','n','g','u','a','g','e',0};
static const WCHAR attr_content_length[] = {'C','o','n','t','e','n','t','-','L','e','n','g','t','h',0};
static const WCHAR attr_content_location[] = {'C','o','n','t','e','n','t','-','L','o','c','a','t','i','o','n',0};
static const WCHAR attr_content_md5[] = {'C','o','n','t','e','n','t','-','M','D','5',0};
static const WCHAR attr_content_range[] = {'C','o','n','t','e','n','t','-','R','a','n','g','e',0};
static const WCHAR attr_content_transfer_encoding[] = {'C','o','n','t','e','n','t','-','T','r','a','n','s','f','e','r','-','E','n','c','o','d','i','n','g',0};
static const WCHAR attr_content_type[] = {'C','o','n','t','e','n','t','-','T','y','p','e',0};
static const WCHAR attr_cookie[] = {'C','o','o','k','i','e',0};
static const WCHAR attr_date[] = {'D','a','t','e',0};
static const WCHAR attr_from[] = {'F','r','o','m',0};
static const WCHAR attr_etag[] = {'E','T','a','g',0};
static const WCHAR attr_expect[] = {'E','x','p','e','c','t',0};
static const WCHAR attr_expires[] = {'E','x','p','i','r','e','s',0};
static const WCHAR attr_host[] = {'H','o','s','t',0};
static const WCHAR attr_if_match[] = {'I','f','-','M','a','t','c','h',0};
static const WCHAR attr_if_modified_since[] = {'I','f','-','M','o','d','i','f','i','e','d','-','S','i','n','c','e',0};
static const WCHAR attr_if_none_match[] = {'I','f','-','N','o','n','e','-','M','a','t','c','h',0};
static const WCHAR attr_if_range[] = {'I','f','-','R','a','n','g','e',0};
static const WCHAR attr_if_unmodified_since[] = {'I','f','-','U','n','m','o','d','i','f','i','e','d','-','S','i','n','c','e',0};
static const WCHAR attr_last_modified[] = {'L','a','s','t','-','M','o','d','i','f','i','e','d',0};
static const WCHAR attr_location[] = {'L','o','c','a','t','i','o','n',0};
static const WCHAR attr_max_forwards[] = {'M','a','x','-','F','o','r','w','a','r','d','s',0};
static const WCHAR attr_mime_version[] = {'M','i','m','e','-','V','e','r','s','i','o','n',0};
static const WCHAR attr_pragma[] = {'P','r','a','g','m','a',0};
static const WCHAR attr_proxy_authenticate[] = {'P','r','o','x','y','-','A','u','t','h','e','n','t','i','c','a','t','e',0};
static const WCHAR attr_proxy_authorization[] = {'P','r','o','x','y','-','A','u','t','h','o','r','i','z','a','t','i','o','n',0};
static const WCHAR attr_proxy_connection[] = {'P','r','o','x','y','-','C','o','n','n','e','c','t','i','o','n',0};
static const WCHAR attr_public[] = {'P','u','b','l','i','c',0};
static const WCHAR attr_range[] = {'R','a','n','g','e',0};
static const WCHAR attr_referer[] = {'R','e','f','e','r','e','r',0};
static const WCHAR attr_retry_after[] = {'R','e','t','r','y','-','A','f','t','e','r',0};
static const WCHAR attr_server[] = {'S','e','r','v','e','r',0};
static const WCHAR attr_set_cookie[] = {'S','e','t','-','C','o','o','k','i','e',0};
static const WCHAR attr_status[] = {'S','t','a','t','u','s',0};
static const WCHAR attr_transfer_encoding[] = {'T','r','a','n','s','f','e','r','-','E','n','c','o','d','i','n','g',0};
static const WCHAR attr_unless_modified_since[] = {'U','n','l','e','s','s','-','M','o','d','i','f','i','e','d','-','S','i','n','c','e',0};
static const WCHAR attr_upgrade[] = {'U','p','g','r','a','d','e',0};
static const WCHAR attr_uri[] = {'U','R','I',0};
static const WCHAR attr_user_agent[] = {'U','s','e','r','-','A','g','e','n','t',0};
static const WCHAR attr_vary[] = {'V','a','r','y',0};
static const WCHAR attr_via[] = {'V','i','a',0};
static const WCHAR attr_warning[] = {'W','a','r','n','i','n','g',0};
static const WCHAR attr_www_authenticate[] = {'W','W','W','-','A','u','t','h','e','n','t','i','c','a','t','e',0};

static const WCHAR *attribute_table[] =
{
    attr_mime_version,              /* WINHTTP_QUERY_MIME_VERSION               = 0  */
    attr_content_type,              /* WINHTTP_QUERY_CONTENT_TYPE               = 1  */
    attr_content_transfer_encoding, /* WINHTTP_QUERY_CONTENT_TRANSFER_ENCODING  = 2  */
    attr_content_id,                /* WINHTTP_QUERY_CONTENT_ID                 = 3  */
    NULL,                           /* WINHTTP_QUERY_CONTENT_DESCRIPTION        = 4  */
    attr_content_length,            /* WINHTTP_QUERY_CONTENT_LENGTH             = 5  */
    attr_content_language,          /* WINHTTP_QUERY_CONTENT_LANGUAGE           = 6  */
    attr_allow,                     /* WINHTTP_QUERY_ALLOW                      = 7  */
    attr_public,                    /* WINHTTP_QUERY_PUBLIC                     = 8  */
    attr_date,                      /* WINHTTP_QUERY_DATE                       = 9  */
    attr_expires,                   /* WINHTTP_QUERY_EXPIRES                    = 10 */
    attr_last_modified,             /* WINHTTP_QUERY_LAST_MODIFIEDcw            = 11 */
    NULL,                           /* WINHTTP_QUERY_MESSAGE_ID                 = 12 */
    attr_uri,                       /* WINHTTP_QUERY_URI                        = 13 */
    attr_from,                      /* WINHTTP_QUERY_DERIVED_FROM               = 14 */
    NULL,                           /* WINHTTP_QUERY_COST                       = 15 */
    NULL,                           /* WINHTTP_QUERY_LINK                       = 16 */
    attr_pragma,                    /* WINHTTP_QUERY_PRAGMA                     = 17 */
    NULL,                           /* WINHTTP_QUERY_VERSION                    = 18 */
    attr_status,                    /* WINHTTP_QUERY_STATUS_CODE                = 19 */
    NULL,                           /* WINHTTP_QUERY_STATUS_TEXT                = 20 */
    NULL,                           /* WINHTTP_QUERY_RAW_HEADERS                = 21 */
    NULL,                           /* WINHTTP_QUERY_RAW_HEADERS_CRLF           = 22 */
    attr_connection,                /* WINHTTP_QUERY_CONNECTION                 = 23 */
    attr_accept,                    /* WINHTTP_QUERY_ACCEPT                     = 24 */
    attr_accept_charset,            /* WINHTTP_QUERY_ACCEPT_CHARSET             = 25 */
    attr_accept_encoding,           /* WINHTTP_QUERY_ACCEPT_ENCODING            = 26 */
    attr_accept_language,           /* WINHTTP_QUERY_ACCEPT_LANGUAGE            = 27 */
    attr_authorization,             /* WINHTTP_QUERY_AUTHORIZATION              = 28 */
    attr_content_encoding,          /* WINHTTP_QUERY_CONTENT_ENCODING           = 29 */
    NULL,                           /* WINHTTP_QUERY_FORWARDED                  = 30 */
    NULL,                           /* WINHTTP_QUERY_FROM                       = 31 */
    attr_if_modified_since,         /* WINHTTP_QUERY_IF_MODIFIED_SINCE          = 32 */
    attr_location,                  /* WINHTTP_QUERY_LOCATION                   = 33 */
    NULL,                           /* WINHTTP_QUERY_ORIG_URI                   = 34 */
    attr_referer,                   /* WINHTTP_QUERY_REFERER                    = 35 */
    attr_retry_after,               /* WINHTTP_QUERY_RETRY_AFTER                = 36 */
    attr_server,                    /* WINHTTP_QUERY_SERVER                     = 37 */
    NULL,                           /* WINHTTP_TITLE                            = 38 */
    attr_user_agent,                /* WINHTTP_QUERY_USER_AGENT                 = 39 */
    attr_www_authenticate,          /* WINHTTP_QUERY_WWW_AUTHENTICATE           = 40 */
    attr_proxy_authenticate,        /* WINHTTP_QUERY_PROXY_AUTHENTICATE         = 41 */
    attr_accept_ranges,             /* WINHTTP_QUERY_ACCEPT_RANGES              = 42 */
    attr_set_cookie,                /* WINHTTP_QUERY_SET_COOKIE                 = 43 */
    attr_cookie,                    /* WINHTTP_QUERY_COOKIE                     = 44 */
    NULL,                           /* WINHTTP_QUERY_REQUEST_METHOD             = 45 */
    NULL,                           /* WINHTTP_QUERY_REFRESH                    = 46 */
    NULL,                           /* WINHTTP_QUERY_CONTENT_DISPOSITION        = 47 */
    attr_age,                       /* WINHTTP_QUERY_AGE                        = 48 */
    attr_cache_control,             /* WINHTTP_QUERY_CACHE_CONTROL              = 49 */
    attr_content_base,              /* WINHTTP_QUERY_CONTENT_BASE               = 50 */
    attr_content_location,          /* WINHTTP_QUERY_CONTENT_LOCATION           = 51 */
    attr_content_md5,               /* WINHTTP_QUERY_CONTENT_MD5                = 52 */
    attr_content_range,             /* WINHTTP_QUERY_CONTENT_RANGE              = 53 */
    attr_etag,                      /* WINHTTP_QUERY_ETAG                       = 54 */
    attr_host,                      /* WINHTTP_QUERY_HOST                       = 55 */
    attr_if_match,                  /* WINHTTP_QUERY_IF_MATCH                   = 56 */
    attr_if_none_match,             /* WINHTTP_QUERY_IF_NONE_MATCH              = 57 */
    attr_if_range,                  /* WINHTTP_QUERY_IF_RANGE                   = 58 */
    attr_if_unmodified_since,       /* WINHTTP_QUERY_IF_UNMODIFIED_SINCE        = 59 */
    attr_max_forwards,              /* WINHTTP_QUERY_MAX_FORWARDS               = 60 */
    attr_proxy_authorization,       /* WINHTTP_QUERY_PROXY_AUTHORIZATION        = 61 */
    attr_range,                     /* WINHTTP_QUERY_RANGE                      = 62 */
    attr_transfer_encoding,         /* WINHTTP_QUERY_TRANSFER_ENCODING          = 63 */
    attr_upgrade,                   /* WINHTTP_QUERY_UPGRADE                    = 64 */
    attr_vary,                      /* WINHTTP_QUERY_VARY                       = 65 */
    attr_via,                       /* WINHTTP_QUERY_VIA                        = 66 */
    attr_warning,                   /* WINHTTP_QUERY_WARNING                    = 67 */
    attr_expect,                    /* WINHTTP_QUERY_EXPECT                     = 68 */
    attr_proxy_connection,          /* WINHTTP_QUERY_PROXY_CONNECTION           = 69 */
    attr_unless_modified_since,     /* WINHTTP_QUERY_UNLESS_MODIFIED_SINCE      = 70 */
    NULL,                           /* WINHTTP_QUERY_PROXY_SUPPORT              = 75 */
    NULL,                           /* WINHTTP_QUERY_AUTHENTICATION_INFO        = 76 */
    NULL,                           /* WINHTTP_QUERY_PASSPORT_URLS              = 77 */
    NULL                            /* WINHTTP_QUERY_PASSPORT_CONFIG            = 78 */
};

static void free_header( header_t *header )
{
    heap_free( header->field );
    heap_free( header->value );
    heap_free( header );
}

static BOOL valid_token_char( WCHAR c )
{
    if (c < 32 || c == 127) return FALSE;
    switch (c)
    {
    case '(': case ')':
    case '<': case '>':
    case '@': case ',':
    case ';': case ':':
    case '\\': case '\"':
    case '/': case '[':
    case ']': case '?':
    case '=': case '{':
    case '}': case ' ':
    case '\t':
        return FALSE;
    default:
        return TRUE;
    }
}

static header_t *parse_header( LPCWSTR string )
{
    const WCHAR *p, *q;
    header_t *header;
    int len;

    p = string;
    if (!(q = strchrW( p, ':' )))
    {
        WARN("no ':' in line %s\n", debugstr_w(string));
        return NULL;
    }
    if (q == string)
    {
        WARN("empty field name in line %s\n", debugstr_w(string));
        return NULL;
    }
    while (*p != ':')
    {
        if (!valid_token_char( *p ))
        {
            WARN("invalid character in field name %s\n", debugstr_w(string));
            return NULL;
        }
        p++;
    }
    len = q - string;
    if (!(header = heap_alloc_zero( sizeof(header_t) ))) return NULL;
    if (!(header->field = heap_alloc( (len + 1) * sizeof(WCHAR) )))
    {
        heap_free( header );
        return NULL;
    }
    memcpy( header->field, string, len * sizeof(WCHAR) );
    header->field[len] = 0;

    q++; /* skip past colon */
    while (*q == ' ') q++;
    if (!*q)
    {
        WARN("no value in line %s\n", debugstr_w(string));
        return header;
    }
    len = strlenW( q );
    if (!(header->value = heap_alloc( (len + 1) * sizeof(WCHAR) )))
    {
        free_header( header );
        return NULL;
    }
    memcpy( header->value, q, len * sizeof(WCHAR) );
    header->value[len] = 0;

    return header;
}

static int get_header_index( request_t *request, LPCWSTR field, int requested_index, BOOL request_only )
{
    int index;

    TRACE("%s\n", debugstr_w(field));

    for (index = 0; index < request->num_headers; index++)
    {
        if (strcmpiW( request->headers[index].field, field )) continue;
        if (request_only && !request->headers[index].is_request) continue;
        if (!request_only && request->headers[index].is_request) continue;

        if (!requested_index) break;
        requested_index--;
    }
    if (index >= request->num_headers) index = -1;
    TRACE("returning %d\n", index);
    return index;
}

static BOOL insert_header( request_t *request, header_t *header )
{
    DWORD count;
    header_t *hdrs;

    TRACE("inserting %s: %s\n", debugstr_w(header->field), debugstr_w(header->value));

    count = request->num_headers + 1;
    if (count > 1)
        hdrs = heap_realloc_zero( request->headers, sizeof(header_t) * count );
    else
        hdrs = heap_alloc_zero( sizeof(header_t) * count );

    if (hdrs)
    {
        request->headers = hdrs;
        request->headers[count - 1].field = strdupW( header->field );
        request->headers[count - 1].value = strdupW( header->value );
        request->headers[count - 1].is_request = header->is_request;
        request->num_headers++;
        return TRUE;
    }
    return FALSE;
}

static BOOL delete_header( request_t *request, DWORD index )
{
    if (!request->num_headers) return FALSE;
    if (index >= request->num_headers) return FALSE;
    request->num_headers--;

    heap_free( request->headers[index].field );
    heap_free( request->headers[index].value );

    memmove( &request->headers[index], &request->headers[index + 1], (request->num_headers - index) * sizeof(header_t) );
    memset( &request->headers[request->num_headers], 0, sizeof(header_t) );
    return TRUE;
}

static BOOL process_header( request_t *request, LPCWSTR field, LPCWSTR value, DWORD flags, BOOL request_only )
{
    int index;
    header_t *header;

    TRACE("%s: %s 0x%08x\n", debugstr_w(field), debugstr_w(value), flags);

    /* replace wins out over add */
    if (flags & WINHTTP_ADDREQ_FLAG_REPLACE) flags &= ~WINHTTP_ADDREQ_FLAG_ADD;

    if (flags & WINHTTP_ADDREQ_FLAG_ADD) index = -1;
    else
        index = get_header_index( request, field, 0, request_only );

    if (index >= 0)
    {
        if (flags & WINHTTP_ADDREQ_FLAG_ADD_IF_NEW) return FALSE;
        header = &request->headers[index];
    }
    else if (value)
    {
        header_t hdr;

        hdr.field = (LPWSTR)field;
        hdr.value = (LPWSTR)value;
        hdr.is_request = request_only;

        return insert_header( request, &hdr );
    }
    /* no value to delete */
    else return TRUE;

    if (flags & WINHTTP_ADDREQ_FLAG_REPLACE)
    {
        delete_header( request, index );
        if (value)
        {
            header_t hdr;

            hdr.field = (LPWSTR)field;
            hdr.value = (LPWSTR)value;
            hdr.is_request = request_only;

            return insert_header( request, &hdr );
        }
        return TRUE;
    }
    else if (flags & (WINHTTP_ADDREQ_FLAG_COALESCE_WITH_COMMA | WINHTTP_ADDREQ_FLAG_COALESCE_WITH_SEMICOLON))
    {
        WCHAR sep, *tmp;
        int len, orig_len, value_len;

        orig_len = strlenW( header->value );
        value_len = strlenW( value );

        if (flags & WINHTTP_ADDREQ_FLAG_COALESCE_WITH_COMMA) sep = ',';
        else sep = ';';

        len = orig_len + value_len + 2;
        if ((tmp = heap_realloc( header->value, (len + 1) * sizeof(WCHAR) )))
        {
            header->value = tmp;

            header->value[orig_len] = sep;
            orig_len++;
            header->value[orig_len] = ' ';
            orig_len++;

            memcpy( &header->value[orig_len], value, value_len * sizeof(WCHAR) );
            header->value[len] = 0;
            return TRUE;
        }
    }
    return TRUE;
}

static BOOL add_request_headers( request_t *request, LPCWSTR headers, DWORD len, DWORD flags )
{
    BOOL ret = FALSE;
    WCHAR *buffer, *p, *q;
    header_t *header;

    if (len == ~0UL) len = strlenW( headers );
    if (!(buffer = heap_alloc( (len + 1) * sizeof(WCHAR) ))) return FALSE;
    strcpyW( buffer, headers );

    p = buffer;
    do
    {
        q = p;
        while (*q)
        {
            if (q[0] == '\r' && q[1] == '\n') break;
            q++;
        }
        if (!*p) break;
        if (*q == '\r')
        {
            *q = 0;
            q += 2; /* jump over \r\n */
        }
        if ((header = parse_header( p )))
        {
            ret = process_header( request, header->field, header->value, flags, TRUE );
            free_header( header );
        }
        p = q;
    } while (ret);

    heap_free( buffer );
    return ret;
}

/***********************************************************************
 *          WinHttpAddRequestHeaders (winhttp.@)
 */
BOOL WINAPI WinHttpAddRequestHeaders( HINTERNET hrequest, LPCWSTR headers, DWORD len, DWORD flags )
{
    BOOL ret;
    request_t *request;

    TRACE("%p, %s, 0x%x, 0x%08x\n", hrequest, debugstr_w(headers), len, flags);

    if (!headers)
    {
        set_last_error( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
    if (!(request = (request_t *)grab_object( hrequest )))
    {
        set_last_error( ERROR_INVALID_HANDLE );
        return FALSE;
    }
    if (request->hdr.type != WINHTTP_HANDLE_TYPE_REQUEST)
    {
        release_object( &request->hdr );
        set_last_error( ERROR_WINHTTP_INCORRECT_HANDLE_TYPE );
        return FALSE;
    }

    ret = add_request_headers( request, headers, len, flags );

    release_object( &request->hdr );
    return ret;
}

static WCHAR *build_request_string( request_t *request, LPCWSTR verb, LPCWSTR path, LPCWSTR version )
{
    static const WCHAR space[]   = {' ',0};
    static const WCHAR crlf[]    = {'\r','\n',0};
    static const WCHAR colon[]   = {':',' ',0};
    static const WCHAR twocrlf[] = {'\r','\n','\r','\n',0};

    WCHAR *ret;
    const WCHAR **headers, **p;
    unsigned int len, i = 0, j;

    /* allocate space for an array of all the string pointers to be added */
    len = request->num_headers * 4 + 7;
    if (!(headers = heap_alloc( len * sizeof(LPCWSTR) ))) return NULL;

    headers[i++] = verb;
    headers[i++] = space;
    headers[i++] = path;
    headers[i++] = space;
    headers[i++] = version;

    for (j = 0; j < request->num_headers; j++)
    {
        if (request->headers[j].is_request)
        {
            headers[i++] = crlf;
            headers[i++] = request->headers[j].field;
            headers[i++] = colon;
            headers[i++] = request->headers[j].value;

            TRACE("adding header %s (%s)\n", debugstr_w(request->headers[j].field),
                  debugstr_w(request->headers[j].value));
        }
    }
    headers[i++] = twocrlf;
    headers[i] = NULL;

    len = 0;
    for (p = headers; *p; p++) len += strlenW( *p );
    len++;

    if (!(ret = heap_alloc( len * sizeof(WCHAR) )))
    {
        heap_free( headers );
        return NULL;
    }
    *ret = 0;
    for (p = headers; *p; p++) strcatW( ret, *p );

    heap_free( headers );
    return ret;
}

#define QUERY_MODIFIER_MASK (WINHTTP_QUERY_FLAG_REQUEST_HEADERS | WINHTTP_QUERY_FLAG_SYSTEMTIME | WINHTTP_QUERY_FLAG_NUMBER)

static BOOL query_headers( request_t *request, DWORD level, LPCWSTR name, LPVOID buffer, LPDWORD buflen, LPDWORD index )
{
    header_t *header = NULL;
    BOOL request_only, ret = FALSE;
    int requested_index, header_index = -1;
    DWORD attr;

    request_only = level & WINHTTP_QUERY_FLAG_REQUEST_HEADERS;
    requested_index = index ? *index : 0;

    attr = level & ~QUERY_MODIFIER_MASK;
    switch (attr)
    {
    case WINHTTP_QUERY_CUSTOM:
    {
        header_index = get_header_index( request, name, requested_index, request_only );
        break;
    }
    case WINHTTP_QUERY_RAW_HEADERS_CRLF:
    {
        WCHAR *headers;
        DWORD len;

        if (request_only)
            headers = build_request_string( request, request->verb, request->path, request->version );
        else
            headers = request->raw_headers;

        len = strlenW( headers ) * sizeof(WCHAR);
        if (len + sizeof(WCHAR) > *buflen)
        {
            len += sizeof(WCHAR);
            set_last_error( ERROR_INSUFFICIENT_BUFFER );
        }
        else if (buffer)
        {
            memcpy( buffer, headers, len + sizeof(WCHAR) );
            TRACE("returning data: %s\n", debugstr_wn(buffer, len / sizeof(WCHAR)));
            ret = TRUE;
        }
        *buflen = len;
        if (request_only) heap_free( headers );
        return ret;
    }
    default:
    {
        if (attr > sizeof(attribute_table)/sizeof(attribute_table[0]) || !attribute_table[attr])
        {
            FIXME("attribute %u not implemented\n", attr);
            return FALSE;
        }
        TRACE("attribute %s\n", debugstr_w(attribute_table[attr]));
        header_index = get_header_index( request, attribute_table[attr], requested_index, request_only );
    }
    }

    if (header_index >= 0)
    {
        header = &request->headers[header_index];
    }
    if (!header || (request_only && !header->is_request))
    {
        set_last_error( ERROR_WINHTTP_HEADER_NOT_FOUND );
        return FALSE;
    }
    if (index) *index += 1;
    if (level & WINHTTP_QUERY_FLAG_NUMBER)
    {
        int *number = buffer;
        if (sizeof(int) > *buflen)
        {
            set_last_error( ERROR_INSUFFICIENT_BUFFER );
        }
        else if (number)
        {
            *number = atoiW( header->value );
            TRACE("returning number: %d\n", *number);
            ret = TRUE;
        }
        *buflen = sizeof(int);
    }
    else if (level & WINHTTP_QUERY_FLAG_SYSTEMTIME)
    {
        SYSTEMTIME *st = buffer;
        if (sizeof(SYSTEMTIME) > *buflen)
        {
            set_last_error( ERROR_INSUFFICIENT_BUFFER );
        }
        else if (st && (ret = WinHttpTimeToSystemTime( header->value, st )))
        {
            TRACE("returning time: %04d/%02d/%02d - %d - %02d:%02d:%02d.%02d\n",
                  st->wYear, st->wMonth, st->wDay, st->wDayOfWeek,
                  st->wHour, st->wMinute, st->wSecond, st->wMilliseconds);
        }
        *buflen = sizeof(SYSTEMTIME);
    }
    else if (header->value)
    {
        WCHAR *string = buffer;
        DWORD len = (strlenW( header->value ) + 1) * sizeof(WCHAR);
        if (len > *buflen)
        {
            set_last_error( ERROR_INSUFFICIENT_BUFFER );
            *buflen = len;
            return FALSE;
        }
        else if (string)
        {
            strcpyW( string, header->value );
            TRACE("returning string: %s\n", debugstr_w(string));
            ret = TRUE;
        }
        *buflen = len - sizeof(WCHAR);
    }
    return ret;
}

/***********************************************************************
 *          WinHttpQueryHeaders (winhttp.@)
 */
BOOL WINAPI WinHttpQueryHeaders( HINTERNET hrequest, DWORD level, LPCWSTR name, LPVOID buffer, LPDWORD buflen, LPDWORD index )
{
    BOOL ret;
    request_t *request;

    TRACE("%p, 0x%08x, %s, %p, %p, %p\n", hrequest, level, debugstr_w(name), buffer, buflen, index);

    if (!(request = (request_t *)grab_object( hrequest )))
    {
        set_last_error( ERROR_INVALID_HANDLE );
        return FALSE;
    }
    if (request->hdr.type != WINHTTP_HANDLE_TYPE_REQUEST)
    {
        release_object( &request->hdr );
        set_last_error( ERROR_WINHTTP_INCORRECT_HANDLE_TYPE );
        return FALSE;
    }

    ret = query_headers( request, level, name, buffer, buflen, index );

    release_object( &request->hdr );
    return ret;
}

static BOOL open_connection( request_t *request )
{
    connect_t *connect;
    char address[32];
    WCHAR *addressW;

    if (netconn_connected( &request->netconn )) return TRUE;

    connect = request->connect;
    if (!netconn_resolve( connect->servername, connect->serverport, &connect->sockaddr )) return FALSE;

    inet_ntop( connect->sockaddr.sin_family, &connect->sockaddr.sin_addr, address, sizeof(address) );
    TRACE("connecting to %s:%u\n", address, ntohs(connect->sockaddr.sin_port));
    addressW = strdupAW( address );

    send_callback( &request->hdr, WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER, addressW, 0 );

    if (!netconn_create( &request->netconn, connect->sockaddr.sin_family, SOCK_STREAM, 0 ))
    {
        heap_free( addressW );
        return FALSE;
    }
    if (!netconn_connect( &request->netconn, (struct sockaddr *)&connect->sockaddr, sizeof(struct sockaddr_in) ))
    {
        netconn_close( &request->netconn );
        heap_free( addressW );
        return FALSE;
    }

    send_callback( &request->hdr, WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER, addressW, 0 );

    heap_free( addressW );
    return TRUE;
}

void close_connection( request_t *request )
{
    if (!netconn_connected( &request->netconn )) return;

    send_callback( &request->hdr, WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION, 0, 0 );
    netconn_close( &request->netconn );
    send_callback( &request->hdr, WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED, 0, 0 );
}

static BOOL send_request( request_t *request, LPCWSTR headers, DWORD headers_len, LPVOID optional,
                          DWORD optional_len, DWORD total_len, DWORD_PTR context )
{
    static const WCHAR keep_alive[] = {'K','e','e','p','-','A','l','i','v','e',0};
    static const WCHAR no_cache[]   = {'n','o','-','c','a','c','h','e',0};
    static const WCHAR length_fmt[] = {'%','l','d',0};

    BOOL ret = FALSE;
    connect_t *connect = request->connect;
    session_t *session = connect->session;
    WCHAR *req = NULL;
    char *req_ascii;
    int bytes_sent;
    DWORD len;

    if (session->agent)
        process_header( request, attr_user_agent, session->agent, WINHTTP_ADDREQ_FLAG_ADD_IF_NEW, TRUE );

    if (connect->hostname)
        process_header( request, attr_host, connect->hostname, WINHTTP_ADDREQ_FLAG_ADD_IF_NEW, TRUE );

    if (optional_len)
    {
        WCHAR length[21]; /* decimal long int + null */
        sprintfW( length, length_fmt, optional_len );
        process_header( request, attr_content_length, length, WINHTTP_ADDREQ_FLAG_ADD_IF_NEW, TRUE );
    }
    if (!(request->hdr.flags & WINHTTP_DISABLE_KEEP_ALIVE))
    {
        process_header( request, attr_connection, keep_alive, WINHTTP_ADDREQ_FLAG_ADD_IF_NEW, TRUE );
    }
    if (request->hdr.flags & WINHTTP_FLAG_REFRESH)
    {
        process_header( request, attr_pragma, no_cache, WINHTTP_ADDREQ_FLAG_ADD_IF_NEW, TRUE );
        process_header( request, attr_cache_control, no_cache, WINHTTP_ADDREQ_FLAG_ADD_IF_NEW, TRUE );
    }
    if (headers && !add_request_headers( request, headers, headers_len, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE ))
    {
        TRACE("failed to add request headers\n");
        return FALSE;
    }

    if (!(ret = open_connection( request ))) goto end;
    if (!(req = build_request_string( request, request->verb, request->path, request->version ))) goto end;

    if (!(req_ascii = strdupWA( req ))) goto end;
    TRACE("full request: %s\n", debugstr_a(req_ascii));
    len = strlen(req_ascii);

    send_callback( &request->hdr, WINHTTP_CALLBACK_STATUS_SENDING_REQUEST, NULL, 0 );

    ret = netconn_send( &request->netconn, req_ascii, len, 0, &bytes_sent );
    heap_free( req_ascii );
    if (!ret) goto end;

    if (optional_len && !netconn_send( &request->netconn, optional, optional_len, 0, &bytes_sent )) goto end;
    len += optional_len;

    send_callback( &request->hdr, WINHTTP_CALLBACK_STATUS_REQUEST_SENT, &len, sizeof(DWORD) );

end:
    heap_free( req );
    return ret;
}

/***********************************************************************
 *          WinHttpSendRequest (winhttp.@)
 */
BOOL WINAPI WinHttpSendRequest( HINTERNET hrequest, LPCWSTR headers, DWORD headers_len,
                                LPVOID optional, DWORD optional_len, DWORD total_len, DWORD_PTR context )
{
    BOOL ret;
    request_t *request;

    TRACE("%p, %s, 0x%x, %u, %u, %lx\n",
          hrequest, debugstr_w(headers), headers_len, optional_len, total_len, context);

    if (!(request = (request_t *)grab_object( hrequest )))
    {
        set_last_error( ERROR_INVALID_HANDLE );
        return FALSE;
    }
    if (request->hdr.type != WINHTTP_HANDLE_TYPE_REQUEST)
    {
        release_object( &request->hdr );
        set_last_error( ERROR_WINHTTP_INCORRECT_HANDLE_TYPE );
        return FALSE;
    }

    ret = send_request( request, headers, headers_len, optional, optional_len, total_len, context );

    release_object( &request->hdr );
    return ret;
}
