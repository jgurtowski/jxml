#ifndef JXML_H
#define JXML_H

#include <stdlib.h>
#include <stdint.h>

typedef uint8_t* jxml_tag;

struct jxml_range {
  uint8_t *start;
  uint8_t *end;
};

struct jxml_attr {
  struct jxml_range name;
  struct jxml_range value;
};

enum jxml_error {
  JXML_SUCCESS,
  JXML_MALFORMED_XML,
  JXML_NO_MORE_TAGS,
  JXML_NO_MORE_ATTRS,
  JXML_ATTRS_MUST_BE_QUOTED,
  JXML_BODYLESS_TAG,
  JXML_COULD_NOT_FIND_ATTR,
  JXML_COULD_NOT_FIND_TAG,
};

static const char *jxml_error_message[] = {
  "NO ERROR",
  "XML is malformed",
  "Could not find any more tags in xml doc",
  "No more attrs for tag",
  "JXML requires attribute values to be quoted",
  "Tag is bodyless, i.e. <name />",
  "Requested attr does not exist in tag",
  "Requested tag does not exist in doc",
};

#define JXML_ERROR_STR(i) (jxml_error_message[i])

#define JXML_PRINT_RANGE(r) ((int)(r.end - r.start) + 1), (r.start)

#define JXML_PRINT_FMT "%.*s"  

#define JXML_RANGE_LEN(r) ((size_t)((r.end - r.start) + 1))
#define JXML_RANGE_LEN_PTR(r) ((size_t)((r->end - r->start) + 1))

#define JXML_RANGE_FROM_STR_LITERAL(literal) {.start = (uint8_t *)(const char *)(literal), \
      .end = (uint8_t *)(const char *)(literal) + (sizeof(literal) - 2) }


int jxml_next_tag(const struct jxml_range const *doc,
		  const jxml_tag prev_tag,
		  jxml_tag *next_tag_out);

int jxml_get_tag_name(const struct jxml_range const *doc,
		      const jxml_tag tag,
		      struct jxml_range *name_out);

int jxml_get_tag_by_name(const struct jxml_range const *doc,
			 const struct jxml_range const *search_tag_name,
			 jxml_tag *tag_out);

int jxml_next_attr(const struct jxml_range const *doc,
		   const jxml_tag tag,
		   const struct jxml_attr const *prev_attr,
		   struct jxml_attr *attr_out);

int jxml_get_attr_by_name(const struct jxml_range const *doc,
			  const jxml_tag tag,
			  const struct jxml_range const *attr_name,
			  struct jxml_attr *attr_out);

int jxml_get_tag_body(const struct jxml_range const *doc,
		      const jxml_tag tag,
		      struct jxml_range *body_out);

int jxml_range_compare( const struct jxml_range const *a,
			const struct jxml_range const *b);

#endif
