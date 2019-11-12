# Shell project (`lsh`)

## Building

Just run the script called `build_script.sh` or run the commands manually that are in the script.

## Running

After the program is built you should be able to run `./build/lsh`assuming you are in the project root directory (the one with the build script and src folder in).



The shell should then boot up and print the version, then some debug information about the env var hashmap and then it should print the result of `$HOME`. After then you should be presented with the shell.

## Running commands

Whenever you run a command it is first parsed into an ExecutionPlan which is a kind of linked list stating how all commands are linked together.  For debugging I printed this out before the command was run but I have kept it because it helps to show what the shell is doing. For example if you run `echo "b\nc\na" | sort`, before running the command it will print `executor=>["echo", ""b\nc\na""] pipe=>{ executor=>["sort"] }`. After it will then show the result of the command (a,b,c in order on newlines).  Also if you run a command in the form `a && b || c` it returns `executor=>["a"] on success=>{ executor=>["b"] on failure=>{ executor=>["c"] }}`. Notice how `on success` is on the executor for b. This means that `c` runs if `b`fails (not `a`).

### Control chars

Here a quick list of control chars and their meaning:

- `|` => pipe the stdout of the previous into the stdin of the next

- `||` => on failure of the previous command

- `&` => fork previous command

- `&&`=>  on success of previous command

- `;`  => after previous command (regardless of what the result is)
  
  

## Command Types

As a user there are three command types:

- built in (lsh has these command programmed in)

- global (lsh searchs for the command in PATH)

- absolute/relative (you specifiy the location to the binary)



### Built in commands

The built in commands are: `which`, `cd`, `pwd`and `exit`, and they function similiarily to bash.

For example the result of `which cd` is "`cd` is a shell built in".

Running `which echo`should return `/bin/echo`.

Let's say you're in the project root: `which ./build/lsh` should return the absolute path to the binary.



Exit, exits the current shell with the status code 0.



`cd` changes  directory, and `pwd`prints the working directory.



The code for these commands are in `built_ins.c`.

## Environment variables and argument parsing

Arguments are parsed twice. The first time it is to split them into lists of arguments, this parsing happens in `parse.c`. This first parsing tries to work  out whether a space is actually a space or if it is escaped (`\ `) or part of a quote (either `"`or `'`). The parsing also tries to find unescaped control chars (see list of control chars above). `a|b` is equivalent to `a | b`. Repeated and leading whitespace is also ignored (`    echo        asd`=`echo asd`). This first parser returns an execution plan (described earlier).



The second time arguments are parsed is when they are about to be run (in the function`sanitise_argument` in the file `executor.c`). At this point outer unescaped quotes are removed  (e.g.`echo "test"` becomes `echo test`) and environment variables are inserted (e.g. `echo $HOME`becomes `echo /PATH_TO_HOME`  where PATH_TO_HOME is your home path).
