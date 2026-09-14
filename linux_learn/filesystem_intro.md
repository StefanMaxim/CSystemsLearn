## ASIDE ON PATHS (MORE IN FILESYSTEM_ADVANCED)

when you mv file target
different things can happen when you specify the trailing slash vs when you do not
If target does not exist, it will rename file to target and call it a day
If target does exist and it is a file, it will replace it with file, unless -i flag is up
If target does exist and it is a direcory, it will put it inside the directory

thus, 
ln -s /dotfiles/ssh ~/.ssh/ is NOT what you want (you want to symlink a directory FILE)
INSTEAD, you MUST ln -s ~/dotfiles/ssh ~/.ssh, so that it will link the directory entry 
dotfiles/ssh to a new directory entry named ~/.ssh, WITHOUT EVALUATING ~/.ssh



when you type cd /files/foo

1: shell builtin cd evelutally runs 

chdir("/files/foo")

2: KEY! the kernel sees

/files/foo and splits it into 3 components:
"", "files", "foo"

wheras /files/foo/ is split into 4 components:
"", "files", "foo", ""

3:VFS (Linux Virtual FileSystem) walks the path

1) Start at root inode (read the git intro )