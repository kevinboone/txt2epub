# txt2epub

Version 0.0.3, May 2023

## What is this?

`txt2epub` is a command-line utility for Linux,
for converting one
or more plain text files into an EPUB document. It will insert the
standard author/title meta-data, generate a table of contents,
and can include a cover image.
Limited formatting is possible using Markdown-style text markup,
or full XHTML if required. 
<p/>
This utility is intended as a relatively quick way to convert books provided
as plain text into a format that can be handled more easily by 
e-readers. Although most portable reading devices and software
can handle plain text perfectly well, the
lack of meta-data or a cover image makes collections of
such documents unwieldy.

Although it is not its main function,  
`txt2epub` can be used with just a plain text editor
to produce a commercial-quality EPUB novel, 
that will pass most publishers' validation checks. However, 
books that have complex formatting, or embedded images, 
need a more sophisticated approach. 

One of the design goals of `txt2epub` is to produce "clean"
documents, free of software-specific stylesheets and formatting.  The EPUBs it
creates will not specify fonts, absolute text sizes, colours, margins, or
layout. The way the text is
rendered is thus completely under the control of the reader. Its output should
thus be acceptably readable on screens of different sizes. 


## Example usage

    txt2epub\ -o\ dickens\_great\_expectations.epub \
      --author\ "Dickens,\ Charles"\ --title\ "Great\ Expectations" \ 
      --cover-image ge.jpg\
      chapter01.txt chapter02.txt chapter03.txt\ ...

Convert files `chapter01.txt`, etc., into an 
EPUB document, setting the 
author and title meta-data appropriately. Each file will receive an
entry in the table of contents. The image `ge.jpg` will
form the book cover. 


## Prerequisites

The only external dependencies are on the standard 
linux `zip`
utility, and the PCRE regular expression parsing library. Both
should be available in the repositories of most Linux distributions. 
For RHEL/Fedora: `yum install zip pcre-devel`.

`txt2epub` will probably build and run on other
Linux-like systems, but this has not been tested. 


## Building and installing

The usual:

    $ make
    $ sudo make install

`txt2epub` may be found in the binary repositories of some Linux distributions.
While installing from a repository will usually be quicker than building
from source, repositories are often less up-to-date than the source.

## Notes

### Markdown support

Unless it is disabled (`--ignore-markdown`), `txt2epub`
processes a small set of markdown-type formatting markers:

    #This is a heading
    ##This is a subheading
    ###This is a subsubheading
    This is _italic_. This is *bold*

A line that ends in two spaces (which may not be visible at all in a text
editor) is terminated with a line-break. This is a simple way to
include pre-formatted text.

These markdown constructions are turned into basic (X)HTML 
tags (not style classes).

Note that Markdown-style markup cannot span lines. A very long italic
passage, for example, must be rendered as a single line, or the
italic marker repeated on subsequent lines. 
`txt2epub` does not support Markdown list constructs, but 
these could be created using (X)HTML tags, if desired.

### XHTML support

Any text files supplied to 
`txt2epub` are just inserted into the body of an XHTML 
document, as this is the presentation format that EPUB specifies.
It follows that files can contain XHTML markup if necessary.
XHTML is a stricter standard than HTML, and many ordinary HTML files will not
pass muster -- a typical problem is not closing tags, or using
the wrong letter case in tags.

Providing XHTML can be useful for heavily formatted pages such as a
title page or dedication. However, the files provided should
_not_ be full XHTML files --   
`txt2epub` will write the proper XHTML header and footer.
Instead, just provide the body text.

Many commercial e-readers use existing HTML rendering software to 
display text; in such cases you might get away with using ordinary
HTML. However, it isn't strictly correct, and won't pass a
publisher's validation checks.
 
### Table of contents

`txt2epub` does not write a contents page in the document.
However, it does write an NCX table of contents, which most e-readers
will be able to display at any point whilst reading a book. 
This form of table of contents also enables the next-chapter/previous-chapter
controls on readers that have them.

By default, the
entries in the table of contents will be taken from
the input filename, after removing any extension. So a nicely-formatted
table of contents will require that the filenames are as a reader
should see them, with capital letters where appropriate and spaces
between words. 
<p/>
An alternative approach to generating a table of contents is
to ensure that the first line of each file is a chapter heading,
and use the `--first-lines` switch. This will also format
the first line as a heading (specifically, it will embed it in an H1 tag).

### Input text formatting issues

E-book text files tend to be formatted in one of four ways:

* One very long line per paragraph, with a blank line between each paragraph
* One very long line per paragraph, with no blank lines between paragraphs
* Variable-length lines with blank lines to indicate paragraph breaks
* Variable-length lines with no blanks; paragraph breaks are indicated by white-space intended lines

No special effort is required to handle the first type. The second
type will be formatted by most readers as a solid block of uninterrupted
text, which is not pleasant to read. The `--extra-para` 
switch might help here, by inserting a paragraph break after each 
input line. 

Files of the third type present no problem.

`txt2epub` attempts to handle the fourth type by treating any
line that starts with three or more whitespace characters as a paragraph
break. Because some files that are formatted as variable-length lines
end up with spaces at the start of each line, this behaviour can be
turned off using `--ignore-indent`.

### stdin

<code>txt2epub</code> will read from standard input (stdin) if
a minus sign (-) is used for the filename. It will be necessary
to specify the EPUB filename (<code>-o</code>) in such a case.


### Cover image support

The switch `--cover-image` can be used to provide the EPUB
document with a leading cover page. This image is presented as a single-page
without any annotation, at the start of the book. EPUB guidelines 
suggest that a cover image should be 590 pixels wide by 750 high. No
check is made that the image meets this guideline -- it is simply
copied into the EPUB. An error message will be shown if the image file
does not exist, but the EPUB will still be created.
<p/>
The EPUB specification states that images files must be in JPEG, GIF,
SVG, or PNG formats. No checks are made that this rule is being followed --
`txt2epub` will install images of any type but, as with
wrongly sized images, EPUB viewers vary in their willingness to 
display them. 

## Hints

### Splitting long documents

`txt2epub` has no built-in support for splitting long
text files into sections or chapters. There are many ways in which
this might be done, and Linux already has useful utilities for doing it.

Consider, for example, a long file called `fred.txt`, that
is divided into sections headed by "Chapter 1", "Chapter 2", etc.
This can be split into chapters like this:

    csplit -f chapter_ -b %02d.txt fred.txt /Chapter.*/ {*}

This command will create the files `chapter\_00.txt`,
`chapter\_01.txt`, etc. These chapters can then be 
assembled into an EPUB like this:

    txt2epub -a "Fred Blogs" -t "My Life as a Dog" -f -o blogs.epub\ chapter*.txt

(being careful about the use of the filename wildcard, as discussed
above.)

The `-f` switch instructs `txt2epub` to use the
first line of each file as a chapter heading, both in formatting and
in the table of contents. This works here because the use of `csplit`
 ensures
that every file (with the possible exception of the first) begins
with the specified pattern. 

### Character encoding

EPUB text is required to be formatted as UTF-8. Plain ASCII works fine,
as it is a subset of UTF-8. 8-bit extended ASCII variants will display
with varying degrees of ugliness, depending on how many extended
characters are used. A typical symptom of encoding mismatches of this
sort is to see double-quotes rendered as upside-down question marks, or
similar punctuation errors.

`txt2epub` assumes that all text input is in UTF-8 or ASCII format.
If this assumption causes problems, the `iconv` utility
may be used to pre-process the text and fix the encoding. Unfortunately,
if you receive a text document that has been converted from
Microsoft Word or some other proprietary word processor, it can often
be quite difficult to guess what the character encoding is. Consequently,
some trial-and-error may be needed.


### Converting PDF, etc

`txt2epub` can not decode PDF documents, but reasonable results
may sometimes be obtained by using it to process the output of
`pdftotext -layout -nopgbrk`. The `-layout` switch tells 
`pdftotext` to
attempt to preserve page layout; this is usually impossible, but it does
mean that you will usually get blank lines between paragraphs. These
are needed for `txt2epub` to identify paragraph breaks.
The `-nopgbrk` switch prevents page break (ctrl-L) characters
being written into the text. These don't usually cause problems in
EPUB viewers -- in fact, they are usually ignored. But, strictly speaking,
they are illegal in UTF-8 XML. 

Documents converted from print sources often have page numbers and
other unhelpful text embedded in the document body. Most of this is
difficult to remove, but 
`txt2epub` will attempt to remove page numbers, if the
`--remove-pagenum` switch is specified. A page number is
taken to be any line that consists of white space, followed by
digits. Unfortunately, while (for example) a single line containing 
"23" will be removed, "Page 23" won't. Documents with this kind
of detritus may need more sophisticated pre-processing.

### Changing paragraph output format

By default `txt2epub` writes plain paragraph tags to delineate paragraphs
in the output. Ebook readers usually render this formatting with a 
blank line between paragraphs. Using the `--para-indent` switch will
make the utility output a `<style>` header to set the paragraph 
separation to a plain left indent, which can be help when reading on a 
small screen. In general `txt2epub` does not try to control formatting,
in the hope that viewer software will be sufficiently flexible as to
allow the user to choose preferences. This -- paragraph separation --
is an area when viewer software tends to fall short.

## Bugs and limitations

`txt2epub` presently does not remove unnecessary byte-order
markers and similar encoding detritus from text files. 

Users should be wary of using constructions like "book\*.txt" to include
lists of files. While Linux shells usually present files in alphanumeric order,
subtleties like locale and collation settings can modify this. It may be 
safer to list the files explicitly.

Files created by this utility will not necessarily pass the validation
in  
`epubcheck`
because the "mimetype" file is not always the first in the archive. This
can be fixed if it causes problems with readers; so far none have been
reported.

`txt2epub` does not write a "guide" section in the 
NCX table-of-contents. This is optional and, so far as I know, no EPUB
reader takes much notice of it. 


## More information

More detailed command-line usage information is avaialble in the manual:
`man txt2epub`.

## Legal, etc

<code>txt2epub</code> is maintained by Kevin Boone and other contributors, and
distributed under the terms of the GNU Public Licence, version 3.0.
Essentially, you may do whatever you like with it, provided the original
authors are acknowledged, and you accept the risks involved in its use. 

## Revisions

0.0.3, May 2023

Fixed a nasty bug where space indents were being processed in the first
line of a file, causing the header to be split between paras

0.0.2, May 2023

Added `--para-indent` feature (contributed by KenH2000)

