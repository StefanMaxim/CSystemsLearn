# Intro to changing environments + vars and stuff


## Changing your Java Version:

1: install the correct version using homebrew



2: set java home path variable:

add to zshrc:
export JAVA_HOME=$(/usr/libexec/java_home -v 14) //$() here wraps the command, ie makes it a command, not a string
export PATH="$JAVA_HOME/bin:$PATH"


## KEY NOTE ON PATH VARS:

everybody knows the standard BASH expression
```bash
export PATH = $PATH:<insert something here>
```

But doesnt this become an issue? Every time this command runs, it will append to PATH, so wont that lead to duplicates?

Recall, bash is ran by some parent process, which fork() and execve()

In the execve, you pass execve(path="/path/to/executable",argv="array of strings for the args",envp="string of the path")
**THE KEY HERE IS THE ENVP**
The parent process of BASH will pass its OWN PATH VARIABLES, usually:
PATH=/usr/bin:/bin

And then the commands in bashrc will run, correctly pre-pending the right amount.

**THE PROBLEM IS WITH RUNNING SOURCE**
if you keep running source, it will keep pre-pending, eventually leading to an issue.

Solution:
1: just start a new bash via
```bash
exec bash -l
```

2: can also do a conditional export:
```bash
if [[ ":$PATH:" != *":/afs/andrew/course/15/122/bin:"* ]]; then
    export PATH="$PATH:/afs/andrew/course/15/122/bin"
fi
```

3: better
paths=(
    "/afs/andrew/course/15/122/bin"
    "/afs/andrew/course/15/213/bin"
    "/afs/andrew/course/15/251/bin"
)

for dir in "${paths[@]}"; do
    if [[ ":$PATH:" != *":$dir:"* ]]; then
        export PATH="$PATH:$dir"
    fi
done

4:
for var in $(compgen -A variable); do
    if [[ "$var" == *_COURSE ]]; then
        dir="${!var}"

        if [[ -d "$dir" && ":$PATH:" != *":$dir:"* ]]; then
            PATH="$PATH:$dir"
        fi
    fi
done

export PATH

type autolab (type of var)
whereis autolab
type -a to see all

(NO PROBLEM, ALIAS WINS)









## dotenv

pip install python-dotenv (want this package)

Then, be sure you have a .env file:
API_KEY=abc123
DATABASE_URL=postgres://user:password@localhost/mydb
DEBUG=True

Lastly, load:
from dotenv import load_dotenv
import os

load_dotenv()  # Loads variables from .env (THIS IS THE ONLY COMMAND YOU NEED)

api_key = os.getenv("API_KEY")
database_url = os.getenv("DATABASE_URL") #can also use os.environ["API_KEY]
debug = os.getenv("DEBUG")

print(api_key)

**HOW DOES IT WORK**
On load_env, it loads those environment variabels into the process's environment.
difference:
os.genenv returns value, else none
os.environ returns value, else KeyError
```python
os.environ["key"] //raises error
os.environ.get("key") //returns null
```

**USEFUL**
api_key = os.getenv("API_KEY", "default_key") #provide default fallback if not found
load_dotenv(path=""), or can use a Path (again, the ONLY COMMAND NEEDED TO LOAD ENVIONRMENT VARIABLES)

THIS IS USEFUL BC IT KEEPS SECRETS OUT OF YOUR CODEBASE!


