#! /usr/bin/perl -w

# man2txh.pl, version 0.1
# Copyright 1999 by Richard Dawe
#
# This is a vague attempt at writing a man page to texinfo include file
# convertor, to try and remove some of the drudgery of writing texinfo docs
# for libsocket.

if ($ARGV[0] eq '') {
	die 'Please specify man page file to convert';
} else {
	open(FH, "<$ARGV[0]") || die;
}

# Print the standard header
print <<EOT;
\@comment The start of a comment is indicated by the following comment. This
\@comment syntax is used by the script funcstxh.sh: \@comment function <name>

EOT

# Figure out the man page name
$PAGENAME = $ARGV[0];
$PAGENAME =~ s?.+/(.+)\.\w+?$1?;

if ($PAGENAME ne '') {
	print "\@comment function $PAGENAME\n\n";
	print "\@node $PAGENAME, , , Function Reference\n";
	print "\@section $PAGENAME\n\n";
} else {
	# The converted page will require changes
	print "\@comment function \@PAGE NAME HERE\@\n\n";
	print "\@node \@PAGE NAME HERE\@, , , Function Reference\n";
	print "\@section \@PAGE NAME HERE\@\n\n";
}

# Print the copyright header
print <<EOT;
\@c This file was originally a Linux man page - see the copyright below. It
\@c was modified for use in libsocket by Richard Dawe <richdawe\@bigfoot.com>.

EOT

while (<FH>) {
	# Handle redirections in the same man page tree
	if (m/^\.so/) {
		$NEWFILENAME = $_;
		$NEWFILENAME =~ s/^\.so\s+//;
		$NEWFILE     = $ARGV[0];
		$NEWFILE     =~ s?^(.+/).+/.+$?$1$NEWFILENAME?;
		close(FH);
		open(FH, "<$NEWFILE") || die;

		# Let the user know
		print STDERR
		      "Followed redirection from $ARGV[0] to $NEWFILE\n";

		next;
	}

	# Don't allow spaces in code, but allow anything else
	s/\\fB(\S+)\\fP/\@code{$1}/g; # Functions (& vars)
	s/\\fI(\S+)\\fP/\@code{$1}/g; # Other stuff

	# Bold, italics => sample text?
	s/^\.B (.+)/\@samp{$1}/;
	s/^\.I (.+)/\@samp{$1}/;

	# Handle subheadings
	s/^\.SH "*(\w)(.+)"*$/"\n\@subheading ".uc($1).lc($2)."\n"/e;
	s/^(\n\@subheading)(.+)"(\n)$/$1$2$3/g;

	# Comments
	s/^\.\\"/\@c/;

	# Paragraph separators are superfluous
	s/^\.sp.*//;
	s/^\.PP.*//;

	# Man page header - naaah
	s/^\.TH.*//;

	# .TP are superfluous
	s/^\.TP.*//;

	print;
}

close(FH);
