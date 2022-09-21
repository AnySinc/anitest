#ifndef _UTILS_URI_H
#define _UTILS_URI_H

#ifdef __cplusplus
extern "C" {
#endif

#define uri_malloc malloc
#define uri_free free

#include <stdbool.h>
#include <stdlib.h>

/**
 * Based on RFC 3986
 *
 *    The following is a example URIs and their component parts:
 *
 *      foo://user:password@example.com:8042/over/there?name=ferret#nose
 *      \_/   \____________________________/\_________/ \_________/ \__/
 *       |                |                     |            |       |
 *     scheme        authority                path        query   fragment
 *
 *
 *      user:password@example.com:8042
 *      \__________/ \_________/ \__/
 *           |            |       |
 *       userinfo       host     port
 * --------------------------------------------------------------------
 *
 */

struct uri {
	const char *str; // Full string
	const char *scheme;
	const char *host;
	const char *userinfo;
	const char *port;
	const char *path;
	const char *query;
	const char *fragment;

	char buf[];
};

/**
 * Parse uri.
 *
 * Internally, it does a single allocation but each part is also represented as
 * NULL ended string.
 *
 * E.g :
 *
 * struct uri* uri;
 *
 * struct uri* uri;
 * uri =
 * uri_create("http://user:pass@any.com:8042/over/there?name=jane#doe");
 *
 * printf("%s \n", uri->str);       // prints full string.
 * printf("%s \n", uri->scheme);    // prints "http"
 * printf("%s \n", uri->host);      // prints "any.com"
 * printf("%s \n", uri->userinfo);  // prints "user:pass"
 * printf("%s \n", uri->port);      // prints "8042"
 * printf("%s \n", uri->path);      // prints "/over/there"
 * printf("%s \n", uri->query);     // prints "name=jane"
 * printf("%s \n", uri->fragment);  // prints "doe"
 *
 *
 * @param str uri string
 * @return    uri, NULL on error
 */
struct uri *uri_create(const char *str);

/**
 * @param uri uri
 */
void uri_destroy(struct uri **uri);

#ifdef __cplusplus
}
#endif

#if	defined(_UTILS_IMPL) || defined(_UTILS_URI_IMPL)

#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

struct uri *uri_create(const char *str)
{
	const char *s1 = "%.*s%.*s%.*s%.*s%.*s%.*s%.*s%.*s";
	const char *s2 = "%.*s%c%.*s%c%.*s%c%.*s%c%.*s%c%.*s%c%.*s%c";
	const char *authority = "//";

	int diff, ret;
	unsigned long val;
	size_t full_len, parts_len;
	size_t scheme_len, authority_len = 0, userinfo_len = 0;
	size_t host_len = 0, port_len = 0, path_len;
	size_t query_len = 0, fragment_len = 0;

	char *scheme, *userinfo = "", *host = "", *port = "";
	char *path, *query = "", *fragment = "";
	char *ptr, *dest, *parse_end;
	char *pos = (char *) str;
	struct uri *uri;

	if (str == NULL || (ptr = strstr(pos, ":")) == NULL) {
		return NULL;
	}

	scheme = pos;
	scheme_len = ptr - str + 1;
	pos = ptr + 1;

	if (*pos == '/' && *(pos + 1) == '/') {
		authority_len = 2;
		pos += authority_len;

		ptr = strchr(pos, '@');
		if (ptr != NULL) {
			userinfo = pos;
			userinfo_len = ptr - pos + strlen("@");
			pos = ptr + 1;
		}

		ptr = pos + strcspn(pos, *pos == '[' ? "]" : ":/?#");
		host = pos;
		host_len = ptr - pos + (*host == '[');
		pos = host + host_len;

		if (*host == '[' && *(host + host_len - 1) != ']') {
			return NULL;
		}

		ptr = strchr(pos, ':');
		if (ptr != NULL) {
			if (*(ptr + 1) == '\0') {
				return NULL;
			}

			errno = 0;
			val = strtoul(ptr + 1, &parse_end, 10);
			if (errno != 0 || val > 65536) {
				return NULL;
			}

			port = ptr;
			port_len = parse_end - ptr;
			pos = port + port_len;
		}
	}

	path = pos;
	path_len = strcspn(path, "?#");
	pos = path + path_len;

	ptr = strchr(pos, '?');
	if (ptr != NULL) {
		query = ptr;
		query_len = strcspn(query, "#");
		pos = query + query_len;
	}

	if (*pos == '#') {
		fragment = pos;
		fragment_len = strlen(pos);
	}

	full_len = scheme_len + authority_len + userinfo_len + host_len +
		   port_len + path_len + query_len + fragment_len + 1;

	parts_len = full_len - authority_len;
	parts_len += 7; // NULL characters for each part.
	parts_len -= (scheme_len != 0);
	parts_len -= (userinfo_len != 0);
	parts_len -= (port_len != 0);
	parts_len -= (query_len != 0);
	parts_len -= (fragment_len != 0);

	uri = uri_malloc(sizeof(*uri) + parts_len + full_len);
	if (uri == NULL) {
		return NULL;
	}

	ret = snprintf(uri->buf, full_len, s1, scheme_len, scheme,
		       authority_len, authority, userinfo_len, userinfo,
		       host_len, host, port_len, port, path_len, path,
		       query_len, query, fragment_len, fragment);
	if (ret < 0 || (size_t) ret != full_len - 1) {
		goto error;
	}

	dest = uri->buf + strlen(uri->buf) + 1;

	scheme_len -= (scheme_len != 0);     // Skip ":"
	userinfo_len -= (userinfo_len != 0); // Skip "@"
	diff = port_len != 0;
	port_len -= diff; // Skip ":"
	port += diff;	  // Skip ":"
	diff = (query_len != 0);
	query_len -= diff; // Skip "?"
	query += diff;	   // Skip "?"
	diff = (fragment_len != 0);
	fragment_len -= diff; // Skip "#"
	fragment += diff;     // Skip "#"

	ret = sprintf(dest, s2, scheme_len, scheme, 0, userinfo_len, userinfo,
		      0, host_len, host, 0, port_len, port, 0, path_len, path,
		      0, query_len, query, 0, fragment_len, fragment, 0);
	if (ret < 0 || (size_t) ret != parts_len - 1) {
		goto error;
	}

	uri->str = uri->buf;
	uri->scheme = dest;
	uri->userinfo = dest + scheme_len + 1;
	uri->host = uri->userinfo + userinfo_len + 1;
	uri->port = uri->host + host_len + 1;
	uri->path = uri->port + port_len + 1;
	uri->query = uri->path + path_len + 1;
	uri->fragment = uri->query + query_len + 1;

	return uri;

error:
	uri_free(uri);
	return NULL;
}

void uri_destroy(struct uri **uri)
{
	if (uri == NULL || *uri == NULL) {
		return;
	}

	uri_free(*uri);
	*uri = NULL;
}

#endif
#endif