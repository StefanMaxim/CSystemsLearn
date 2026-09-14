# Basics to know

1: on mac, your zshrc has a default one provided, which is in /etc/zshrc

## What should be in your zshrc

1: copy the default section from the file, likely all good things

2: Homebrew integration
be sure to add the homebrew eval commands to there

eval "$(/opt/homebrew/bin/brew shellenv)"


#IDEA: every time you re-run this command, it will keep adding java_home to the top of
the path. Thus, solution is to instead create a shell variable that is the OG shell, and then just keep appending to THAT
one, and then finally assign it to the PATH.



NOTE: noticed path_helper in libexec, could be interesting:
export HOMEBREW_PREFIX="/opt/homebrew";
export HOMEBREW_CELLAR="/opt/homebrew/Cellar";
export HOMEBREW_REPOSITORY="/opt/homebrew";
fpath[1,0]="/opt/homebrew/share/zsh/site-functions";
export FPATH;
eval "$(/usr/bin/env PATH_HELPER_ROOT="/opt/homebrew" /usr/libexec/path_helper -s)"
[ -z "${MANPATH-}" ] || export MANPATH=":${MANPATH#:}";
export INFOPATH="/opt/homebrew/share/info:${INFOPATH:-}";
(same on in this command)



/usr/local/bin:/System/Cryptexes/App/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/local/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/appleinternal/bin:/pkg/env/global/bin:/Library/TeX/texbin:/Applications/VMware Fusion.app/Contents/Public


WHat is in teh BASE_PATH:

1: usr/local/bin, read filesystem_learn
2: /System/Cryptexes/App/usr/bin
3: /usr/bin read filesystem_learn
4: /bin
5: /usr/sbin
6 /sbin
7: /var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/local/bin
8: /var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/bin
9: /var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/appleinternal/bin


10: /pkg/env/global/bin
pkg is the apple package manager, like dpkg for debian package manager sudo dpkg -i package.deb and such





11: /Library/TeX/texbin
12: /Applications/VMware Fusion.app/Contents/Public

bottom 2 not needed fr





