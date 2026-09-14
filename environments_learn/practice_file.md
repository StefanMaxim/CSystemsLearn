This is a test file, which i will first commit, then modify, and see whether the change appears as modified, or a new file, suggesting that it uses atomic-saves, where it just creates a new file, and then renames it. 

Here is the new bit now. Interesting, it appears as modified.

This means that it synced the .git file as well, containing my new commit, compressed, as well as the updates DAG (which is really just the commits refs +  the dag created by following the parent commits of those commits)

Because the .git file is OS-independent, using its own SHA1 compression where the contents are compressed as data, that means that it can work seemlessly across OS's.

That also means that git status works by looking in your directory, and seeing all of the files there, likely by name not by inode. By keeping inodes out of the git behavior, it lets the syncing acutally be rather convenient.

Thus, if I put my dotfiles here, it should actually work like an automatic GIT, since the symlinks just refer to the icloud directory by path (NOT INODE CRUTUALLY), meaning even if the file is changed or overridden, all is good! 