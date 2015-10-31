# RDIR #

### What is it? ###

This program is a file browser/selector with the visual style and interactivity of [ranger](http://ranger.nongnu.org/). It will not be a full on file manager, rather a file _selector_, to be used in a similar fashion to [selecta](https://github.com/garybernhardt/selecta) or [fzf](https://github.com/junegunn/fzf). 


It will excel in two things, that is: traversing directories quickly, and writing selected file and directory names to STDOUT. Ideally it will be used in conjunction with scripts and shell keybindings to quickly move around directories, or select files and output their paths into the command line.

It aims to do this purely in C so no python or ruby will be required.

At the moment, it can function as a directory traveler using functions in a helper script.

### TODO ###
* ~~Render window~~
* ~~movement~~
* ~~show selected dirs and change dirs~~
* ~~output selected directory~~
* scroll middle column scrolling with scrollbar
* multiple directories
* marks
* tmux integration
* fuzzy directory/file matching
* make script more cross platform