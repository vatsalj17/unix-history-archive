


DIRED(LOCAL)        UNIX Programmer's Manual         DIRED(LOCAL)



NAME
     dired - directory editor

SYNOPSIS
     dired
     [ -[s|r][nsrw] ] [ -w[f|h|number] ] [ [dir-name|file-list] ]

DESCRIPTION
     Dired displays a long-form ls directory listing on the
     screen of a display terminal and allows you to 'edit' and
     peruse that listing by moving up and down it, deleting,
     editing, and displaying entries.  Your shell TERM variable
     should be set to the standard string which the Berkeley
     termcap library uses for distinguishing terminals.  With no
     argument, the connected directory is used. With only one
     argument, if that argument is a directory, it is used.  With
     multiple arguments, (or a single non-directory argument) the
     argument(s) are interpreted as filenames.  Dired then types
     'Reading' and gets information about the various
     files/directories in your specification. This may take a
     short while (depending on how many you give it), so it types
     one period (.) after the word 'reading' for every 10 files
     it has gathered information about. With this, you can keep
     track of its progress. Interrupts, hangups, and the like are
     disabled since your terminal is put into a special mode that
     is only changed when you quit with the 'q' command.

     Options, which are inherited by recursive invocations of
     dired:

     -[sr][n|s|r|w]
          Sort or reverse sort by Name, Size, Read date, or Write
          date respectively.  "Normal sort" is the order you are
          most likely to desire, so is largest first for size,
          and most recent for read and write date sorts.  Default
          is to sort by name.

     -w[f|h|number]
          Use number lines for the directory index window,
          reserving the other half for quick file display.  f
          means use the full screen for the index.  h means use
          half of the screen for the index.  h is the default.

     The format of the screen is as follows: each line represents
     a file (or directory), the name of which is right-most. From
     left the fields are: mode, link count, owner, size, write
     date and name. See ls(1) for a description of what each of
     these mean. You move up and down the column immediately left
     to the filename.  The bottom half of the screen is used for
     displaying files via the 't'ype command.  If there are too
     many files to all fit on one window, more windows are allo-
     cated. The 'f' and 'b' commands can be used for stepping



Printed 1/10/87                                                 1






DIRED(LOCAL)        UNIX Programmer's Manual         DIRED(LOCAL)



     forward and backward windows.  The last screen line is used
     as an 'echo' line for displaying error messages and reading
     arguments.  It also displays the full directory name if
     'direding' a directory.  When in split screen mode, the
     divider serves also as a 'linear indicator' showing where
     the current window is relative to the entire list of files.
     The symbols '(' and ')' denote the window. Square brackets
     replace
      '(' and/or ')' when the window is the first and/or last
     window.  A single 'o' is used to represent the window when
     the window size is small compared to the total number of
     files.

     Commands consist of single characters, with any necessary
     arguments prompted for, and echoed in the 'echo' line. The
     only commands which take arguments are '!' (exclamation
     point), 'r', and 's'.  The commands 'G', 'f, 'b', and the
     commands for moving up and down (e.g. <cr> and '-') may be
     preceded by a count as in 'vi'.

     Command list:

     <space>
          re-prints the dir path name.

     <ESC>
          cancels a sort or a count.
     <lf>
     ^N
     j
          steps to the next file. If this crosses a window boun-
          dary, the next window is displayed with a one line
          overlap. May be preceded by a count.

     ^
     -
     k
     <backspace>
     ^P   steps to previous file. If this crosses a window boun-
          dary, the previous window is displayed with a one line
          overlap. May be preceded by a count

     !    prompts for a system command to invoke. The command is
          executed, and confirmation is required before returning
          to the display. All % characters in the command are
          replaced with the full pathname of the current entry,
          and all # chars are replaced with just the trailing
          filename component (what you see on the screen).

     .    Repeats the previous ! shell command, substituting the
          current entry for any special chars (%#) in the origi-
          nal command.



Printed 1/10/87                                                 2






DIRED(LOCAL)        UNIX Programmer's Manual         DIRED(LOCAL)



     \    changes from split-screen mode to full-screen mode, or
          vice-versa.

     /    locates a file matching the given regular expression,
          where the expressions are as defined for re_comp(3) and
          re_exec(3). The search occurs in the forward direction.

     a    aborts out of the current directory. No deletions are
          done.

     A    aborts completely out of dired, with no deletions.

     b    goes backward a window, leaving a one line overlap. May
          be preceded by a count.

     c    refreshes the current line.

     d    marks for deletion the current entry. Upon exit and
          confirmation (or re-reading using the 'R' command),
          this entry will be deleted.  WARNING: this includes
          directories!  If it is a directory, everything in it
          and underneath it will be removed.  An entry that is
          marked for deletion will also be put in boldface if
          your terminal can handle it.

     e    runs the editor defined in your EDITOR environment
          variable upon the current file.  If EDITOR is not
          defined, 'vi' is used.  However, if the current file is
          a directory it is not edited, but rather, dired forks a
          copy of itself upon that directory. In this manner, you
          can examine the contents of that directory and thus
          move down the directory hierarchy.

     E    goes up to the next higher level directory. In the case
          of an argument list of files to dired, it goes to the
          parent of the directory which contains the current
          file.

     f    goes forward a window, leaving a one line overlap.  May
          be preceded by a count.

     <controlG>
          shows the current file number, the total number of
          files, and the percentage through the file. Useful in
          full screen mode when there is no linear indicator.

     G    goes to the file number given by the preceding count.
          With no count, goes to the last file as in 'vi'.
     ?    displays a help file.

     l
     ^L   refreshes the current window.



Printed 1/10/87                                                 3






DIRED(LOCAL)        UNIX Programmer's Manual         DIRED(LOCAL)



     m    runs Berkeley's more program on the current entry.

     n    find the next instance of the previously defined reg
          expression, searching in the forward direction.

     N    find the next instance of the previously defined reg
          expression, searching in the reverse direction.

     p    prints the full path name of the current file; embedded
          control characters are shown with graphics.

     P    prints the current file on the line-printer.

     q    exits the program, displaying files marked for deletion
          and requiring confirmation before deleting them. If no
          confirmation is given (typing anything other than y),
          dired goes back to its display.

     R    re-reads the directory or file list. If files are
          marked for deletion, will first ask for confirmation
          and then delete them before re-reading.  Useful after
          operations done during shell escapes (e.g. chmod).

     Q    quits dired, with no deletions, pushd's you into the
          dir of the current file.

     r
     s    sorts the file list by various fields: name, read date,
          size, write date. Only the first letter (e.g. n, r, s,
          or w) is required after giving the r and s commands. s
          sorts in increasing alphabetic, decreasing size, newest
          to oldest dates. r reverses the sense of s. For the n,
          s, and w subcommands, the date field is the write date.
          For the r subcommand, the date field is the read date.
          Whenever a sort is done, you are positioned at the top
          of the list afterwards.  A sort can be aborted via
          <ESC>.

     t    types the file out to the terminal, which is consider-
          ably faster than firing up an editor on the file. If in
          two-window mode, the bottom window is used, pausing
          after each screenful.  The type-out may be interrupted
          by Ctrl-C or 'q'.  If the file is a symbolic link, the
          contents of the link are printed (i.e.  the name of the
          file the link is pointing at).

     T    same as 't' but without any pauses.

     u    undeletes the current entry, if it was previously
          marked for deletion.





Printed 1/10/87                                                 4






DIRED(LOCAL)        UNIX Programmer's Manual         DIRED(LOCAL)



FILES
      /usr/local/dired
      /usr/local/lib/dired.hlp    help file for ? and h

SEE ALSO
     ls(1)

DIAGNOSTICS
     The error messages are basically self-explanatory.

AUTHOR
     Stuart Mclure Cracraft
     Enhancements by Jay Lepreau
     Fixes and enhancements by Charles Hill
     Full 4.2 support (4.2 support?) by Dave Cohrs @ Wisconsin

BUGS
     Long lines sometimes screw up the 't' display.





































Printed 1/10/87                                                 5



