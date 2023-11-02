#ifndef HTML_H_
#define HTML_H_

// begin text
#define HTML_BEGIN "<pre>\n"
// end text
#define HTML_END "</pre>\n"

// Colors
#define HTML_FONT_RED   "<font color=\"red\">"
#define HTML_FONT_GREEN "<font color=\"green\">"

#define HTML_END_FONT "</font>"

// Macros:
#define HTML_TEXT(str)  HTML_BEGIN str HTML_END

#define HTML_RED(str)   HTML_FONT_RED str HTML_END_FONT
#define HTML_GREEN(str) HTML_FONT_GREEN str HTML_END_FONT

#define HTML_H1(str) "<h1>" str "</h1>\n"
#define HTML_H2(str) "<h2>" str "</h2>\n"
#define HTML_H3(str) "<h3>" str "</h3>\n"

#endif // #ifndef HTML_H_
