
# Very simple xml parser

Simple xml parser that does not allocate memory.

## Why?

libxml2, aws_xml_parser etc are complicated and allocate memory.
aws_xml_parser is incomplete and could not handle self closing xml tags.


All we want to do is iterate through an xml doc. If the user wants to
create another representation of the doc in memory they are free to do that.

## Usage

Depth first traversal of the xml tree:

```
  struct jxml_range doc = {/* provide memory range of xml doc */ };
  jxml_tag tag = {0};
  struct jxml_range tag_name = {0};
  struct jxml_attr attr = {0};
  struct jxml_range tag_body = {0};
  
  while (jxml_next_tag(&doc, tag, &tag) == JXML_SUCCESS &&
	 jxml_get_tag_name(&doc, tag, &tag_name) == JXML_SUCCESS){

   // print tag name
   printf(JXML_PRINT_FMT "\n", JXML_PRINT_RANGE(tag_name));

   // get attrs
   while (jxml_next_attr(&doc, tag, &attr, &attr) == JXML_SUCCESS){
         printf("  " JXML_PRINT_FMT "=" JXML_PRINT_FMT "\n"
             ,JXML_PRINT_RANGE(attr.name),
	      JXML_PRINT_RANGE(attr.value));
    }

    //Get body
    if (jxml_get_tag_body(&doc, tag, &tag_body) == JXML_SUCCESS){
      printf(JXML_PRINT_FMT "\n----\n", JXML_PRINT_RANGE(tag_body));
    }
  }
```