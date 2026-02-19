Include information for grader about your assign5 here

Finished all parts of the extension
1. Left and Right arrow keys move the cursor within the current line and allow inserting and deleting characters at the point of the cursor.

2. ctrl-a and ctrl-e to move the cursor to the first and last character of the line

3. ctrl-l to clear the screen (Preserves what you are writing in the current line like zsh)

4. history command to see a rolling list of previously executed commands numbered. Last 10 commands entered. The numbers will keep increasing as they correspond to the actual number of the command itself compared to all commands ever entered rather than just the last 10.
i.e if 20 commands are entered the list will look like
11 echo
12 echo
13 echo
14 echo
15 echo
16 echo
17 echo
18 echo
19 echo
20 echo

5. Up and down arrow keys to access commands from the history. Typing an up arrow moves backwards to earlier commands in the history. Typing a down arrow moves forward to commands later in the history. shell_bell to alert the user if they try to move beyond either end of the history. When the user chooses to recall a command from the history, that command replaces the current line and the cursor is placed at the end of the line, the user can reissue by typing enter. The user can also use command-line editing to modify the recalled command such as to fix a typo before reissuing it. 

6. !! to repeat the last command, displaying what command its executing onto the next line before the execution starts (just like zsh it will repeat faulty commands as well).

7. disassemble (using the assign3 disassemble extension). It takes a single argument which is a 4-byte aligned address and returns the disassembled instruction at that address in this form:
[The address entered reformatted]:     [mneumonic] [registers]
