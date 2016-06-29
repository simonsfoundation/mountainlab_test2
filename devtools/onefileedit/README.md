# onefileedit

Edit a bunch of files simultaneously as though they were one single file!

Have you ever wanted to edit all source files in a single directory (or directory tree) as though they were a single file?
It's not just for search/replace. Sometimes you want to scroll through everything at once in your favorite code editor.

Here's what you can do from the console:

```bash
./onefileedit my/folder --type=cpp --editor=/usr/bin/emacs --recursive=1
```

Then you see all the files concatenated in your editor (separated by some header syntax of course).
After editing and saving, return to the console and you will have the option to reject or accept the changes for each modified file.

Please keep a backup of your code because I can't be held responsible if something goes wrong. But I use it a lot and
haven't had any problems.

Usage (Unix):
```bash
qmake
make
bin/onefileedit my/folder --type=h --editor=subl --recursive=0
```
