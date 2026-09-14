
# Multiplexers:
Tmux is an example of a terminal multiplexer:  
lets you:  
- keep long-running files alive even when you close laptop
- split one terminal into multiple panes
- organize work into sessions/windows

> **CRUTIAL**:
> WHY IS THIS IMPORTANT?
> normally, when sshing, the chain is: your terminal app -> ssh connection -> remote shell -> your programs
> thus, when your ssh connection dies or drops, the remote shell gets a hangup and dies, and so do all programs that are currently running on it
> TMUX changes the chain:
> NOW: terminal app - ssh connection -> tmux client -> tmux server (on remote machine) -> your shells / programs
> this way, when your ssh disconnects, only loose the tmux client/viewer. the server keeps running on the remote and keeps your shell and programs
>attached to its virtual terminals
EXE:
- ssh into server `ssh user@ipaddr`
- start tmux via `tmux new -s work`
- start something long-running `python train.py`
- detach via `C-b d`, then you can even exit ssh `exit`
- Later can `ssh user@ipaddr` and `tmux attach -t work` and see your program still running


It contains 3 parts:

### Session
A workplace that persists (exe a session called server where you run ssh, logs, editors, etc)

### Window
Like a tab inside a sessions (let window 0 be editor, window 1 be logs, window 2 be shell, etc )

### Pane
a split region inside a window, (ie left pane runs code, right pane watches htop)

> **CRUTIAL**
> all commands here are based on a prefix, then a key, (prefix usually is command/control + b, followed by a key)
> we will denote command + b as just C-b for short, so C-b % is actually command b then %



> **CRUTIAL 2**

WHY does code . work outside of TMUX whereas it breaks inside of tmux?
Reason: the vscode server created (/run/user/2256949/vscode-ipc-...sock) **IS NOT** STABLE ACROSS LOGIN SESSIONS  
this means that this socket(more on that later) is only valid for the current session created
(vscode works by creating a node.js process, a unix domain socet at /run/user/uid, and env variables pointing to it)

OKAY, but they where is the problem:
when you write `tmux new -s work`, it copies over the env variables from the OG shell and doesnt update them
thus, when using a new shell or connecting back again, your old vscode server(the on which is linked in your
frozen env variables) is no longer the correct server, making the connection invalid.

There is a new server with its own new socket path that your current tmux session doesnt have access to

```bash
VSCODE_IPC_HOOK_CLI=/run/user/2256949/vscode-ipc-OLD.sock
XDG_RUNTIME_DIR=/run/user/2256949
```

> TEST:

RUN INSIDE OF TMUX
```bash
env | grep VSCODE
```

RUN OUTSIDE IN THE SHELL
```bash
env | grep VSCODE
```


> **SOLUTION**:
1. kill the old session and make a new one each time

2. manually change the variable:
```bash
tmux set-environment -g VSCODE_IPC_HOOK_CLI "$VSCODE_IPC_HOOK_CLI"
```

Solution: ssh, then open the tmux 

### Commands:

- tmux new -s work //starts a new named session called work
- C-b % // vertically splits the panes (use exit to kill it)
- C-b " //horizontally splits the panes

- C-b (arrow keys) //to move across the panes
- C-b c //creates a new window
- C-b w //lists windows
- C-b n / p //used to switch between windows

- C-b z //zooms or unzooms in current pane
- C-b , //renames current window

- C-b d //detached the tmux instance, but leaves it running
- C-d //kills the pane same as exit

- tmux ls // lists all currently running tmux sessions
- tmux attach -t work //reattaches the session to tmux

- tmux kill-session -t NAME //kills the session
