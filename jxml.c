#include "jxml.h"

int jxml_get_tag_name(const struct jxml_range const *doc,
		      const jxml_tag tag,
		      struct jxml_range *name_out){
  uint8_t *start = tag;
  if (*start != '<')
    return JXML_MALFORMED_XML;

  for (;start <= doc->end
	 && *start != '>'
	 && *start != '\n'
	 && *start != ' ';
       start++){}
  //start == tag + 1 because start is currently one of the break chars
  if (start > doc->end || start == tag + 1)
    return JXML_MALFORMED_XML;

  name_out->start = tag + 1;
  name_out->end = start - 1;

  return JXML_SUCCESS;
}

// find the next tag in the depth first search
int jxml_next_tag(const struct jxml_range const *doc,
		  const jxml_tag prev_tag,
		  jxml_tag *next_tag_out){
  
  uint8_t *start = prev_tag == NULL ? doc->start : (prev_tag + 1);
  
  while(1){
   
    if ( start == NULL || start > doc->end ){
      return JXML_MALFORMED_XML;
    }

    for(; start <= doc->end 
	  && *start != '<'; start++){}
    
    if (start > doc->end){
      return JXML_NO_MORE_TAGS;
    }
    
    if (!((start + 1) <= doc->end)){
      return JXML_MALFORMED_XML;
    }

    if (*(start + 1) != '/'){
      *next_tag_out = start;
      return JXML_SUCCESS;
    }
    start++;
  } 
}


int jxml_get_tag_by_name(const struct jxml_range const *doc,
			 const struct jxml_range const *search_tag_name,
			 jxml_tag *tag_out){
  jxml_tag tag = {0};
  struct jxml_range tag_name = {0};
  int next_tag_ret, get_tag_name_ret;
  
  while ((next_tag_ret = jxml_next_tag(doc, tag, &tag)) == JXML_SUCCESS
	 && (get_tag_name_ret = jxml_get_tag_name(doc, tag, &tag_name)) == JXML_SUCCESS){
    if( jxml_range_compare(search_tag_name, &tag_name) == 0 ){
      *tag_out = tag;
      return JXML_SUCCESS;
    }
  }

  if (next_tag_ret == JXML_NO_MORE_TAGS)
    return JXML_COULD_NOT_FIND_TAG;
  
  return next_tag_ret == JXML_SUCCESS ? get_tag_name_ret : next_tag_ret;
}

int jxml_next_attr(const struct jxml_range const *doc,
		   const jxml_tag tag,
		   const struct jxml_attr const *prev_attr,
		   struct jxml_attr *attr_out){
  
  uint8_t *start = prev_attr->value.end < tag ? tag : prev_attr->value.end;

  for(;start <= doc->end && *start != '>' && *start != '='; start++){}

  if (start > doc->end)
    return JXML_MALFORMED_XML;
    
  if (*start == '>')
    return JXML_NO_MORE_ATTRS;

  //else we found '=', get the name and values
  //NOTE: assume all values are quoted
  struct jxml_range name = {0};
  struct jxml_range value = {0};

  //assume all attr values are quoted
  if (start + 1 > doc->end || *(start + 1) != '"')
    return JXML_ATTRS_MUST_BE_QUOTED;
  
  name.end = start - 1;
  name.start = start - 1;
  // + 2 b/c of the quote char
  value.start = start + 2;
  value.end = start + 2;

  //expand name->start
  //break chars '\n' ' ' '>' '<' bod eod
  for (;name.start >= doc->start
	 && *name.start != '\n'
	 && *name.start != ' '
	 && *name.start != '<'; name.start--){}

  if (name.start < doc->start || *name.start == '<')
    return JXML_MALFORMED_XML;

  /*name->start has to be ' ' or '\n' here*/
  if (name.start == name.end)
    return JXML_MALFORMED_XML;
  
  name.start++;
  
  //expand value->end
  for (;value.end <= doc->end
	 && *value.end != '\n'
	 && *value.end != ' '
	 && *value.end != '>'
	 && *value.end != '"'; value.end++){}

  if (value.end > doc->end)
    return JXML_MALFORMED_XML;

  if (*value.end != '"')
    return JXML_ATTRS_MUST_BE_QUOTED;

  //empty value
  if (value.end == value.start)
    return JXML_MALFORMED_XML;
  
  value.end--;

  attr_out->name = name;
  attr_out->value = value;
  
  return JXML_SUCCESS;
}


int jxml_get_attr_by_name(const struct jxml_range const *doc,
			  const jxml_tag tag,
			  const struct jxml_range const *attr_name,
			  struct jxml_attr *attr_out){
  struct jxml_attr tmp = {0};
  int next_attr_ret;

  while ((next_attr_ret = jxml_next_attr(doc, tag, &tmp, &tmp)) == JXML_SUCCESS){
    if (jxml_range_compare(attr_name, &tmp.name) == 0){
      *attr_out = tmp;
      return JXML_SUCCESS;
    }
  }

  if (next_attr_ret == JXML_NO_MORE_ATTRS)
    return JXML_COULD_NOT_FIND_ATTR;

  return next_attr_ret;
}

int jxml_get_tag_body(const struct jxml_range const *doc,
		      const jxml_tag tag,
		      struct jxml_range *body_out){
  uint8_t *start = tag;
  uint8_t *end = {0};

  //check if we have a bodyless tag
  //find end of tag
  for (;start <= doc->end
	 && *start != '>'
	 ; start++){}

  if ( start > doc->end || start <= doc->start)
    return JXML_MALFORMED_XML;

  if (*(start -1) == '/')
    return JXML_BODYLESS_TAG;

  //get the tag name
  struct jxml_range tag_name;
  int get_name_ret;
  if ((get_name_ret = jxml_get_tag_name(doc,tag,&tag_name))
      != JXML_SUCCESS){
    return get_name_ret;
  }

  struct jxml_range potential_closer;
  
  end = start;
  while (1){
    for(;end <= doc->end
	  && *end != '<'
	  ;end++){}

    if(end + 1 > doc->end)
      return JXML_MALFORMED_XML;

    if ( *(end + 1) == '/' ){

      if (end + 2 + JXML_RANGE_LEN(tag_name) > doc->end)
	return JXML_MALFORMED_XML;
      
      potential_closer.start = end + 2;
      potential_closer.end = potential_closer.start + JXML_RANGE_LEN(tag_name) - 1;
      
      if(jxml_range_compare( &tag_name, &potential_closer) == 0){
	body_out->start = start + 1;
	body_out->end = end - 1;
	return JXML_SUCCESS;
      }
    }
    end++;
  }
}

int jxml_range_compare( const struct jxml_range const *a,
			const struct jxml_range const *b){
  int len_diff;
  if ( (len_diff = JXML_RANGE_LEN_PTR(a) - JXML_RANGE_LEN_PTR(b)) != 0 )
    return len_diff;

  for( int i = 0; i < JXML_RANGE_LEN_PTR(a); ++i){
    if ( (len_diff = *(a->start + i) - *(b->start + i)) != 0)
      break;
  }
  return len_diff;
}
