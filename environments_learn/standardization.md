# Standardization

Idea: want macbook and mac mini to be functional clones of eachother, how to facilitate:

1: pull all config into a dotfile repo
(zshrc, gitconfig, ssh config, terminal themes, editor settings etc)

can use a bare git repo, or chezmoi

Chezmoi:
KEY:
git is used for the storing of repos, chezmoi is for handling the symlinks in your system.

alternatively, just create a directory that houses your dotfiles, and manage the symlinking yourself.

```bash
mkdir ~/dotfiles
cd ~/dotfiles
git init
```
creates the repo, where inside you will store all of your dotfiles
(zshrc, gitconfig, nvim/, tmux.conf, etc)

KEY: now, move your dot files into the repo and symlink them back into place:

mv ~/.zshrc ~/dotfiles/zshrc
ln -s ~/dotfiles/zshrc ~/.zshrc //This way, its back to normal!
(note: when a process asks for ~/.zshrc, the OS transparently redirects it to ~/dotfiles/zshrc)

Then, push that stuff to github with all the files, and then just symlink those files back
where they belong.

git clone git@github.com:YOU/dotfiles.git ~/dotfiles

and
ln -s ~/dotfiles/zshrc ~/.zshrc
(NOTE: symlink replaces the path entry, so the zshrc will be overridden as the symlink)
(will either error, saying file exists, or it will override.)

KEY 2: IF ~/.ssh already exists, running that command will not work, and will just put it inside of the folder. Thus, solution is



KEY NOTE: TRAILING SLASH

~/.ssh vs ~/.ssh/ ARE NOT THE SAME THING

## ASIDE ON PATHS (MORE IN FILESYSTEM_ADVANCED OR FILESYSTEM_LEARN)

when you mv file target
different things can happen when you specify the trailing slash vs when you do not
If target does not exist, it will rename file to target and call it a day
If target does nto exist, but you specify trailing slash, it will error, and say directory not found
If target does exist and it is a file, it will replace it with file, unless -i flag is up
If target does exist and it is a direcory, it will put it inside the directory

thus, 
ln -s /dotfiles/ssh ~/.ssh/ is NOT what you want (you want to symlink a directory FILE)
THIS WILL PUT THE SYLINK INSIDE OF THE ~/.ssh/ FOLDER!

INSTEAD, you MUST ln -s ~/dotfiles/ssh ~/.ssh, so that it will link the directory entry 
dotfiles/ssh to a new directory entry named ~/.ssh, WITHOUT EVALUATING ~/.ssh

KEY 2: REMEMBER TO USE ABSOLUTE SYMLINKS LINKS!!!!!

IF YOU JUST:

ln -s zshrc ~/.zshrc

then, that is a relative symlink, meaning the symlink file will literally only say:

~/.zshrc -> zshrc, but nothing more (only has meaning if within the directory from where you created it!)
instead, use absolute path









NOTE: for .gitignore files however, you want the / so its clear that you ignore the directory, not just the file

ln 
-s for symlink

-l = long listing format
Shows permissions, link count, owner, group, size, timestamp, and filename.

-i = inode number
Shows the inode number at the start of each line.

avoid hardlinks when working on teh cloud:

**NOTES** ICLOUD IS A FILE-SYNC SERVICE, NOT A SHARED FILESYSTEM
iCloud Drive is basically a folder on each Mac + Apple’s cloud copy + a sync daemon.

Think of it as:
MacBook local disk
  ~/Library/Mobile Documents/com~apple~CloudDocs/
        ⇅ syncs
Apple iCloud servers
        ⇅ syncs
Mac mini local disk
  ~/Library/Mobile Documents/com~apple~CloudDocs/

**CRUTIALLY** Each Mac has its own local copy, icloud watches for changes, uploads
them, and then downloads the changed version onto your other devices

It FEELS like a shared filesystem but isnt!

THATS WHY YOU CANNOT USE HARDLINKS! The APFS filesystem doesnt share a common
inode table, so just because they have the same number doesnt mean anything!
That is because it, along with other text editors can do atomic-save style edits, where
it creates a new file, and then replaces the old file with the new one, thereby making it 
have a new inode entry

Symlinks on the other hand, have their own inode, and the contents of that inode is just the string path

can use github and sync over that or can treat them as just documents, and sync over icloud, but git + icloud is when things get messed up bc icloud sync is random, and can
mess up the git version of that data, plus git has version control, which is nicer

AS FOR USING GITHUB AND ICLOUD AT THE SAME TIME: regarding local files, like the .py
and .mds not an issue. Problem arises

*******KEY********************
file given_file

ls -ld: -l means long listing, -d means list directories our themselves (not the content within them)
KEY MISTAKE:

file ~/.ssh
VS
file ~/.ssh/ 

USE UNLINK COMMAND TO UNLINK FOR THE SAME REASON!!!
unlink file vs rm file, because it doesnt have recursive options!!!
rm -rf mylink vs rm -rf mylink/ is VERY DIFFERENT BECAUSE OF THE TRAILING BACKSLASH WHICH EVALUATES IT

BY USING THE SECOND SLASH, IT WILL EVALUATE THE SYMLINK!!! THEREFORE IT WILL RETURN DIRECTORY!!!
YOU WROTE FILE ~/.SSH/, WHICH MESSED IT UP

ln -s ~/dotfiles/ssh ~/.ssh
vs
ln -s ~/dotfiles/ssh/ ~/.ssh
effectively no difference, but be sure to not have a trailing / on the ssh
(directory vs contents not important here bc both refer to same dir)
( ~/dotfiles/ssh is the directory file, and ~/dotfiles/ssh/ is the directories contents)


CRUTIAL!!!!
ON MACOS!!!!
"file" ***DOES*** FOLLOW THE SYMLINK, AND WILL RETURN DIRECTORY, EVEN ON A SYMLINK

exe:

linked_test -> test

file linked_test returns directory, and so does file test
BUT
ls -ld linked_test RIGHTFULLY SAYS ITS A SYMLINK, WHEREAS ls -ld test is a directory!!!
file -h does not follow symlinks

ls -ld is the safer option!!!


***************************************/





For recreating ssh:

cat > ~/.ssh/.gitignore <<'EOF'
# Never commit private SSH keys
id_*
!id_*.pub

# Other private/machine-specific SSH files
*.pem
*.key
known_hosts*
known_hosts.old
authorized_keys
agent
EOF

(This is a complex command, using both a redirection and a here document)

cat: cat copies its input to its output.
if you type cat, and then type hello, it will echo back hello
(because reading from stdin(keyboard) and writing to stdout(terminal))

<<'EOF' is the here document, meaning pretend everything up to here is a document untill you reach a line containing EOF

so 
cat <<'EOF'
hello
world
EOF

is teh same as cat from a document whose input was "hello \n world"
(EOF here is just a marker, it could be any other string you desire, with no quotes around it)

so cat [file1] [file2] means read those files and output then to stdout or whatever is your output

thus:

cat > ~/.ssh/.gitignore <<'EOF' redirects teh output to that file, its like an easy way to do it without vim!



BETTER SETUP IDEA:
HAVE JUST THE CONFIG SHARED, THE ACTUALY KEYS JUST REFERRED TO IN THE FILE, AND EACH WILL RE-IMPLEMENT LOCALLY 
OR BETTER YET, SCP / RSYNC IT OVER

# >>> conda initialize >>>
# !! Contents within this block are managed by 'conda init' !!
__conda_setup="$('/Users/stefancelmare/miniconda3/bin/conda' 'shell.zsh' 'hook' 2> /dev/null)"
if [ $? -eq 0 ]; then
    eval "$__conda_setup"
else
    if [ -f "/Users/stefancelmare/miniconda3/etc/profile.d/conda.sh" ]; then
        . "/Users/stefancelmare/miniconda3/etc/profile.d/conda.sh"
    else
        export PATH="/Users/stefancelmare/miniconda3/bin:$PATH"
    fi
fi
unset __conda_setup
# <<< conda initialize <<<
export PATH=$PATH:/Users/stefancelmare/Downloads/Visual\ Studio\ Code.app/Contents/Resources/app/bin/









## ANOTHER GREAT IDEA:

RSYNC + FILEWATCHER

brew install fswatch (a tool used to watch files, and do somethign when something happens)

fswatch -o ~/shared | while read event
do
  rsync -az --delete ~/shared/ macmini:~/shared/
done

-a   archive mode: keeps permissions, timestamps, folders, symlinks, etc.
-v   verbose: show what is happening
-z   compress during transfer
--delete  removes files that are not matching the source (exact mirror)
--dry-run shows you what would happen

(this way, whenever somethign changes, it will update it on your local device)