# Shell project (`lsh`)

## Building

Just run the script called `build_script.sh` or run the commands manually that are in the script.

## Running

After the program is built you should be able to run `./build/lsh`assuming you are in the project root directory (the one with the build script and src folder in).



The shell should then boot up and print the version, then some debug information about the env var hashmap and then it should print the result of `$HOME`. After then you should be presented with the shell.

The shell is in the format `(lsh) FOLDER >>`, where folder is the last segment of the pwd. For example in my home folder it would show `(lsh) louisdewardt >>`. The colour of the terminal is green when the last command was successful and red otherwise. If you hit enter with nothing typed (which is an error) the terminal will go red.

## Running commands

Whenever you run a command it is first parsed into an ExecutionPlan which is a kind of linked list stating how all commands are linked together.  For debugging I printed this out before the command was run but I have kept it because it helps to show what the shell is doing. For example if you run `echo "b\nc\na" | sort`, before running the command it will print `executor=>["echo", ""b\nc\na""] pipe=>{ executor=>["sort"] }`. After it will then show the result of the command (a,b,c in order on newlines).  Also if you run a command in the form `a && b || c` it returns `executor=>["a"] on success=>{ executor=>["b"] on failure=>{ executor=>["c"] }}`. Notice how `on success` is on the executor for b. This means that `c` runs if `b`fails (not `a`).

### Control chars

Here a quick list of control chars and their meaning:

- `|` => pipe the stdout of the previous into the stdin of the next

- `||` => on failure of the previous command

- `&` => fork previous command

- `&&`=>  on success of previous command

- `;`  => after previous command (regardless of what the result is)

The way `&&`  and `||` work is that they skip the next command unless their condition is met. For example `a && b || c`, `b` will be skipped if `a` doesn't exit with status 0, and `c` will be skipped if `b` exits with status  0.

## Command Types

As a user there are three command types:

- built in (lsh has these command programmed in)

- global (lsh searchs for the command in PATH)

- absolute/relative (you specifiy the location to the binary either with a `/`meaning abolute or you just type the path relative to the current location `build/lsh`)



### Built in commands

The built in commands are: `which`, `cd`, `pwd`,`export`and `exit`, and they function similiarily to bash.

For example the result of `which cd` is "`cd` is a shell built in".

Running `which echo`should return `/bin/echo`.

Let's say you're in the project root: `which ./build/lsh` should return the absolute path to the binary.



Exit, exits the current shell with the status code 0.



`cd` changes  directory, and `pwd`prints the working directory.



The code for these commands are in `built_ins.c`.

## Environment variables and argument parsing

Arguments are parsed twice. The first time it is to split them into lists of arguments, this parsing happens in `parse.c`. This first parsing tries to work  out whether a space is actually a space or if it is escaped (`\ `) or part of a quote (either `"`or `'`). The parsing also tries to find unescaped control chars (see list of control chars above). `a|b` is equivalent to `a | b`. Repeated and leading whitespace is also ignored (`    echo        asd` = `echo asd`). This first parser returns an execution plan (described earlier).



The second time arguments are parsed is when they are about to be run (in the function`sanitise_argument` in the file `executor.c`). At this point outer unescaped quotes are removed  (e.g.`echo "test"` becomes `echo test`) and environment variables are inserted (e.g. `echo $HOME`becomes `echo /PATH_TO_HOME`  where PATH_TO_HOME is your home path).



Enviornment variables can be escaped in two ways: using `echo \$HOME`will print `$HOME` (literally). Also using single quotes will not expand the variable e.g. `echo '$HOME'` will also print (`$HOME`).



When the shell resolves paths either to find the location of a command or in cd, `~`and `..` are appropriately handled. (e.g. `echo ~`is the same as `echo $HOME` and `cd ..`will move you up a directory - `cd ../..` is the only was to move up two directories).

### Setting env variables

To set an environment variable you just write `export NAME=VALUE`, where `NAME` is the variable name and `VALUE` is the value. For example if you run `export my_var="this is a test" && echo $my_var`it will print `this is a test`.



There is also a special env var `$?` which is the value of the last exit status.



#### Forking

Although forking is implemented there is no built in jobs command so you cannot recover a process or handle them, it should have been possible to add this command if I had the time.

For example `sleep 10 &`works.`&` only forks the command immediately previous to it.



### A note on ctrl-c

I haven't overriden the signal yet for ctrl-c so currently is exits the shell. There is no way to terminate a running program from within the shell currently.

## High level overview of the source

At startup all the environment variables passed to lsh are parsed and put into a hashmap (see `hashmap.c` and attribution notice on the general ideas for the algorithm) which is stored with the `shell` struct.

`main.c`has a loop that prints the shell and reads the stdin. It then calls a function in `parse.c` which returns an execution plan. This plan contains a list of executors (just a wrapper around a list of strings representing the arguments - this struct used to be more complicated which is why it exists standalone). 

`main.c` then takes this execution plan and checks if it is NULL (empty command), if it isn't it runs it calling a function in `execution_plan.c`which has a loop which goes over all the elements of the linked list running each in turn and handling the different types of connections (e.g. fork, on success, after ...).

The execution plan runs a function in `exeuctor.c` for each executor. This file santises the arguments (removes outer quotes and handles inserting env_vars). Any file descriptors that need redirecting (e.g. for fork) are redirected and the process is forked. Executor returns the pid of the forked process to `execution_plan.c` which then decides whether to wait and check the outcome (e.g. for on success) or carry on (e.g. fork).


